
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
  ProtobufCAllocator *allocator;
};

void     protobuf_c_data_buffer_init                (ProtobufCDataBuffer       *buffer,
                                                     ProtobufCAllocator    *allocator);
void     protobuf_c_data_buffer_clear               (ProtobufCDataBuffer       *buffer);
void     protobuf_c_data_buffer_reset               (ProtobufCDataBuffer       *buffer);

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

int      protobuf_c_data_buffer_index_of(ProtobufCDataBuffer *buffer,
                                         char       char_to_find);
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

/* This deallocates memory used by the buffer-- you are responsible
 * for the allocation and deallocation of the ProtobufCDataBuffer itself. */
void     protobuf_c_data_buffer_destruct            (ProtobufCDataBuffer    *to_destroy);

/* Free all unused buffer fragments. */
void     protobuf_c_data_buffer_cleanup_recycling_bin ();

#endif
