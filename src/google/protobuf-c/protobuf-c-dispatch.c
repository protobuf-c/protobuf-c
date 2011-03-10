/* NOTE: this may not work very well on windows, where i'm
   not sure that "SOCKETs" are allocated nicely like
   file-descriptors are */
/* TODO:
 *  * epoll() implementation
 *  * kqueue() implementation
 *  * windows port (yeah, right, volunteers are DEFINITELY needed for this one...)
 */
#if HAVE_PROTOBUF_C_CONFIG_H
#include "protobuf-c-config.h"
#endif
#include <assert.h>
#if HAVE_ALLOCA_H
# include <alloca.h>
#elif HAVE_MALLOC_H
# include <malloc.h>
#endif
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#if HAVE_SYS_POLL_H
# include <sys/poll.h>
# define USE_POLL              1
#elif HAVE_SYS_SELECT_H
# include <sys/select.h>
# define USE_POLL              0
#endif

/* windows annoyances:  use select, use a full-fledges map for fds */
#ifdef WIN32
# include <winsock.h>
# define USE_POLL              0
# define HAVE_SMALL_FDS            0
#endif
#include <limits.h>
#include <errno.h>
#include <signal.h>
#include "protobuf-c-dispatch.h"
#include "gskrbtreemacros.h"
#include "gsklistmacros.h"

#define DEBUG_DISPATCH_INTERNALS  0
#define DEBUG_DISPATCH            0

#ifndef HAVE_SMALL_FDS
# define HAVE_SMALL_FDS           1
#endif

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

#if !HAVE_SMALL_FDS
typedef struct _FDMapNode FDMapNode;
struct _FDMapNode
{
  ProtobufC_FD fd;
  FDMapNode *left, *right, *parent;
  protobuf_c_boolean is_red;
  FDMap map;
};
#endif


typedef struct _RealDispatch RealDispatch;
struct _RealDispatch
{
  ProtobufCDispatch base;
  Callback *callbacks;          /* parallels notifies_desired */
  size_t notifies_desired_alloced;
  size_t changes_alloced;
#if HAVE_SMALL_FDS
  FDMap *fd_map;                /* map indexed by fd */
  size_t fd_map_size;           /* number of elements of fd_map */
#else
  FDMapNode *fd_map_tree;       /* map indexed by fd */
#endif


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

#if !HAVE_SMALL_FDS
#define FD_MAP_NODES_COMPARE(a,b, rv) \
  if (a->fd < b->fd) rv = -1; \
  else if (a->fd > b->fd) rv = 1; \
  else rv = 0;
#define GET_FD_MAP_TREE(d) \
  (d)->fd_map_tree, FDMapNode *, \
  TIMER_GET_IS_RED, TIMER_SET_IS_RED, \
  parent, left, right, \
  FD_MAP_NODES_COMPARE
#define COMPARE_FD_TO_FD_MAP_NODE(a,b, rv) \
  if (a < b->fd) rv = -1; \
  else if (a > b->fd) rv = 1; \
  else rv = 0;
#endif

/* declare the idle-handler list */
#define GET_IDLE_LIST(d) \
  ProtobufCDispatchIdle *, d->first_idle, d->last_idle, prev, next

/* Create or destroy a Dispatch */
ProtobufCDispatch *protobuf_c_dispatch_new (ProtobufCAllocator *allocator)
{
  RealDispatch *rv = ALLOC (sizeof (RealDispatch));
  struct timeval tv;
  rv->base.n_changes = 0;
  rv->notifies_desired_alloced = 8;
  rv->base.notifies_desired = ALLOC (sizeof (ProtobufC_FDNotify) * rv->notifies_desired_alloced);
  rv->base.n_notifies_desired = 0;
  rv->callbacks = ALLOC (sizeof (Callback) * rv->notifies_desired_alloced);
  rv->changes_alloced = 8;
  rv->base.changes = ALLOC (sizeof (ProtobufC_FDNotify) * rv->changes_alloced);
#if HAVE_SMALL_FDS
  rv->fd_map_size = 16;
  rv->fd_map = ALLOC (sizeof (FDMap) * rv->fd_map_size);
  memset (rv->fd_map, 255, sizeof (FDMap) * rv->fd_map_size);
#else
  rv->fd_map_tree = NULL;
#endif
  rv->allocator = allocator;
  rv->timer_tree = NULL;
  rv->first_idle = rv->last_idle = NULL;
  rv->recycled_idles = NULL;
  rv->recycled_timeouts = NULL;

  /* need to handle SIGPIPE more gracefully than default */
  signal (SIGPIPE, SIG_IGN);

  gettimeofday (&tv, NULL);
  rv->base.last_dispatch_secs = tv.tv_sec;
  rv->base.last_dispatch_usecs = tv.tv_usec;

  return &rv->base;
}

#if !HAVE_SMALL_FDS
void free_fd_tree_recursive (ProtobufCAllocator *allocator,
                             FDMapNode          *node)
{
  if (node)
    {
      free_fd_tree_recursive (allocator, node->left);
      free_fd_tree_recursive (allocator, node->right);
      FREE (node);
    }
}
#endif

/* XXX: leaking timer_tree seemingly? */
void
protobuf_c_dispatch_free(ProtobufCDispatch *dispatch)
{
  RealDispatch *d = (RealDispatch *) dispatch;
  ProtobufCAllocator *allocator = d->allocator;
  while (d->recycled_timeouts != NULL)
    {
      ProtobufCDispatchTimer *t = d->recycled_timeouts;
      d->recycled_timeouts = t->right;
      FREE (t);
    }
  while (d->recycled_idles != NULL)
    {
      ProtobufCDispatchIdle *i = d->recycled_idles;
      d->recycled_idles = i->next;
      FREE (i);
    }
  FREE (d->base.notifies_desired);
  FREE (d->base.changes);
  FREE (d->callbacks);

#if HAVE_SMALL_FDS
  FREE (d->fd_map);
#else
  free_fd_tree_recursive (allocator, d->fd_map_tree);
#endif
  FREE (d);
}

ProtobufCAllocator *
protobuf_c_dispatch_peek_allocator (ProtobufCDispatch *dispatch)
{
  RealDispatch *d = (RealDispatch *) dispatch;
  return d->allocator;
}

/* TODO: perhaps thread-private dispatches make more sense? */
static ProtobufCDispatch *def = NULL;
ProtobufCDispatch  *protobuf_c_dispatch_default (void)
{
  if (def == NULL)
    def = protobuf_c_dispatch_new (&protobuf_c_default_allocator);
  return def;
}

#if HAVE_SMALL_FDS
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
#endif

static unsigned
allocate_notifies_desired_index (RealDispatch *d)
{
  unsigned rv = d->base.n_notifies_desired++;
  ProtobufCAllocator *allocator = d->allocator;
  if (rv == d->notifies_desired_alloced)
    {
      unsigned new_size = d->notifies_desired_alloced * 2;
      ProtobufC_FDNotify *n = ALLOC (new_size * sizeof (ProtobufC_FDNotify));
      Callback *c = ALLOC (new_size * sizeof (Callback));
      memcpy (n, d->base.notifies_desired, d->notifies_desired_alloced * sizeof (ProtobufC_FDNotify));
      FREE (d->base.notifies_desired);
      memcpy (c, d->callbacks, d->notifies_desired_alloced * sizeof (Callback));
      FREE (d->callbacks);
      d->base.notifies_desired = n;
      d->callbacks = c;
      d->notifies_desired_alloced = new_size;
    }
#if DEBUG_DISPATCH_INTERNALS
  fprintf (stderr, "allocate_notifies_desired_index: returning %u\n", rv);
#endif
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
      ProtobufC_FDNotifyChange *n = ALLOC (new_size * sizeof (ProtobufC_FDNotifyChange));
      memcpy (n, d->base.changes, d->changes_alloced * sizeof (ProtobufC_FDNotifyChange));
      FREE (d->base.changes);
      d->base.changes = n;
      d->changes_alloced = new_size;
    }
  return rv;
}

static inline FDMap *
get_fd_map (RealDispatch *d, ProtobufC_FD fd)
{
#if HAVE_SMALL_FDS
  if ((unsigned)fd >= d->fd_map_size)
    return NULL;
  else
    return d->fd_map + fd;
#else
  FDMapNode *node;
  GSK_RBTREE_LOOKUP_COMPARATOR (GET_FD_MAP_TREE (d), fd, COMPARE_FD_TO_FD_MAP_NODE, node);
  return node ? &node->map : NULL;
#endif
}
static inline FDMap *
force_fd_map (RealDispatch *d, ProtobufC_FD fd)
{
#if HAVE_SMALL_FDS
  ensure_fd_map_big_enough (d, fd);
  return d->fd_map + fd;
#else
  {
    FDMap *fm = get_fd_map (d, fd);
    ProtobufCAllocator *allocator = d->allocator;
    if (fm == NULL)
      {
        FDMapNode *node = ALLOC (sizeof (FDMapNode));
        FDMapNode *conflict;
        node->fd = fd;
        memset (&node->map, 255, sizeof (FDMap));
        GSK_RBTREE_INSERT (GET_FD_MAP_TREE (d), node, conflict);
        assert (conflict == NULL);
        fm = &node->map;
      }
    return fm;
  }
#endif
}

static void
deallocate_change_index (RealDispatch *d,
                         FDMap        *fm)
{
  unsigned ch_ind = fm->change_index;
  unsigned from = d->base.n_changes - 1;
  ProtobufC_FD from_fd;
  fm->change_index = -1;
  if (ch_ind == from)
    {
      d->base.n_changes--;
      return;
    }
  from_fd = d->base.changes[ch_ind].fd;
  get_fd_map (d, from_fd)->change_index = ch_ind;
  d->base.changes[ch_ind] = d->base.changes[from];
  d->base.n_changes--;
}

static void
deallocate_notify_desired_index (RealDispatch *d,
                                 ProtobufC_FD  fd,
                                 FDMap        *fm)
{
  unsigned nd_ind = fm->notify_desired_index;
  unsigned from = d->base.n_notifies_desired - 1;
  ProtobufC_FD from_fd;
  (void) fd;
#if DEBUG_DISPATCH_INTERNALS
  fprintf (stderr, "deallocate_notify_desired_index: fd=%d, nd_ind=%u\n",fd,nd_ind);
#endif
  fm->notify_desired_index = -1;
  if (nd_ind == from)
    {
      d->base.n_notifies_desired--;
      return;
    }
  from_fd = d->base.notifies_desired[from].fd;
  get_fd_map (d, from_fd)->notify_desired_index = nd_ind;
  d->base.notifies_desired[nd_ind] = d->base.notifies_desired[from];
  d->callbacks[nd_ind] = d->callbacks[from];
  d->base.n_notifies_desired--;
}

/* Registering file-descriptors to watch. */
void
protobuf_c_dispatch_watch_fd (ProtobufCDispatch *dispatch,
                              ProtobufC_FD        fd,
                              unsigned            events,
                              ProtobufCDispatchCallback callback,
                              void               *callback_data)
{
  RealDispatch *d = (RealDispatch *) dispatch;
  unsigned f = fd;              /* avoid tiring compiler warnings: "comparison of signed versus unsigned" */
  unsigned nd_ind, change_ind;
  unsigned old_events;
  FDMap *fm;
#if DEBUG_DISPATCH
  fprintf (stderr, "dispatch: watch_fd: %d, %s%s\n",
           fd,
           (events&PROTOBUF_C_EVENT_READABLE)?"r":"",
           (events&PROTOBUF_C_EVENT_WRITABLE)?"w":"");
#endif
  if (callback == NULL)
    assert (events == 0);
  else
    assert (events != 0);
  fm = force_fd_map (d, f);

  /* XXX: should we set fm->map.closed_since_notify_started=0 ??? */

  if (fm->notify_desired_index == -1)
    {
      if (callback != NULL)
        nd_ind = fm->notify_desired_index = allocate_notifies_desired_index (d);
      old_events = 0;
    }
  else
    {
      old_events = dispatch->notifies_desired[fm->notify_desired_index].events;
      if (callback == NULL)
        deallocate_notify_desired_index (d, fd, fm);
      else
        nd_ind = fm->notify_desired_index;
    }
  if (callback == NULL)
    {
      if (fm->change_index == -1)
        {
          change_ind = fm->change_index = allocate_change_index (d);
          dispatch->changes[change_ind].old_events = old_events;
        }
      else
        change_ind = fm->change_index;
      d->base.changes[change_ind].fd = f;
      d->base.changes[change_ind].events = 0;
      return;
    }
  assert (callback != NULL && events != 0);
  if (fm->change_index == -1)
    {
      change_ind = fm->change_index = allocate_change_index (d);
      dispatch->changes[change_ind].old_events = old_events;
    }
  else
    change_ind = fm->change_index;

  d->base.changes[change_ind].fd = fd;
  d->base.changes[change_ind].events = events;
  d->base.notifies_desired[nd_ind].fd = fd;
  d->base.notifies_desired[nd_ind].events = events;
  d->callbacks[nd_ind].func = callback;
  d->callbacks[nd_ind].data = callback_data;
}

void
protobuf_c_dispatch_close_fd (ProtobufCDispatch *dispatch,
                              ProtobufC_FD        fd)
{
  protobuf_c_dispatch_fd_closed (dispatch, fd);
  close (fd);
}

void
protobuf_c_dispatch_fd_closed(ProtobufCDispatch *dispatch,
                              ProtobufC_FD        fd)
{
  RealDispatch *d = (RealDispatch *) dispatch;
  FDMap *fm;
#if DEBUG_DISPATCH
  fprintf (stderr, "dispatch: fd %d closed\n", fd);
#endif
  fm = force_fd_map (d, fd);
  fm->closed_since_notify_started = 1;
  if (fm->change_index != -1)
    deallocate_change_index (d, fm);
  if (fm->notify_desired_index != -1)
    deallocate_notify_desired_index (d, fd, fm);
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

  /* clear changes */
  for (i = 0; i < dispatch->n_changes; i++)
    d->fd_map[dispatch->changes[i].fd].change_index = -1;
  dispatch->n_changes = 0;

  /* handle idle functions */
  while (d->first_idle != NULL)
    {
      ProtobufCDispatchIdle *idle = d->first_idle;
      ProtobufCDispatchIdleFunc func = idle->func;
      void *data = idle->func_data;
      GSK_LIST_REMOVE_FIRST (GET_IDLE_LIST (d));

      idle->func = NULL;                /* set to NULL to render remove_idle a no-op */
      func (dispatch, data);

      idle->next = d->recycled_idles;
      d->recycled_idles = idle;
    }
  dispatch->has_idle = 0;

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
      FDMap *fm = get_fd_map (d, dispatch->changes[i].fd);
      assert (fm->change_index == (int) i);
      fm->change_index = -1;
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
  if (dispatch->has_idle)
    timeout = 0;
  else if (!dispatch->has_timeout)
    timeout = -1;
  else
    {
      struct timeval tv;
      gettimeofday (&tv, NULL);
      if (dispatch->timeout_secs < (unsigned long) tv.tv_sec
       || (dispatch->timeout_secs == (unsigned long) tv.tv_sec
        && dispatch->timeout_usecs <= (unsigned) tv.tv_usec))
        timeout = 0;
      else
        {
          int du = dispatch->timeout_usecs - tv.tv_usec;
          int ds = dispatch->timeout_secs - tv.tv_sec;
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
  ProtobufCDispatchTimer *at;
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
  
  /* is this the first element in the tree */
  for (at = rv; at != NULL; at = at->parent)
    if (at->parent && at->parent->right == at)
      break;
  if (at == NULL)               /* yes, so set the public members */
    {
      dispatch->has_timeout = 1;
      dispatch->timeout_secs = rv->timeout_secs;
      dispatch->timeout_usecs = rv->timeout_usecs;
    }
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
  dispatch->has_idle = 1;
  return rv;
}

void
protobuf_c_dispatch_remove_idle (ProtobufCDispatchIdle *idle)
{
  if (idle->func != NULL)
    {
      RealDispatch *d = idle->dispatch;
      GSK_LIST_REMOVE (GET_IDLE_LIST (d), idle);
      idle->next = d->recycled_idles;
      d->recycled_idles = idle;
    }
}
void protobuf_c_dispatch_destroy_default (void)
{
  if (def)
    {
      ProtobufCDispatch *to_kill = def;
      def = NULL;
      protobuf_c_dispatch_free (to_kill);
    }
}
