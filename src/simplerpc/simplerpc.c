#include <gsk/gsk.h>

struct _SimplerpcServerStream
{
  SimplerpcServer *server;
  GskStream *connection;
  GskBuffer incoming, outgoing;
};

struct _SimplerpcServer
{
  GskStreamListener *listener;
  GHashTable *domain_to_service;
  guint ref_count;
};


static SimplerpcServer *
simplerpc_server_from_listener (GskStreamListener *listener)
{
  SimplerpcServer *rv = g_slice_new (SimplerpcServer);
  rv->listener = listener;
  rv->ref_count = 1;
  rv->domain_to_service = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                 g_free,
                                                 protobuf_c_server_destroy);
  gsk_stream_listener_handle_accept (listener, handle_accept,
                                     handle_listener_error,
                                     rv, NULL);
}

SimplerpcServer *
simplerpc_bind_ipv4   (int              tcp_port,
                       SimplerpcBindIpv4Flags flags,
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
  return simplerpc_server_from_listener (listener);
}

SimplerpcServer *
simplerpc_bind_local  (const char      *path,
                       ProtobufCError **error)
{
  ...
  return simplerpc_server_from_listener (listener);
}
void
simplerpc_add_service (SimplerpcServer *server,
                       const char      *domain,
                       ProtobufCService *service)
{
  ...
}
void
simplerpc_server_destroy (SimplerpcServer *server)
{
  ...
}



SimplerpcClient  *
simplerpc_client_new_ipv4 (const uint8_t   *ip_addr,
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
