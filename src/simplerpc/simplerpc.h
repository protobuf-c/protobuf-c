#ifndef __PROTOBUF_SIMPLERPC_H_
#ifndef __PROTOBUF_SIMPLERPC_H_

#include <google/protobuf-c/protobuf.h>

SimplerpcContext *simplerpc_context_new (void);
void              simplerpc_context_add_service
                                        (SimplerpcServer *server,
                                         const char      *domain,
                                         ProtobufCService *service);

SimplerpcServer * simplerpc_bind_ipv4   (int              tcp_port,
                                         SimplerpcBindIpv4Flags flags,
                                         SimplerpcContext *context,
                                         ProtobufCError **error);
SimplerpcServer * simplerpc_bind_local  (const char      *path,
                                         SimplerpcContext *context,
                                         ProtobufCError **error);
void              simplerpc_server_destroy (SimplerpcServer *server);



SimplerpcClient  *simplerpc_client_new_ipv4 (const uint8_t   *ip_addr,
                                             uint16_t         port,
					     ProtobufCError **error);

ProtobufCService *simplerpc_client_new_service (SimplerpcClient *client,
					     const char      *domain,
					     const ProtobufCServiceDescriptor *descriptor,
					     ProtobufCError **error);

#endif
