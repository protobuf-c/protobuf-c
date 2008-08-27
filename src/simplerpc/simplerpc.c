#include <gsk/gsk.h>
#include "generated/simplerpc.pb-c.h"

/* === server === */

struct _SimplerpcServerStream
{
  SimplerpcServer *server;
  GskStream *connection;
  GskBuffer incoming, outgoing;
};

struct _SimplerpcContext
{
  GHashTable *domain_to_service;
  guint ref_count;
};

struct _SimplerpcServer
{
  GskStreamListener *listener;
  SimplerpcContext *context;
};

static SimplerpcServer *
simplerpc_server_from_listener (GskStreamListener *listener,
                                SimplerpcContext  *context)
{
  SimplerpcServer *rv = g_slice_new (SimplerpcServer);
  rv->listener = listener;
  rv->domain_to_service = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                 g_free,
                                                 protobuf_c_server_destroy);
  gsk_stream_listener_handle_accept (listener, handle_accept,
                                     handle_listener_error,
                                     rv, NULL);
}

SimplerpcServer *
simplerpc_server_bind_ipv4   (int              tcp_port,
                              SimplerpcBindIpv4Flags flags,
                              SimplerpcContext *context,
                              ProtobufCError **error)
{
  GskStreamListener *listener;
  GskSocketAddress *addr;
  const guint8 *ipaddr = (flags & SIMPLERPC_BIND_IPV4_LOCALHOST)
                       ? gsk_ipv4_ip_address_localhost
                       : gsk_ipv4_ip_address_any;
  addr = gsk_socket_address_new_ipv4 (ipaddr, tcp_port);
  listener = gsk_stream_listener_socket_new_bind (addr, &ge);
  if (listener == NULL)
    {
      set_protobuf_c_error_from_gerror (error, ge);
      return NULL;
    }
  return simplerpc_server_from_listener (listener, context);
}

SimplerpcServer *
simplerpc_server_bind_local  (const char      *path,
                              SimplerpcContext *context,
                              ProtobufCError **error)
{
  GskStreamListener *listener;
  GskSocketAddress *addr;
  addr = gsk_socket_address_new_local (path);
  listener = gsk_stream_listener_socket_new_bind (addr, &ge);
  if (listener == NULL)
    {
      set_protobuf_c_error_from_gerror (error, ge);
      return NULL;
    }
  return simplerpc_server_from_listener (listener, context);
}


typedef struct _BuiltinService BuiltinService;
struct _BuiltinService
{
  Simplerpc__Builtin_Service base;
  SimplerpcContext *context;
};

static void
builtin_service_list_domains (Simplerpc__Builtin_Service *service,
                              const Simplerpc__DomainListRequest *input,
                              Simplerpc__DomainListResponse__ClosureFunc closure,
                              void *closure_data)
{
  Simplerpc__DomainList dl = SIMPLERPC__DOMAIN_INFO__INIT;
  ...
}

static void
builtin_service_destroy (Simplerpc__Builtin_Service *ser)
{
  BuiltinService *bs = (BuiltinService *) ser;
  g_slice_free (BuiltinService, bs);
}

SimplerpcContext *
simplerpc_context_new (void)
{
  SimplerpcContext *context = g_slice_new (SimplerpcContext);
  BuiltinService *service;
  context->domain_to_service
    = g_hash_table_new_full (g_str_hash, g_str_equal,
                             g_free,
                             (GDestroyNotify) protobuf_c_server_destroy);
  context->ref_count = 1;
  
  service = g_slice_new (BuiltinService);
  simplerpc__builtin__init (&service->base);
  service->base.list_domains = builtin_service_list_domains;
  service->base.base.destroy = builtin_service_free;
  service->context = context;
  simplerpc_context_add_service (context, "simplerpc.builtin",
                                 (ProtobufCService *) service);
  return context;
}


void
simplerpc_context_add_service (SimplerpcContext *context,
                               const char      *domain,
                               ProtobufCService *service)
{
  g_hash_table_insert (context, g_strdup (domain), service);
}

void
simplerpc_server_destroy (SimplerpcServer *server)
{
  ...
}

/* === client === */
typedef struct _SimplerpcClientRequest SimplerpcClientRequest;
typedef struct _SimplerpcClient SimplerpcClient;

struct _SimplerpcClientRequest
{
  guint64 request_id;
  gsize request_len;
  guint8 *request_data;
  ProtobufCClosure *closure;
  SimplerpcClientRequest *prev, *next;
};

struct _SimplerpcClient
{
  GskStream *transport;
  GskBuffer incoming;
  GskBuffer outgoing;
  GHashTable *request_id_to_request;

  guint ref_count;
  SimplerpcClientRequest *first, *last;
};

SimplerpcClient  *
simplerpc_client_new_ipv4 (const char      *hostname,
                           uint16_t         port,
                           ProtobufCError **error)
{
  ...
}

ProtobufCService *
simplerpc_client_new_service  (SimplerpcClient *client,
                               const char      *domain,
                               const ProtobufCServiceDescriptor *descriptor,
                               ProtobufCError **error)
{
  ...
}
