
#ifndef __PROTOBUF_C_DATA_BUFFER_H_
#define __PROTOBUF_C_DATA_BUFFER_H_

#include "protobuf-c.h"
#include <stdarg.h>


typedef struct _ProtobufCDataBuffer ProtobufCDataBuffer;
typedef struct _ProtobufCDataBufferFragment ProtobufCDataBufferFragment;

struct _ProtobufCDataBufferFragment
{
  ProtobufCDataBufferFragment *next;
  unsigned buf_start;	/* offset in buf of valid data */
  unsigned buf_length;	/* length of valid data in buf */
};

struct _ProtobufCDataBuffer
{
  size_t size;

  ProtobufCDataBufferFragment    *first_frag;
  ProtobufCDataBufferFragment    *last_frag;
};

#define PROTOBUF_C_DATA_BUFFER_STATIC_INIT		{ 0, NULL, NULL }


void     protobuf_c_data_buffer_construct           (ProtobufCDataBuffer       *buffer);

size_t   protobuf_c_data_buffer_read                (ProtobufCDataBuffer    *buffer,
                                                     void*      data,
                                                     size_t         max_length);
size_t   protobuf_c_data_buffer_peek                (const ProtobufCDataBuffer* buffer,
                                                     void*      data,
                                                     size_t        max_length);
size_t   protobuf_c_data_buffer_discard             (ProtobufCDataBuffer    *buffer,
                                                     size_t        max_discard);
char    *protobuf_c_data_buffer_read_line           (ProtobufCDataBuffer    *buffer);

char    *protobuf_c_data_buffer_parse_string0       (ProtobufCDataBuffer    *buffer);
                        /* Returns first char of buffer, or -1. */
int      protobuf_c_data_buffer_peek_char           (const ProtobufCDataBuffer *buffer);
int      protobuf_c_data_buffer_read_char           (ProtobufCDataBuffer    *buffer);

/* 
 * Appending to the buffer.
 */
void     protobuf_c_data_buffer_append              (ProtobufCDataBuffer    *buffer, 
                                         const void   *data,
                                         size_t        length);
void     protobuf_c_data_buffer_append_string       (ProtobufCDataBuffer    *buffer, 
                                         const char   *string);
void     protobuf_c_data_buffer_append_char         (ProtobufCDataBuffer    *buffer, 
                                         char          character);
void     protobuf_c_data_buffer_append_repeated_char(ProtobufCDataBuffer    *buffer, 
                                         char          character,
                                         size_t        count);
#define protobuf_c_data_buffer_append_zeros(buffer, count) \
  protobuf_c_data_buffer_append_repeated_char ((buffer), 0, (count))

/* XXX: protobuf_c_data_buffer_append_repeated_data() is UNIMPLEMENTED */
void     protobuf_c_data_buffer_append_repeated_data(ProtobufCDataBuffer    *buffer, 
                                         const void   *data_to_repeat,
                                         size_t        data_length,
                                         size_t        count);


void     protobuf_c_data_buffer_append_string0      (ProtobufCDataBuffer    *buffer,
                                         const char   *string);


void     protobuf_c_data_buffer_printf              (ProtobufCDataBuffer    *buffer,
					 const char   *format,
					 ...) PROTOBUF_C_GNUC_PRINTF(2,3);
void     protobuf_c_data_buffer_vprintf             (ProtobufCDataBuffer    *buffer,
					 const char   *format,
					 va_list       args);

/* Take all the contents from src and append
 * them to dst, leaving src empty.
 */
size_t   protobuf_c_data_buffer_drain               (ProtobufCDataBuffer    *dst,
                                         ProtobufCDataBuffer    *src);

/* Like `drain', but only transfers some of the data. */
size_t   protobuf_c_data_buffer_transfer            (ProtobufCDataBuffer    *dst,
                                          ProtobufCDataBuffer    *src,
					 size_t        max_transfer);

/* file-descriptor mucking */
int      protobuf_c_data_buffer_writev              (ProtobufCDataBuffer       *read_from,
                                         int              fd);
int      protobuf_c_data_buffer_writev_len          (ProtobufCDataBuffer       *read_from,
                                         int              fd,
					 size_t           max_bytes);
int      protobuf_c_data_buffer_read_in_fd          (ProtobufCDataBuffer       *write_to,
                                         int              read_from);

/*
 * Scanning the buffer.
 */
ssize_t  protobuf_c_data_buffer_index_of            (ProtobufCDataBuffer    *buffer,
                                         char          char_to_find);
ssize_t  protobuf_c_data_buffer_str_index_of        (ProtobufCDataBuffer    *buffer,
                                         const char   *str_to_find);
ssize_t  protobuf_c_data_buffer_polystr_index_of    (ProtobufCDataBuffer    *buffer,
                                         char        **strings);

/* This deallocates memory used by the buffer-- you are responsible
 * for the allocation and deallocation of the ProtobufCDataBuffer itself. */
void     protobuf_c_data_buffer_destruct            (ProtobufCDataBuffer    *to_destroy);

/* Free all unused buffer fragments. */
void     protobuf_c_data_buffer_cleanup_recycling_bin ();


/* intended for use on the stack */
typedef struct _ProtobufCDataBufferIterator ProtobufCDataBufferIterator;
struct _ProtobufCDataBufferIterator
{
  ProtobufCDataBufferFragment *fragment;
  size_t in_cur;
  size_t cur_length;
  const uint8_t *cur_data;
  size_t offset;
};

#define protobuf_c_data_buffer_iterator_offset(iterator)	((iterator)->offset)
void     protobuf_c_data_buffer_iterator_construct (ProtobufCDataBufferIterator *iterator,
           			        ProtobufCDataBuffer         *to_iterate);
unsigned protobuf_c_data_buffer_iterator_peek      (ProtobufCDataBufferIterator *iterator,
           			        void              *out,
           			        unsigned           max_length);
unsigned protobuf_c_data_buffer_iterator_read      (ProtobufCDataBufferIterator *iterator,
           			        void              *out,
           			        unsigned           max_length);
unsigned protobuf_c_data_buffer_iterator_skip      (ProtobufCDataBufferIterator *iterator,
           			        unsigned           max_length);
protobuf_c_boolean protobuf_c_data_buffer_iterator_find_char (ProtobufCDataBufferIterator *iterator,
					char               c);



#endif
