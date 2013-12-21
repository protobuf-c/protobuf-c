#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/un.h>

#include "protobuf-c-rpc.h"
#include "protobuf-c-data-buffer.h"
#include "gsklistmacros.h"

#undef TRUE
#define TRUE 1
#undef FALSE
#define FALSE 0

#define MAX_FAILED_MSG_LENGTH   512

/* enabled for efficiency, can be useful to disable for debugging */
#define RECYCLE_REQUESTS                1

/* === Server === */
typedef struct _ServerRequest ServerRequest;
typedef struct _ServerConnection ServerConnection;
struct _ServerRequest
{
  uint32_t request_id;                  /* in little-endian */
  uint32_t method_index;                /* in native-endian */
  ServerConnection *conn;
  ProtobufC_RPC_Server *server;
  union {
    /* if conn != NULL, then the request is alive: */
    struct { ServerRequest *prev, *next; } alive;

    /* if conn == NULL, then the request is defunct: */
    struct { ProtobufCAllocator *allocator; } defunct;

    /* well, if it is in the recycled list, then it's recycled :/ */
    struct { ServerRequest *next; } recycled;
  } info;
};
struct _ServerConnection
{
  int fd;
  ProtobufCBuffer *incoming, *outgoing;

  ProtobufC_RPC_Server *server;
  ServerConnection *prev, *next;

  unsigned n_pending_requests;
  ServerRequest *first_pending_request, *last_pending_request;
};


/* When we get a response in the wrong thread,
   we proxy it over a system pipe.  Actually, we allocate one
   of these structures and pass the pointer over the pipe.  */
typedef struct _ProxyResponse ProxyResponse;
struct _ProxyResponse
{
  ServerRequest *request;
  unsigned len;
  /* data follows the structure */
};

struct _ProtobufC_RPC_Server
{
  ProtobufCDispatch *dispatch;
  ProtobufCAllocator *allocator;
  ProtobufCService *underlying;
  ProtobufC_RPC_AddressType address_type;
  char *bind_name;
  ServerConnection *first_connection, *last_connection;
  ProtobufC_FD listening_fd;

  ServerRequest *recycled_requests;

  /* multithreading support */
  ProtobufC_RPC_IsRpcThreadFunc is_rpc_thread_func;
  void * is_rpc_thread_data;
  int proxy_pipe[2];
  unsigned proxy_extra_data_len;
  uint8_t proxy_extra_data[sizeof (void*)];

  ProtobufC_RPC_Error_Func error_handler;
  void *error_handler_data;

  ProtobufC_RPC_Protocol rpc_protocol;

  /* configuration */
  unsigned max_pending_requests_per_connection;
};

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
set_fd_nonblocking(int fd)
{
  int flags = fcntl (fd, F_GETFL);

  if (flags >= 0)
    fcntl (fd, F_SETFL, flags | O_NONBLOCK);
}

static void
error_handler (ProtobufC_RPC_Error_Code code,
               const char              *message,
               void                    *error_func_data)
{
  fprintf (stderr, "*** error: %s: %s\n",
           (char*) error_func_data, message);
}

#define GET_PENDING_REQUEST_LIST(conn) \
  ServerRequest *, conn->first_pending_request, conn->last_pending_request, info.alive.prev, info.alive.next
#define GET_CONNECTION_LIST(server) \
  ServerConnection *, server->first_connection, server->last_connection, prev, next

static void
server_connection_close (ServerConnection *conn)
{
  ProtobufCAllocator *allocator = conn->server->allocator;

  /* general cleanup */
  protobuf_c_dispatch_close_fd (conn->server->dispatch, conn->fd);
  conn->fd = -1;
  protobuf_c_data_buffer_destruct (conn->incoming);
  protobuf_c_data_buffer_destruct (conn->outgoing);

  /* remove this connection from the server's list */
  GSK_LIST_REMOVE (GET_CONNECTION_LIST (conn->server), conn);

  /* disassocate all the requests from the connection */
  while (conn->first_pending_request != NULL)
    {
      ServerRequest *req = conn->first_pending_request;
      conn->first_pending_request = req->info.alive.next;
      req->conn = NULL;
      req->info.defunct.allocator = allocator;
    }

  /* free the connection itself */
  allocator->free (allocator, conn);
}

static void
server_failed_literal (ProtobufC_RPC_Server *server,
                       ProtobufC_RPC_Error_Code code,
                       const char *msg)
{
  if (server->error_handler != NULL)
    server->error_handler (code, msg, server->error_handler_data);
}

#if 0
static void
server_failed (ProtobufC_RPC_Server *server,
               ProtobufC_RPC_Error_Code code,
               const char           *format,
               ...)
{
  va_list args;
  char buf[MAX_FAILED_MSG_LENGTH];
  va_start (args, format);
  vsnprintf (buf, sizeof (buf), format, args);
  buf[sizeof(buf)-1] = 0;
  va_end (args);

  server_failed_literal (server, code, buf);
}
#endif

static protobuf_c_boolean
address_to_name (const struct sockaddr *addr,
                 unsigned               addr_len,
                 char                  *name_out,
                 unsigned               name_out_buf_length)
{
  if (addr->sa_family == PF_INET)
    {
      /* convert to dotted address + port */
      const struct sockaddr_in *addr_in = (const struct sockaddr_in *) addr;
      const uint8_t *addr = (const uint8_t *) &(addr_in->sin_addr);
      uint16_t port = htons (addr_in->sin_port);
      snprintf (name_out, name_out_buf_length,
                "%u.%u.%u.%u:%u",
                addr[0], addr[1], addr[2], addr[3], port);
      return TRUE;
    }
  return FALSE;
}

static void
server_connection_failed (ServerConnection *conn,
                          ProtobufC_RPC_Error_Code code,
                          const char       *format,
                          ...)
{
  char remote_addr_name[64];
  char msg[MAX_FAILED_MSG_LENGTH];
  char *msg_end = msg + sizeof (msg);
  char *msg_at;
  struct sockaddr addr;
  socklen_t addr_len = sizeof (addr);
  va_list args;

  /* if we can, find the remote name of this connection */
  if (getpeername (conn->fd, &addr, &addr_len) == 0
   && address_to_name (&addr, addr_len, remote_addr_name, sizeof (remote_addr_name)))
    snprintf (msg, sizeof (msg), "connection to %s from %s: ",
              conn->server->bind_name, remote_addr_name);
  else
    snprintf (msg, sizeof (msg), "connection to %s: ",
              conn->server->bind_name);
  msg[sizeof(msg)-1] = 0;
  msg_at = strchr (msg, 0);

  /* do vsnprintf() */
  va_start (args, format);
  vsnprintf(msg_at, msg_end - msg_at, format, args);
  va_end (args);
  msg[sizeof(msg)-1] = 0;

  /* invoke server error hook */
  server_failed_literal (conn->server, code, msg);

  server_connection_close (conn);
}

static ServerRequest *
create_server_request (ServerConnection *conn,
                       uint32_t          request_id,
                       uint32_t          method_index)
{
  ServerRequest *rv;
  if (conn->server->recycled_requests != NULL)
    {
      rv = conn->server->recycled_requests;
      conn->server->recycled_requests = rv->info.recycled.next;
    }
  else
    {
      ProtobufCAllocator *allocator = conn->server->allocator;
      rv = allocator->alloc (allocator, sizeof (ServerRequest));
    }
  rv->server = conn->server;
  rv->conn = conn;
  rv->request_id = request_id;
  rv->method_index = method_index;
  conn->n_pending_requests++;
  GSK_LIST_APPEND (GET_PENDING_REQUEST_LIST (conn), rv);
  return rv;
}

static void
free_server_request (ProtobufC_RPC_Server *server,
                     ServerRequest        *request)
{
#if RECYCLE_REQUESTS
  /* recycle request */
  request->info.recycled.next = server->recycled_requests;
  server->recycled_requests = request;
#else
  /* free the request immediately */
  server->allocator->free (server->allocator, request);
#endif
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

static void handle_server_connection_events (int fd,
                                             unsigned events,
                                             void *data);
static void
server_connection_response_closure (const ProtobufCMessage *message,
                                    void                   *closure_data)
{
  ServerRequest *request = closure_data;
  ProtobufC_RPC_Server *server = request->server;
  protobuf_c_boolean must_proxy = 0;
  ProtobufCAllocator *allocator = server->allocator;
  if (server->is_rpc_thread_func != NULL)
    {
      must_proxy = !server->is_rpc_thread_func (server,
                                                server->dispatch,
                                                server->is_rpc_thread_data);
    }


  uint8_t buffer_slab[512];
  ProtobufCBufferSimple buffer_simple = PROTOBUF_C_BUFFER_SIMPLE_INIT (buffer_slab);

  ProtobufC_RPC_Payload payload = { request->method_index,
                                    request->request_id,
                                    (ProtobufCMessage *)message };
  ProtobufC_RPC_ProtocolStatus status =
    server->rpc_protocol.serialize_func (&buffer_simple.base, payload);

  if (status != PROTOBUF_C_RPC_PROTOCOL_STATUS_SUCCESS)
    {
      server_failed_literal (server, PROTOBUF_C_ERROR_CODE_BAD_REQUEST,
                             "error serializing the response");
      return;
    }

  if (must_proxy)
    {
      ProxyResponse *pr = allocator->alloc (allocator, sizeof (ProxyResponse) + buffer_simple.base.size);
      int rv;
      pr->request = request;
      pr->len = buffer_simple.base.size;
      memcpy (pr + 1, buffer_simple.data, buffer_simple.base.size);

      /* write pointer to proxy pipe */
retry_write:
      rv = write (server->proxy_pipe[1], &pr, sizeof (void*));
      if (rv < 0)
        {
          if (errno == EINTR || errno == EAGAIN)
            goto retry_write;
          server_failed_literal (server, PROTOBUF_C_ERROR_CODE_PROXY_PROBLEM,
                                 "error writing to proxy-pipe");
          allocator->free (allocator, pr);
        }
      else if (rv < sizeof (void *))
        {
          server_failed_literal (server, PROTOBUF_C_ERROR_CODE_PROXY_PROBLEM,
                                 "partial write to proxy-pipe");
          allocator->free (allocator, pr);
        }
    }
  else if (request->conn == NULL)
    {
      /* defunct request */
      allocator->free (allocator, request);
      free_server_request (server, request);
    }
  else
    {
      ServerConnection *conn = request->conn;
      protobuf_c_boolean must_set_output_watch = (conn->outgoing->size == 0);
      protobuf_c_data_buffer_append (conn->outgoing, buffer_simple.data, buffer_simple.base.size);
      if (must_set_output_watch)
        protobuf_c_dispatch_watch_fd (conn->server->dispatch,
                                      conn->fd,
                                      PROTOBUF_C_EVENT_READABLE|PROTOBUF_C_EVENT_WRITABLE,
                                      handle_server_connection_events,
                                      conn);
      GSK_LIST_REMOVE (GET_PENDING_REQUEST_LIST (conn), request);
      conn->n_pending_requests--;

      free_server_request (server, request);
    }
  PROTOBUF_C_BUFFER_SIMPLE_CLEAR (&buffer_simple);
}

static const ProtobufCMessageDescriptor *
get_rcvd_message_descriptor (const ProtobufC_RPC_Payload *payload, void *data)
{
   ServerConnection *conn = data;
   ProtobufCService *service = conn->server->underlying;
   uint32_t method_index = payload->method_index;

   if (method_index >= service->descriptor->n_methods)
   {
      server_connection_failed (conn,
            PROTOBUF_C_ERROR_CODE_BAD_REQUEST,
            "bad method_index %u", method_index);
      return NULL;
   }

   return service->descriptor->methods[method_index].input;
}

static void
handle_server_connection_events (int fd,
                                 unsigned events,
                                 void *data)
{
  ServerConnection *conn = data;
  ProtobufCService *service = conn->server->underlying;
  ProtobufCAllocator *allocator = conn->server->allocator;
  if (events & PROTOBUF_C_EVENT_READABLE)
    {
      int read_rv = protobuf_c_data_buffer_read_in_fd (conn->incoming, fd);
      if (read_rv < 0)
        {
          if (!errno_is_ignorable (errno))
            {
              server_connection_failed (conn,
                                        PROTOBUF_C_ERROR_CODE_CLIENT_TERMINATED,
                                        "reading from file-descriptor: %s",
                                        strerror (errno));
              return;
            }
        }
      else if (read_rv == 0)
        {
          if (conn->first_pending_request != NULL)
            server_connection_failed (conn,
                                      PROTOBUF_C_ERROR_CODE_CLIENT_TERMINATED,
                                      "closed while calls pending");
          else
            server_connection_close (conn);
          return;
        }
      else
        while (conn->incoming->size > 0)
          {
            /* Deserialize the buffer */
            ProtobufC_RPC_Payload payload = {0};
            ProtobufC_RPC_ProtocolStatus status =
              conn->server->rpc_protocol.deserialize_func (allocator,
                                                           conn->incoming,
                                                           &payload,
                                                           get_rcvd_message_descriptor,
                                                           data);
            if (status == PROTOBUF_C_RPC_PROTOCOL_STATUS_INCOMPLETE_BUFFER)
              break;

            if (status == PROTOBUF_C_RPC_PROTOCOL_STATUS_FAILED)
            {
              server_connection_failed (conn,
                                        PROTOBUF_C_ERROR_CODE_BAD_REQUEST,
                                        "error deserializing client request");
              return;
            }

            /* Invoke service (note that it may call back immediately) */
            ServerRequest *server_request = create_server_request (conn,
                                                                   payload.request_id,
                                                                   payload.method_index);
            service->invoke (service, payload.method_index, payload.message,
                             server_connection_response_closure, server_request);

            /* clean up */
            if (payload.message)
              protobuf_c_message_free_unpacked (payload.message, allocator);
          }
    }
  if ((events & PROTOBUF_C_EVENT_WRITABLE) != 0
    && conn->outgoing->size > 0)
    {
      int write_rv = protobuf_c_data_buffer_writev (conn->outgoing, fd);
      if (write_rv < 0)
        {
          if (!errno_is_ignorable (errno))
            {
              server_connection_failed (conn,
                                        PROTOBUF_C_ERROR_CODE_CLIENT_TERMINATED,
                                        "writing to file-descriptor: %s",
                                        strerror (errno));
              return;
            }
        }
      if (conn->outgoing->size == 0)
        protobuf_c_dispatch_watch_fd (conn->server->dispatch, conn->fd, PROTOBUF_C_EVENT_READABLE,
                                      handle_server_connection_events, conn);
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
static ProtobufC_RPC_ProtocolStatus
server_serialize (ProtobufCBuffer      *out_buffer,
                  ProtobufC_RPC_Payload payload)
{
   if (!out_buffer)
      return PROTOBUF_C_RPC_PROTOCOL_STATUS_FAILED;

   uint32_t header[4];
   if (!protobuf_c_message_check (payload.message))
   {
      /* send failed response */
      header[0] = uint32_to_le (PROTOBUF_C_STATUS_CODE_SERVICE_FAILED);
      header[1] = uint32_to_le (payload.method_index);
      header[2] = 0;            /* no message */
      header[3] = payload.request_id;
      out_buffer->append (out_buffer, sizeof (header), (uint8_t *)header);
   }
   else
   {
      /* send success response */
      size_t message_length = protobuf_c_message_get_packed_size (payload.message);
      header[0] = uint32_to_le (PROTOBUF_C_STATUS_CODE_SUCCESS);
      header[1] = uint32_to_le (payload.method_index);
      header[2] = uint32_to_le (message_length);
      header[3] = payload.request_id;

      out_buffer->append (out_buffer, sizeof (header), (uint8_t *)header);
      if (protobuf_c_message_pack_to_buffer (payload.message, out_buffer)
            != message_length)
         return PROTOBUF_C_RPC_PROTOCOL_STATUS_FAILED;
   }
   return PROTOBUF_C_RPC_PROTOCOL_STATUS_SUCCESS;
}

static ProtobufC_RPC_ProtocolStatus
server_deserialize (ProtobufCAllocator    *allocator,
                    ProtobufCBuffer       *in_buffer,
                    ProtobufC_RPC_Payload *payload,
                    ProtobufC_RPC_GetDescriptor get_descriptor,
                    void *get_descriptor_data)
{
   if (!allocator || !in_buffer || !payload)
      return PROTOBUF_C_RPC_PROTOCOL_STATUS_FAILED;

   uint32_t header[3];
   if (in_buffer->size < sizeof (header))
      return PROTOBUF_C_RPC_PROTOCOL_STATUS_INCOMPLETE_BUFFER;

   uint32_t message_length;
   uint8_t *packed_data;
   ProtobufCMessage *message;

   protobuf_c_data_buffer_peek (in_buffer, header, sizeof (header));
   payload->method_index = uint32_from_le (header[0]);
   message_length = uint32_from_le (header[1]);
   /* store request_id in whatever endianness it comes in */
   payload->request_id = header[2];

   if (in_buffer->size < sizeof (header) + message_length)
      return PROTOBUF_C_RPC_PROTOCOL_STATUS_INCOMPLETE_BUFFER;

   const ProtobufCMessageDescriptor *desc =
      get_descriptor (payload, get_descriptor_data);
   if (!desc)
      return PROTOBUF_C_RPC_PROTOCOL_STATUS_FAILED;

   /* Read message */
   protobuf_c_data_buffer_discard (in_buffer, sizeof (header));
   packed_data = allocator->alloc (allocator, message_length);
   protobuf_c_data_buffer_read (in_buffer, packed_data, message_length);

   /* Unpack message */
   message = protobuf_c_message_unpack (desc, allocator, message_length, packed_data);
   allocator->free (allocator, packed_data);
   if (message == NULL)
      return PROTOBUF_C_RPC_PROTOCOL_STATUS_FAILED;

   payload->message = message;

   return PROTOBUF_C_RPC_PROTOCOL_STATUS_SUCCESS;
}


static void
handle_server_listener_readable (int fd,
                                 unsigned events,
                                 void *data)
{
  ProtobufC_RPC_Server *server = data;
  struct sockaddr addr;
  socklen_t addr_len = sizeof (addr);
  int new_fd = accept (fd, &addr, &addr_len);
  ServerConnection *conn;
  ProtobufCAllocator *allocator = server->allocator;
  if (new_fd < 0)
    {
      if (errno_is_ignorable (errno))
        return;
      fprintf (stderr, "error accept()ing file descriptor: %s\n",
               strerror (errno));
      return;
    }
  conn = allocator->alloc (allocator, sizeof (ServerConnection));
  conn->fd = new_fd;
  conn->incoming = protobuf_c_data_buffer_create (server->allocator);
  conn->outgoing = protobuf_c_data_buffer_create (server->allocator);
  conn->n_pending_requests = 0;
  conn->first_pending_request = conn->last_pending_request = NULL;
  conn->server = server;
  GSK_LIST_APPEND (GET_CONNECTION_LIST (server), conn);
  protobuf_c_dispatch_watch_fd (server->dispatch, conn->fd, PROTOBUF_C_EVENT_READABLE,
                                handle_server_connection_events, conn);
}

static ProtobufC_RPC_Server *
server_new_from_fd (ProtobufC_FD              listening_fd,
                    ProtobufC_RPC_AddressType address_type,
                    const char               *bind_name,
                    ProtobufCService         *service,
                    ProtobufCDispatch       *orig_dispatch)
{
  ProtobufCDispatch *dispatch = orig_dispatch ? orig_dispatch : protobuf_c_dispatch_default ();
  ProtobufCAllocator *allocator = protobuf_c_dispatch_peek_allocator (dispatch);
  ProtobufC_RPC_Server *server = allocator->alloc (allocator, sizeof (ProtobufC_RPC_Server));
  server->dispatch = dispatch;
  server->allocator = allocator;
  server->underlying = service;
  server->first_connection = server->last_connection = NULL;
  server->max_pending_requests_per_connection = 32;
  server->address_type = address_type;
  server->bind_name = allocator->alloc (allocator, strlen (bind_name) + 1);
  server->error_handler = error_handler;
  server->error_handler_data = "protobuf-c rpc server";
  server->listening_fd = listening_fd;
  server->recycled_requests = NULL;
  server->is_rpc_thread_func = NULL;
  server->is_rpc_thread_data = NULL;
  server->proxy_pipe[0] = server->proxy_pipe[1] = -1;
  server->proxy_extra_data_len = 0;
  ProtobufC_RPC_Protocol default_rpc_protocol = {server_serialize, server_deserialize};
  server->rpc_protocol = default_rpc_protocol;
  strcpy (server->bind_name, bind_name);
  set_fd_nonblocking (listening_fd);
  protobuf_c_dispatch_watch_fd (dispatch, listening_fd, PROTOBUF_C_EVENT_READABLE,
                                handle_server_listener_readable, server);
  return server;
}

/* this function is for handling the common problem
   that we bind over-and-over again to the same
   unix path.

   ideally, you'd think the OS's SO_REUSEADDR flag would
   cause this to happen, but it doesn't,
   at least on my linux 2.6 box.

   in fact, we really need a way to test without
   actually connecting to the remote server,
   which might annoy it.

   XXX: we should survey what others do here... like x-windows...
 */
/* NOTE: stolen from gsk, obviously */
static void
_gsk_socket_address_local_maybe_delete_stale_socket (const char *path,
                                                     struct sockaddr *addr,
                                                     unsigned addr_len)
{
  int fd;
  struct stat statbuf;
  if (stat (path, &statbuf) < 0)
    return;
  if (!S_ISSOCK (statbuf.st_mode))
    {
      fprintf (stderr, "%s existed but was not a socket\n", path);
      return;
    }

  fd = socket (PF_UNIX, SOCK_STREAM, 0);
  if (fd < 0)
    return;
  set_fd_nonblocking (fd);
  if (connect (fd, addr, addr_len) < 0)
    {
      if (errno == EINPROGRESS)
        {
          close (fd);
          return;
        }
    }
  else
    {
      close (fd);
      return;
    }

  /* ok, we should delete the stale socket */
  close (fd);
  if (unlink (path) < 0)
    fprintf (stderr, "unable to delete %s: %s\n",
             path, strerror(errno));
}
ProtobufC_RPC_Server *
protobuf_c_rpc_server_new       (ProtobufC_RPC_AddressType type,
                                 const char               *name,
                                 ProtobufCService         *service,
                                 ProtobufCDispatch       *dispatch)
{
  int fd = -1;
  int protocol_family;
  struct sockaddr *address;
  socklen_t address_len;
  struct sockaddr_un addr_un;
  struct sockaddr_in addr_in;
  switch (type)
    {
    case PROTOBUF_C_RPC_ADDRESS_LOCAL:
      protocol_family = PF_UNIX;
      memset (&addr_un, 0, sizeof (addr_un));
      addr_un.sun_family = AF_UNIX;
      strncpy (addr_un.sun_path, name, sizeof (addr_un.sun_path));
      address_len = sizeof (addr_un);
      address = (struct sockaddr *) (&addr_un);
      _gsk_socket_address_local_maybe_delete_stale_socket (name,
                                                           address,
                                                           address_len);
      break;
    case PROTOBUF_C_RPC_ADDRESS_TCP:
      protocol_family = PF_INET;
      memset (&addr_in, 0, sizeof (addr_in));
      addr_in.sin_family = AF_INET;
      {
        unsigned port = atoi (name);
        addr_in.sin_port = htons (port);
      }
      address_len = sizeof (addr_in);
      address = (struct sockaddr *) (&addr_in);
      break;
    default:
      return NULL;
    }

  fd = socket (protocol_family, SOCK_STREAM, 0);
  if (fd < 0)
    {
      fprintf (stderr, "protobuf_c_rpc_server_new: socket() failed: %s\n",
               strerror (errno));
      return NULL;
    }
  if (bind (fd, address, address_len) < 0)
    {
      fprintf (stderr, "protobuf_c_rpc_server_new: error binding to port: %s\n",
               strerror (errno));
      return NULL;
    }
  if (listen (fd, 255) < 0)
    {
      fprintf (stderr, "protobuf_c_rpc_server_new: listen() failed: %s\n",
               strerror (errno));
      return NULL;
    }
  return server_new_from_fd (fd, type, name, service, dispatch);
}

ProtobufCService *
protobuf_c_rpc_server_destroy (ProtobufC_RPC_Server *server,
                               protobuf_c_boolean    destroy_underlying)
{
  ProtobufCService *rv = destroy_underlying ? NULL : server->underlying;
  while (server->first_connection != NULL)
    server_connection_close (server->first_connection);

  if (server->address_type == PROTOBUF_C_RPC_ADDRESS_LOCAL)
    unlink (server->bind_name);
  server->allocator->free (server->allocator, server->bind_name);

  while (server->recycled_requests != NULL)
    {
      ServerRequest *req = server->recycled_requests;
      server->recycled_requests = req->info.recycled.next;
      server->allocator->free (server->allocator, req);
    }

  protobuf_c_dispatch_close_fd (server->dispatch, server->listening_fd);

  if (destroy_underlying)
    protobuf_c_service_destroy (server->underlying);

  server->allocator->free (server->allocator, server);

  return rv;
}

/* Number of proxied requests to try to grab in a single read */
#define PROXY_BUF_SIZE   256
static void
handle_proxy_pipe_readable (ProtobufC_FD   fd,
                            unsigned       events,
                            void          *callback_data)
{
  int nread;
  ProtobufC_RPC_Server *server = callback_data;
  ProtobufCAllocator *allocator = server->allocator;
  union {
    char buf[sizeof(void*) * PROXY_BUF_SIZE];
    ProxyResponse *responses[PROXY_BUF_SIZE];
  } u;
  unsigned amt, i;
  memcpy (u.buf, server->proxy_extra_data, server->proxy_extra_data_len);
  nread = read (fd, u.buf + server->proxy_extra_data_len, sizeof (u.buf) - server->proxy_extra_data_len);
  if (nread <= 0)
    return;             /* TODO: handle 0 and non-retryable errors separately */
  amt = server->proxy_extra_data_len + nread;
  for (i = 0; i < amt / sizeof(void*); i++)
    {
      ProxyResponse *pr = u.responses[i];
      ServerRequest *request = pr->request;
      if (request->conn == NULL)
        {
          /* defunct request */
          allocator->free (allocator, request);
        }
      else
        {
          ServerConnection *conn = request->conn;
          protobuf_c_boolean must_set_output_watch = (conn->outgoing->size == 0);
          protobuf_c_data_buffer_append (conn->outgoing, (pr+1), pr->len);
          if (must_set_output_watch)
            protobuf_c_dispatch_watch_fd (conn->server->dispatch,
                                          conn->fd,
                                          PROTOBUF_C_EVENT_READABLE|PROTOBUF_C_EVENT_WRITABLE,
                                          handle_server_connection_events,
                                          conn);
          GSK_LIST_REMOVE (GET_PENDING_REQUEST_LIST (conn), request);
          conn->n_pending_requests--;

          free_server_request (conn->server, request);
        }
      allocator->free (allocator, pr);
    }
  memcpy (server->proxy_extra_data, u.buf + i * sizeof(void*), amt - i * sizeof(void*));
  server->proxy_extra_data_len = amt - i * sizeof(void*);
}

void
protobuf_c_rpc_server_configure_threading (ProtobufC_RPC_Server *server,
                                           ProtobufC_RPC_IsRpcThreadFunc func,
                                           void          *is_rpc_data)
{
  server->is_rpc_thread_func = func;
  server->is_rpc_thread_data = is_rpc_data;
retry_pipe:
  if (pipe (server->proxy_pipe) < 0)
    {
      if (errno == EINTR || errno == EAGAIN)
        goto retry_pipe;
      server_failed_literal (server, PROTOBUF_C_ERROR_CODE_PROXY_PROBLEM,
                             "error creating pipe for thread-proxying");
      return;
    }

  /* make the read side non-blocking, since we will use it from the main-loop;
     leave the write side blocking, since it will be used from foreign threads */
  set_fd_nonblocking (server->proxy_pipe[0]);
  protobuf_c_dispatch_watch_fd (server->dispatch, server->proxy_pipe[0],
                                PROTOBUF_C_EVENT_READABLE,
                                handle_proxy_pipe_readable, server);
}

void
protobuf_c_rpc_server_set_error_handler (ProtobufC_RPC_Server *server,
                                         ProtobufC_RPC_Error_Func func,
                                         void                 *error_func_data)
{
  server->error_handler = func;
  server->error_handler_data = error_func_data;
}

void
protobuf_c_rpc_server_set_rpc_protocol (ProtobufC_RPC_Server *server,
                                        ProtobufC_RPC_Protocol protocol)
{
   server->rpc_protocol = protocol;
}
