#include "protobuf-c-data-buffer.h"

typedef struct _ProtobufC_RPC_Client ProtobufC_RPC_Client;

typedef enum
{
  PROTOBUF_C_CLIENT_STATE_NAME_LOOKUP,
  PROTOBUF_C_CLIENT_STATE_CONNECTING,
  PROTOBUF_C_CLIENT_STATE_CONNECTED,
  PROTOBUF_C_CLIENT_STATE_FAILED_WAITING
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
};


ProtobufCService *
protobuf_c_rpc_client_new (ProtobufC_RPC_Options *options,
                           const ProtobufCServiceDescriptor *descriptor)
{
  ProtobufCAllocator *allocator = options->allocator;
  ProtobufCDispatch *dispatch = options->dispatch ? options->dispatch : protobuf_c_dispatch_default ();
  ProtobufC
  ...
}
-
void              protobuf_c_rpc_server_new (ProtobufC_RPC_Options *options,
                                             ProtobufCService *service);
                                                  unsigned    port,
			     		          const ProtobufCServiceDescriptor *descriptor,
                                                  ProtobufCAllocator *allocator,
                                                  ProtobufCDispatch *dispatch);
ProtobufCService *protobuf_c_rpc_client_new_local(const char *socket_name,
			     		          const ProtobufCServiceDescriptor *descripto,
                                                  ProtobufCAllocator *allocator,
                                                  ProtobufCDispatch *dispatch);
void              protobuf_c_rpc_server_new_tcp  (ProtobufCService *service,
                                                  unsigned          port,
                                                  ProtobufCAllocator *allocator,
                                                  ProtobufCDispatch *dispatch);
void              protobuf_c_rpc_server_new_local(ProtobufCService *service,
                                                  const char       *socket_nam,
                                                  ProtobufCAllocator *allocator,
                                                  ProtobufCDispatch *dispatch);

---
