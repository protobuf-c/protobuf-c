/* Free blocks to hold around to avoid repeated mallocs... */
#define MAX_RECYCLED		16

/* Size of allocations to make. */
#define BUF_CHUNK_SIZE		8192

/* Max fragments in the iovector to writev. */
#define MAX_FRAGMENTS_TO_WRITE	16

/* This causes fragments not to be transferred from buffer to buffer,
 * and not to be allocated in pools.  The result is that stack-trace
 * based debug-allocators work much better with this on.
 *
 * On the other hand, this can mask over some abuses (eg stack-based
 * foreign buffer fragment bugs) so we disable it by default.
 */ 
#define GSK_DEBUG_BUFFER_ALLOCATIONS	0

#define BUFFER_RECYCLING                0

#if HAVE_PROTOBUF_C_CONFIG_H
#include "protobuf-c-config.h"
#endif
#include <sys/types.h>
#if HAVE_SYS_UIO_H /* writev function isn't available on Windows */
#include <sys/uio.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#elif HAVE_IO_H
#include <io.h>
#define read    _read
#endif
#include <string.h>
#include <errno.h>
#if HAVE_ALLOCA_H
# include <alloca.h>
#elif HAVE_MALLOC_H
# include <malloc.h>
#endif
#include <stdlib.h>
#include "protobuf-c-data-buffer.h"

#undef TRUE
#define TRUE 1
#undef FALSE
#define FALSE 0

#define PROTOBUF_C_FRAGMENT_DATA_SIZE        4096
#define PROTOBUF_C_FRAGMENT_DATA(frag)     ((uint8_t*)(((ProtobufCDataBufferFragment*)(frag))+1))

/* --- ProtobufCDataBufferFragment implementation --- */
static inline int 
protobuf_c_data_buffer_fragment_avail (ProtobufCDataBufferFragment *frag)
{
  return PROTOBUF_C_FRAGMENT_DATA_SIZE - frag->buf_start - frag->buf_length;
}
static inline uint8_t *
protobuf_c_data_buffer_fragment_start (ProtobufCDataBufferFragment *frag)
{
  return PROTOBUF_C_FRAGMENT_DATA(frag) + frag->buf_start;
}
static inline uint8_t *
protobuf_c_data_buffer_fragment_end (ProtobufCDataBufferFragment *frag)
{
  return PROTOBUF_C_FRAGMENT_DATA(frag) + frag->buf_start + frag->buf_length;
}

/* --- ProtobufCDataBufferFragment recycling --- */
#if BUFFER_RECYCLING
static int num_recycled = 0;
static ProtobufCDataBufferFragment* recycling_stack = 0;
#endif

static ProtobufCDataBufferFragment *
new_native_fragment(ProtobufCAllocator *allocator)
{
  ProtobufCDataBufferFragment *frag;
#if !BUFFER_RECYCLING
  frag = (ProtobufCDataBufferFragment *) allocator->alloc (allocator, BUF_CHUNK_SIZE);
#else  /* optimized (?) */
  if (recycling_stack)
    {
      frag = recycling_stack;
      recycling_stack = recycling_stack->next;
      num_recycled--;
    }
  else
    {
      frag = (ProtobufCDataBufferFragment *) g_malloc (BUF_CHUNK_SIZE);
    }
#endif	/* !GSK_DEBUG_BUFFER_ALLOCATIONS */
  frag->buf_start = frag->buf_length = 0;
  frag->next = 0;
  return frag;
}

#if GSK_DEBUG_BUFFER_ALLOCATIONS || !BUFFER_RECYCLING
#define recycle(allocator, frag) allocator->free (allocator, frag)
#else	/* optimized (?) */
static void
recycle(ProtobufCDataBufferFragment* frag,
        ProtobufCAllocator *allocator)
{
  frag->next = recycling_stack;
  recycling_stack = frag;
  num_recycled++;
}
#endif	/* !GSK_DEBUG_BUFFER_ALLOCATIONS */

/* --- Global public methods --- */
/**
 * protobuf_c_data_buffer_cleanup_recycling_bin:
 * 
 * Free unused buffer fragments.  (Normally some are
 * kept around to reduce strain on the global allocator.)
 */
void
protobuf_c_data_buffer_cleanup_recycling_bin ()
{
#if !GSK_DEBUG_BUFFER_ALLOCATIONS && BUFFER_RECYCLING
  G_LOCK (recycling_stack);
  while (recycling_stack != NULL)
    {
      ProtobufCDataBufferFragment *next;
      next = recycling_stack->next;
      g_free (recycling_stack);
      recycling_stack = next;
    }
  num_recycled = 0;
  G_UNLOCK (recycling_stack);
#endif
}
      
/* --- Public methods --- */
/**
 * protobuf_c_data_buffer_init:
 * @buffer: buffer to initialize (as empty).
 *
 * Construct an empty buffer out of raw memory.
 * (This is equivalent to filling the buffer with 0s)
 */
void
protobuf_c_data_buffer_init(ProtobufCDataBuffer *buffer,
                            ProtobufCAllocator *allocator)
{
  buffer->first_frag = buffer->last_frag = NULL;
  buffer->size = 0;
  buffer->allocator = allocator;
}

#if defined(GSK_DEBUG) || GSK_DEBUG_BUFFER_ALLOCATIONS
static inline gboolean
verify_buffer (const ProtobufCDataBuffer *buffer)
{
  const ProtobufCDataBufferFragment *frag;
  size_t total = 0;
  for (frag = buffer->first_frag; frag != NULL; frag = frag->next)
    total += frag->buf_length;
  return total == buffer->size;
}
#define CHECK_INTEGRITY(buffer)	g_assert (verify_buffer (buffer))
#else
#define CHECK_INTEGRITY(buffer)
#endif

/**
 * protobuf_c_data_buffer_append:
 * @buffer: the buffer to add data to.  Data is put at the end of the buffer.
 * @data: binary data to add to the buffer.
 * @length: length of @data to add to the buffer.
 *
 * Append data into the buffer.
 */
void
protobuf_c_data_buffer_append(ProtobufCDataBuffer    *buffer,
                  const void   *data,
		  size_t         length)
{
  CHECK_INTEGRITY (buffer);
  buffer->size += length;
  while (length > 0)
    {
      size_t avail;
      if (!buffer->last_frag)
	{
	  buffer->last_frag = buffer->first_frag = new_native_fragment (buffer->allocator);
	  avail = protobuf_c_data_buffer_fragment_avail (buffer->last_frag);
	}
      else
	{
	  avail = protobuf_c_data_buffer_fragment_avail (buffer->last_frag);
	  if (avail <= 0)
	    {
	      buffer->last_frag->next = new_native_fragment (buffer->allocator);
	      avail = protobuf_c_data_buffer_fragment_avail (buffer->last_frag);
	      buffer->last_frag = buffer->last_frag->next;
	    }
	}
      if (avail > length)
	avail = length;
      memcpy (protobuf_c_data_buffer_fragment_end (buffer->last_frag), data, avail);
      data = (const char *) data + avail;
      length -= avail;
      buffer->last_frag->buf_length += avail;
    }
  CHECK_INTEGRITY (buffer);
}

void
protobuf_c_data_buffer_append_repeated_char (ProtobufCDataBuffer    *buffer, 
                                 char          character,
                                 size_t        count)
{
  CHECK_INTEGRITY (buffer);
  buffer->size += count;
  while (count > 0)
    {
      size_t avail;
      if (!buffer->last_frag)
	{
	  buffer->last_frag = buffer->first_frag = new_native_fragment (buffer->allocator);
	  avail = protobuf_c_data_buffer_fragment_avail (buffer->last_frag);
	}
      else
	{
	  avail = protobuf_c_data_buffer_fragment_avail (buffer->last_frag);
	  if (avail <= 0)
	    {
	      buffer->last_frag->next = new_native_fragment (buffer->allocator);
	      avail = protobuf_c_data_buffer_fragment_avail (buffer->last_frag);
	      buffer->last_frag = buffer->last_frag->next;
	    }
	}
      if (avail > count)
	avail = count;
      memset (protobuf_c_data_buffer_fragment_end (buffer->last_frag), character, avail);
      count -= avail;
      buffer->last_frag->buf_length += avail;
    }
  CHECK_INTEGRITY (buffer);
}

#if 0
void
protobuf_c_data_buffer_append_repeated_data (ProtobufCDataBuffer    *buffer, 
                                 gconstpointer data_to_repeat,
                                 gsize         data_length,
                                 gsize         count)
{
  ...
}
#endif

/**
 * protobuf_c_data_buffer_append_string:
 * @buffer: the buffer to add data to.  Data is put at the end of the buffer.
 * @string: NUL-terminated string to append to the buffer.
 *  The NUL is not appended.
 *
 * Append a string to the buffer.
 */
void
protobuf_c_data_buffer_append_string(ProtobufCDataBuffer  *buffer,
                         const char *string)
{
  assert (string != NULL);
  protobuf_c_data_buffer_append (buffer, string, strlen (string));
}

/**
 * protobuf_c_data_buffer_append_char:
 * @buffer: the buffer to add the byte to.
 * @character: the byte to add to the buffer.
 *
 * Append a byte to a buffer.
 */
void
protobuf_c_data_buffer_append_char(ProtobufCDataBuffer *buffer,
		       char       character)
{
  protobuf_c_data_buffer_append (buffer, &character, 1);
}

/**
 * protobuf_c_data_buffer_append_string0:
 * @buffer: the buffer to add data to.  Data is put at the end of the buffer.
 * @string: NUL-terminated string to append to the buffer;
 *  NUL is appended.
 *
 * Append a NUL-terminated string to the buffer.  The NUL is appended.
 */
void
protobuf_c_data_buffer_append_string0      (ProtobufCDataBuffer    *buffer,
				const char   *string)
{
  protobuf_c_data_buffer_append (buffer, string, strlen (string) + 1);
}

/**
 * protobuf_c_data_buffer_read:
 * @buffer: the buffer to read data from.
 * @data: buffer to fill with up to @max_length bytes of data.
 * @max_length: maximum number of bytes to read.
 *
 * Removes up to @max_length data from the beginning of the buffer,
 * and writes it to @data.  The number of bytes actually read
 * is returned.
 *
 * returns: number of bytes transferred.
 */
size_t
protobuf_c_data_buffer_read(ProtobufCDataBuffer    *buffer,
                void         *data,
		size_t         max_length)
{
  size_t rv = 0;
  size_t orig_max_length = max_length;
  CHECK_INTEGRITY (buffer);
  while (max_length > 0 && buffer->first_frag)
    {
      ProtobufCDataBufferFragment *first = buffer->first_frag;
      if (first->buf_length <= max_length)
	{
	  memcpy (data, protobuf_c_data_buffer_fragment_start (first), first->buf_length);
	  rv += first->buf_length;
	  data = (char *) data + first->buf_length;
	  max_length -= first->buf_length;
	  buffer->first_frag = first->next;
	  if (!buffer->first_frag)
	    buffer->last_frag = NULL;
	  recycle (buffer->allocator, first);
	}
      else
	{
	  memcpy (data, protobuf_c_data_buffer_fragment_start (first), max_length);
	  rv += max_length;
	  first->buf_length -= max_length;
	  first->buf_start += max_length;
	  data = (char *) data + max_length;
	  max_length = 0;
	}
    }
  buffer->size -= rv;
  assert (rv == orig_max_length || buffer->size == 0);
  CHECK_INTEGRITY (buffer);
  return rv;
}

/**
 * protobuf_c_data_buffer_peek:
 * @buffer: the buffer to peek data from the front of.
 *    This buffer is unchanged by the operation.
 * @data: buffer to fill with up to @max_length bytes of data.
 * @max_length: maximum number of bytes to peek.
 *
 * Copies up to @max_length data from the beginning of the buffer,
 * and writes it to @data.  The number of bytes actually copied
 * is returned.
 *
 * This function is just like protobuf_c_data_buffer_read() except that the 
 * data is not removed from the buffer.
 *
 * returns: number of bytes copied into data.
 */
size_t
protobuf_c_data_buffer_peek     (const ProtobufCDataBuffer *buffer,
                     void            *data,
		     size_t            max_length)
{
  int rv = 0;
  ProtobufCDataBufferFragment *frag = (ProtobufCDataBufferFragment *) buffer->first_frag;
  CHECK_INTEGRITY (buffer);
  while (max_length > 0 && frag)
    {
      if (frag->buf_length <= max_length)
	{
	  memcpy (data, protobuf_c_data_buffer_fragment_start (frag), frag->buf_length);
	  rv += frag->buf_length;
	  data = (char *) data + frag->buf_length;
	  max_length -= frag->buf_length;
	  frag = frag->next;
	}
      else
	{
	  memcpy (data, protobuf_c_data_buffer_fragment_start (frag), max_length);
	  rv += max_length;
	  data = (char *) data + max_length;
	  max_length = 0;
	}
    }
  return rv;
}

/**
 * protobuf_c_data_buffer_read_line:
 * @buffer: buffer to read a line from.
 *
 * Parse a newline (\n) terminated line from
 * buffer and return it as a newly allocated string.
 * The newline is changed to a NUL character.
 *
 * If the buffer does not contain a newline, then NULL is returned.
 *
 * returns: a newly allocated NUL-terminated string, or NULL.
 */
char *
protobuf_c_data_buffer_read_line(ProtobufCDataBuffer *buffer)
{
  int len = 0;
  char *rv;
  ProtobufCDataBufferFragment *at;
  int newline_length;
  CHECK_INTEGRITY (buffer);
  for (at = buffer->first_frag; at; at = at->next)
    {
      uint8_t *start = protobuf_c_data_buffer_fragment_start (at);
      uint8_t *got;
      got = memchr (start, '\n', at->buf_length);
      if (got)
	{
	  len += got - start;
	  break;
	}
      len += at->buf_length;
    }
  if (at == NULL)
    return NULL;
  rv = buffer->allocator->alloc (buffer->allocator, len + 1);
  /* If we found a newline, read it out, truncating
   * it with NUL before we return from the function... */
  if (at)
    newline_length = 1;
  else
    newline_length = 0;
  protobuf_c_data_buffer_read (buffer, rv, len + newline_length);
  rv[len] = 0;
  CHECK_INTEGRITY (buffer);
  return rv;
}

/**
 * protobuf_c_data_buffer_parse_string0:
 * @buffer: buffer to read a line from.
 *
 * Parse a NUL-terminated line from
 * buffer and return it as a newly allocated string.
 *
 * If the buffer does not contain a newline, then NULL is returned.
 *
 * returns: a newly allocated NUL-terminated string, or NULL.
 */
char *
protobuf_c_data_buffer_parse_string0(ProtobufCDataBuffer *buffer)
{
  int index0 = protobuf_c_data_buffer_index_of (buffer, '\0');
  char *rv;
  if (index0 < 0)
    return NULL;
  rv = buffer->allocator->alloc (buffer->allocator, index0 + 1);
  protobuf_c_data_buffer_read (buffer, rv, index0 + 1);
  return rv;
}

/**
 * protobuf_c_data_buffer_peek_char:
 * @buffer: buffer to peek a single byte from.
 *
 * Get the first byte in the buffer as a positive or 0 number.
 * If the buffer is empty, -1 is returned.
 * The buffer is unchanged.
 *
 * returns: an unsigned character or -1.
 */
int
protobuf_c_data_buffer_peek_char(const ProtobufCDataBuffer *buffer)
{
  const ProtobufCDataBufferFragment *frag;

  if (buffer->size == 0)
    return -1;

  for (frag = buffer->first_frag; frag; frag = frag->next)
    if (frag->buf_length > 0)
      break;
  return * protobuf_c_data_buffer_fragment_start ((ProtobufCDataBufferFragment*)frag);
}

/**
 * protobuf_c_data_buffer_read_char:
 * @buffer: buffer to read a single byte from.
 *
 * Get the first byte in the buffer as a positive or 0 number,
 * and remove the character from the buffer.
 * If the buffer is empty, -1 is returned.
 *
 * returns: an unsigned character or -1.
 */
int
protobuf_c_data_buffer_read_char (ProtobufCDataBuffer *buffer)
{
  char c;
  if (protobuf_c_data_buffer_read (buffer, &c, 1) == 0)
    return -1;
  return (int) (uint8_t) c;
}

/**
 * protobuf_c_data_buffer_discard:
 * @buffer: the buffer to discard data from.
 * @max_discard: maximum number of bytes to discard.
 *
 * Removes up to @max_discard data from the beginning of the buffer,
 * and returns the number of bytes actually discarded.
 *
 * returns: number of bytes discarded.
 */
size_t
protobuf_c_data_buffer_discard(ProtobufCDataBuffer *buffer,
                   size_t      max_discard)
{
  int rv = 0;
  CHECK_INTEGRITY (buffer);
  while (max_discard > 0 && buffer->first_frag)
    {
      ProtobufCDataBufferFragment *first = buffer->first_frag;
      if (first->buf_length <= max_discard)
	{
	  rv += first->buf_length;
	  max_discard -= first->buf_length;
	  buffer->first_frag = first->next;
	  if (!buffer->first_frag)
	    buffer->last_frag = NULL;
	  recycle (buffer->allocator, first);
	}
      else
	{
	  rv += max_discard;
	  first->buf_length -= max_discard;
	  first->buf_start += max_discard;
	  max_discard = 0;
	}
    }
  buffer->size -= rv;
  CHECK_INTEGRITY (buffer);
  return rv;
}

static inline protobuf_c_boolean
errno_is_ignorable (int e)
{
#ifdef EWOULDBLOCK              /* for windows */
  if (e == EWOULDBLOCK)
    return 1;
#endif
  return e == EINTR || e == EAGAIN;
}

#if HAVE_SYS_UIO_H
/**
 * protobuf_c_data_buffer_writev:
 * @read_from: buffer to take data from.
 * @fd: file-descriptor to write data to.
 *
 * Writes as much data as possible to the
 * given file-descriptor using the writev(2)
 * function to deal with multiple fragments
 * efficiently, where available.
 *
 * returns: the number of bytes transferred,
 * or -1 on a write error (consult errno).
 */
int
protobuf_c_data_buffer_writev (ProtobufCDataBuffer       *read_from,
		   int              fd)
{
  int rv;
  struct iovec *iov;
  int nfrag, i;
  ProtobufCDataBufferFragment *frag_at = read_from->first_frag;
  CHECK_INTEGRITY (read_from);
  for (nfrag = 0; frag_at != NULL
#ifdef MAX_FRAGMENTS_TO_WRITE
       && nfrag < MAX_FRAGMENTS_TO_WRITE
#endif
       ; nfrag++)
    frag_at = frag_at->next;
  iov = (struct iovec *) alloca (sizeof (struct iovec) * nfrag);
  frag_at = read_from->first_frag;
  for (i = 0; i < nfrag; i++)
    {
      iov[i].iov_len = frag_at->buf_length;
      iov[i].iov_base = protobuf_c_data_buffer_fragment_start (frag_at);
      frag_at = frag_at->next;
    }
  rv = writev (fd, iov, nfrag);
  if (rv < 0 && errno_is_ignorable (errno))
    return 0;
  if (rv <= 0)
    return rv;
  protobuf_c_data_buffer_discard (read_from, rv);
  return rv;
}
#endif

#if HAVE_SYS_UIO_H
/**
 * protobuf_c_data_buffer_writev_len:
 * @read_from: buffer to take data from.
 * @fd: file-descriptor to write data to.
 * @max_bytes: maximum number of bytes to write.
 *
 * Writes up to max_bytes bytes to the
 * given file-descriptor using the writev(2)
 * function to deal with multiple fragments
 * efficiently, where available.
 *
 * returns: the number of bytes transferred,
 * or -1 on a write error (consult errno).
 */
#undef MIN
#define MIN(a,b)   ((a) < (b) ? (a) : (b))
int
protobuf_c_data_buffer_writev_len (ProtobufCDataBuffer *read_from,
		       int        fd,
		       size_t      max_bytes)
{
  int rv;
  struct iovec *iov;
  int nfrag, i;
  size_t bytes;
  ProtobufCDataBufferFragment *frag_at = read_from->first_frag;
  CHECK_INTEGRITY (read_from);
  for (nfrag = 0, bytes = 0; frag_at != NULL && bytes < max_bytes
#ifdef MAX_FRAGMENTS_TO_WRITE
       && nfrag < MAX_FRAGMENTS_TO_WRITE
#endif
       ; nfrag++)
    {
      bytes += frag_at->buf_length;
      frag_at = frag_at->next;
    }
  iov = (struct iovec *) alloca (sizeof (struct iovec) * nfrag);
  frag_at = read_from->first_frag;
  for (bytes = max_bytes, i = 0; i < nfrag && bytes > 0; i++)
    {
      size_t frag_bytes = MIN (frag_at->buf_length, bytes);
      iov[i].iov_len = frag_bytes;
      iov[i].iov_base = protobuf_c_data_buffer_fragment_start (frag_at);
      frag_at = frag_at->next;
      bytes -= frag_bytes;
    }
  rv = writev (fd, iov, i);
  if (rv < 0 && errno_is_ignorable (errno))
    return 0;
  if (rv <= 0)
    return rv;
  protobuf_c_data_buffer_discard (read_from, rv);
  return rv;
}
#endif

/**
 * protobuf_c_data_buffer_read_in_fd:
 * @write_to: buffer to append data to.
 * @read_from: file-descriptor to read data from.
 *
 * Append data into the buffer directly from the
 * given file-descriptor.
 *
 * returns: the number of bytes transferred,
 * or -1 on a read error (consult errno).
 */
/* TODO: zero-copy! */
int
protobuf_c_data_buffer_read_in_fd(ProtobufCDataBuffer *write_to,
                      int        read_from)
{
  char buf[8192];
  int rv = read (read_from, buf, sizeof (buf));
  if (rv < 0)
    return rv;
  protobuf_c_data_buffer_append (write_to, buf, rv);
  return rv;
}

/**
 * protobuf_c_data_buffer_destruct:
 * @to_destroy: the buffer to empty.
 *
 * Remove all fragments from a buffer, leaving it empty.
 * The buffer is guaranteed to not to be consuming any resources,
 * but it also is allowed to start using it again.
 */
void
protobuf_c_data_buffer_reset(ProtobufCDataBuffer *to_destroy)
{
  ProtobufCDataBufferFragment *at = to_destroy->first_frag;
  CHECK_INTEGRITY (to_destroy);
  while (at)
    {
      ProtobufCDataBufferFragment *next = at->next;
      recycle (to_destroy->allocator, at);
      at = next;
    }
  to_destroy->first_frag = to_destroy->last_frag = NULL;
  to_destroy->size = 0;
}

void
protobuf_c_data_buffer_clear(ProtobufCDataBuffer *to_destroy)
{
  ProtobufCDataBufferFragment *at = to_destroy->first_frag;
  CHECK_INTEGRITY (to_destroy);
  while (at)
    {
      ProtobufCDataBufferFragment *next = at->next;
      recycle (to_destroy->allocator, at);
      at = next;
    }
}

/**
 * protobuf_c_data_buffer_index_of:
 * @buffer: buffer to scan.
 * @char_to_find: a byte to look for.
 *
 * Scans for the first instance of the given character.
 * returns: its index in the buffer, or -1 if the character
 * is not in the buffer.
 */
int
protobuf_c_data_buffer_index_of(ProtobufCDataBuffer *buffer,
                    char       char_to_find)
{
  ProtobufCDataBufferFragment *at = buffer->first_frag;
  int rv = 0;
  while (at)
    {
      uint8_t *start = protobuf_c_data_buffer_fragment_start (at);
      uint8_t *saught = memchr (start, char_to_find, at->buf_length);
      if (saught)
	return (saught - start) + rv;
      else
	rv += at->buf_length;
      at = at->next;
    }
  return -1;
}

/**
 * protobuf_c_data_buffer_str_index_of:
 * @buffer: buffer to scan.
 * @str_to_find: a string to look for.
 *
 * Scans for the first instance of the given string.
 * returns: its index in the buffer, or -1 if the string
 * is not in the buffer.
 */
int 
protobuf_c_data_buffer_str_index_of (ProtobufCDataBuffer *buffer,
                         const char *str_to_find)
{
  ProtobufCDataBufferFragment *frag = buffer->first_frag;
  size_t rv = 0;
  for (frag = buffer->first_frag; frag; frag = frag->next)
    {
      const uint8_t *frag_at = PROTOBUF_C_FRAGMENT_DATA (frag);
      size_t frag_rem = frag->buf_length;
      while (frag_rem > 0)
        {
          ProtobufCDataBufferFragment *subfrag;
          const uint8_t *subfrag_at;
          size_t subfrag_rem;
          const char *str_at;
          if (*frag_at != str_to_find[0])
            {
              frag_at++;
              frag_rem--;
              rv++;
              continue;
            }
          subfrag = frag;
          subfrag_at = frag_at + 1;
          subfrag_rem = frag_rem - 1;
          str_at = str_to_find + 1;
          if (*str_at == '\0')
            return rv;
          while (subfrag != NULL)
            {
              while (subfrag_rem == 0)
                {
                  subfrag = subfrag->next;
                  if (subfrag == NULL)
                    goto bad_guess;
                  subfrag_at = protobuf_c_data_buffer_fragment_start (subfrag);
                  subfrag_rem = subfrag->buf_length;
                }
              while (*str_at != '\0' && subfrag_rem != 0)
                {
                  if (*str_at++ != *subfrag_at++)
                    goto bad_guess;
                  subfrag_rem--;
                }
              if (*str_at == '\0')
                return rv;
            }
bad_guess:
          frag_at++;
          frag_rem--;
          rv++;
        }
    }
  return -1;
}

/**
 * protobuf_c_data_buffer_drain:
 * @dst: buffer to add to.
 * @src: buffer to remove from.
 *
 * Transfer all data from @src to @dst,
 * leaving @src empty.
 *
 * returns: the number of bytes transferred.
 */
#if GSK_DEBUG_BUFFER_ALLOCATIONS
size_t
protobuf_c_data_buffer_drain (ProtobufCDataBuffer *dst,
		  ProtobufCDataBuffer *src)
{
  size_t rv = src->size;
  ProtobufCDataBufferFragment *frag;
  CHECK_INTEGRITY (dst);
  CHECK_INTEGRITY (src);
  for (frag = src->first_frag; frag; frag = frag->next)
    protobuf_c_data_buffer_append (dst,
                       protobuf_c_data_buffer_fragment_start (frag),
                       frag->buf_length);
  protobuf_c_data_buffer_discard (src, src->size);
  CHECK_INTEGRITY (dst);
  CHECK_INTEGRITY (src);
  return rv;
}
#else	/* optimized */
size_t
protobuf_c_data_buffer_drain (ProtobufCDataBuffer *dst,
		  ProtobufCDataBuffer *src)
{
  size_t rv = src->size;

  CHECK_INTEGRITY (dst);
  CHECK_INTEGRITY (src);
  if (src->first_frag == NULL)
    return rv;

  dst->size += src->size;

  if (dst->last_frag != NULL)
    {
      dst->last_frag->next = src->first_frag;
      dst->last_frag = src->last_frag;
    }
  else
    {
      dst->first_frag = src->first_frag;
      dst->last_frag = src->last_frag;
    }
  src->size = 0;
  src->first_frag = src->last_frag = NULL;
  CHECK_INTEGRITY (dst);
  return rv;
}
#endif

/**
 * protobuf_c_data_buffer_transfer:
 * @dst: place to copy data into.
 * @src: place to read data from.
 * @max_transfer: maximum number of bytes to transfer.
 *
 * Transfer data out of @src and into @dst.
 * Data is removed from @src.  The number of bytes
 * transferred is returned.
 *
 * returns: the number of bytes transferred.
 */
#if GSK_DEBUG_BUFFER_ALLOCATIONS
size_t
protobuf_c_data_buffer_transfer(ProtobufCDataBuffer *dst,
		    ProtobufCDataBuffer *src,
		    size_t max_transfer)
{
  size_t rv = 0;
  ProtobufCDataBufferFragment *frag;
  CHECK_INTEGRITY (dst);
  CHECK_INTEGRITY (src);
  for (frag = src->first_frag; frag && max_transfer > 0; frag = frag->next)
    {
      size_t len = frag->buf_length;
      if (len >= max_transfer)
        {
          protobuf_c_data_buffer_append (dst, protobuf_c_data_buffer_fragment_start (frag), max_transfer);
          rv += max_transfer;
          break;
        }
      else
        {
          protobuf_c_data_buffer_append (dst, protobuf_c_data_buffer_fragment_start (frag), len);
          rv += len;
          max_transfer -= len;
        }
    }
  protobuf_c_data_buffer_discard (src, rv);
  CHECK_INTEGRITY (dst);
  CHECK_INTEGRITY (src);
  return rv;
}
#else	/* optimized */
size_t
protobuf_c_data_buffer_transfer(ProtobufCDataBuffer *dst,
		    ProtobufCDataBuffer *src,
		    size_t max_transfer)
{
  size_t rv = 0;
  CHECK_INTEGRITY (dst);
  CHECK_INTEGRITY (src);
  while (src->first_frag && max_transfer >= src->first_frag->buf_length)
    {
      ProtobufCDataBufferFragment *frag = src->first_frag;
      src->first_frag = frag->next;
      frag->next = NULL;
      if (src->first_frag == NULL)
	src->last_frag = NULL;

      if (dst->last_frag)
	dst->last_frag->next = frag;
      else
	dst->first_frag = frag;
      dst->last_frag = frag;

      rv += frag->buf_length;
      max_transfer -= frag->buf_length;
    }
  dst->size += rv;
  if (src->first_frag && max_transfer)
    {
      ProtobufCDataBufferFragment *frag = src->first_frag;
      protobuf_c_data_buffer_append (dst, protobuf_c_data_buffer_fragment_start (frag), max_transfer);
      frag->buf_start += max_transfer;
      frag->buf_length -= max_transfer;
      rv += max_transfer;
    }
  src->size -= rv;
  CHECK_INTEGRITY (dst);
  CHECK_INTEGRITY (src);
  return rv;
}
#endif	/* !GSK_DEBUG_BUFFER_ALLOCATIONS */

#if 0
/**
 * protobuf_c_data_buffer_printf:
 * @buffer: the buffer to append to.
 * @format: printf-style format string describing what to append to buffer.
 * @Varargs: values referenced by @format string.
 *
 * Append printf-style content to a buffer.
 */
void     protobuf_c_data_buffer_printf              (ProtobufCDataBuffer    *buffer,
					 const char   *format,
					 ...)
{
  va_list args;
  va_start (args, format);
  protobuf_c_data_buffer_vprintf (buffer, format, args);
  va_end (args);
}

/**
 * protobuf_c_data_buffer_vprintf:
 * @buffer: the buffer to append to.
 * @format: printf-style format string describing what to append to buffer.
 * @args: values referenced by @format string.
 *
 * Append printf-style content to a buffer, given a va_list.
 */
void     protobuf_c_data_buffer_vprintf             (ProtobufCDataBuffer    *buffer,
					 const char   *format,
					 va_list       args)
{
  gsize size = g_printf_string_upper_bound (format, args);
  if (size < 1024)
    {
      char buf[1024];
      g_vsnprintf (buf, sizeof (buf), format, args);
      protobuf_c_data_buffer_append_string (buffer, buf);
    }
  else
    {
      char *buf = g_strdup_vprintf (format, args);
      protobuf_c_data_buffer_append_foreign (buffer, buf, strlen (buf), g_free, buf);
    }
}

/* --- protobuf_c_data_buffer_polystr_index_of implementation --- */
/* Test to see if a sequence of buffer fragments
 * starts with a particular NUL-terminated string.
 */
static gboolean
fragment_n_str(ProtobufCDataBufferFragment   *frag,
               size_t                frag_index,
               const char          *string)
{
  size_t len = strlen (string);
  for (;;)
    {
      size_t test_len = frag->buf_length - frag_index;
      if (test_len > len)
        test_len = len;

      if (memcmp (string,
                  protobuf_c_data_buffer_fragment_start (frag) + frag_index,
                  test_len) != 0)
        return FALSE;

      len -= test_len;
      string += test_len;

      if (len <= 0)
        return TRUE;
      frag_index += test_len;
      if (frag_index >= frag->buf_length)
        {
          frag = frag->next;
          if (frag == NULL)
            return FALSE;
        }
    }
}

/**
 * protobuf_c_data_buffer_polystr_index_of:
 * @buffer: buffer to scan.
 * @strings: NULL-terminated set of string.
 *
 * Scans for the first instance of any of the strings
 * in the buffer.
 *
 * returns: the index of that instance, or -1 if not found.
 */
int     
protobuf_c_data_buffer_polystr_index_of    (ProtobufCDataBuffer    *buffer,
                                char        **strings)
{
  uint8_t init_char_map[16];
  int num_strings;
  int num_bits = 0;
  int total_index = 0;
  ProtobufCDataBufferFragment *frag;
  memset (init_char_map, 0, sizeof (init_char_map));
  for (num_strings = 0; strings[num_strings] != NULL; num_strings++)
    {
      uint8_t c = strings[num_strings][0];
      uint8_t mask = (1 << (c % 8));
      uint8_t *rack = init_char_map + (c / 8);
      if ((*rack & mask) == 0)
        {
          *rack |= mask;
          num_bits++;
        }
    }
  if (num_bits == 0)
    return 0;
  for (frag = buffer->first_frag; frag != NULL; frag = frag->next)
    {
      const char *frag_start;
      const char *at;
      int remaining = frag->buf_length;
      frag_start = protobuf_c_data_buffer_fragment_start (frag);
      at = frag_start;
      while (at != NULL)
        {
          const char *start = at;
          if (num_bits == 1)
            {
              at = memchr (start, strings[0][0], remaining);
              if (at == NULL)
                remaining = 0;
              else
                remaining -= (at - start);
            }
          else
            {
              while (remaining > 0)
                {
                  uint8_t i = (uint8_t) (*at);
                  if (init_char_map[i / 8] & (1 << (i % 8)))
                    break;
                  remaining--;
                  at++;
                }
              if (remaining == 0)
                at = NULL;
            }

          if (at == NULL)
            break;

          /* Now test each of the strings manually. */
          {
            char **test;
            for (test = strings; *test != NULL; test++)
              {
                if (fragment_n_str(frag, at - frag_start, *test))
                  return total_index + (at - frag_start);
              }
            at++;
          }
        }
      total_index += frag->buf_length;
    }
  return -1;
}
#endif

