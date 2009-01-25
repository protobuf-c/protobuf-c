#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "protobuf-c-rpc.h"
#include "protobuf-c-data-buffer.h"

#define protobuf_c_assert(x) assert(x)

#undef TRUE
#define TRUE 1
#undef FALSE
#define FALSE 0

#define UINT_TO_POINTER(ui)     ((void*)(ui))
#define POINTER_TO_UINT(ptr)     ((unsigned)(ptr))

#define MAX_FAILED_MSG_LENGTH   512

typedef enum
{
  PROTOBUF_C_CLIENT_STATE_INIT,
  PROTOBUF_C_CLIENT_STATE_NAME_LOOKUP,
  PROTOBUF_C_CLIENT_STATE_CONNECTING,
  PROTOBUF_C_CLIENT_STATE_CONNECTED,
  PROTOBUF_C_CLIENT_STATE_FAILED_WAITING,
  PROTOBUF_C_CLIENT_STATE_FAILED,               /* if no autoretry */
  PROTOBUF_C_CLIENT_STATE_DESTROYED
} ProtobufC_RPC_ClientState;

typedef struct _Closure Closure;
struct _Closure
{
  /* these will be NULL for unallocated request ids */
  const ProtobufCMessageDescriptor *response_type;
  ProtobufCClosure closure;

  /* this is the next request id, or 0 for none */
  void *closure_data;
};

struct _ProtobufC_RPC_Client
{
  ProtobufCService base_service;
  ProtobufCDataBuffer incoming;
  ProtobufCDataBuffer outgoing;
  ProtobufCAllocator *allocator;
  ProtobufCDispatch *dispatch;
  ProtobufC_RPC_AddressType address_type;
  char *name;
  ProtobufC_FD fd;
  protobuf_c_boolean autoretry;
  unsigned autoretry_millis;
  ProtobufC_NameLookup_Func resolver;
  ProtobufC_RPC_ClientState state;
  union {
    struct {
      ProtobufCDispatchIdle *idle;
    } init;
    struct {
      protobuf_c_boolean pending;
      protobuf_c_boolean destroyed_while_pending;
      uint16_t port;
    } name_lookup;
    struct {
      unsigned closures_alloced;
      unsigned first_free_request_id;
      /* indexed by (request_id-1) */
      Closure *closures;
    } connected;
    struct {
      ProtobufCDispatchTimer *timer;
      char *error_message;
    } failed_waiting;
    struct {
      char *error_message;
    } failed;
  } info;
};

static void begin_name_lookup (ProtobufC_RPC_Client *client);
static void destroy_client_rpc (ProtobufCService *service);


static void
set_fd_nonblocking(int fd)
{
  int flags = fcntl (fd, F_GETFL);
  protobuf_c_assert (flags >= 0);
  fcntl (fd, F_SETFL, flags | O_NONBLOCK);
}

static void
handle_autoretry_timeout (ProtobufCDispatch *dispatch,
                          void              *func_data)
{
  begin_name_lookup (func_data);
}

static void
client_failed (ProtobufC_RPC_Client *client,
               const char           *format_str,
               ...)
{
  va_list args;
  char buf[MAX_FAILED_MSG_LENGTH];
  size_t msg_len;
  char *msg;
  size_t n_closures = 0;
  Closure *closures = NULL;
  switch (client->state)
    {
    case PROTOBUF_C_CLIENT_STATE_NAME_LOOKUP:
      protobuf_c_assert (!client->info.name_lookup.pending);
      break;
    case PROTOBUF_C_CLIENT_STATE_CONNECTING:
      /* nothing to do */
      break;
    case PROTOBUF_C_CLIENT_STATE_CONNECTED:
      n_closures = client->info.connected.closures_alloced;
      closures = client->info.connected.closures;
      break;

      /* should not get here */
    case PROTOBUF_C_CLIENT_STATE_INIT:
    case PROTOBUF_C_CLIENT_STATE_FAILED_WAITING:
    case PROTOBUF_C_CLIENT_STATE_FAILED:
    case PROTOBUF_C_CLIENT_STATE_DESTROYED:
      protobuf_c_assert (FALSE);
      break;
    }
  if (client->fd >= 0)
    {
      protobuf_c_dispatch_close_fd (client->dispatch, client->fd);
      client->fd = -1;
    }
  protobuf_c_data_buffer_reset (&client->incoming);
  protobuf_c_data_buffer_reset (&client->outgoing);

  /* Compute the message */
  va_start (args, format_str);
  vsnprintf (buf, sizeof (buf), format_str, args);
  va_end (args);
  buf[sizeof(buf)-1] = 0;
  msg_len = strlen (buf);
  msg = client->allocator->alloc (client->allocator, msg_len + 1);
  strcpy (msg, buf);

  /* go to one of the failed states */
  if (client->autoretry)
    {
      client->state = PROTOBUF_C_CLIENT_STATE_FAILED_WAITING;
      client->info.failed_waiting.timer
        = protobuf_c_dispatch_add_timer_millis (client->dispatch,
                                                client->autoretry_millis,
                                                handle_autoretry_timeout,
                                                client);
      client->info.failed_waiting.error_message = msg;
    }
  else
    {
      client->state = PROTOBUF_C_CLIENT_STATE_FAILED;
      client->info.failed.error_message = msg;
    }

  /* we defer calling the closures to avoid
     any re-entrancy issues (e.g. people further RPC should
     not see a socket in the "connected" state-- at least,
     it shouldn't be accessing the array of closures that we are considering */
  if (closures != NULL)
    {
      unsigned i;

      for (i = 0; i < n_closures; i++)
        if (closures[i].response_type != NULL)
          closures[i].closure (NULL, closures[i].closure_data);
      client->allocator->free (client->allocator, closures);
    }
}

static inline protobuf_c_boolean
errno_is_ignorable (int e)
{
#ifdef EWOULDBLOCK              /* for windows */
  if (e == EWOULDBLOCK)
    return 1;
#endif
  return e == EINTR || e == EAGAIN;
}

static void
set_state_connected (ProtobufC_RPC_Client *client)
{
  client->state = PROTOBUF_C_CLIENT_STATE_CONNECTED;

  client->info.connected.closures_alloced = 1;
  client->info.connected.first_free_request_id = 1;
  client->info.connected.closures = client->allocator->alloc (client->allocator, sizeof (Closure));
  client->info.connected.closures[0].closure = NULL;
  client->info.connected.closures[0].response_type = NULL;
  client->info.connected.closures[0].closure_data = UINT_TO_POINTER (0);
}

static void
handle_client_fd_connect_events (int         fd,
                                 unsigned    events,
                                 void       *callback_data)
{
  ProtobufC_RPC_Client *client = callback_data;
  socklen_t size_int = sizeof (int);
  int fd_errno = EINVAL;
  if (getsockopt (fd, SOL_SOCKET, SO_ERROR, &fd_errno, &size_int) < 0)
    {
      /* Note: this behavior is vaguely hypothetically broken,
       *       in terms of ignoring getsockopt's error;
       *       however, this shouldn't happen, and EINVAL is ok if it does.
       *       Furthermore some broken OS's return an error code when
       *       fetching SO_ERROR!
       */
    }

  if (fd_errno == 0)
    {
      /* goto state CONNECTED */
      protobuf_c_dispatch_watch_fd (client->dispatch,
                                    client->fd,
                                    0, NULL, NULL);
      set_state_connected (client);
    }
  else if (errno_is_ignorable (fd_errno))
    {
      /* remain in CONNECTING state */
      return;
    }
  else
    {
      /* Call error handler */
      protobuf_c_dispatch_close_fd (client->dispatch, client->fd);
      client_failed (client,
                     "failed connecting to server: %s",
                     strerror (fd_errno));
    }
}

static void
begin_connecting (ProtobufC_RPC_Client *client,
                  struct sockaddr      *address,
                  size_t                addr_len)
{
  protobuf_c_assert (client->state == PROTOBUF_C_CLIENT_STATE_NAME_LOOKUP);

  client->state = PROTOBUF_C_CLIENT_STATE_CONNECTING;
  client->fd = socket (PF_UNIX, SOCK_STREAM, 0);
  if (client->fd < 0)
    {
      client_failed (client, "error creating socket: %s", strerror (errno));
      return;
    }
  set_fd_nonblocking (client->fd);
  if (connect (client->fd, address, addr_len) < 0)
    {
      if (errno == EINPROGRESS)
        {
          /* register interest in fd */
          protobuf_c_dispatch_watch_fd (client->dispatch,
                                        client->fd,
                                        PROTOBUF_C_EVENT_READABLE|PROTOBUF_C_EVENT_WRITABLE,
                                        handle_client_fd_connect_events,
                                        client);
          return;
        }
      close (client->fd);
      client->fd = -1;
      client_failed (client, "error connecting to remote host: %s", strerror (errno));
      return;
    }

  set_state_connected (client);
}
static void
handle_name_lookup_success (const uint8_t *address,
                            void          *callback_data)
{
  ProtobufC_RPC_Client *client = callback_data;
  struct sockaddr_in addr;
  protobuf_c_assert (client->state == PROTOBUF_C_CLIENT_STATE_NAME_LOOKUP);
  protobuf_c_assert (client->info.name_lookup.pending);
  client->info.name_lookup.pending = 0;
  if (client->info.name_lookup.destroyed_while_pending)
    {
      destroy_client_rpc (&client->base_service);
      return;
    }
  addr.sin_family = PF_INET;
  memcpy (&addr.sin_addr, address, 4);
  addr.sin_port = htons (client->info.name_lookup.port);
  begin_connecting (client, (struct sockaddr *) &addr, sizeof (addr));
}

static void
handle_name_lookup_failure (const char    *error_message,
                            void          *callback_data)
{
  ProtobufC_RPC_Client *client = callback_data;
  protobuf_c_assert (client->state == PROTOBUF_C_CLIENT_STATE_NAME_LOOKUP);
  protobuf_c_assert (client->info.name_lookup.pending);
  client->info.name_lookup.pending = 0;
  if (client->info.name_lookup.destroyed_while_pending)
    {
      destroy_client_rpc (&client->base_service);
      return;
    }
  client_failed (client, "name lookup failed (for name from %s): %s", client->name, error_message);
}

static void
begin_name_lookup (ProtobufC_RPC_Client *client)
{
  protobuf_c_assert (client->state == PROTOBUF_C_CLIENT_STATE_INIT
                 ||  client->state == PROTOBUF_C_CLIENT_STATE_FAILED_WAITING
                 ||  client->state == PROTOBUF_C_CLIENT_STATE_FAILED);
  client->state = PROTOBUF_C_CLIENT_STATE_NAME_LOOKUP;
  client->info.name_lookup.pending = 0;
  switch (client->address_type)
    {
    case PROTOBUF_C_RPC_ADDRESS_LOCAL:
      {
        struct sockaddr_un addr;
        addr.sun_family = AF_UNIX;
        strncpy (addr.sun_path, client->name, sizeof (addr.sun_path));
        begin_connecting (client, (struct sockaddr *) &addr,
                          sizeof (addr));
        return;
      }

    case PROTOBUF_C_RPC_ADDRESS_TCP:
      {
        /* parse hostname:port from client->name */
        const char *colon = strchr (client->name, ':');
        char *host;
        unsigned port;
        if (colon == NULL)
          {
            client_failed (client,
                           "name '%s' does not have a : in it (supposed to be HOST:PORT)",
                           client->name);
            return;
          }
        host = client->allocator->alloc (client->allocator, colon + 1 - client->name);
        memcpy (host, client->name, colon - client->name);
        host[colon - client->name] = 0;
        port = atoi (colon + 1);

        client->info.name_lookup.pending = 1;
        client->info.name_lookup.destroyed_while_pending = 0;
        client->info.name_lookup.port = port;
        client->resolver (client->dispatch,
                          host,
                          handle_name_lookup_success,
                          handle_name_lookup_failure,
                          client);

        /* cleanup */
        client->allocator->free (client->allocator, host);
        return;
      }
    default:
      assert (0);
    }
}


void
handle_init_idle (ProtobufCDispatch *dispatch,
                  void              *data)
{
  ProtobufC_RPC_Client *client = data;
  protobuf_c_assert (client->state == PROTOBUF_C_CLIENT_STATE_INIT);
  begin_name_lookup (client);
}

static void
grow_closure_array (ProtobufC_RPC_Client *client)
{
  /* resize array */
  unsigned old_size = client->info.connected.closures_alloced;
  unsigned new_size = old_size * 2;
  unsigned i;
  Closure *new_closures = client->allocator->alloc (client->allocator, sizeof (Closure) * new_size);
  memcpy (new_closures,
          client->info.connected.closures,
          sizeof (Closure) * old_size);

  /* build new free list */
  for (i = old_size; i < new_size - 1; i++)
    {
      new_closures[i].response_type = NULL;
      new_closures[i].closure = NULL;
      new_closures[i].closure_data = UINT_TO_POINTER (i+2);
    }
  new_closures[i].closure_data = UINT_TO_POINTER (client->info.connected.first_free_request_id);
  new_closures[i].response_type = NULL;
  new_closures[i].closure = NULL;

  client->allocator->free (client->allocator, client->info.connected.closures);
  client->info.connected.closures = new_closures;
  client->info.connected.closures_alloced = new_size;
}
static uint32_t 
uint32_to_le (uint32_t le)
{
#if IS_LITTLE_ENDIAN
  return le;
#else
  return (le << 24) | (le >> 24)
       | ((le >> 8) & 0xff0000)
       | ((le << 8) & 0xff00);
#endif
}
#define uint32_from_le uint32_to_le             /* make the code more readable, i guess */

static void
enqueue_request (ProtobufC_RPC_Client *client,
                 unsigned          method_index,
                 const ProtobufCMessage *input,
                 ProtobufCClosure  closure,
                 void             *closure_data)
{
  uint32_t request_id;
  struct {
    uint32_t method_index;
    uint32_t packed_size;
    uint32_t request_id;
  } header;
  size_t packed_size;
  uint8_t *packed_data;
  Closure *cl;
  const ProtobufCServiceDescriptor *desc = client->base_service.descriptor;
  const ProtobufCMethodDescriptor *method = desc->methods + method_index;

  protobuf_c_assert (method_index < desc->n_methods);

  /* Allocate request_id */
  protobuf_c_assert (client->state == PROTOBUF_C_CLIENT_STATE_CONNECTED);
  if (client->info.connected.first_free_request_id == 0)
    grow_closure_array (client);
  request_id = client->info.connected.first_free_request_id;
  cl = client->info.connected.closures + (request_id - 1);
  client->info.connected.first_free_request_id = POINTER_TO_UINT (cl->closure_data);

  /* Pack message */
  packed_size = protobuf_c_message_get_packed_size (input);
  if (packed_size < client->allocator->max_alloca)
    packed_data = alloca (packed_size);
  else
    packed_data = client->allocator->alloc (client->allocator, packed_size);
  protobuf_c_message_pack (input, packed_data);

  /* Append to buffer */
  protobuf_c_assert (sizeof (header) == 16);
  header.method_index = uint32_to_le (method_index);
  header.packed_size = uint32_to_le (packed_size);
  header.request_id = request_id;
  protobuf_c_data_buffer_append (&client->outgoing, &header, 16);
  protobuf_c_data_buffer_append (&client->outgoing, packed_data, packed_size);

  /* Clean up if not using alloca() */
  if (packed_size >= client->allocator->max_alloca)
    client->allocator->free (client->allocator, packed_data);

  /* Add closure to request-tree */
  cl->response_type = method->output;
  cl->closure = closure;
  cl->closure_data = closure_data;
}

static void
handle_client_fd_events (int                fd,
                         unsigned           events,
                         void              *func_data)
{
  ProtobufC_RPC_Client *client = func_data;
  protobuf_c_assert (client->state == PROTOBUF_C_CLIENT_STATE_CONNECTED);
  if (events & PROTOBUF_C_EVENT_WRITABLE)
    {
      int write_rv = protobuf_c_data_buffer_writev (&client->outgoing,
                                                    client->fd);
      if (write_rv < 0 && !errno_is_ignorable (errno))
        {
          client_failed (client,
                         "writing to file-descriptor: %s",
                         strerror (errno));
          return;
        }

      if (client->outgoing.size == 0)
        protobuf_c_dispatch_watch_fd (client->dispatch, client->fd,
                                      PROTOBUF_C_EVENT_READABLE,
                                      handle_client_fd_events, client);
    }
  if (events & PROTOBUF_C_EVENT_READABLE)
    {
      /* do read */
      int read_rv = protobuf_c_data_buffer_read_in_fd (&client->incoming,
                                                       client->fd);
      if (read_rv < 0)
        {
          if (!errno_is_ignorable (errno))
            {
              client_failed (client,
                             "reading from file-descriptor: %s",
                             strerror (errno));
            }
        }
      else if (read_rv == 0)
        {
          /* handle eof */
          client_failed (client,
                         "got end-of-file from server [%u bytes incoming, %u bytes outgoing]",
                         client->incoming.size, client->outgoing.size);
        }
      else
        {
          /* try processing buffer */
          while (client->incoming.size >= 12)
            {
              uint32_t header[3];
              unsigned service_index, message_length, request_id;
              Closure *closure;
              uint8_t *packed_data;
              ProtobufCMessage *msg;
              protobuf_c_data_buffer_peek (&client->incoming, header, sizeof (header));
              service_index = uint32_from_le (header[0]);
              message_length = uint32_from_le (header[1]);
              request_id = header[2];           /* already native-endian */

              if (12 + message_length > client->incoming.size)
                break;

              /* lookup request by id */
              if (request_id >= client->info.connected.closures_alloced
               || request_id == 0
               || client->info.connected.closures[request_id-1].response_type == NULL)
                {
                  client_failed (client, "bad request-id in response from server");
                  return;
                }
              closure = client->info.connected.closures + (request_id - 1);

              /* read message and unpack */
              protobuf_c_data_buffer_discard (&client->incoming, 12);
              packed_data = client->allocator->alloc (client->allocator, message_length);
              protobuf_c_data_buffer_read (&client->incoming, packed_data, message_length);

              /* TODO: use fast temporary allocator */
              msg = protobuf_c_message_unpack (closure->response_type,
                                               client->allocator,
                                               message_length,
                                               packed_data);
              if (msg == NULL)
                {
                  client_failed (client, "failed to unpack message");
                  client->allocator->free (client->allocator, packed_data);
                  return;
                }

              /* invoke closure */
              closure->closure (msg, closure->closure_data);

              /* clean up */
              protobuf_c_message_free_unpacked (msg, client->allocator);
              client->allocator->free (client->allocator, packed_data);
            }
        }
    }
}

static void
update_connected_client_watch (ProtobufC_RPC_Client *client)
{
  unsigned events = PROTOBUF_C_EVENT_READABLE;
  protobuf_c_assert (client->state == PROTOBUF_C_CLIENT_STATE_CONNECTED);
  protobuf_c_assert (client->fd >= 0);
  if (client->outgoing.size > 0)
    events |= PROTOBUF_C_EVENT_WRITABLE;
  protobuf_c_dispatch_watch_fd (client->dispatch,
                                client->fd,
                                events,
                                handle_client_fd_events, client);
}

static void
invoke_client_rpc (ProtobufCService *service,
                   unsigned          method_index,
                   const ProtobufCMessage *input,
                   ProtobufCClosure  closure,
                   void             *closure_data)
{
  ProtobufC_RPC_Client *client = (ProtobufC_RPC_Client *) service;
  protobuf_c_assert (service->invoke == invoke_client_rpc);
  switch (client->state)
    {
    case PROTOBUF_C_CLIENT_STATE_INIT:
    case PROTOBUF_C_CLIENT_STATE_NAME_LOOKUP:
    case PROTOBUF_C_CLIENT_STATE_CONNECTING:
      enqueue_request (client, method_index, input, closure, closure_data);
      break;
      
    case PROTOBUF_C_CLIENT_STATE_CONNECTED:
      {
        int had_outgoing = (client->outgoing.size > 0);
        enqueue_request (client, method_index, input, closure, closure_data);
        if (!had_outgoing)
          update_connected_client_watch (client);
      }
      break;

    case PROTOBUF_C_CLIENT_STATE_FAILED_WAITING:
    case PROTOBUF_C_CLIENT_STATE_FAILED:
    case PROTOBUF_C_CLIENT_STATE_DESTROYED:
      closure (NULL, closure_data);
      break;
    }
}

static void
destroy_client_rpc (ProtobufCService *service)
{
  ProtobufC_RPC_Client *client = (ProtobufC_RPC_Client *) service;
  ProtobufC_RPC_ClientState state = client->state;
  unsigned i;
  unsigned n_closures = 0;
  Closure *closures = NULL;
  switch (state)
    {
    case PROTOBUF_C_CLIENT_STATE_INIT:
      protobuf_c_dispatch_remove_idle (client->info.init.idle);
      break;
    case PROTOBUF_C_CLIENT_STATE_NAME_LOOKUP:
      if (client->info.name_lookup.pending)
        {
          client->info.name_lookup.destroyed_while_pending = 1;
          return;
        }
      break;
    case PROTOBUF_C_CLIENT_STATE_CONNECTING:
      break;
    case PROTOBUF_C_CLIENT_STATE_CONNECTED:
      n_closures = client->info.connected.closures_alloced;
      closures = client->info.connected.closures;
      break;
    case PROTOBUF_C_CLIENT_STATE_FAILED_WAITING:
      protobuf_c_dispatch_remove_timer (client->info.failed_waiting.timer);
      client->allocator->free (client->allocator, client->info.failed_waiting.error_message);
      break;
    case PROTOBUF_C_CLIENT_STATE_FAILED:
      client->allocator->free (client->allocator, client->info.failed.error_message);
      break;
    case PROTOBUF_C_CLIENT_STATE_DESTROYED:
      protobuf_c_assert (0);
      break;
    }
  if (client->fd >= 0)
    {
      protobuf_c_dispatch_close_fd (client->dispatch, client->fd);
      client->fd = -1;
    }
  protobuf_c_data_buffer_clear (&client->incoming);
  protobuf_c_data_buffer_clear (&client->outgoing);
  client->state = PROTOBUF_C_CLIENT_STATE_DESTROYED;

  /* free closures only once we are in the destroyed state */
  for (i = 0; i < n_closures; i++)
    closures[i].closure (NULL, closures[i].closure_data);
  if (closures)
    client->allocator->free (client->allocator, closures);

  client->allocator->free (client->allocator, client);
}

ProtobufCService *protobuf_c_rpc_client_new (ProtobufC_RPC_AddressType type,
                                             const char               *name,
                                             const ProtobufCServiceDescriptor *descriptor,
                                             ProtobufCDispatch       *orig_dispatch)
{
  ProtobufCDispatch *dispatch = orig_dispatch ? orig_dispatch : protobuf_c_dispatch_default ();
  ProtobufCAllocator *allocator = protobuf_c_dispatch_peek_allocator (dispatch);
  ProtobufC_RPC_Client *rv = allocator->alloc (allocator, sizeof (ProtobufC_RPC_Client));
  rv->base_service.descriptor = descriptor;
  rv->base_service.invoke = invoke_client_rpc;
  rv->base_service.destroy = destroy_client_rpc;
  protobuf_c_data_buffer_init (&rv->incoming, allocator);
  protobuf_c_data_buffer_init (&rv->outgoing, allocator);
  rv->allocator = allocator;
  rv->dispatch = dispatch;
  rv->address_type = type;
  rv->name = strcpy (allocator->alloc (allocator, strlen (name) + 1), name);
  rv->state = PROTOBUF_C_CLIENT_STATE_INIT;
  rv->fd = -1;
  rv->info.init.idle = protobuf_c_dispatch_add_idle (dispatch, handle_init_idle, rv);
  return &rv->base_service;
}
