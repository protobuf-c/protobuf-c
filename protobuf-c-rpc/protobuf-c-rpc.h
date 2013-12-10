#ifndef __PROTOBUF_C_RPC_H_
#define __PROTOBUF_C_RPC_H_

#include "protobuf-c-dispatch.h"

typedef enum
{
  PROTOBUF_C_RPC_ADDRESS_LOCAL,  /* unix-domain socket */
  PROTOBUF_C_RPC_ADDRESS_TCP     /* host/port tcp socket */
} ProtobufC_RPC_AddressType;

typedef enum
{
  PROTOBUF_C_ERROR_CODE_HOST_NOT_FOUND,
  PROTOBUF_C_ERROR_CODE_CONNECTION_REFUSED,
  PROTOBUF_C_ERROR_CODE_CLIENT_TERMINATED,
  PROTOBUF_C_ERROR_CODE_BAD_REQUEST,
  PROTOBUF_C_ERROR_CODE_PROXY_PROBLEM
} ProtobufC_RPC_Error_Code;

typedef void (*ProtobufC_RPC_Error_Func)   (ProtobufC_RPC_Error_Code code,
                                            const char              *message,
                                            void                    *error_func_data);

typedef enum
{
  PROTOBUF_C_STATUS_CODE_SUCCESS,
  PROTOBUF_C_STATUS_CODE_SERVICE_FAILED,
  PROTOBUF_C_STATUS_CODE_TOO_MANY_PENDING
} ProtobufC_RPC_Status_Code;

/* Default RPC Protocol is:
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

/* --- Custom RPC Protocol API --- */
typedef struct
{
   uint32_t method_index;
   uint32_t request_id;
   ProtobufCMessage *message;
} ProtobufC_RPC_Payload;

typedef enum
{
   PROTOBUF_C_RPC_PROTOCOL_STATUS_SUCCESS,
   PROTOBUF_C_RPC_PROTOCOL_STATUS_FAILED,
   /* INCOMPLETE_BUFFER status indicates that the caller must read more data into
    * the buffer before it can be deserialized. */
   PROTOBUF_C_RPC_PROTOCOL_STATUS_INCOMPLETE_BUFFER
} ProtobufC_RPC_ProtocolStatus;

/* This function needs to be called by the deserializer with
 * the received payload (without the message) to get the descriptor
 * for the message to be unpacked */
typedef const ProtobufCMessageDescriptor* (*ProtobufC_RPC_GetDescriptor)
                                          (const ProtobufC_RPC_Payload* payload,
                                           void *data);
typedef struct
{
/* Serializes the payload into the provided buffer */
ProtobufC_RPC_ProtocolStatus (*serialize_func) (ProtobufCBuffer      *out_buffer,
                                                ProtobufC_RPC_Payload payload);
/* Deserializes the incoming buffer into a provided payload structure.
 * The unpacked message will be freed by the protobuf-c-rpc library. */
ProtobufC_RPC_ProtocolStatus (*deserialize_func) (ProtobufCAllocator    *allocator,
                                                  ProtobufCBuffer       *in_buffer,
                                                  ProtobufC_RPC_Payload *payload,
                                                  ProtobufC_RPC_GetDescriptor get_descriptor,
                                                  void *get_descriptor_data);
} ProtobufC_RPC_Protocol;

/* --- Client API --- */
typedef struct _ProtobufC_RPC_Client ProtobufC_RPC_Client;

/* The return value (the service) may be cast to ProtobufC_RPC_Client* */
ProtobufCService *protobuf_c_rpc_client_new (ProtobufC_RPC_AddressType type,
                                             const char               *name,
                                             const ProtobufCServiceDescriptor *descriptor,
                                             ProtobufCDispatch       *dispatch /* or NULL */
                                            );

/* forcing the client to connect */
typedef enum
{
  PROTOBUF_C_RPC_CLIENT_CONNECT_SUCCESS,/* also returned if already connected */
  PROTOBUF_C_RPC_CLIENT_CONNECT_ERROR_NAME_LOOKUP,
  PROTOBUF_C_RPC_CLIENT_CONNECT_ERROR_CONNECT
} ProtobufC_RPC_Client_ConnectStatus;

ProtobufC_RPC_Client_ConnectStatus
protobuf_c_rpc_client_connect (ProtobufC_RPC_Client *client);

/* --- configuring the client */

/* Custom RPC protocol */
void protobuf_c_rpc_client_set_rpc_protocol (ProtobufC_RPC_Client *client,
                                             ProtobufC_RPC_Protocol protocol);

/* Pluginable async dns hooks */
/* TODO: use adns library or port evdns? ugh */
typedef void (*ProtobufC_NameLookup_Found) (const uint8_t *address,
                                            void          *callback_data);
typedef void (*ProtobufC_NameLookup_Failed)(const char    *error_message,
                                            void          *callback_data);
typedef void (*ProtobufC_NameLookup_Func)  (ProtobufCDispatch *dispatch,
                                            const char        *name,
                                            ProtobufC_NameLookup_Found found_func,
                                            ProtobufC_NameLookup_Failed failed_func,
                                            void *callback_data);
void protobuf_c_rpc_client_set_name_resolver (ProtobufC_RPC_Client *client,
                                              ProtobufC_NameLookup_Func resolver);

/* Error handling */
void protobuf_c_rpc_client_set_error_handler (ProtobufC_RPC_Client *client,
                                              ProtobufC_RPC_Error_Func func,
                                              void                 *error_func_data);

/* Configuring the autoreconnect behavior.
   If the client is disconnected, all pending requests get an error.
   If autoreconnect is set, and it is by default, try connecting again
   after a certain amount of time has elapsed. */
void protobuf_c_rpc_client_disable_autoreconnect (ProtobufC_RPC_Client *client);
void protobuf_c_rpc_client_set_autoreconnect_period (ProtobufC_RPC_Client *client,
                                                 unsigned              millis);

/* checking the state of the client */
protobuf_c_boolean protobuf_c_rpc_client_is_connected (ProtobufC_RPC_Client *client);

/* NOTE: we don't actually start connecting til the main-loop runs,
   so you may configure the client immediately after creation */

/* --- Server API --- */
typedef struct _ProtobufC_RPC_Server ProtobufC_RPC_Server;
ProtobufC_RPC_Server *
     protobuf_c_rpc_server_new        (ProtobufC_RPC_AddressType type,
                                       const char               *name,
                                       ProtobufCService         *service,
                                       ProtobufCDispatch       *dispatch /* or NULL */
                                      );

ProtobufCService *
     protobuf_c_rpc_server_destroy    (ProtobufC_RPC_Server     *server,
                                       protobuf_c_boolean        free_underlying_service);

/* NOTE: these do not have guaranteed semantics if called after there are actually
   clients connected to the server!
   NOTE 2:  The purist in me has left the default of no-autotimeout.
   The pragmatist in me knows thats going to be a pain for someone.
   Please set autotimeout, and if you really don't want it, disable it explicitly,
   because i might just go and make it the default! */
void protobuf_c_rpc_server_disable_autotimeout(ProtobufC_RPC_Server *server);
void protobuf_c_rpc_server_set_autotimeout (ProtobufC_RPC_Server *server,
                                            unsigned              timeout_millis);

/* Custom RPC protocol */
void protobuf_c_rpc_server_set_rpc_protocol (ProtobufC_RPC_Server *server,
                                             ProtobufC_RPC_Protocol protocol);

typedef protobuf_c_boolean
          (*ProtobufC_RPC_IsRpcThreadFunc) (ProtobufC_RPC_Server *server,
                                            ProtobufCDispatch    *dispatch,
                                            void                 *is_rpc_data);
void protobuf_c_rpc_server_configure_threading (ProtobufC_RPC_Server *server,
                                                ProtobufC_RPC_IsRpcThreadFunc func,
                                                void          *is_rpc_data);


/* Error handling */
void protobuf_c_rpc_server_set_error_handler (ProtobufC_RPC_Server *server,
                                              ProtobufC_RPC_Error_Func func,
                                              void                 *error_func_data);

#endif
