#include "../google/protobuf-c/protobuf-c.h"

typedef struct _ProtobufC_SCTP_Dispatch ProtobufC_SCTP_Dispatch;
typedef struct _ProtobufC_SCTP_Channel ProtobufC_SCTP_Channel;
typedef struct _ProtobufC_SCTP_Listener ProtobufC_SCTP_Listener;

/* For servers and clients, you can get at the array of
   remote service objects from the closure-data of your local-service's
   invocation method. */
ProtobufCService **
protobuf_c_sctp_closure_data_get_remote_services (void *closure_data);

typedef struct {
  const char *name;
  ProtobufCService *service;
} ProtobufC_SCTP_LocalService;
typedef struct {
  const char *name;
  ProtobufCServiceDescriptor *descriptor;
} ProtobufC_SCTP_RemoteService;

ProtobufC_SCTP_Config *
protobuf_c_sctp_config_new (size_t       n_local_services,
                            ProtobufC_SCTP_LocalService *local_services,
                            size_t       n_remote_services,
                            ProtobufC_SCTP_RemoteService *remote_services);

/* a channel: an sctp association (technically sctp allows multiple
   streams within a single association (unlike tcp), but we do not
   use that ability.  when created as a client, this does automatic
   reconnecting. */
ProtobufC_SCTP_Channel *
     protobuf_c_sctp_client_new_ipv4(const uint8_t          *addr,
                                     uint16_t                port,
                                     ProtobufC_SCTP_Config  *config,
                                     ProtobufC_Dispatch     *dispatch);
ProtobufC_SCTP_Channel *
     protobuf_c_sctp_client_new_ipv4_dns (const char           *host,
                                          uint16_t              port,
                                          ProtobufC_SCTP_Config  *config,
                                          ProtobufC_Dispatch *dispatch);
ProtobufC_SCTP_Channel *
     protobuf_c_sctp_client_new_ipv4_hostport(const char *host_port,
                                              ProtobufC_SCTP_Config  *config,
                                              ProtobufC_SCTP_Dispatch *dispatch);
void protobuf_c_sctp_channel_set_error_handler (ProtobufC_SCTP_Channel *channel,
                                                ProtobufC_SCTP_ErrorFunc func,
                                                void                    *data,
                                                ProtobufC_SCTP_Destroy   destroy);
ProtobufCService *
     protobuf_c_sctp_channel_peek_remote_service(ProtobufC_SCTP_Channel *channel,
                                                 unsigned                index);
void protobuf_c_sctp_channel_shutdown          (ProtobufC_SCTP_Channel *channel);
void protobuf_c_sctp_channel_destroy           (ProtobufC_SCTP_Channel *channel);



/* a listener: a passive sctp socket awaiting new connections */
ProtobufC_SCTP_Server *
     protobuf_c_sctp_server_new_ipv4           (const uint8_t          *bind_addr,
                                                uint16_t                port,
                                                ProtobufC_SCTP_Config  *config);
void protobuf_c_sctp_server_destroy            (ProtobufC_SCTP_Server  *server);


