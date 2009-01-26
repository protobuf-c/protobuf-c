
/* We define three data structures:
 * 1.  a Stack.  a singly-ended, singly-linked list.
 * 2.  a Queue.  a doubly-ended, singly-linked list.
 * 3.  a List.  a doubly-ended, doubly-linked list.
 *
 * Stack operations:
 *    PUSH(stack, node)                                        [O(1)]
 *    POP(stack, rv_node)                                      [O(1)]
 *    INSERT_AFTER(stack, above_node, new_node)                [O(1)]
 *    IS_EMPTY(stack)                                          [O(1)]
 *    REVERSE(stack)                                           [O(N)]
 *    SORT(stack, comparator)                                  [O(NlogN)]
 *    GET_BOTTOM(stack, rv_node)                               [O(N)]
 * Queue operations:
 *    ENQUEUE(queue, node)                                     [O(1)]
 *    DEQUEUE(queue, rv_node)                                  [O(1)]
 *    PREPEND(queue, node)                                     [O(1)]
 *    IS_EMPTY(queue)                                          [O(1)]
 *    REVERSE(queue)                                           [O(N)]
 *    SORT(queue, comparator)                                  [O(NlogN)] 
 * List operations:
 *    PREPEND(list, node)                                      [O(1)]
 *    APPEND(list, node)                                       [O(1)]
 *    REMOVE_FIRST(list)                                       [O(1)]
 *    REMOVE_LAST(list)                                        [O(1)]
 *    REMOVE(list, node)                                       [O(1)]
 *    INSERT_AFTER(list, position_node, new_node)              [O(1)]
 *    INSERT_BEFORE(list, position_node, new_node)             [O(1)]
 *    IS_EMPTY(list)                                           [O(1)]
 *    REVERSE(list)                                            [O(N)]
 *    SORT(list)                                               [O(NlogN)]
 *
 * note: the SORT operation is stable, i.e.  if two
 * elements are equal according to the comparator,
 * then their relative order in the list
 * will be preserved.
 *
 * In the above, 'stack', 'queue', and 'list' are
 * a comma-separated list of arguments, actually.
 * In particular:
 *   stack => NodeType*, top, next_member_name
 *   queue => NodeType*, head, tail, next_member_name
 *   list => NodeType*, first, last, prev_member_name, next_member_name
 * We recommend making macros that end in GET_STACK_ARGS, GET_QUEUE_ARGS,
 * and GET_LIST_ARGS that return the relevant N-tuples.
 */

#define GSK_LOG2_MAX_LIST_SIZE          (GLIB_SIZEOF_SIZE_T*8)

/* --- Stacks --- */
#define GSK_STACK_PUSH(stack, node) GSK_STACK_PUSH_(stack, node)
#define GSK_STACK_POP(stack, rv_node) GSK_STACK_POP_(stack, rv_node)
#define GSK_STACK_INSERT_AFTER(stack, above_node, new_node) \
        GSK_STACK_INSERT_AFTER_(stack, above_node, new_node)
#define GSK_STACK_IS_EMPTY(stack) GSK_STACK_IS_EMPTY_(stack)
#define GSK_STACK_REVERSE(stack) GSK_STACK_REVERSE_(stack)
#define GSK_STACK_FOREACH(stack, iter_var, code) GSK_STACK_FOREACH_(stack, iter_var, code)
#define GSK_STACK_SORT(stack, comparator) GSK_STACK_SORT_(stack, comparator)
#define GSK_STACK_GET_BOTTOM(stack, rv_node) GSK_STACK_GET_BOTTOM_(stack, rv_node)

#define G_STMT_START do
#define G_STMT_END   while(0)

#define GSK_STACK_PUSH_(type, top, next, node) 				\
  G_STMT_START{								\
    type _gsk_tmp = (node);                                             \
    _gsk_tmp->next = (top);						\
    (top) = _gsk_tmp;							\
  }G_STMT_END
#define GSK_STACK_POP_(type, top, next, rv_node) 	                \
  G_STMT_START{								\
    rv_node = (top);							\
    (top) = (top)->next;						\
  }G_STMT_END
#define GSK_STACK_INSERT_AFTER_(type, top, next, above_node, new_node)  \
  G_STMT_START{								\
    type _gsk_tmp = (new_node);                                         \
    type _gsk_above = (above_node);                                     \
    _gsk_tmp->next = _gsk_above->next;				        \
    _gsk_above->next = _gsk_tmp;					\
  }G_STMT_END
#define GSK_STACK_IS_EMPTY_(type, top, next)				\
  ((top) == NULL)
#define GSK_STACK_REVERSE_(type, top, next)				\
  G_STMT_START{								\
    type _gsk___prev = NULL;                                            \
    type _gsk___at = (top);						\
    while (_gsk___at != NULL)						\
      {									\
	type _gsk__next = _gsk___at->next;				\
        _gsk___at->next = _gsk___prev;					\
	_gsk___prev = _gsk___at;					\
	_gsk___at = _gsk__next;						\
      }									\
    (top) = _gsk___prev;						\
  }G_STMT_END
#define GSK_STACK_FOREACH_(type, top, next, iter_var, code)              \
  for (iter_var = top; iter_var != NULL; )                              \
    {                                                                   \
      type _gsk__next = iter_var->next;                                 \
      code;                                                             \
      iter_var = _gsk__next;                                            \
    }
/* sort algorithm:
 *   in order to implement SORT in a macro, it cannot use recursion.
 *   but that's ok because you can just manually implement a stack,
 *   which is probably faster anyways.
 */
#define GSK_STACK_SORT_(type, top, next, comparator)			\
  G_STMT_START{								\
    type _gsk_stack[GSK_LOG2_MAX_LIST_SIZE];				\
    guint _gsk_stack_size = 0;						\
    guint _gsk_i;                                                       \
    type _gsk_at;							\
    for (_gsk_at = top; _gsk_at != NULL; )				\
      {									\
	type _gsk_a = _gsk_at;						\
	type _gsk_b;							\
        type _gsk_cur_list;                                             \
        int _gsk_comparator_rv;                                         \
	_gsk_at = _gsk_at->next;					\
	if (_gsk_at)							\
	  {								\
	    _gsk_b = _gsk_at;						\
	    _gsk_at = _gsk_at->next;					\
	    comparator (_gsk_a, _gsk_b, _gsk_comparator_rv);		\
	    if (_gsk_comparator_rv > 0)					\
	      {								\
                /* sort first two elements */                           \
		type _gsk_swap = _gsk_b;				\
		_gsk_b = _gsk_a;					\
		_gsk_a = _gsk_swap;					\
		_gsk_a->next = _gsk_b;					\
		_gsk_b->next = NULL;					\
	      }								\
	    else							\
              {                                                         \
                /* first two elements already sorted */                 \
	        _gsk_b->next = NULL;					\
              }                                                         \
	  }								\
	else								\
	  {								\
            /* only one element remains */                              \
	    _gsk_a->next = NULL;					\
            _gsk_at = NULL;                                             \
	  }								\
	_gsk_cur_list = _gsk_a;						\
									\
	/* merge _gsk_cur_list up the stack */				\
	for (_gsk_i = 0; TRUE; _gsk_i++)				\
	  {								\
	    /* expanding the stack is marked unlikely,         */	\
	    /* since in the case it matters (where the number  */	\
	    /* of elements is big), the stack rarely grows.    */	\
	    if (G_UNLIKELY (_gsk_i == _gsk_stack_size))                 \
	      {                                                         \
		_gsk_stack[_gsk_stack_size++] = _gsk_cur_list;          \
		break;                                                  \
	      }                                                         \
	    else if (_gsk_stack[_gsk_i] == NULL)                        \
	      {                                                         \
		_gsk_stack[_gsk_i] = _gsk_cur_list;                     \
		break;                                                  \
	      }                                                         \
	    else                                                        \
	      {                                                         \
		/* Merge _gsk_stack[_gsk_i] and _gsk_cur_list. */       \
		type _gsk_merge_list = _gsk_stack[_gsk_i];              \
                type _gsk_new_cur_list;                                 \
		_gsk_stack[_gsk_i] = NULL;                              \
                                                                        \
                _GSK_MERGE_NONEMPTY_LISTS (_gsk_merge_list,             \
                                           _gsk_cur_list,               \
                                           _gsk_new_cur_list,           \
                                           type, next, comparator);     \
                _gsk_cur_list = _gsk_new_cur_list;			\
                _gsk_stack[_gsk_i] = NULL;				\
              }                                                         \
	  }								\
      }									\
                                                                        \
    /* combine all the elements on the stack into a final output */     \
    top = NULL;                                                         \
    for (_gsk_i = 0; _gsk_i < _gsk_stack_size; _gsk_i++)		\
      if (_gsk_stack[_gsk_i] != NULL)                                   \
        {                                                               \
          if (top == NULL)                                              \
            top = _gsk_stack[_gsk_i];                                   \
          else                                                          \
            {                                                           \
              type _gsk_new_top;                                        \
              _GSK_MERGE_NONEMPTY_LISTS (_gsk_stack[_gsk_i],            \
                                         top,                           \
                                         _gsk_new_top,                  \
                                         type, next, comparator);       \
              top = _gsk_new_top;                                       \
            }                                                           \
        }                                                               \
  }G_STMT_END

#define GSK_STACK_GET_BOTTOM_(type, top, next, rv_node)                  \
  G_STMT_START{                                                         \
    rv_node = top;                                                      \
    if (rv_node != NULL)                                                \
      while (rv_node->next)                                             \
        rv_node = rv_node->next;                                        \
  }G_STMT_END

/* --- Queues --- */
#define GSK_QUEUE_ENQUEUE(queue, node) GSK_QUEUE_ENQUEUE_(queue, node)
#define GSK_QUEUE_DEQUEUE(queue, rv_node) GSK_QUEUE_DEQUEUE_(queue, rv_node) 
#define GSK_QUEUE_PREPEND(queue, node) GSK_QUEUE_PREPEND_(queue, node) 
#define GSK_QUEUE_IS_EMPTY(queue) GSK_QUEUE_IS_EMPTY_(queue) 
#define GSK_QUEUE_REVERSE(queue) GSK_QUEUE_REVERSE_(queue) 
#define GSK_QUEUE_SORT(queue, comparator) GSK_QUEUE_SORT_(queue, comparator) 

#define GSK_QUEUE_ENQUEUE_(type, head, tail, next, node)                \
  G_STMT_START{                                                         \
    type _gsk_tmp = (node);                                             \
    if (tail)                                                           \
      tail->next = _gsk_tmp;                                            \
    else                                                                \
      head = _gsk_tmp;                                                  \
    tail = _gsk_tmp;                                                    \
    node->next = NULL;                                                  \
  }G_STMT_END
#define GSK_QUEUE_DEQUEUE_(type, head, tail, next, rv_node)             \
  G_STMT_START{                                                         \
    rv_node = head;                                                     \
    if (head)                                                           \
      {                                                                 \
        head = head->next;                                              \
        if (head == NULL)                                               \
          tail = NULL;                                                  \
      }                                                                 \
  }G_STMT_END
#define GSK_QUEUE_PREPEND_(type, head, tail, next, node)                \
  G_STMT_START{                                                         \
    type _gsk_tmp = (node);                                             \
    _gsk_tmp->next = head;                                              \
    head = _gsk_tmp;                                                    \
    if (tail == NULL)                                                   \
      tail = head;                                                      \
  }G_STMT_END

#define GSK_QUEUE_IS_EMPTY_(type, head, tail, next)                     \
  ((head) == NULL)

#define GSK_QUEUE_REVERSE_(type, head, tail, next)                      \
  G_STMT_START{                                                         \
    type _gsk_queue_new_tail = head;                                    \
    GSK_STACK_REVERSE_(type, head, next);                               \
    tail = _gsk_queue_new_tail;                                         \
  }G_STMT_END

#define GSK_QUEUE_SORT_(type, head, tail, next, comparator)             \
  G_STMT_START{                                                         \
    GSK_STACK_SORT_(type, head, next, comparator);                      \
    GSK_STACK_GET_BOTTOM_(type, head, next, tail);                      \
  }G_STMT_END

/* --- List --- */
#define GSK_LIST_PREPEND(list, node) GSK_LIST_PREPEND_(list, node)
#define GSK_LIST_APPEND(list, node) GSK_LIST_APPEND_(list, node)
#define GSK_LIST_REMOVE_FIRST(list) GSK_LIST_REMOVE_FIRST_(list)
#define GSK_LIST_REMOVE_LAST(list) GSK_LIST_REMOVE_LAST_(list)
#define GSK_LIST_REMOVE(list, node) GSK_LIST_REMOVE_(list, node)
#define GSK_LIST_INSERT_AFTER(list, at, node) GSK_LIST_INSERT_AFTER_(list, at, node)
#define GSK_LIST_INSERT_BEFORE(list, at, node) GSK_LIST_INSERT_BEFORE_(list, at, node)
#define GSK_LIST_IS_EMPTY(list) GSK_LIST_IS_EMPTY_(list)
#define GSK_LIST_REVERSE(list) GSK_LIST_REVERSE_(list)
#define GSK_LIST_SORT(list, comparator) GSK_LIST_SORT_(list, comparator)

#define GSK_LIST_PREPEND_(type, first, last, prev, next, node)          \
  G_STMT_START{                                                         \
    type _gsk_tmp = (node);                                             \
    if (first)                                                          \
      first->prev = (_gsk_tmp);                                         \
    else                                                                \
      last = (_gsk_tmp);                                                \
    node->next = first;                                                 \
    node->prev = NULL;                                                  \
    first = node;                                                       \
  }G_STMT_END
#define GSK_LIST_APPEND_(type, first, last, prev, next, node)           \
  GSK_LIST_PREPEND_(type, last, first, next, prev, node)
#define GSK_LIST_REMOVE_FIRST_(type, first, last, prev, next)           \
  G_STMT_START{                                                         \
    first = first->next;                                                \
    if (first == NULL)                                                  \
      last = NULL;                                                      \
    else                                                                \
      first->prev = NULL;                                               \
  }G_STMT_END
#define GSK_LIST_REMOVE_LAST_(type, first, last, prev, next)            \
  GSK_LIST_REMOVE_FIRST_(type, last, first, next, prev)
#define GSK_LIST_REMOVE_(type, first, last, prev, next, node)           \
  G_STMT_START{                                                         \
    type _gsk_tmp = (node);                                             \
    if (_gsk_tmp->prev)                                                 \
      _gsk_tmp->prev->next = _gsk_tmp->next;                            \
    else                                                                \
      first = _gsk_tmp->next;                                           \
    if (_gsk_tmp->next)                                                 \
      _gsk_tmp->next->prev = _gsk_tmp->prev;                            \
    else                                                                \
      last = _gsk_tmp->prev;                                            \
  }G_STMT_END

#define GSK_LIST_INSERT_AFTER_(type, first, last, prev, next, at, node) \
  G_STMT_START{                                                         \
    type _gsk_at = (at);                                                \
    type _gsk_node = (node);                                            \
    _gsk_node->prev = _gsk_at;                                          \
    _gsk_node->next = _gsk_at->next;                                    \
    if (_gsk_node->next)                                                \
      _gsk_node->next->prev = _gsk_node;                                \
    else                                                                \
      last = _gsk_node;                                                 \
    _gsk_at->next = _gsk_node;                                          \
  }G_STMT_END
#define GSK_LIST_INSERT_BEFORE_(type, first, last, prev, next, at, node)\
  GSK_LIST_INSERT_AFTER_(type, last, first, next, prev, at, node)
#define GSK_LIST_IS_EMPTY_(type, first, last, prev, next)               \
  ((first) == NULL)
#define GSK_LIST_REVERSE_(type, first, last, prev, next)                \
  G_STMT_START{                                                         \
    type _gsk_at = first;                                               \
    first = last;                                                       \
    last = _gsk_at;                                                     \
    while (_gsk_at)                                                     \
      {                                                                 \
        type _gsk_old_next = _gsk_at->next;                             \
        _gsk_at->next = _gsk_at->prev;                                  \
        _gsk_at->prev = _gsk_old_next;                                  \
        _gsk_at = _gsk_old_next;                                        \
      }                                                                 \
  }G_STMT_END
#define GSK_LIST_SORT_(type, first, last, prev, next, comparator)       \
  G_STMT_START{                                                         \
    type _gsk_prev = NULL;                                              \
    type _gsk_at;                                                       \
    GSK_STACK_SORT_(type, first, next, comparator);                     \
    for (_gsk_at = first; _gsk_at; _gsk_at = _gsk_at->next)             \
      {                                                                 \
        _gsk_at->prev = _gsk_prev;                                      \
        _gsk_prev = _gsk_at;                                            \
      }                                                                 \
    last = _gsk_prev;                                                   \
  }G_STMT_END

/* --- Internals --- */
#define _GSK_MERGE_NONEMPTY_LISTS(a,b,out,type,next,comparator)         \
  G_STMT_START{                                                         \
    type _gsk_out_at;                                                   \
    int _gsk_comparator_rv;                                             \
    /* merge 'a' and 'b' into 'out' -- in order to make the sort stable,*/  \
    /* always put elements if 'a' first in the event of a tie (i.e. */  \
    /* when comparator_rv==0)                                       */  \
    comparator (a, b, _gsk_comparator_rv);                              \
    if (_gsk_comparator_rv <= 0)                                        \
      {                                                                 \
        out = a;                                                        \
        a = a->next;                                                    \
      }                                                                 \
    else                                                                \
      {                                                                 \
        out = b;                                                        \
        b = b->next;                                                    \
      }                                                                 \
    _gsk_out_at = out;			                                \
    while (a && b)                                                      \
      {                                                                 \
        comparator (a, b, _gsk_comparator_rv);                          \
        if (_gsk_comparator_rv <= 0)				        \
          {							        \
            _gsk_out_at->next = a;		                        \
            _gsk_out_at = a;			                        \
            a = a->next;		                                \
          }                                                             \
        else                                                            \
          {							        \
            _gsk_out_at->next = b;		                        \
            _gsk_out_at = b;			                        \
            b = b->next;	                                        \
          }							        \
      }							                \
    _gsk_out_at->next = (a != NULL) ? a : b;                            \
  }G_STMT_END
