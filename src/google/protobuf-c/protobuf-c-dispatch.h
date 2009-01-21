typedef enum
{
  PROTOBUF_C_EVENT_READABLE = (1<<0),
  PROTOBUF_C_EVENT_WRITABLE = (1<<1)
} ProtobufC_Events;

/* Create or destroy a Dispatch */
ProtobufC_Dispatch *protobuf_c_dispatch_new (ProtobufCAllocator *allocator);
void                protobuf_c_dispatch_free(ProtobufC_Dispatch *dispatch);

ProtobufCAllocator *protobuf_c_dispatch_peek_allocator (ProtobufC_Dispatch *);

typedef void (*ProtobufC_DispatchCallback) (int         fd,
                                            unsigned    events,
                                            void       *callback_data);

/* Registering file-descriptors to watch. */
void  protobuf_c_dispatch_watch_fd (ProtobufC_Dispatch *dispatch,
                                    int                 fd,
                                    unsigned            events,
                                    ProtobufC_DispatchCallback callback,
                                    void               *callback_data);
void  protobuf_c_dispatch_close_fd (ProtobufC_Dispatch *dispatch,
                                    int                 fd);
void  protobuf_c_dispatch_fd_closed(ProtobufC_Dispatch *dispatch,
                                    int                 fd);



/* --- API for use in standalone application --- */
/* Where you are happy just to run poll(2). */

/* protobuf_c_dispatch_run() 
 * Run one main-loop iteration, using poll(2) (or some system-level event system);
 * 'timeout' is in milliseconds, -1 for no timeout.
 */
void  protobuf_c_dispatch_run      (ProtobufC_Dispatch *dispatch,
                                    int                 timeout);


/* --- API for those who want to embed a dispatch into their own main-loop --- */
void  protobuf_c_dispatch_dispatch (ProtobufC_Dispatch *dispatch,
                                    size_t              n_notifies,
                                    ProtobufC_FDNotify *notifies);
void  protobuf_c_dispatch_clear_changes (ProtobufC_Dispatch *);

#ifdef WIN32
typedef SOCKET ProtobufC_FD;
#else
typedef int ProtobufC_FD;
#endif

typedef struct {
  ProtobufC_FD fd;
  ProtobufC_Events events;
} ProtobufC_FDNotify;


struct _ProtobufC_Dispatch
{
  /* changes to the events you are interested in. */
  size_t n_changes;
  ProtobufC_FDNotify *changes;

  /* the complete set of events you are interested in. */
  size_t n_notifies_desired;
  ProtobufC_FDNotify *notifies_desired;

  /* private data follows */
};

