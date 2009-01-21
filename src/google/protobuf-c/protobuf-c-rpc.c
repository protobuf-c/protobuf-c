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
  union {
    struct {
      ProtobufCDispatch_Idle *idle;
    } init;
    struct {
      ProtobufCDispatch_Timer *timer;
    } failed_waiting;
  };
};

void
handle_init_idle (ProtobufCDispatch *

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
-
