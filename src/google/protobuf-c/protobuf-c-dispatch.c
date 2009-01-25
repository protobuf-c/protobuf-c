#include <assert.h>
#include <alloca.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/poll.h>
#include <limits.h>
#include <errno.h>
#include "protobuf-c-dispatch.h"
#include "gskrbtreemacros.h"
#include "gsklistmacros.h"

#define protobuf_c_assert(condition) assert(condition)

#define ALLOC_WITH_ALLOCATOR(allocator, size) ((allocator)->alloc ((allocator)->allocator_data, (size)))
#define FREE_WITH_ALLOCATOR(allocator, ptr)   ((allocator)->free ((allocator)->allocator_data, (ptr)))

/* macros that assume you have a ProtobufCAllocator* named
   allocator in scope */
#define ALLOC(size)   ALLOC_WITH_ALLOCATOR((allocator), size)
#define FREE(ptr)     FREE_WITH_ALLOCATOR((allocator), ptr)

typedef struct _Callback Callback;
struct _Callback
{
  ProtobufCDispatchCallback func;
  void *data;
};

typedef struct _FDMap FDMap;
struct _FDMap
{
  int notify_desired_index;     /* -1 if not an known fd */
  int change_index;             /* -1 if no prior change */
  int closed_since_notify_started;
};

typedef struct _RealDispatch RealDispatch;
struct _RealDispatch
{
  ProtobufCDispatch base;
  Callback *callbacks;          /* parallels notifies_desired */
  size_t notifies_desired_alloced;
  size_t changes_alloced;
  FDMap *fd_map;                /* map indexed by fd */
  size_t fd_map_size;           /* number of elements of fd_map */

  ProtobufCDispatchTimer *timer_tree;
  ProtobufCAllocator *allocator;
  ProtobufCDispatchTimer *recycled_timeouts;

  ProtobufCDispatchIdle *first_idle, *last_idle;
  ProtobufCDispatchIdle *recycled_idles;
};

struct _ProtobufCDispatchTimer
{
  RealDispatch *dispatch;

  /* the actual timeout time */
  unsigned long timeout_secs;
  unsigned timeout_usecs;

  /* red-black tree stuff */
  ProtobufCDispatchTimer *left, *right, *parent;
  protobuf_c_boolean is_red;

  /* user callback */
  ProtobufCDispatchTimerFunc func;
  void *func_data;
};

struct _ProtobufCDispatchIdle
{
  RealDispatch *dispatch;

  ProtobufCDispatchIdle *prev, *next;

  /* user callback */
  ProtobufCDispatchIdleFunc func;
  void *func_data;
};
/* Define the tree of timers, as per gskrbtreemacros.h */
#define TIMER_GET_IS_RED(n)      ((n)->is_red)
#define TIMER_SET_IS_RED(n,v)    ((n)->is_red = (v))
#define TIMERS_COMPARE(a,b, rv) \
  if (a->timeout_secs < b->timeout_secs) rv = -1; \
  else if (a->timeout_secs > b->timeout_secs) rv = 1; \
  else if (a->timeout_usecs < b->timeout_usecs) rv = -1; \
  else if (a->timeout_usecs > b->timeout_usecs) rv = 1; \
  else if (a < b) rv = -1; \
  else if (a > b) rv = 1; \
  else rv = 0;
#define GET_TIMER_TREE(d) \
  (d)->timer_tree, ProtobufCDispatchTimer *, \
  TIMER_GET_IS_RED, TIMER_SET_IS_RED, \
  parent, left, right, \
  TIMERS_COMPARE

/* declare the idle-handler list */
#define GET_IDLE_LIST(d) \
  ProtobufCDispatchIdle *, d->first_idle, d->last_idle, prev, next

/* Create or destroy a Dispatch */
ProtobufCDispatch *protobuf_c_dispatch_new (ProtobufCAllocator *allocator)
{
  RealDispatch *rv = ALLOC (sizeof (RealDispatch));
  rv->base.n_changes = 0;
  rv->notifies_desired_alloced = 8;
  rv->base.notifies_desired = ALLOC (sizeof (ProtobufC_FDNotify) * rv->notifies_desired_alloced);
  rv->callbacks = ALLOC (sizeof (Callback) * rv->notifies_desired_alloced);
  rv->changes_alloced = 8;
  rv->base.changes = ALLOC (sizeof (ProtobufC_FDNotify) * rv->changes_alloced);
  rv->fd_map_size = 16;
  rv->fd_map = ALLOC (sizeof (FDMap) * rv->fd_map_size);
  memset (rv->fd_map, 255, sizeof (FDMap) * rv->fd_map_size);
  return &rv->base;
}

void
protobuf_c_dispatch_free(ProtobufCDispatch *dispatch)
{
  RealDispatch *d = (RealDispatch *) dispatch;
  ProtobufCAllocator *allocator = d->allocator;
  FREE (d->base.notifies_desired);
  FREE (d->callbacks);
  FREE (d->fd_map);
  FREE (d);
}

ProtobufCAllocator *
protobuf_c_dispatch_peek_allocator (ProtobufCDispatch *dispatch)
{
  RealDispatch *d = (RealDispatch *) dispatch;
  return d->allocator;
}

/* TODO: perhaps thread-private dispatches make more sense? */
ProtobufCDispatch  *protobuf_c_dispatch_default (void)
{
  static ProtobufCDispatch *def = NULL;
  if (def == NULL)
    def = protobuf_c_dispatch_new (NULL);
  return def;
}

static void
enlarge_fd_map (RealDispatch *d,
                unsigned      fd)
{
  size_t new_size = d->fd_map_size * 2;
  FDMap *new_map;
  ProtobufCAllocator *allocator = d->allocator;
  while (fd >= new_size)
    new_size *= 2;
  new_map = ALLOC (sizeof (FDMap) * new_size);
  memcpy (new_map, d->fd_map, d->fd_map_size * sizeof (FDMap));
  memset (new_map + d->fd_map_size,
          255,
          sizeof (FDMap) * (new_size - d->fd_map_size));
  FREE (d->fd_map);
  d->fd_map = new_map;
  d->fd_map_size = new_size;
}

static inline void
ensure_fd_map_big_enough (RealDispatch *d,
                          unsigned      fd)
{
  if (fd >= d->fd_map_size)
    enlarge_fd_map (d, fd);
}

static unsigned
allocate_notifies_desired_index (RealDispatch *d)
{
  unsigned rv = d->base.n_notifies_desired++;
  ProtobufCAllocator *allocator = d->allocator;
  if (rv == d->notifies_desired_alloced)
    {
      unsigned new_size = d->notifies_desired_alloced * 2;
      ProtobufC_FDNotify *n = ALLOC (new_size * sizeof (ProtobufC_FDNotify));
      memcpy (n, d->base.notifies_desired, d->notifies_desired_alloced * sizeof (ProtobufC_FDNotify));
      FREE (d->base.notifies_desired);
      d->base.notifies_desired = n;
      d->notifies_desired_alloced = new_size;
    }
  return rv;
}
static unsigned
allocate_change_index (RealDispatch *d)
{
  unsigned rv = d->base.n_changes++;
  if (rv == d->changes_alloced)
    {
      ProtobufCAllocator *allocator = d->allocator;
      unsigned new_size = d->changes_alloced * 2;
      ProtobufC_FDNotify *n = ALLOC (new_size * sizeof (ProtobufC_FDNotify));
      memcpy (n, d->base.changes, d->changes_alloced * sizeof (ProtobufC_FDNotify));
      FREE (d->base.changes);
      d->base.changes = n;
      d->changes_alloced = new_size;
    }
  return rv;
}
static void
deallocate_change_index (RealDispatch *d,
                         int fd)
{
  unsigned ch_ind = d->fd_map[fd].change_index;
  unsigned from = d->base.n_changes - 1;
  unsigned from_fd;
  if (ch_ind == from)
    {
      d->base.n_changes--;
      return;
    }
  from_fd = d->base.changes[ch_ind].fd;
  d->fd_map[from_fd].change_index = ch_ind;
  d->base.changes[ch_ind] = d->base.changes[from];
  d->base.n_changes--;
}

static void
deallocate_notify_desired_index (RealDispatch *d,
                                 int fd)
{
  unsigned nd_ind = d->fd_map[fd].notify_desired_index;
  unsigned from = d->base.n_notifies_desired - 1;
  unsigned from_fd;
  if (nd_ind == from)
    {
      d->base.n_notifies_desired--;
      return;
    }
  from_fd = d->base.notifies_desired[nd_ind].fd;
  d->fd_map[from_fd].notify_desired_index = nd_ind;
  d->base.notifies_desired[nd_ind] = d->base.notifies_desired[from];
  d->base.n_notifies_desired--;
}


/* Registering file-descriptors to watch. */
void
protobuf_c_dispatch_watch_fd (ProtobufCDispatch *dispatch,
                              int                 fd,
                              unsigned            events,
                              ProtobufCDispatchCallback callback,
                              void               *callback_data)
{
  RealDispatch *d = (RealDispatch *) dispatch;
  unsigned f = fd;              /* avoid tiring compiler warnings: "comparison of signed versus unsigned" */
  unsigned nd_ind, change_ind;
  if (callback == NULL)
    assert (events == 0);
  else
    assert (events != 0);
  ensure_fd_map_big_enough (d, f);
  if (d->fd_map[f].notify_desired_index == -1)
    {
      d->fd_map[f].notify_desired_index = allocate_notifies_desired_index (d);
    }
  else
    {
      if (callback == NULL)
        deallocate_notify_desired_index (d, f);
      nd_ind = d->fd_map[f].notify_desired_index;
    }
  if (callback == NULL)
    {
      if (d->fd_map[f].change_index == -1)
        d->fd_map[f].change_index = allocate_change_index (d);
      change_ind = d->fd_map[f].change_index;
      d->base.changes[change_ind].fd = f;
      d->base.changes[change_ind].events = 0;
      return;
    }
  assert (callback != NULL && events != 0);
  if (d->fd_map[f].change_index == -1)
    d->fd_map[f].change_index = allocate_change_index (d);
  change_ind = d->fd_map[f].change_index;

  d->base.changes[change_ind].fd = fd;
  d->base.changes[change_ind].events = events;
  d->base.notifies_desired[nd_ind].fd = fd;
  d->base.notifies_desired[nd_ind].events = events;
  d->callbacks[nd_ind].func = callback;
  d->callbacks[nd_ind].data = callback_data;
}

void
protobuf_c_dispatch_close_fd (ProtobufCDispatch *dispatch,
                              int                 fd)
{
  protobuf_c_dispatch_fd_closed (dispatch, fd);
  close (fd);
}

void
protobuf_c_dispatch_fd_closed(ProtobufCDispatch *dispatch,
                              int                 fd)
{
  unsigned f = fd;
  RealDispatch *d = (RealDispatch *) dispatch;
  ensure_fd_map_big_enough (d, f);
  d->fd_map[fd].closed_since_notify_started = 1;
  if (d->fd_map[f].change_index != -1)
    deallocate_change_index (d, f);
  if (d->fd_map[f].notify_desired_index != -1)
    deallocate_notify_desired_index (d, f);
}

static void
free_timer (ProtobufCDispatchTimer *timer)
{
  RealDispatch *d = timer->dispatch;
  timer->right = d->recycled_timeouts;
  d->recycled_timeouts = timer;
}

void
protobuf_c_dispatch_dispatch (ProtobufCDispatch *dispatch,
                              size_t              n_notifies,
                              ProtobufC_FDNotify *notifies)
{
  RealDispatch *d = (RealDispatch *) dispatch;
  unsigned fd_max;
  unsigned i;
  FDMap *fd_map = d->fd_map;
  struct timeval tv;
  if (n_notifies == 0)
    return;
  fd_max = 0;
  for (i = 0; i < n_notifies; i++)
    if (fd_max < (unsigned) notifies[i].fd)
      fd_max = notifies[i].fd;
  ensure_fd_map_big_enough (d, fd_max);
  for (i = 0; i < n_notifies; i++)
    fd_map[notifies[i].fd].closed_since_notify_started = 0;
  for (i = 0; i < n_notifies; i++)
    {
      unsigned fd = notifies[i].fd;
      if (!fd_map[fd].closed_since_notify_started
       && fd_map[fd].notify_desired_index != -1)
        {
          unsigned nd_ind = fd_map[fd].notify_desired_index;
          unsigned events = d->base.notifies_desired[nd_ind].events & notifies[i].events;
          if (events != 0)
            d->callbacks[nd_ind].func (fd, events, d->callbacks[nd_ind].data);
        }
    }


  /* handle timers */
  gettimeofday (&tv, NULL);
  dispatch->last_dispatch_secs = tv.tv_sec;
  dispatch->last_dispatch_usecs = tv.tv_usec;
  while (d->timer_tree != NULL)
    {
      ProtobufCDispatchTimer *min_timer;
      GSK_RBTREE_FIRST (GET_TIMER_TREE (d), min_timer);
      if (min_timer->timeout_secs < (unsigned long) tv.tv_sec
       || (min_timer->timeout_secs == (unsigned long) tv.tv_sec
        && min_timer->timeout_usecs <= (unsigned) tv.tv_usec))
        {
          ProtobufCDispatchTimerFunc func = min_timer->func;
          void *func_data = min_timer->func_data;
          GSK_RBTREE_REMOVE (GET_TIMER_TREE (d), min_timer);
          /* Set to NULL as a way to tell protobuf_c_dispatch_remove_timer()
             that we are in the middle of notifying */
          min_timer->func = NULL;
          min_timer->func_data = NULL;
          func (&d->base, func_data);
          free_timer (min_timer);
        }
      else
        {
          d->base.has_timeout = 1;
          d->base.timeout_secs = min_timer->timeout_secs;
          d->base.timeout_usecs = min_timer->timeout_usecs;
          break;
        }
    }
  if (d->timer_tree == NULL)
    d->base.has_timeout = 0;
}

void
protobuf_c_dispatch_clear_changes (ProtobufCDispatch *dispatch)
{
  RealDispatch *d = (RealDispatch *) dispatch;
  unsigned i;
  for (i = 0; i < dispatch->n_changes; i++)
    {
      assert (d->fd_map[dispatch->changes[i].fd].change_index == (int) i);
      d->fd_map[dispatch->changes[i].fd].change_index = -1;
    }
  dispatch->n_changes = 0;
}

static inline unsigned
events_to_pollfd_events (unsigned ev)
{
  return  ((ev & PROTOBUF_C_EVENT_READABLE) ? POLLIN : 0)
       |  ((ev & PROTOBUF_C_EVENT_WRITABLE) ? POLLOUT : 0)
       ;
}
static inline unsigned
pollfd_events_to_events (unsigned ev)
{
  return  ((ev & POLLIN) ? PROTOBUF_C_EVENT_READABLE : 0)
       |  ((ev & POLLOUT) ? PROTOBUF_C_EVENT_WRITABLE : 0)
       ;
}

void
protobuf_c_dispatch_run (ProtobufCDispatch *dispatch)
{
  struct pollfd *fds;
  void *to_free = NULL, *to_free2 = NULL;
  size_t n_events;
  RealDispatch *d = (RealDispatch *) dispatch;
  ProtobufCAllocator *allocator = d->allocator;
  unsigned i;
  int timeout;
  ProtobufC_FDNotify *events;
  if (dispatch->n_notifies_desired < 128)
    fds = alloca (sizeof (struct pollfd) * dispatch->n_notifies_desired);
  else
    to_free = fds = ALLOC (sizeof (struct pollfd) * dispatch->n_notifies_desired);
  for (i = 0; i < dispatch->n_notifies_desired; i++)
    {
      fds[i].fd = dispatch->notifies_desired[i].fd;
      fds[i].events = events_to_pollfd_events (dispatch->notifies_desired[i].events);
      fds[i].revents = 0;
    }

  /* compute timeout */
  if (d->first_idle != NULL)
    timeout = 0;
  else if (d->timer_tree == NULL)
    timeout = -1;
  else
    {
      ProtobufCDispatchTimer *min_timer;
      GSK_RBTREE_FIRST (GET_TIMER_TREE (d), min_timer);
      struct timeval tv;
      gettimeofday (&tv, NULL);
      if (min_timer->timeout_secs < (unsigned long) tv.tv_sec
       || (min_timer->timeout_secs == (unsigned long) tv.tv_sec
        && min_timer->timeout_usecs <= (unsigned) tv.tv_usec))
        timeout = 0;
      else
        {
          int du = min_timer->timeout_usecs - tv.tv_usec;
          int ds = min_timer->timeout_secs - tv.tv_sec;
          if (du < 0)
            {
              du += 1000000;
              ds -= 1;
            }
          if (ds > INT_MAX / 1000)
            timeout = INT_MAX / 1000 * 1000;
          else
            /* Round up, so that we ensure that something can run
               if they just wait the full duration */
            timeout = ds * 1000 + (du + 999) / 1000;
        }
    }

  if (poll (fds, dispatch->n_notifies_desired, timeout) < 0)
    {
      if (errno == EINTR)
        return;   /* probably a signal interrupted the poll-- let the user have control */

      /* i don't really know what would plausibly cause this */
      fprintf (stderr, "error polling: %s\n", strerror (errno));
      return;
    }
  n_events = 0;
  for (i = 0; i < dispatch->n_notifies_desired; i++)
    if (fds[i].revents)
      n_events++;
  if (n_events < 128)
    events = alloca (sizeof (ProtobufC_FDNotify) * n_events);
  else
    to_free2 = events = ALLOC (sizeof (ProtobufC_FDNotify) * n_events);
  n_events = 0;
  for (i = 0; i < dispatch->n_notifies_desired; i++)
    if (fds[i].revents)
      {
        events[n_events].fd = fds[i].fd;
        events[n_events].events = pollfd_events_to_events (fds[i].revents);

        /* note that we may actually wind up with fewer events
           now that we actually call pollfd_events_to_events() */
        if (events[n_events].events != 0)
          n_events++;
      }
  protobuf_c_dispatch_clear_changes (dispatch);
  protobuf_c_dispatch_dispatch (dispatch, n_events, events);
  if (to_free)
    FREE (to_free);
  if (to_free2)
    FREE (to_free2);
}

ProtobufCDispatchTimer *
protobuf_c_dispatch_add_timer(ProtobufCDispatch *dispatch,
                              unsigned            timeout_secs,
                              unsigned            timeout_usecs,
                              ProtobufCDispatchTimerFunc func,
                              void               *func_data)
{
  RealDispatch *d = (RealDispatch *) dispatch;
  ProtobufCDispatchTimer *rv;
  ProtobufCDispatchTimer *conflict;
  protobuf_c_assert (func != NULL);
  if (d->recycled_timeouts != NULL)
    {
      rv = d->recycled_timeouts;
      d->recycled_timeouts = rv->right;
    }
  else
    {
      rv = d->allocator->alloc (d->allocator, sizeof (ProtobufCDispatchTimer));
    }
  rv->timeout_secs = timeout_secs;
  rv->timeout_usecs = timeout_usecs;
  rv->func = func;
  rv->func_data = func_data;
  rv->dispatch = d;
  GSK_RBTREE_INSERT (GET_TIMER_TREE (d), rv, conflict);
  return rv;
}

ProtobufCDispatchTimer *
protobuf_c_dispatch_add_timer_millis
                             (ProtobufCDispatch *dispatch,
                              unsigned            millis,
                              ProtobufCDispatchTimerFunc func,
                              void               *func_data)
{
  unsigned tsec = dispatch->last_dispatch_secs;
  unsigned tusec = dispatch->last_dispatch_usecs;
  tusec += 1000 * (millis % 1000);
  tsec += millis / 1000;
  if (tusec >= 1000*1000)
    {
      tusec -= 1000*1000;
      tsec += 1;
    }
  return protobuf_c_dispatch_add_timer (dispatch, tsec, tusec, func, func_data);
}

void  protobuf_c_dispatch_remove_timer (ProtobufCDispatchTimer *timer)
{
  protobuf_c_boolean may_be_first;
  RealDispatch *d = timer->dispatch;

  /* ignore mid-notify removal */
  if (timer->func == NULL)
    return;

  may_be_first = d->base.timeout_usecs == timer->timeout_usecs
              && d->base.timeout_secs == timer->timeout_secs;

  GSK_RBTREE_REMOVE (GET_TIMER_TREE (d), timer);

  if (may_be_first)
    {
      if (d->timer_tree == NULL)
        d->base.has_timeout = 0;
      else
        {
          ProtobufCDispatchTimer *min;
          GSK_RBTREE_FIRST (GET_TIMER_TREE (d), min);
          d->base.timeout_secs = min->timeout_secs;
          d->base.timeout_usecs = min->timeout_usecs;
        }
    }
}
ProtobufCDispatchIdle *
protobuf_c_dispatch_add_idle (ProtobufCDispatch *dispatch,
                              ProtobufCDispatchIdleFunc func,
                              void               *func_data)
{
  RealDispatch *d = (RealDispatch *) dispatch;
  ProtobufCDispatchIdle *rv;
  if (d->recycled_idles != NULL)
    {
      rv = d->recycled_idles;
      d->recycled_idles = rv->next;
    }
  else
    {
      ProtobufCAllocator *allocator = d->allocator;
      rv = ALLOC (sizeof (ProtobufCDispatchIdle));
    }
  GSK_LIST_APPEND (GET_IDLE_LIST (d), rv);
  rv->func = func;
  rv->func_data = func_data;
  rv->dispatch = d;
  return rv;
}

void
protobuf_c_dispatch_remove_idle (ProtobufCDispatchIdle *idle)
{
  RealDispatch *d = idle->dispatch;
  GSK_LIST_REMOVE (GET_IDLE_LIST (d), idle);
  idle->next = d->recycled_idles;
  d->recycled_idles = idle;
}
