#ifndef __PROTOBUF_C_DISPATCH_H_
#define __PROTOBUF_C_DISPATCH_H_

typedef struct _ProtobufCDispatch ProtobufCDispatch;
typedef struct _ProtobufCDispatchTimer ProtobufCDispatchTimer;
typedef struct _ProtobufCDispatchIdle ProtobufCDispatchIdle;

#include "protobuf-c.h"

typedef enum
{
  PROTOBUF_C_EVENT_READABLE = (1<<0),
  PROTOBUF_C_EVENT_WRITABLE = (1<<1)
} ProtobufC_Events;

#ifdef WIN32
typedef SOCKET ProtobufC_FD;
#else
typedef int ProtobufC_FD;
#endif

/* Create or destroy a Dispatch */
ProtobufCDispatch  *protobuf_c_dispatch_new (ProtobufCAllocator *allocator);
void                protobuf_c_dispatch_free(ProtobufCDispatch *dispatch);

ProtobufCDispatch  *protobuf_c_dispatch_default (void);

ProtobufCAllocator *protobuf_c_dispatch_peek_allocator (ProtobufCDispatch *);

typedef void (*ProtobufCDispatchCallback)  (ProtobufC_FD   fd,
                                            unsigned       events,
                                            void          *callback_data);

/* Registering file-descriptors to watch. */
void  protobuf_c_dispatch_watch_fd (ProtobufCDispatch *dispatch,
                                    ProtobufC_FD        fd,
                                    unsigned            events,
                                    ProtobufCDispatchCallback callback,
                                    void               *callback_data);
void  protobuf_c_dispatch_close_fd (ProtobufCDispatch *dispatch,
                                    ProtobufC_FD        fd);
void  protobuf_c_dispatch_fd_closed(ProtobufCDispatch *dispatch,
                                    ProtobufC_FD        fd);

/* Timers */
typedef void (*ProtobufCDispatchTimerFunc) (ProtobufCDispatch *dispatch,
                                            void              *func_data);
ProtobufCDispatchTimer *
      protobuf_c_dispatch_add_timer(ProtobufCDispatch *dispatch,
                                    unsigned           timeout_secs,
                                    unsigned           timeout_usecs,
                                    ProtobufCDispatchTimerFunc func,
                                    void               *func_data);
ProtobufCDispatchTimer *
      protobuf_c_dispatch_add_timer_millis
                                   (ProtobufCDispatch *dispatch,
                                    unsigned           milliseconds,
                                    ProtobufCDispatchTimerFunc func,
                                    void               *func_data);
void  protobuf_c_dispatch_remove_timer (ProtobufCDispatchTimer *);

/* Idle functions */
typedef void (*ProtobufCDispatchIdleFunc)   (ProtobufCDispatch *dispatch,
                                             void               *func_data);
ProtobufCDispatchIdle *
      protobuf_c_dispatch_add_idle (ProtobufCDispatch *dispatch,
                                    ProtobufCDispatchIdleFunc func,
                                    void               *func_data);
void  protobuf_c_dispatch_remove_idle (ProtobufCDispatchIdle *);

/* --- API for use in standalone application --- */
/* Where you are happy just to run poll(2). */

/* protobuf_c_dispatch_run() 
 * Run one main-loop iteration, using poll(2) (or some system-level event system);
 * 'timeout' is in milliseconds, -1 for no timeout.
 */
void  protobuf_c_dispatch_run      (ProtobufCDispatch *dispatch);


/* --- API for those who want to embed a dispatch into their own main-loop --- */
typedef struct {
  ProtobufC_FD fd;
  ProtobufC_Events events;
} ProtobufC_FDNotify;

typedef struct {
  ProtobufC_FD fd;
  ProtobufC_Events old_events;
  ProtobufC_Events events;
} ProtobufC_FDNotifyChange;

void  protobuf_c_dispatch_dispatch (ProtobufCDispatch *dispatch,
                                    size_t              n_notifies,
                                    ProtobufC_FDNotify *notifies);
void  protobuf_c_dispatch_clear_changes (ProtobufCDispatch *);


struct _ProtobufCDispatch
{
  /* changes to the events you are interested in. */
  /* (this handles closed file-descriptors 
     in a manner agreeable to epoll(2) and kqueue(2)) */
  size_t n_changes;
  ProtobufC_FDNotifyChange *changes;

  /* the complete set of events you are interested in. */
  size_t n_notifies_desired;
  ProtobufC_FDNotify *notifies_desired;

  /* number of milliseconds to wait if no events occur */
  protobuf_c_boolean has_timeout;
  unsigned long timeout_secs;
  unsigned timeout_usecs;

  /* true if there is an idle function, in which case polling with
     timeout 0 is appropriate */
  protobuf_c_boolean has_idle;

  unsigned long last_dispatch_secs;
  unsigned last_dispatch_usecs;

  /* private data follows (see RealDispatch structure in .c file) */
};

void protobuf_c_dispatch_destroy_default (void);

#endif
