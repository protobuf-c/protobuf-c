/* KNOWN DEFECTS:
    - server does not obey max_pending_requests_per_connection
    - no ipv6 support
 */
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>
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

#define UINT_TO_POINTER(ui)      ((void*)(uintptr_t)(ui))
#define POINTER_TO_UINT(ptr)     ((unsigned)(uintptr_t)(ptr))

#define MAX_FAILED_MSG_LENGTH   512

typedef enum
{
  PROTOBUF_C_CLIENT_STATE_INIT,
  PROTOBUF_C_CLIENT_STATE_NAME_LOOKUP,
  PROTOBUF_C_CLIENT_STATE_CONNECTING,
  PROTOBUF_C_CLIENT_STATE_CONNECTED,
  PROTOBUF_C_CLIENT_STATE_FAILED_WAITING,
  PROTOBUF_C_CLIENT_STATE_FAILED,               /* if no autoreconnect */
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

static void
error_handler (ProtobufC_RPC_Error_Code code,
               const char              *message,
               void                    *error_func_data)
{
  fprintf (stderr, "*** error: %s: %s\n",
           (char*) error_func_data, message);
}

struct _ProtobufC_RPC_Client
{
  ProtobufCService base_service;
  ProtobufCBuffer *incoming;
  ProtobufCBuffer *outgoing;
  ProtobufCAllocator *allocator;
  ProtobufCDispatch *dispatch;
  ProtobufC_RPC_AddressType address_type;
  char *name;
  ProtobufC_FD fd;
  protobuf_c_boolean autoreconnect;
  unsigned autoreconnect_millis;
  ProtobufC_NameLookup_Func resolver;
  ProtobufC_RPC_Error_Func error_handler;
  void *error_handler_data;
  ProtobufC_RPC_Protocol rpc_protocol;
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
handle_autoreconnect_timeout (ProtobufCDispatch *dispatch,
                          void              *func_data)
{
  ProtobufC_RPC_Client *client = func_data;
  protobuf_c_assert (client->state == PROTOBUF_C_CLIENT_STATE_FAILED_WAITING);
  client->allocator->free (client->allocator,
                           client->info.failed_waiting.error_message);
  begin_name_lookup (client);
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
  protobuf_c_data_buffer_reset (client->incoming);
  protobuf_c_data_buffer_reset (client->outgoing);

  /* Compute the message */
  va_start (args, format_str);
  vsnprintf (buf, sizeof (buf), format_str, args);
  va_end (args);
  buf[sizeof(buf)-1] = 0;
  msg_len = strlen (buf);
  msg = client->allocator->alloc (client->allocator, msg_len + 1);
  strcpy (msg, buf);

  /* go to one of the failed states */
  if (client->autoreconnect)
    {
      client->state = PROTOBUF_C_CLIENT_STATE_FAILED_WAITING;
      client->info.failed_waiting.timer
        = protobuf_c_dispatch_add_timer_millis (client->dispatch,
                                                client->autoreconnect_millis,
                                                handle_autoreconnect_timeout,
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
  client->fd = socket (address->sa_family, SOCK_STREAM, 0);
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
  memset (&addr, 0, sizeof (addr));
  addr.sin_family = AF_INET;
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

static void
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
  client->info.connected.first_free_request_id = old_size + 1;

  client->allocator->free (client->allocator, client->info.connected.closures);
  client->info.connected.closures = new_closures;
  client->info.connected.closures_alloced = new_size;
}
static uint32_t
uint32_to_le (uint32_t le)
{
#if !defined(WORDS_BIGENDIAN)
  return le;
#else
  return (le << 24) | (le >> 24)
       | ((le >> 8) & 0xff00)
       | ((le << 8) & 0xff0000);
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
  const ProtobufCServiceDescriptor *desc = client->base_service.descriptor;
  const ProtobufCMethodDescriptor *method = desc->methods + method_index;

  protobuf_c_assert (method_index < desc->n_methods);

  /* Allocate request_id */
  //protobuf_c_assert (client->state == PROTOBUF_C_CLIENT_STATE_CONNECTED);
  if (client->info.connected.first_free_request_id == 0)
    grow_closure_array (client);

  uint32_t request_id = client->info.connected.first_free_request_id;

  /* Serialize the message */
  ProtobufC_RPC_Payload payload = {method_index,
                                   request_id,
                                   (ProtobufCMessage *)input};

  client->rpc_protocol.serialize_func (client->outgoing, payload);

  /* Add closure to request-tree */
  Closure *cl = client->info.connected.closures + (request_id - 1);
  client->info.connected.first_free_request_id = POINTER_TO_UINT (cl->closure_data);
  cl->response_type = method->output;
  cl->closure = closure;
  cl->closure_data = closure_data;
}

static const ProtobufCMessageDescriptor *
get_rcvd_message_descriptor (const ProtobufC_RPC_Payload *payload, void *data)
{
   ProtobufC_RPC_Client *client = (ProtobufC_RPC_Client *)data;
   uint32_t request_id = payload->request_id;

   /* lookup request by id */
   if (request_id > client->info.connected.closures_alloced
         || request_id == 0
         || client->info.connected.closures[request_id-1].response_type == NULL)
   {
      client_failed (client, "bad request-id in response from server");
      return NULL;
   }

   Closure *closure = client->info.connected.closures + (request_id - 1);
   return closure->response_type;
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
      int write_rv = protobuf_c_data_buffer_writev (client->outgoing,
                                                    client->fd);
      if (write_rv < 0 && !errno_is_ignorable (errno))
        {
          client_failed (client,
                         "writing to file-descriptor: %s",
                         strerror (errno));
          return;
        }

      if (client->outgoing->size == 0)
        protobuf_c_dispatch_watch_fd (client->dispatch, client->fd,
                                      PROTOBUF_C_EVENT_READABLE,
                                      handle_client_fd_events, client);
    }
  if (events & PROTOBUF_C_EVENT_READABLE)
    {
      /* do read */
      int read_rv = protobuf_c_data_buffer_read_in_fd (client->incoming,
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
                         client->incoming->size, client->outgoing->size);
        }
      else
        {
          /* try processing buffer */
          while (client->incoming->size > 0)
            {
              /* Deserialize the buffer */
              ProtobufC_RPC_Payload payload = {0};
              ProtobufC_RPC_ProtocolStatus status =
                client->rpc_protocol.deserialize_func (client->allocator,
                                                       client->incoming,
                                                       &payload,
                                                       get_rcvd_message_descriptor,
                                                       (void *) client);

              if (status == PROTOBUF_C_RPC_PROTOCOL_STATUS_INCOMPLETE_BUFFER)
                break;

              if (status == PROTOBUF_C_RPC_PROTOCOL_STATUS_FAILED)
              {
                client_failed (client, "error deserialing server response");
                return;
              }

              /* invoke closure */
              Closure *closure = client->info.connected.closures + (payload.request_id - 1);
              closure->closure (payload.message, closure->closure_data);
              closure->response_type = NULL;
              closure->closure = NULL;
              closure->closure_data = UINT_TO_POINTER (client->info.connected.first_free_request_id);
              client->info.connected.first_free_request_id = payload.request_id;

              /* clean up */
              if (payload.message)
                protobuf_c_message_free_unpacked (payload.message, client->allocator);
            }
        }
    }
}

/* Default RPC Protocol implementation:
 *    client issues request with header:
 *         method_index              32-bit little-endian
 *         message_length            32-bit little-endian
 *         request_id                32-bit any-endian
 *    server responds with header:
 *         status_code               32-bit little-endian
 *         method_index              32-bit little-endian
 *         message_length            32-bit little-endian
 *         request_id                32-bit any-endian
 */
static ProtobufC_RPC_ProtocolStatus client_serialize (ProtobufCBuffer      *out_buffer,
                                                      ProtobufC_RPC_Payload payload)
{
   if (!out_buffer)
      return PROTOBUF_C_RPC_PROTOCOL_STATUS_FAILED;

   if (!protobuf_c_message_check (payload.message))
      return PROTOBUF_C_RPC_PROTOCOL_STATUS_FAILED;

   size_t message_length = protobuf_c_message_get_packed_size (payload.message);
   uint32_t header[3];
   header[0] = uint32_to_le (payload.method_index);
   header[1] = uint32_to_le (message_length);
   header[2] = payload.request_id;
   protobuf_c_data_buffer_append (out_buffer, header, sizeof (header));

   if (protobuf_c_message_pack_to_buffer (payload.message, out_buffer)
         != message_length)
      return PROTOBUF_C_RPC_PROTOCOL_STATUS_FAILED;

   return PROTOBUF_C_RPC_PROTOCOL_STATUS_SUCCESS;
}

static ProtobufC_RPC_ProtocolStatus client_deserialize (ProtobufCAllocator    *allocator,
                                                        ProtobufCBuffer       *in_buffer,
                                                        ProtobufC_RPC_Payload *payload,
                                                        ProtobufC_RPC_GetDescriptor get_descriptor,
                                                        void *get_descriptor_data)
{
   if (!allocator || !in_buffer || !payload)
      return PROTOBUF_C_RPC_PROTOCOL_STATUS_FAILED;

   uint32_t header[4];
   if (in_buffer->size < sizeof (header))
      return PROTOBUF_C_RPC_PROTOCOL_STATUS_INCOMPLETE_BUFFER;

   /* try processing buffer */
   protobuf_c_data_buffer_peek (in_buffer, header, sizeof (header));
   uint32_t status_code = uint32_from_le (header[0]);
   payload->method_index = uint32_from_le (header[1]);
   uint32_t message_length = uint32_from_le (header[2]);
   payload->request_id = header[3];           /* already native-endian */

   if (sizeof (header) + message_length > in_buffer->size)
      return PROTOBUF_C_RPC_PROTOCOL_STATUS_INCOMPLETE_BUFFER;

   /* Discard the RPC header */
   protobuf_c_data_buffer_discard (in_buffer, sizeof (header));

   ProtobufCMessage *msg;
   if (status_code == PROTOBUF_C_STATUS_CODE_SUCCESS)
   {
      /* read message and unpack */
      const ProtobufCMessageDescriptor *desc = get_descriptor (payload, get_descriptor_data);
      if (!desc)
         return PROTOBUF_C_RPC_PROTOCOL_STATUS_FAILED;

      uint8_t *packed_data = allocator->alloc (allocator, message_length);

      if (!packed_data && message_length > 0)
         return PROTOBUF_C_RPC_PROTOCOL_STATUS_FAILED;

      if (message_length !=
          protobuf_c_data_buffer_read (in_buffer, packed_data, message_length))
      {
         if (packed_data)
            allocator->free (allocator, packed_data);
         return PROTOBUF_C_RPC_PROTOCOL_STATUS_FAILED;
      }

      /* TODO: use fast temporary allocator */
      msg = protobuf_c_message_unpack (desc,
            allocator,
            message_length,
            packed_data);

      if (packed_data)
         allocator->free (allocator, packed_data);
      if (msg == NULL)
         return PROTOBUF_C_RPC_PROTOCOL_STATUS_FAILED;
   }
   else
   {
      /* Server did not send a response message */
      protobuf_c_assert (message_length == 0);
      msg = NULL;
   }
   payload->message = msg;

   return PROTOBUF_C_RPC_PROTOCOL_STATUS_SUCCESS;
}

static void
update_connected_client_watch (ProtobufC_RPC_Client *client)
{
  unsigned events = PROTOBUF_C_EVENT_READABLE;
  protobuf_c_assert (client->state == PROTOBUF_C_CLIENT_STATE_CONNECTED);
  protobuf_c_assert (client->fd >= 0);
  if (client->outgoing->size > 0)
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
        int had_outgoing = (client->outgoing->size > 0);
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
      client->allocator->free (client->allocator, client->info.failed_waiting.timer);
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
  protobuf_c_data_buffer_destruct (client->incoming);
  protobuf_c_data_buffer_destruct (client->outgoing);
  client->state = PROTOBUF_C_CLIENT_STATE_DESTROYED;
  client->allocator->free (client->allocator, client->name);

  /* free closures only once we are in the destroyed state */
  for (i = 0; i < n_closures; i++)
    if (closures[i].response_type != NULL)
      closures[i].closure (NULL, closures[i].closure_data);
  if (closures)
    client->allocator->free (client->allocator, closures);

  client->allocator->free (client->allocator, client);
}

static void
trivial_sync_libc_resolver (ProtobufCDispatch *dispatch,
                            const char        *name,
                            ProtobufC_NameLookup_Found found_func,
                            ProtobufC_NameLookup_Failed failed_func,
                            void *callback_data)
{
  struct hostent *ent;
  ent = gethostbyname (name);
  if (ent == NULL)
    failed_func (hstrerror (h_errno), callback_data);
  else
    found_func ((const uint8_t *) ent->h_addr_list[0], callback_data);
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
  rv->incoming = protobuf_c_data_buffer_create (allocator);
  rv->outgoing = protobuf_c_data_buffer_create (allocator);
  rv->allocator = allocator;
  rv->dispatch = dispatch;
  rv->address_type = type;
  rv->name = strcpy (allocator->alloc (allocator, strlen (name) + 1), name);
  rv->state = PROTOBUF_C_CLIENT_STATE_INIT;
  rv->fd = -1;
  rv->autoreconnect = 1;
  rv->autoreconnect_millis = 2*1000;
  rv->resolver = trivial_sync_libc_resolver;
  rv->error_handler = error_handler;
  rv->error_handler_data = "protobuf-c rpc client";
  rv->info.init.idle = protobuf_c_dispatch_add_idle (dispatch, handle_init_idle, rv);
  ProtobufC_RPC_Protocol default_rpc_protocol = {client_serialize, client_deserialize};
  rv->rpc_protocol = default_rpc_protocol;
  return &rv->base_service;
}

protobuf_c_boolean
protobuf_c_rpc_client_is_connected (ProtobufC_RPC_Client *client)
{
  return client->state == PROTOBUF_C_CLIENT_STATE_CONNECTED;
}

void
protobuf_c_rpc_client_set_autoreconnect_period (ProtobufC_RPC_Client *client,
                                            unsigned millis)
{
  client->autoreconnect = 1;
  client->autoreconnect_millis = millis;
}


void
protobuf_c_rpc_client_set_error_handler (ProtobufC_RPC_Client *client,
                                         ProtobufC_RPC_Error_Func func,
                                         void                    *func_data)
{
  client->error_handler = func;
  client->error_handler_data = func_data;
}

void
protobuf_c_rpc_client_disable_autoreconnect (ProtobufC_RPC_Client *client)
{
  client->autoreconnect = 0;
}

void
protobuf_c_rpc_client_set_rpc_protocol (ProtobufC_RPC_Client *client,
                                        ProtobufC_RPC_Protocol protocol)
{
   client->rpc_protocol = protocol;
}
