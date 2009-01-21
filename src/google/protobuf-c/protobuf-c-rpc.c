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
      ProtobufCDispatch_Timer *timer;
      char *error_message;
    } failed_waiting;
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
  ...
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
  if (client->first_outgoing_request != NULL)
    {
      /* register interest in writing to fd (there can be no pending requests, of course,
         since we just connected) */
      protobuf_c_dispatch_watch_fd (client->dispatch,
                                    client->fd,
                                    PROTOBUF_C_EVENT_WRITABLE,
                                    handle_connected_client_fd_events,
                                    client);
    }
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
enqueue_request (ProtobufC_RPC_Client *client,
                 unsigned          method_index,
                 const ProtobufCMessage *input,
                 ProtobufCClosure  closure,
                 void             *closure_data)
{
  ...
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
