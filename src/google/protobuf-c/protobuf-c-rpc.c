#include "protobuf-c-rpc.h"
#include "protobuf-c-data-buffer.h"

typedef struct _ProtobufC_RPC_Client ProtobufC_RPC_Client;

typedef enum
{
  PROTOBUF_C_CLIENT_STATE_INIT,
  PROTOBUF_C_CLIENT_STATE_NAME_LOOKUP,
  PROTOBUF_C_CLIENT_STATE_CONNECTING,
  PROTOBUF_C_CLIENT_STATE_CONNECTED,
  PROTOBUF_C_CLIENT_STATE_FAILED_WAITING,
  PROTOBUF_C_CLIENT_STATE_FAILED                /* if no autoretry */
} ProtobufC_ClientState;

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
  ProtobufC_ClientState client_state;
  ProtobufC_FD fd;
  protobuf_c_boolean autoretry;
  unsigned autoretry_millis;
  ProtobufC_NameLookup_Func resolver;
  union {
    struct {
      ProtobufCDispatch_Idle *idle;
    } init;
    struct {
      protobuf_c_boolean pending;
    } name_lookup;
    struct {
      ProtobufCDispatchTimer *timer;
      char *error_message;
    } failed_waiting;
    struct {
      unsigned closures_alloced;
      unsigned first_free_request_id;
      /* indexed by (request_id-1) */
      Closure *closures;
    } connected;
    struct {
      char *error_message;
    } failed;
  };
};

static void
set_fd_nonblocking(int fd)
{
  int flags = fcntl (fd, F_GETFL);
  protobuf_c_assert (flags >= 0);
  fcntl (fd, F_SETFL, flags | O_NONBLOCK);
}

static void
client_failed (ProtobufC_RPC_Client *client,
               const char           *format_str,
               ...)
{
  va_list args;
  switch (client->state)
    {
    case PROTOBUF_C_CLIENT_STATE_NAME_LOOKUP:
      protobuf_c_assert (!client->info.name_lookup.pending);
      break;
    case PROTOBUF_C_CLIENT_STATE_CONNECTING:
      /* nothing to do */
      break;
    case PROTOBUF_C_CLIENT_STATE_CONNECTED:
      /* nothing to do */
      break;

      /* should not get here */
    case PROTOBUF_C_CLIENT_STATE_INIT:
    case PROTOBUF_C_CLIENT_STATE_FAILED_WAITING:
    case PROTOBUF_C_CLIENT_STATE_FAILED:
      protobuf_c_assert (FALSE);
      break;
    }
  if (client->fd >= 0)
    {
      protobuf_c_dispatch_close (client->dispatch, client->fd);
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
}

static void
begin_connecting (ProtobufC_RPC_Client *client,
                  struct sockaddr_t    *address,
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

  client->state = PROTOBUF_C_CLIENT_STATE_CONNECTED;

  client->info.connected.closures_alloced = 1;
  client->info.connected.first_free_request_id = 1;
  client->info.connected.closures = client->allocator->alloc (client->allocator, sizeof (Closure));
  client->info.connected.closures[0].closure = NULL;
  client->info.connected.closures[0].response_type = NULL;
  client->info.connected.closures[0].closure_data = UINT_TO_POINTER (0);
}
static void
handle_name_lookup_success (const uint8_t *address,
                            void          *callback_data)
{
  ProtobufC_RPC_Client *client = callback_data;
  struct sockaddr_in address;
  protobuf_c_assert (client->state == PROTOBUF_C_CLIENT_STATE_NAME_LOOKUP);
  protobuf_c_assert (client->info.name_lookup.pending);
  client->info.name_lookup.pending = 0;
  address.sin_family = PF_INET;
  memcpy (address.sin_addr, address, 4);
  address.sin_port = htons (client->info.name_lookup.port);
  begin_connecting (client, (struct sockaddr *) &address, sizeof (address));
}

static void
handle_name_lookup_failure (const char    *error_message,
                            void          *callback_data)
{
  ProtobufC_RPC_Client *client = callback_data;
  protobuf_c_assert (client->state == PROTOBUF_C_CLIENT_STATE_NAME_LOOKUP);
  protobuf_c_assert (client->info.name_lookup.pending);
  client->info.name_lookup.pending = 0;
  client_failed ("name lookup failed (for name from %s): %s", client->name, error_message);
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
        begin_connecting (client, (struct sockaddr *) &addr);
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
        client->info.name_lookup.port = port;
        client->resolver (client->dispatch,
                          hostname,
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
}

static void
grow_closure_array (ProtobufC_RPC_Client *client)
{
  /* resize array */
  unsigned old_size = client->info.connected.closures_alloced;
  unsigned new_size = old_size * 2;
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
  new_closures[i].closure_data = client->info.connected.first_free_request_id;
  new_closures[i].response_type = NULL;
  new_closures[i].closure = NULL;

  client->allocator->free (client->allocator, client->info.connected.closures);
  client->info.connected.closures = new_closures;
  client->info.connected.closures_alloced = new_size;
}

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
  Closure *closure;
  const ProtobufCServiceDescriptor *desc = client->base_service.descriptor;
  const ProtobufCMethodDescriptor *method = descriptor->methods + method_index;

  protobuf_c_assert (method_index < desc->n_methods);

  /* Allocate request_id */
  protobuf_c_assert (client->state == PROTOBUF_C_CLIENT_STATE_CONNECTED);
  if (client->info.connected.first_free_request_id == 0)
    grow_closure_array (client);
  request_id = client->info.connected.first_free_request_id;
  closure = client->info.connected.closures + (request_id - 1);
  client->info.connected.first_free_request_id = POINTER_TO_UINT (closure->closure_data);

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
  client->info.connected.closures[request_id-1].response_type = client->descriptor->methods[method_index].output;
  client->info.connected.closures[request_id-1].closure = closure;
  client->info.connected.closures[request_id-1].closure_data = closure_data;
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
        int had_outgoing = (client->first_outgoing_request != NULL);
        enqueue_request (client, method_index, input, closure, closure_data);
        if (!had_outgoing)
          update_connected_client_watch (client);
      }
      break;

    case PROTOBUF_C_CLIENT_STATE_FAILED_WAITING:
    case PROTOBUF_C_CLIENT_STATE_FAILED:               /* if no autoretry */
      closure (NULL, closure_data);
      break;
    }
}

ProtobufCService *protobuf_c_rpc_client_new (ProtobufC_RPC_AddressType type,
                                             const char               *name,
                                             const ProtobufCServiceDescriptor *descriptor,
                                             ProtobufCDispatch       *dispatch);
{
  ProtobufCDispatch *dispatch = options->dispatch ? options->dispatch : protobuf_c_dispatch_default ();
  ProtobufCAllocator *allocator = protobuf_c_dispatch_peek_allocator (dispatch);
  ProtobufC_RPC_Client *rv = allocator->alloc (allocator, sizeof (ProtobufC_RPC_Client));
  rv->base.descriptor = descriptor;
  rv->base.invoke = invoke_client_rpc;
  rv->base.destroy = destroy_client_rpc;
  protobuf_c_data_buffer_init (&rv->incoming);
  protobuf_c_data_buffer_init (&rv->outgoing);
  rv->allocator = allocator;
  rv->dispatch = dispatch;
  rv->address_type = type;
  rv->name = strcpy (allocator->alloc (allocator, strlen (name) + 1), name);
  rv->client_state = PROTOBUF_C_CLIENT_STATE_INIT;
  rv->fd = -1;
  rv->info.init = protobuf_c_dispatch_add_idle (dispatch, handle_init_idle, rv);
  return &rv->base;
}
