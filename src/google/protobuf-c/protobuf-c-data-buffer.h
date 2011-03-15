/*
 * Copyright (c) 2008-2011, Dave Benson.
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that the
 * following conditions are met:
 * 
 * Redistributions of source code must retain the above
 * copyright notice, this list of conditions and the following
 * disclaimer.

 * Redistributions in binary form must reproduce
 * the above copyright notice, this list of conditions and
 * the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * Neither the name
 * of "protobuf-c" nor the names of its contributors
 * may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


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
