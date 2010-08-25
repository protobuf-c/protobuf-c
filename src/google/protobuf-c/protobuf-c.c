/* --- protobuf-c.c: public protobuf c runtime implementation --- */

/*
 * Copyright 2008, Dave Benson.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with
 * the License. You may obtain a copy of the License
 * at http://www.apache.org/licenses/LICENSE-2.0 Unless
 * required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/* TODO items:

     * 64-BIT OPTIMIZATION: certain implementations use 32-bit math even on 64-bit platforms
        (uint64_size, uint64_pack, parse_uint64)

     * get_packed_size and pack seem to use type-prefixed names,
       whereas parse uses type-suffixed names.  pick one and stick with it.
       Decision:  go with type-suffixed, since the type (or its instance)
       is typically the object of the verb.
       NOTE: perhaps the "parse" methods should be reanemd to "unpack"
       at the same time. (this only affects internal (static) functions)

     * use TRUE and FALSE instead of 1 and 0 as appropriate

     * use size_t consistently
 */

#include <stdio.h>                      /* for occasional printf()s */
#include <stdlib.h>                     /* for abort(), malloc() etc */
#include <string.h>                     /* for strlen(), memcpy(), memmove() */

#ifndef PRINT_UNPACK_ERRORS
#define PRINT_UNPACK_ERRORS              1
#endif

#include "protobuf-c.h"

#define MAX_UINT64_ENCODED_SIZE 10

/* convenience macros */
#define TMPALLOC(allocator, size) ((allocator)->tmp_alloc ((allocator)->allocator_data, (size)))
#define FREE(allocator, ptr)   \
   do { if ((ptr) != NULL) ((allocator)->free ((allocator)->allocator_data, (ptr))); } while(0)
#define UNALIGNED_ALLOC(allocator, size) ALLOC (allocator, size) /* placeholder */
#define STRUCT_MEMBER_P(struct_p, struct_offset)   \
    ((void *) ((uint8_t*) (struct_p) + (struct_offset)))
#define STRUCT_MEMBER(member_type, struct_p, struct_offset)   \
    (*(member_type*) STRUCT_MEMBER_P ((struct_p), (struct_offset)))
#define STRUCT_MEMBER_PTR(member_type, struct_p, struct_offset)   \
    ((member_type*) STRUCT_MEMBER_P ((struct_p), (struct_offset)))
#define TRUE 1
#define FALSE 0

static void
alloc_failed_warning (unsigned size, const char *filename, unsigned line)
{
  fprintf (stderr,
           "WARNING: out-of-memory allocating a block of size %u (%s:%u)\n",
           size, filename, line);
}

/* Try to allocate memory, running some special code if it fails. */
#define DO_ALLOC(dst, allocator, size, fail_code)                           \
{ size_t da__allocation_size = (size);                                      \
  if (da__allocation_size == 0)                                             \
    dst = NULL;                                                             \
  else if ((dst=((allocator)->alloc ((allocator)->allocator_data,           \
                                     da__allocation_size))) == NULL)        \
    {                                                                       \
      alloc_failed_warning (da__allocation_size, __FILE__, __LINE__);       \
      fail_code;                                                            \
    }                                                                       \
}
#define DO_UNALIGNED_ALLOC  DO_ALLOC            /* placeholder */



#define ASSERT_IS_ENUM_DESCRIPTOR(desc) \
  assert((desc)->magic == PROTOBUF_C_ENUM_DESCRIPTOR_MAGIC)
#define ASSERT_IS_MESSAGE_DESCRIPTOR(desc) \
  assert((desc)->magic == PROTOBUF_C_MESSAGE_DESCRIPTOR_MAGIC)
#define ASSERT_IS_MESSAGE(message) \
  ASSERT_IS_MESSAGE_DESCRIPTOR((message)->descriptor)
#define ASSERT_IS_SERVICE_DESCRIPTOR(desc) \
  assert((desc)->magic == PROTOBUF_C_SERVICE_DESCRIPTOR_MAGIC)

/* --- allocator --- */

static void protobuf_c_out_of_memory_default (void)
{
  fprintf (stderr, "Out Of Memory!!!\n");
  abort ();
}
void (*protobuf_c_out_of_memory) (void) = protobuf_c_out_of_memory_default;

static void *system_alloc(void *allocator_data, size_t size)
{
  void *rv;
  (void) allocator_data;
  if (size == 0)
    return NULL;
  rv = malloc (size);
  if (rv == NULL)
    protobuf_c_out_of_memory ();
  return rv;
}

static void system_free (void *allocator_data, void *data)
{
  (void) allocator_data;
  if (data)
    free (data);
}

/* Some users may configure the default allocator;
   providing your own allocator to unpack() is prefered.
   this allocator is still used for packing nested messages. */
ProtobufCAllocator protobuf_c_default_allocator =
{
  system_alloc,
  system_free,
  NULL,
  8192,
  NULL
};

/* Users should NOT modify this structure,
   but it's difficult to prevent.

   please modify protobuf_c_default_allocator instead. */
ProtobufCAllocator protobuf_c_system_allocator =
{
  system_alloc,
  system_free,
  NULL,
  8192,
  NULL
};

/* === buffer-simple === */
void
protobuf_c_buffer_simple_append (ProtobufCBuffer *buffer,
                                 size_t           len,
                                 const uint8_t   *data)
{
  ProtobufCBufferSimple *simp = (ProtobufCBufferSimple *) buffer;
  size_t new_len = simp->len + len;
  if (new_len > simp->alloced)
    {
      size_t new_alloced = simp->alloced * 2;
      uint8_t *new_data;
      while (new_alloced < new_len)
        new_alloced += new_alloced;
      DO_ALLOC (new_data, &protobuf_c_default_allocator, new_alloced, return);
      memcpy (new_data, simp->data, simp->len);
      if (simp->must_free_data)
        FREE (&protobuf_c_default_allocator, simp->data);
      else
        simp->must_free_data = 1;
      simp->data = new_data;
      simp->alloced = new_alloced;
    }
  memcpy (simp->data + simp->len, data, len);
  simp->len = new_len;
}

/* === get_packed_size() === */

/* Return the number of bytes required to store the
   tag for the field (which includes 3 bits for
   the wire-type, and a single bit that denotes the end-of-tag. */
static inline size_t
get_tag_size (unsigned number)
{
  if (number < (1<<4))
    return 1;
  else if (number < (1<<11))
    return 2;
  else if (number < (1<<18))
    return 3;
  else if (number < (1<<25))
    return 4;
  else
    return 5;
}

/* Return the number of bytes required to store
   a variable-length unsigned integer that fits in 32-bit uint
   in base-128 encoding. */
static inline size_t
uint32_size (uint32_t v)
{
  if (v < (1<<7))
    return 1;
  else if (v < (1<<14))
    return 2;
  else if (v < (1<<21))
    return 3;
  else if (v < (1<<28))
    return 4;
  else
    return 5;
}
/* Return the number of bytes required to store
   a variable-length signed integer that fits in 32-bit int
   in base-128 encoding. */
static inline size_t
int32_size (int32_t v)
{
  if (v < 0)
    return 10;
  else if (v < (1<<7))
    return 1;
  else if (v < (1<<14))
    return 2;
  else if (v < (1<<21))
    return 3;
  else if (v < (1<<28))
    return 4;
  else
    return 5;
}
/* return the zigzag-encoded 32-bit unsigned int from a 32-bit signed int */
static inline uint32_t
zigzag32 (int32_t v)
{
  if (v < 0)
    return ((uint32_t)(-v)) * 2 - 1;
  else
    return v * 2;
}
/* Return the number of bytes required to store
   a variable-length signed integer that fits in 32-bit int,
   converted to unsigned via the zig-zag algorithm,
   then packed using base-128 encoding. */
static inline size_t
sint32_size (int32_t v)
{
  return uint32_size(zigzag32(v));
}

/* Return the number of bytes required to store
   a variable-length unsigned integer that fits in 64-bit uint
   in base-128 encoding. */
static inline size_t
uint64_size (uint64_t v)
{
  uint32_t upper_v = (v>>32);
  if (upper_v == 0)
    return uint32_size ((uint32_t)v);
  else if (upper_v < (1<<3))
    return 5;
  else if (upper_v < (1<<10))
    return 6;
  else if (upper_v < (1<<17))
    return 7;
  else if (upper_v < (1<<24))
    return 8;
  else if (upper_v < (1U<<31))
    return 9;
  else
    return 10;
}

/* return the zigzag-encoded 64-bit unsigned int from a 64-bit signed int */
static inline uint64_t
zigzag64 (int64_t v)
{
  if (v < 0)
    return ((uint64_t)(-v)) * 2 - 1;
  else
    return v * 2;
}

/* Return the number of bytes required to store
   a variable-length signed integer that fits in 64-bit int,
   converted to unsigned via the zig-zag algorithm,
   then packed using base-128 encoding. */
static inline size_t
sint64_size (int64_t v)
{
  return uint64_size(zigzag64(v));
}

/* Get serialized size of a single field in the message,
   including the space needed by the identifying tag. */
static size_t
required_field_get_packed_size (const ProtobufCFieldDescriptor *field,
                                const void *member)
{
  size_t rv = get_tag_size (field->id);
  switch (field->type)
    {
    case PROTOBUF_C_TYPE_SINT32:
      return rv + sint32_size (*(const int32_t *) member);
    case PROTOBUF_C_TYPE_INT32:
      return rv + int32_size (*(const uint32_t *) member);
    case PROTOBUF_C_TYPE_UINT32:
      return rv + uint32_size (*(const uint32_t *) member);
    case PROTOBUF_C_TYPE_SINT64:
      return rv + sint64_size (*(const int64_t *) member);
    case PROTOBUF_C_TYPE_INT64:
    case PROTOBUF_C_TYPE_UINT64:
      return rv + uint64_size (*(const uint64_t *) member);
    case PROTOBUF_C_TYPE_SFIXED32:
    case PROTOBUF_C_TYPE_FIXED32:
      return rv + 4;
    case PROTOBUF_C_TYPE_SFIXED64:
    case PROTOBUF_C_TYPE_FIXED64:
      return rv + 8;
    case PROTOBUF_C_TYPE_BOOL:
      return rv + 1;
    case PROTOBUF_C_TYPE_FLOAT:
      return rv + 4;
    case PROTOBUF_C_TYPE_DOUBLE:
      return rv + 8;
    case PROTOBUF_C_TYPE_ENUM:
      // TODO: is this correct for negative-valued enums?
      return rv + uint32_size (*(const uint32_t *) member);
    case PROTOBUF_C_TYPE_STRING:
      {
        const char *str = *(char * const *) member;
        size_t len = str ? strlen (str) : 0;
        return rv + uint32_size (len) + len;
      }
    case PROTOBUF_C_TYPE_BYTES:
      {
        size_t len = ((const ProtobufCBinaryData*) member)->len;
        return rv + uint32_size (len) + len;
      }
    //case PROTOBUF_C_TYPE_GROUP:
    case PROTOBUF_C_TYPE_MESSAGE:
      {
        const ProtobufCMessage *msg = * (ProtobufCMessage * const *) member;
        size_t subrv = msg ? protobuf_c_message_get_packed_size (msg) : 0;
        return rv + uint32_size (subrv) + subrv;
      }
    }
  PROTOBUF_C_ASSERT_NOT_REACHED ();
  return 0;
}

/* Get serialized size of a single optional field in the message,
   including the space needed by the identifying tag.
   Returns 0 if the optional field isn't set. */
static size_t
optional_field_get_packed_size (const ProtobufCFieldDescriptor *field,
                                const protobuf_c_boolean *has,
                                const void *member)
{
  if (field->type == PROTOBUF_C_TYPE_MESSAGE
   || field->type == PROTOBUF_C_TYPE_STRING)
    {
      const void *ptr = * (const void * const *) member;
      if (ptr == NULL
       || ptr == field->default_value)
        return 0;
    }
  else
    {
      if (!*has)
        return 0;
    }
  return required_field_get_packed_size (field, member);
}

/* Get serialized size of a repeated field in the message,
   which may consist of any number of values (including 0).
   Includes the space needed by the identifying tags (as needed). */
static size_t
repeated_field_get_packed_size (const ProtobufCFieldDescriptor *field,
                                size_t count,
                                const void *member)
{
  size_t header_size;
  size_t rv = 0;
  unsigned i;
  void *array = * (void * const *) member;
  if (count == 0)
    return 0;
  header_size = get_tag_size (field->id);
  if (!field->packed)
    header_size *= count;
  switch (field->type)
    {
    case PROTOBUF_C_TYPE_SINT32:
      for (i = 0; i < count; i++)
        rv += sint32_size (((int32_t*)array)[i]);
      break;
    case PROTOBUF_C_TYPE_INT32:
      for (i = 0; i < count; i++)
        rv += int32_size (((uint32_t*)array)[i]);
      break;
    case PROTOBUF_C_TYPE_UINT32:
    case PROTOBUF_C_TYPE_ENUM:
      for (i = 0; i < count; i++)
        rv += uint32_size (((uint32_t*)array)[i]);
      break;
    case PROTOBUF_C_TYPE_SINT64:
      for (i = 0; i < count; i++)
        rv += sint64_size (((int64_t*)array)[i]);
      break;
    case PROTOBUF_C_TYPE_INT64:
    case PROTOBUF_C_TYPE_UINT64:
      for (i = 0; i < count; i++)
        rv += uint64_size (((uint64_t*)array)[i]);
      break;
    case PROTOBUF_C_TYPE_SFIXED32:
    case PROTOBUF_C_TYPE_FIXED32:
    case PROTOBUF_C_TYPE_FLOAT:
      rv += 4 * count;
      break;
    case PROTOBUF_C_TYPE_SFIXED64:
    case PROTOBUF_C_TYPE_FIXED64:
    case PROTOBUF_C_TYPE_DOUBLE:
      rv += 8 * count;
      break;
    case PROTOBUF_C_TYPE_BOOL:
      rv += count;
      break;
    case PROTOBUF_C_TYPE_STRING:
      for (i = 0; i < count; i++)
        {
          size_t len = strlen (((char**) array)[i]);
          rv += uint32_size (len) + len;
        }
      break;
      
    case PROTOBUF_C_TYPE_BYTES:
      for (i = 0; i < count; i++)
        {
          size_t len = ((ProtobufCBinaryData*) array)[i].len;
          rv += uint32_size (len) + len;
        }
      break;
    case PROTOBUF_C_TYPE_MESSAGE:
      for (i = 0; i < count; i++)
        {
          size_t len = protobuf_c_message_get_packed_size (((ProtobufCMessage **) array)[i]);
          rv += uint32_size (len) + len;
        }
      break;
    //case PROTOBUF_C_TYPE_GROUP:          // NOT SUPPORTED
    }
  if (field->packed)
    header_size += uint32_size (rv);
  return header_size + rv;
}

/* Get the packed size of a unknown field (meaning one that
   is passed through mostly uninterpreted... this is done
   for forward compatibilty with the addition of new fields). */
static inline size_t
unknown_field_get_packed_size (const ProtobufCMessageUnknownField *field)
{
  return get_tag_size (field->tag) + field->len;
}

/* Get the number of bytes that the message will occupy once serialized. */
size_t
protobuf_c_message_get_packed_size(const ProtobufCMessage *message)
{
  unsigned i;
  size_t rv = 0;
  ASSERT_IS_MESSAGE (message);
  for (i = 0; i < message->descriptor->n_fields; i++)
    {
      const ProtobufCFieldDescriptor *field = message->descriptor->fields + i;
      const void *member = ((const char *) message) + field->offset;
      const void *qmember = ((const char *) message) + field->quantifier_offset;

      if (field->label == PROTOBUF_C_LABEL_REQUIRED)
        rv += required_field_get_packed_size (field, member);
      else if (field->label == PROTOBUF_C_LABEL_OPTIONAL)
        rv += optional_field_get_packed_size (field, qmember, member);
      else 
        rv += repeated_field_get_packed_size (field, * (const size_t *) qmember, member);
    }
  for (i = 0; i < message->n_unknown_fields; i++)
    rv += unknown_field_get_packed_size (&message->unknown_fields[i]);
  return rv;
}
/* === pack() === */
/* Pack an unsigned 32-bit integer in base-128 encoding, and return the number of bytes needed:
   this will be 5 or less. */
static inline size_t
uint32_pack (uint32_t value, uint8_t *out)
{
  unsigned rv = 0;
  if (value >= 0x80)
    {
      out[rv++] = value | 0x80;
      value >>= 7;
      if (value >= 0x80)
        {
          out[rv++] = value | 0x80;
          value >>= 7;
          if (value >= 0x80)
            {
              out[rv++] = value | 0x80;
              value >>= 7;
              if (value >= 0x80)
                {
                  out[rv++] = value | 0x80;
                  value >>= 7;
                }
            }
        }
    }
  /* assert: value<128 */
  out[rv++] = value;
  return rv;
}

/* Pack a 32-bit signed integer, returning the number of bytes needed.
   Negative numbers are packed as twos-complement 64-bit integers. */
static inline size_t
int32_pack (int32_t value, uint8_t *out)
{
  if (value < 0)
    {
      out[0] = value | 0x80;
      out[1] = (value>>7) | 0x80;
      out[2] = (value>>14) | 0x80;
      out[3] = (value>>21) | 0x80;
      out[4] = (value>>28) | 0x80;
      out[5] = out[6] = out[7] = out[8] = 0xff;
      out[9] = 0x01;
      return 10;
    }
  else
    return uint32_pack (value, out);
}

/* Pack a 32-bit integer in zigwag encoding. */
static inline size_t
sint32_pack (int32_t value, uint8_t *out)
{
  return uint32_pack (zigzag32 (value), out);
}

/* Pack a 64-bit unsigned integer that fits in a 64-bit uint,
   using base-128 encoding. */
static size_t
uint64_pack (uint64_t value, uint8_t *out)
{
  uint32_t hi = value>>32;
  uint32_t lo = value;
  unsigned rv;
  if (hi == 0)
    return uint32_pack ((uint32_t)lo, out);
  out[0] = (lo) | 0x80;
  out[1] = (lo>>7) | 0x80;
  out[2] = (lo>>14) | 0x80;
  out[3] = (lo>>21) | 0x80;
  if (hi < 8)
    {
      out[4] = (hi<<4) | (lo>>28);
      return 5;
    }
  else
    {
      out[4] = ((hi&7)<<4) | (lo>>28) | 0x80;
      hi >>= 3;
    }
  rv = 5;
  while (hi >= 128)
    {
      out[rv++] = hi | 0x80;
      hi >>= 7;
    }
  out[rv++] = hi;
  return rv;
}

/* Pack a 64-bit signed integer in zigzan encoding,
   return the size of the packed output.
   (Max returned value is 10) */
static inline size_t
sint64_pack (int64_t value, uint8_t *out)
{
  return uint64_pack (zigzag64 (value), out);
}

/* Pack a 32-bit value, little-endian.
   Used for fixed32, sfixed32, float) */
static inline size_t
fixed32_pack (uint32_t value, uint8_t *out)
{
#if IS_LITTLE_ENDIAN
  memcpy (out, &value, 4);
#else
  out[0] = value;
  out[1] = value>>8;
  out[2] = value>>16;
  out[3] = value>>24;
#endif
  return 4;
}

/* Pack a 64-bit fixed-length value.
   (Used for fixed64, sfixed64, double) */
/* XXX: the big-endian impl is really only good for 32-bit machines,
   a 64-bit version would be appreciated, plus a way
   to decide to use 64-bit math where convenient. */
static inline size_t
fixed64_pack (uint64_t value, uint8_t *out)
{
#if IS_LITTLE_ENDIAN
  memcpy (out, &value, 8);
#else
  fixed32_pack (value, out);
  fixed32_pack (value>>32, out+4);
#endif
  return 8;
}


/* Pack a boolean as 0 or 1, even though the protobuf_c_boolean
   can really assume any integer value. */
/* XXX: perhaps on some platforms "*out = !!value" would be
   a better impl, b/c that is idiotmatic c++ in some stl impls. */
static inline size_t
boolean_pack (protobuf_c_boolean value, uint8_t *out)
{
  *out = value ? 1 : 0;
  return 1;
}

/* Pack a length-prefixed string.
   The input string is NUL-terminated.

   The NULL pointer is treated as an empty string.
   This isn't really necessary, but it allows people
   to leave required strings blank.
   (See Issue 13 in the bug tracker for a 
   little more explanation).
 */
static inline size_t
string_pack (const char * str, uint8_t *out)
{
  if (str == NULL)
    {
      out[0] = 0;
      return 1;
    }
  else
    {
      size_t len = strlen (str);
      size_t rv = uint32_pack (len, out);
      memcpy (out + rv, str, len);
      return rv + len;
    }
}

static inline size_t
binary_data_pack (const ProtobufCBinaryData *bd, uint8_t *out)
{
  size_t len = bd->len;
  size_t rv = uint32_pack (len, out);
  memcpy (out + rv, bd->data, len);
  return rv + len;
}

static inline size_t
prefixed_message_pack (const ProtobufCMessage *message, uint8_t *out)
{
  if (message == NULL)
    {
      out[0] = 0;
      return 1;
    }
  else
    {
      size_t rv = protobuf_c_message_pack (message, out + 1);
      uint32_t rv_packed_size = uint32_size (rv);
      if (rv_packed_size != 1)
        memmove (out + rv_packed_size, out + 1, rv);
      return uint32_pack (rv, out) + rv;
    }
}

/* wire-type will be added in required_field_pack() */
/* XXX: just call uint64_pack on 64-bit platforms. */
static size_t
tag_pack (uint32_t id, uint8_t *out)
{
  if (id < (1<<(32-3)))
    return uint32_pack (id<<3, out);
  else
    return uint64_pack (((uint64_t)id) << 3, out);
}

static size_t
required_field_pack (const ProtobufCFieldDescriptor *field,
                     const void *member,
                     uint8_t *out)
{
  size_t rv = tag_pack (field->id, out);
  switch (field->type)
    {
    case PROTOBUF_C_TYPE_SINT32:
      out[0] |= PROTOBUF_C_WIRE_TYPE_VARINT;
      return rv + sint32_pack (*(const int32_t *) member, out + rv);
    case PROTOBUF_C_TYPE_INT32:
      out[0] |= PROTOBUF_C_WIRE_TYPE_VARINT;
      return rv + int32_pack (*(const uint32_t *) member, out + rv);
    case PROTOBUF_C_TYPE_UINT32:
    case PROTOBUF_C_TYPE_ENUM:
      out[0] |= PROTOBUF_C_WIRE_TYPE_VARINT;
      return rv + uint32_pack (*(const uint32_t *) member, out + rv);
    case PROTOBUF_C_TYPE_SINT64:
      out[0] |= PROTOBUF_C_WIRE_TYPE_VARINT;
      return rv + sint64_pack (*(const int64_t *) member, out + rv);
    case PROTOBUF_C_TYPE_INT64:
    case PROTOBUF_C_TYPE_UINT64:
      out[0] |= PROTOBUF_C_WIRE_TYPE_VARINT;
      return rv + uint64_pack (*(const uint64_t *) member, out + rv);
    case PROTOBUF_C_TYPE_SFIXED32:
    case PROTOBUF_C_TYPE_FIXED32:
    case PROTOBUF_C_TYPE_FLOAT:
      out[0] |= PROTOBUF_C_WIRE_TYPE_32BIT;
      return rv + fixed32_pack (*(const uint32_t *) member, out + rv);
    case PROTOBUF_C_TYPE_SFIXED64:
    case PROTOBUF_C_TYPE_FIXED64:
    case PROTOBUF_C_TYPE_DOUBLE:
      out[0] |= PROTOBUF_C_WIRE_TYPE_64BIT;
      return rv + fixed64_pack (*(const uint64_t *) member, out + rv);
    case PROTOBUF_C_TYPE_BOOL:
      out[0] |= PROTOBUF_C_WIRE_TYPE_VARINT;
      return rv + boolean_pack (*(const protobuf_c_boolean *) member, out + rv);
    case PROTOBUF_C_TYPE_STRING:
      {
        out[0] |= PROTOBUF_C_WIRE_TYPE_LENGTH_PREFIXED;
        return rv + string_pack (*(char * const *) member, out + rv);
      }
      
    case PROTOBUF_C_TYPE_BYTES:
      {
        const ProtobufCBinaryData * bd = ((const ProtobufCBinaryData*) member);
        out[0] |= PROTOBUF_C_WIRE_TYPE_LENGTH_PREFIXED;
        return rv + binary_data_pack (bd, out + rv);
      }
    //case PROTOBUF_C_TYPE_GROUP:          // NOT SUPPORTED
    case PROTOBUF_C_TYPE_MESSAGE:
      {
        out[0] |= PROTOBUF_C_WIRE_TYPE_LENGTH_PREFIXED;
        return rv + prefixed_message_pack (*(ProtobufCMessage * const *) member,
                                           out + rv);
      }
    }
  PROTOBUF_C_ASSERT_NOT_REACHED ();
  return 0;
}
static size_t
optional_field_pack (const ProtobufCFieldDescriptor *field,
                     const protobuf_c_boolean *has,
                     const void *member,
                     uint8_t *out)
{
  if (field->type == PROTOBUF_C_TYPE_MESSAGE
   || field->type == PROTOBUF_C_TYPE_STRING)
    {
      const void *ptr = * (const void * const *) member;
      if (ptr == NULL
       || ptr == field->default_value)
        return 0;
    }
  else
    {
      if (!*has)
        return 0;
    }
  return required_field_pack (field, member, out);
}

/* TODO: implement as a table lookup */
static inline size_t
sizeof_elt_in_repeated_array (ProtobufCType type)
{
  switch (type)
    {
    case PROTOBUF_C_TYPE_SINT32:
    case PROTOBUF_C_TYPE_INT32:
    case PROTOBUF_C_TYPE_UINT32:
    case PROTOBUF_C_TYPE_SFIXED32:
    case PROTOBUF_C_TYPE_FIXED32:
    case PROTOBUF_C_TYPE_FLOAT:
    case PROTOBUF_C_TYPE_ENUM:
      return 4;
    case PROTOBUF_C_TYPE_SINT64:
    case PROTOBUF_C_TYPE_INT64:
    case PROTOBUF_C_TYPE_UINT64:
    case PROTOBUF_C_TYPE_SFIXED64:
    case PROTOBUF_C_TYPE_FIXED64:
    case PROTOBUF_C_TYPE_DOUBLE:
      return 8;
    case PROTOBUF_C_TYPE_BOOL:
      return sizeof (protobuf_c_boolean);
    case PROTOBUF_C_TYPE_STRING:
    case PROTOBUF_C_TYPE_MESSAGE:
      return sizeof (void *);
    case PROTOBUF_C_TYPE_BYTES:
      return sizeof (ProtobufCBinaryData);
    }
  PROTOBUF_C_ASSERT_NOT_REACHED ();
  return 0;
}

static void
copy_to_little_endian_32 (void *out, const void *in, unsigned N)
{
#if IS_LITTLE_ENDIAN
  memcpy (out, in, N * 4);
#else
  unsigned i;
  const uint32_t *ini = in;
  for (i = 0; i < N; i++)
    fixed32_pack (ini[i], (uint32_t*)out + i);
#endif
}
static void
copy_to_little_endian_64 (void *out, const void *in, unsigned N)
{
#if IS_LITTLE_ENDIAN
  memcpy (out, in, N * 8);
#else
  unsigned i;
  const uint64_t *ini = in;
  for (i = 0; i < N; i++)
    fixed64_pack (ini[i], (uint64_t*)out + i);
#endif
}

static unsigned
get_type_min_size (ProtobufCType type)
{
  if (type == PROTOBUF_C_TYPE_SFIXED32
   || type == PROTOBUF_C_TYPE_FIXED32
   || type == PROTOBUF_C_TYPE_FLOAT)
    return 4;
  if (type == PROTOBUF_C_TYPE_SFIXED64
   || type == PROTOBUF_C_TYPE_FIXED64
   || type == PROTOBUF_C_TYPE_DOUBLE)
    return 8;
  return 1;
}

static size_t
repeated_field_pack (const ProtobufCFieldDescriptor *field,
                     size_t count,
                     const void *member,
                     uint8_t *out)
{
  char *array = * (char * const *) member;
  unsigned i;
  if (field->packed)
    {
      unsigned header_len;
      unsigned len_start;
      unsigned min_length;
      unsigned payload_len;
      unsigned length_size_min;
      unsigned actual_length_size;
      uint8_t *payload_at;
      if (count == 0)
        return 0;
      header_len = tag_pack (field->id, out);
      out[0] |= PROTOBUF_C_WIRE_TYPE_LENGTH_PREFIXED;
      len_start = header_len;
      min_length = get_type_min_size (field->type) * count;
      length_size_min = uint32_size (min_length);
      header_len += length_size_min;
      payload_at = out + header_len;
      switch (field->type)
        {
        case PROTOBUF_C_TYPE_SFIXED32:
        case PROTOBUF_C_TYPE_FIXED32:
        case PROTOBUF_C_TYPE_FLOAT:
          copy_to_little_endian_32 (payload_at, array, count);
          payload_at += count * 4;
          break;

        case PROTOBUF_C_TYPE_SFIXED64:
        case PROTOBUF_C_TYPE_FIXED64:
        case PROTOBUF_C_TYPE_DOUBLE:
          copy_to_little_endian_64 (payload_at, array, count);
          payload_at += count * 8;
          break;

        case PROTOBUF_C_TYPE_INT32:
          {
            const int32_t *arr = (const int32_t *) array;
            for (i = 0; i < count; i++)
              payload_at += int32_pack (arr[i], payload_at);
          }
          break;

        case PROTOBUF_C_TYPE_SINT32:
          {
            const int32_t *arr = (const int32_t *) array;
            for (i = 0; i < count; i++)
              payload_at += sint32_pack (arr[i], payload_at);
          }
          break;

        case PROTOBUF_C_TYPE_SINT64:
          {
            const int64_t *arr = (const int64_t *) array;
            for (i = 0; i < count; i++)
              payload_at += sint64_pack (arr[i], payload_at);
          }
          break;
        case PROTOBUF_C_TYPE_ENUM:
        case PROTOBUF_C_TYPE_UINT32:
          {
            const uint32_t *arr = (const uint32_t *) array;
            for (i = 0; i < count; i++)
              payload_at += uint32_pack (arr[i], payload_at);
          }
          break;
        case PROTOBUF_C_TYPE_INT64:
        case PROTOBUF_C_TYPE_UINT64:
          {
            const uint64_t *arr = (const uint64_t *) array;
            for (i = 0; i < count; i++)
              payload_at += uint64_pack (arr[i], payload_at);
          }
          break;
        case PROTOBUF_C_TYPE_BOOL:
          {
            const protobuf_c_boolean *arr = (const protobuf_c_boolean *) array;
            for (i = 0; i < count; i++)
              payload_at += boolean_pack (arr[i], payload_at);
          }
          break;
          
        default:
          assert (0);
        }
      payload_len = payload_at - (out + header_len);
      actual_length_size = uint32_size (payload_len);
      if (length_size_min != actual_length_size)
        {
          assert (actual_length_size == length_size_min + 1);
          memmove (out + header_len + 1, out + header_len, payload_len);
          header_len++;
        }
      uint32_pack (payload_len, out + len_start);
      return header_len + payload_len;
    }
  else
    {
      /* CONSIDER: optimize this case a bit (by putting the loop inside the switch) */
      size_t rv = 0;
      unsigned siz = sizeof_elt_in_repeated_array (field->type);
      for (i = 0; i < count; i++)
        {
          rv += required_field_pack (field, array, out + rv);
          array += siz;
        }
      return rv;
    }
}
static size_t
unknown_field_pack (const ProtobufCMessageUnknownField *field,
                    uint8_t *out)
{
  size_t rv = tag_pack (field->tag, out);
  out[0] |= field->wire_type;
  memcpy (out + rv, field->data, field->len);
  return rv + field->len;
}

size_t
protobuf_c_message_pack           (const ProtobufCMessage *message,
                                   uint8_t                *out)
{
  unsigned i;
  size_t rv = 0;
  ASSERT_IS_MESSAGE (message);
  for (i = 0; i < message->descriptor->n_fields; i++)
    {
      const ProtobufCFieldDescriptor *field = message->descriptor->fields + i;
      const void *member = ((const char *) message) + field->offset;

      /* it doesn't hurt to compute qmember (a pointer to the quantifier
         field of the structure), but the pointer is only valid if
         the field is one of:
           - a repeated field
           - an optional field that isn't a pointer type
             (meaning: not a message or a string) */
      const void *qmember = ((const char *) message) + field->quantifier_offset;

      if (field->label == PROTOBUF_C_LABEL_REQUIRED)
        rv += required_field_pack (field, member, out + rv);
      else if (field->label == PROTOBUF_C_LABEL_OPTIONAL)
        /* note that qmember is bogus for strings and messages,
           but it isn't used */
        rv += optional_field_pack (field, qmember, member, out + rv);
      else
        rv += repeated_field_pack (field, * (const size_t *) qmember, member, out + rv);
    }
  for (i = 0; i < message->n_unknown_fields; i++)
    rv += unknown_field_pack (&message->unknown_fields[i], out + rv);
  return rv;
}

/* === pack_to_buffer() === */
static size_t
required_field_pack_to_buffer (const ProtobufCFieldDescriptor *field,
                               const void *member,
                               ProtobufCBuffer *buffer)
{
  size_t rv;
  uint8_t scratch[MAX_UINT64_ENCODED_SIZE * 2];
  rv = tag_pack (field->id, scratch);
  switch (field->type)
    {
    case PROTOBUF_C_TYPE_SINT32:
      scratch[0] |= PROTOBUF_C_WIRE_TYPE_VARINT;
      rv += sint32_pack (*(const int32_t *) member, scratch + rv);
      buffer->append (buffer, rv, scratch);
      break;
    case PROTOBUF_C_TYPE_INT32:
      scratch[0] |= PROTOBUF_C_WIRE_TYPE_VARINT;
      rv += int32_pack (*(const uint32_t *) member, scratch + rv);
      buffer->append (buffer, rv, scratch);
      break;
    case PROTOBUF_C_TYPE_UINT32:
    case PROTOBUF_C_TYPE_ENUM:
      scratch[0] |= PROTOBUF_C_WIRE_TYPE_VARINT;
      rv += uint32_pack (*(const uint32_t *) member, scratch + rv);
      buffer->append (buffer, rv, scratch);
      break;
    case PROTOBUF_C_TYPE_SINT64:
      scratch[0] |= PROTOBUF_C_WIRE_TYPE_VARINT;
      rv += sint64_pack (*(const int64_t *) member, scratch + rv);
      buffer->append (buffer, rv, scratch);
      break;
    case PROTOBUF_C_TYPE_INT64:
    case PROTOBUF_C_TYPE_UINT64:
      scratch[0] |= PROTOBUF_C_WIRE_TYPE_VARINT;
      rv += uint64_pack (*(const uint64_t *) member, scratch + rv);
      buffer->append (buffer, rv, scratch);
      break;
    case PROTOBUF_C_TYPE_SFIXED32:
    case PROTOBUF_C_TYPE_FIXED32:
    case PROTOBUF_C_TYPE_FLOAT:
      scratch[0] |= PROTOBUF_C_WIRE_TYPE_32BIT;
      rv += fixed32_pack (*(const uint32_t *) member, scratch + rv);
      buffer->append (buffer, rv, scratch);
      break;
    case PROTOBUF_C_TYPE_SFIXED64:
    case PROTOBUF_C_TYPE_FIXED64:
    case PROTOBUF_C_TYPE_DOUBLE:
      scratch[0] |= PROTOBUF_C_WIRE_TYPE_64BIT;
      rv += fixed64_pack (*(const uint64_t *) member, scratch + rv);
      buffer->append (buffer, rv, scratch);
      break;
    case PROTOBUF_C_TYPE_BOOL:
      scratch[0] |= PROTOBUF_C_WIRE_TYPE_VARINT;
      rv += boolean_pack (*(const protobuf_c_boolean *) member, scratch + rv);
      buffer->append (buffer, rv, scratch);
      break;
    case PROTOBUF_C_TYPE_STRING:
      {
        const char *str = *(char * const *) member;
        size_t sublen = str ? strlen (str) : 0;
        scratch[0] |= PROTOBUF_C_WIRE_TYPE_LENGTH_PREFIXED;
        rv += uint32_pack (sublen, scratch + rv);
        buffer->append (buffer, rv, scratch);
        buffer->append (buffer, sublen, (const uint8_t *) str);
        rv += sublen;
        break;
      }
      
    case PROTOBUF_C_TYPE_BYTES:
      {
        const ProtobufCBinaryData * bd = ((const ProtobufCBinaryData*) member);
        size_t sublen = bd->len;
        scratch[0] |= PROTOBUF_C_WIRE_TYPE_LENGTH_PREFIXED;
        rv += uint32_pack (sublen, scratch + rv);
        buffer->append (buffer, rv, scratch);
        buffer->append (buffer, sublen, bd->data);
        rv += sublen;
        break;
      }
    //PROTOBUF_C_TYPE_GROUP,          // NOT SUPPORTED
    case PROTOBUF_C_TYPE_MESSAGE:
      {
        uint8_t simple_buffer_scratch[256];
        size_t sublen;
        ProtobufCBufferSimple simple_buffer
          = PROTOBUF_C_BUFFER_SIMPLE_INIT (simple_buffer_scratch);
        const ProtobufCMessage *msg = *(ProtobufCMessage * const *) member;
        scratch[0] |= PROTOBUF_C_WIRE_TYPE_LENGTH_PREFIXED;
        if (msg == NULL)
          sublen = 0;
        else
          sublen = protobuf_c_message_pack_to_buffer (msg, &simple_buffer.base);
        rv += uint32_pack (sublen, scratch + rv);
        buffer->append (buffer, rv, scratch);
        buffer->append (buffer, sublen, simple_buffer.data);
        rv += sublen;
        PROTOBUF_C_BUFFER_SIMPLE_CLEAR (&simple_buffer);
        break;
      }
    default:
      PROTOBUF_C_ASSERT_NOT_REACHED ();
    }
  return rv;
}
static size_t
optional_field_pack_to_buffer (const ProtobufCFieldDescriptor *field,
                               const protobuf_c_boolean *has,
                               const void *member,
                               ProtobufCBuffer *buffer)
{
  if (field->type == PROTOBUF_C_TYPE_MESSAGE
   || field->type == PROTOBUF_C_TYPE_STRING)
    {
      const void *ptr = * (const void * const *) member;
      if (ptr == NULL
       || ptr == field->default_value)
        return 0;
    }
  else
    {
      if (!*has)
        return 0;
    }
  return required_field_pack_to_buffer (field, member, buffer);
}

static size_t
get_packed_payload_length (const ProtobufCFieldDescriptor *field,
                           unsigned count,
                           const void *array)
{
  unsigned rv = 0;
  unsigned i;
  switch (field->type)
    {
    case PROTOBUF_C_TYPE_SFIXED32:
    case PROTOBUF_C_TYPE_FIXED32:
    case PROTOBUF_C_TYPE_FLOAT:
      return count * 4;

    case PROTOBUF_C_TYPE_SFIXED64:
    case PROTOBUF_C_TYPE_FIXED64:
    case PROTOBUF_C_TYPE_DOUBLE:
      return count * 8;

    case PROTOBUF_C_TYPE_INT32:
      {
        const int32_t *arr = (const int32_t *) array;
        for (i = 0; i < count; i++)
          rv += int32_size (arr[i]);
      }
      break;

    case PROTOBUF_C_TYPE_SINT32:
      {
        const int32_t *arr = (const int32_t *) array;
        for (i = 0; i < count; i++)
          rv += sint32_size (arr[i]);
      }
      break;
    case PROTOBUF_C_TYPE_ENUM:
    case PROTOBUF_C_TYPE_UINT32:
      {
        const uint32_t *arr = (const uint32_t *) array;
        for (i = 0; i < count; i++)
          rv += uint32_size (arr[i]);
      }
      break;

    case PROTOBUF_C_TYPE_SINT64:
      {
        const int64_t *arr = (const int64_t *) array;
        for (i = 0; i < count; i++)
          rv += sint64_size (arr[i]);
      }
      break;
    case PROTOBUF_C_TYPE_INT64:
    case PROTOBUF_C_TYPE_UINT64:
      {
        const uint64_t *arr = (const uint64_t *) array;
        for (i = 0; i < count; i++)
          rv += uint64_size (arr[i]);
      }
      break;
    case PROTOBUF_C_TYPE_BOOL:
      return count;
    default:
      assert (0);
    }
  return rv;
}
static size_t
pack_buffer_packed_payload (const ProtobufCFieldDescriptor *field,
                            unsigned count,
                            const void *array,
                            ProtobufCBuffer *buffer)
{
  uint8_t scratch[16];
  size_t rv = 0;
  unsigned i;
  switch (field->type)
    {
      case PROTOBUF_C_TYPE_SFIXED32:
      case PROTOBUF_C_TYPE_FIXED32:
      case PROTOBUF_C_TYPE_FLOAT:
#if IS_LITTLE_ENDIAN
        rv = count * 4;
        goto no_packing_needed;
#else
        for (i = 0; i < count; i++)
          {
            unsigned len = fixed32_pack (((uint32_t*)array)[i], scratch);
            buffer->append (buffer, len, scratch);
            rv += len;
          }
#endif
        break;
      case PROTOBUF_C_TYPE_SFIXED64:
      case PROTOBUF_C_TYPE_FIXED64:
      case PROTOBUF_C_TYPE_DOUBLE:
#if IS_LITTLE_ENDIAN
        rv = count * 8;
        goto no_packing_needed;
#else
        for (i = 0; i < count; i++)
          {
            unsigned len = fixed64_pack (((uint64_t*)array)[i], scratch);
            buffer->append (buffer, len, scratch);
            rv += len;
          }
        break;
#endif
      case PROTOBUF_C_TYPE_INT32:
        for (i = 0; i < count; i++)
          {
            unsigned len = int32_pack (((int32_t*)array)[i], scratch);
            buffer->append (buffer, len, scratch);
            rv += len;
          }
        break;

      case PROTOBUF_C_TYPE_SINT32:
        for (i = 0; i < count; i++)
          {
            unsigned len = sint32_pack (((int32_t*)array)[i], scratch);
            buffer->append (buffer, len, scratch);
            rv += len;
          }
        break;
      case PROTOBUF_C_TYPE_ENUM:
      case PROTOBUF_C_TYPE_UINT32:
        for (i = 0; i < count; i++)
          {
            unsigned len = uint32_pack (((uint32_t*)array)[i], scratch);
            buffer->append (buffer, len, scratch);
            rv += len;
          }
        break;

      case PROTOBUF_C_TYPE_SINT64:
        for (i = 0; i < count; i++)
          {
            unsigned len = sint64_pack (((int64_t*)array)[i], scratch);
            buffer->append (buffer, len, scratch);
            rv += len;
          }
        break;
      case PROTOBUF_C_TYPE_INT64:
      case PROTOBUF_C_TYPE_UINT64:
        for (i = 0; i < count; i++)
          {
            unsigned len = uint64_pack (((uint64_t*)array)[i], scratch);
            buffer->append (buffer, len, scratch);
            rv += len;
          }
        break;
      case PROTOBUF_C_TYPE_BOOL:
        for (i = 0; i < count; i++)
          {
            unsigned len = boolean_pack (((protobuf_c_boolean*)array)[i], scratch);
            buffer->append (buffer, len, scratch);
            rv += len;
          }
        return count;
      default:
        assert(0);
    }
  return rv;

no_packing_needed:
  buffer->append (buffer, rv, array);
  return rv;
}

static size_t
repeated_field_pack_to_buffer (const ProtobufCFieldDescriptor *field,
                               unsigned count,
                               const void *member,
                               ProtobufCBuffer *buffer)
{
  char *array = * (char * const *) member;
  if (count == 0)
    return 0;
  if (field->packed)
    {
      uint8_t scratch[MAX_UINT64_ENCODED_SIZE * 2];
      size_t rv = tag_pack (field->id, scratch);
      size_t payload_len = get_packed_payload_length (field, count, array);
      size_t tmp;
      scratch[0] |= PROTOBUF_C_WIRE_TYPE_LENGTH_PREFIXED;
      rv += uint32_pack (payload_len, scratch + rv);
      buffer->append (buffer, rv, scratch);
      tmp = pack_buffer_packed_payload (field, count, array, buffer);
      assert (tmp == payload_len);
      return rv + payload_len;
    }
  else
    {
      size_t siz;
      unsigned i;
      /* CONSIDER: optimize this case a bit (by putting the loop inside the switch) */
      unsigned rv = 0;
      siz = sizeof_elt_in_repeated_array (field->type);
      for (i = 0; i < count; i++)
        {
          rv += required_field_pack_to_buffer (field, array, buffer);
          array += siz;
        }
      return rv;
    }
}

static size_t
unknown_field_pack_to_buffer (const ProtobufCMessageUnknownField *field,
                              ProtobufCBuffer *buffer)
{
  uint8_t header[MAX_UINT64_ENCODED_SIZE];
  size_t rv = tag_pack (field->tag, header);
  header[0] |= field->wire_type;
  buffer->append (buffer, rv, header);
  buffer->append (buffer, field->len, field->data);
  return rv + field->len;
}

size_t
protobuf_c_message_pack_to_buffer (const ProtobufCMessage *message,
                                   ProtobufCBuffer  *buffer)
{
  unsigned i;
  size_t rv = 0;
  ASSERT_IS_MESSAGE (message);
  for (i = 0; i < message->descriptor->n_fields; i++)
    {
      const ProtobufCFieldDescriptor *field = message->descriptor->fields + i;
      const void *member = ((const char *) message) + field->offset;
      const void *qmember = ((const char *) message) + field->quantifier_offset;

      if (field->label == PROTOBUF_C_LABEL_REQUIRED)
        rv += required_field_pack_to_buffer (field, member, buffer);
      else if (field->label == PROTOBUF_C_LABEL_OPTIONAL)
        rv += optional_field_pack_to_buffer (field, qmember, member, buffer);
      else
        rv += repeated_field_pack_to_buffer (field, * (const size_t *) qmember, member, buffer);
    }
  for (i = 0; i < message->n_unknown_fields; i++)
    rv += unknown_field_pack_to_buffer (&message->unknown_fields[i], buffer);

  return rv;
}

/* === unpacking === */
#if PRINT_UNPACK_ERRORS
# define UNPACK_ERROR(args)  do { printf args;printf("\n"); }while(0)
#else
# define UNPACK_ERROR(args)  do { } while (0)
#endif

static inline int
int_range_lookup (unsigned n_ranges,
                  const ProtobufCIntRange *ranges,
                  int value)
{
  unsigned start, n;
  if (n_ranges == 0)
    return -1;
  start = 0;
  n = n_ranges;
  while (n > 1)
    {
      unsigned mid = start + n / 2;
      if (value < ranges[mid].start_value)
        {
          n = mid - start;
        }
      else if (value >= ranges[mid].start_value + (int)(ranges[mid+1].orig_index-ranges[mid].orig_index))
        {
          unsigned new_start = mid + 1;
          n = start + n - new_start;
          start = new_start;
        }
      else
        return (value - ranges[mid].start_value) + ranges[mid].orig_index;
    }
  if (n > 0)
    {
      unsigned start_orig_index = ranges[start].orig_index;
      unsigned range_size = ranges[start+1].orig_index - start_orig_index;

      if (ranges[start].start_value <= value
       && value < (int)(ranges[start].start_value + range_size))
        return (value - ranges[start].start_value) + start_orig_index;
    }
  return -1;
}

static size_t
parse_tag_and_wiretype (size_t len,
                        const uint8_t *data,
                        uint32_t *tag_out,
                        ProtobufCWireType *wiretype_out)
{
  unsigned max_rv = len > 5 ? 5 : len;
  uint32_t tag = (data[0]&0x7f) >> 3;
  unsigned shift = 4;
  unsigned rv;
  *wiretype_out = data[0] & 7;
  if ((data[0] & 0x80) == 0)
    {
      *tag_out = tag;
      return 1;
    }
  for (rv = 1; rv < max_rv; rv++)
    if (data[rv] & 0x80)
      {
        tag |= (data[rv] & 0x7f) << shift;
        shift += 7;
      }
    else
      {
        tag |= data[rv] << shift;
        *tag_out = tag;
        return rv + 1;
      }
  return 0;                   /* error: bad header */
}

/* sizeof(ScannedMember) must be <= (1<<BOUND_SIZEOF_SCANNED_MEMBER_LOG2) */
#define BOUND_SIZEOF_SCANNED_MEMBER_LOG2  5
typedef struct _ScannedMember ScannedMember;
struct _ScannedMember
{
  uint32_t tag;
  uint8_t wire_type;
  uint8_t length_prefix_len;
  const ProtobufCFieldDescriptor *field;
  size_t len;
  const uint8_t *data;
};

static inline uint32_t
scan_length_prefixed_data (size_t len, const uint8_t *data, size_t *prefix_len_out)
{
  unsigned hdr_max = len < 5 ? len : 5;
  unsigned hdr_len;
  uint32_t val = 0;
  unsigned i;
  unsigned shift = 0;
  for (i = 0; i < hdr_max; i++)
    {
      val |= (data[i] & 0x7f) << shift;
      shift += 7;
      if ((data[i] & 0x80) == 0)
        break;
    }
  if (i == hdr_max)
    {
      UNPACK_ERROR (("error parsing length for length-prefixed data"));
      return 0;
    }
  hdr_len = i + 1;
  *prefix_len_out = hdr_len;
  if (hdr_len + val > len)
    {
      UNPACK_ERROR (("data too short after length-prefix of %u",
                     val));
      return 0;
    }
  return hdr_len + val;
}

static size_t 
max_b128_numbers (size_t len, const uint8_t *data)
{
  size_t rv = 0;
  while (len--)
    if ((*data++ & 0x80) == 0)
      ++rv;
  return rv;
}


/* Given a raw slab of packed-repeated values,
   determine the number of elements.
   This function detects certain kinds of errors
   but not others; the remaining error checking is done by
   parse_packed_repeated_member() */
static protobuf_c_boolean
count_packed_elements (ProtobufCType type,
                       size_t len,
                       const uint8_t *data,
                       size_t *count_out)
{
  switch (type)
    {
    case PROTOBUF_C_TYPE_SFIXED32:
    case PROTOBUF_C_TYPE_FIXED32:
    case PROTOBUF_C_TYPE_FLOAT:
      if (len % 4 != 0)
        {
          UNPACK_ERROR (("length must be a multiple of 4 for fixed-length 32-bit types"));
          return FALSE;
        }
      *count_out = len / 4;
      return TRUE;

    case PROTOBUF_C_TYPE_SFIXED64:
    case PROTOBUF_C_TYPE_FIXED64:
    case PROTOBUF_C_TYPE_DOUBLE:
      if (len % 8 != 0)
        {
          UNPACK_ERROR (("length must be a multiple of 8 for fixed-length 64-bit types"));
          return FALSE;
        }
      *count_out = len / 8;
      return TRUE;

    case PROTOBUF_C_TYPE_INT32:
    case PROTOBUF_C_TYPE_SINT32:
    case PROTOBUF_C_TYPE_ENUM:
    case PROTOBUF_C_TYPE_UINT32:
    case PROTOBUF_C_TYPE_INT64:
    case PROTOBUF_C_TYPE_SINT64:
    case PROTOBUF_C_TYPE_UINT64:
      *count_out = max_b128_numbers (len, data);
      return TRUE;
    case PROTOBUF_C_TYPE_BOOL:
      *count_out = len;
      return TRUE;

    case PROTOBUF_C_TYPE_STRING:
    case PROTOBUF_C_TYPE_BYTES:
    case PROTOBUF_C_TYPE_MESSAGE:
    default:
      UNPACK_ERROR (("bad protobuf-c type %u for packed-repeated", type));
      return FALSE;
    }
}

static inline uint32_t
parse_uint32 (unsigned len, const uint8_t *data)
{
  unsigned rv = data[0] & 0x7f;
  if (len > 1)
    {
      rv |= ((data[1] & 0x7f) << 7);
      if (len > 2)
        {
          rv |= ((data[2] & 0x7f) << 14);
          if (len > 3)
            {
              rv |= ((data[3] & 0x7f) << 21);
              if (len > 4)
                rv |= (data[4] << 28);
            }
        }
    }
  return rv;
}
static inline uint32_t
parse_int32 (unsigned len, const uint8_t *data)
{
  return parse_uint32 (len, data);
}
static inline int32_t
unzigzag32 (uint32_t v)
{
  if (v&1)
    return -(v>>1) - 1;
  else
    return v>>1;
}
static inline uint32_t
parse_fixed_uint32 (const uint8_t *data)
{
#if IS_LITTLE_ENDIAN
  uint32_t t;
  memcpy (&t, data, 4);
  return t;
#else
  return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
#endif
}
static uint64_t
parse_uint64 (unsigned len, const uint8_t *data)
{
  unsigned shift, i;
  if (len < 5)
    return parse_uint32 (len, data);
  uint64_t rv = ((data[0] & 0x7f))
              | ((data[1] & 0x7f)<<7)
              | ((data[2] & 0x7f)<<14)
              | ((data[3] & 0x7f)<<21);
  shift = 28;
  for (i = 4; i < len; i++)
    {
      rv |= (((uint64_t)(data[i]&0x7f)) << shift);
      shift += 7;
    }
  return rv;
}
static inline int64_t
unzigzag64 (uint64_t v)
{
  if (v&1)
    return -(v>>1) - 1;
  else
    return v>>1;
}
static inline uint64_t
parse_fixed_uint64 (const uint8_t *data)
{
#if IS_LITTLE_ENDIAN
  uint64_t t;
  memcpy (&t, data, 8);
  return t;
#else
  return (uint64_t)parse_fixed_uint32 (data)
      | (((uint64_t)parse_fixed_uint32(data+4)) << 32);
#endif
}
static protobuf_c_boolean
parse_boolean (unsigned len, const uint8_t *data)
{
  unsigned i;
  for (i = 0; i < len; i++)
    if (data[i] & 0x7f)
      return 1;
  return 0;
}
static protobuf_c_boolean
parse_required_member (ScannedMember *scanned_member,
                       void *member,
                       ProtobufCAllocator *allocator,
                       protobuf_c_boolean maybe_clear)
{
  unsigned len = scanned_member->len;
  const uint8_t *data = scanned_member->data;
  ProtobufCWireType wire_type = scanned_member->wire_type;
  switch (scanned_member->field->type)
    {
    case PROTOBUF_C_TYPE_INT32:
      if (wire_type != PROTOBUF_C_WIRE_TYPE_VARINT)
        return 0;
      *(uint32_t*)member = parse_int32 (len, data);
      return 1;
    case PROTOBUF_C_TYPE_UINT32:
      if (wire_type != PROTOBUF_C_WIRE_TYPE_VARINT)
        return 0;
      *(uint32_t*)member = parse_uint32 (len, data);
      return 1;
    case PROTOBUF_C_TYPE_SINT32:
      if (wire_type != PROTOBUF_C_WIRE_TYPE_VARINT)
        return 0;
      *(int32_t*)member = unzigzag32 (parse_uint32 (len, data));
      return 1;
    case PROTOBUF_C_TYPE_SFIXED32:
    case PROTOBUF_C_TYPE_FIXED32:
    case PROTOBUF_C_TYPE_FLOAT:
      if (wire_type != PROTOBUF_C_WIRE_TYPE_32BIT)
        return 0;
      *(uint32_t*)member = parse_fixed_uint32 (data);
      return 1;

    case PROTOBUF_C_TYPE_INT64:
    case PROTOBUF_C_TYPE_UINT64:
      if (wire_type != PROTOBUF_C_WIRE_TYPE_VARINT)
        return 0;
      *(uint64_t*)member = parse_uint64 (len, data);
      return 1;
    case PROTOBUF_C_TYPE_SINT64:
      if (wire_type != PROTOBUF_C_WIRE_TYPE_VARINT)
        return 0;
      *(int64_t*)member = unzigzag64 (parse_uint64 (len, data));
      return 1;
    case PROTOBUF_C_TYPE_SFIXED64:
    case PROTOBUF_C_TYPE_FIXED64:
    case PROTOBUF_C_TYPE_DOUBLE:
      if (wire_type != PROTOBUF_C_WIRE_TYPE_64BIT)
        return 0;
      *(uint64_t*)member = parse_fixed_uint64 (data);
      return 1;

    case PROTOBUF_C_TYPE_BOOL:
      *(protobuf_c_boolean*)member = parse_boolean (len, data);
      return 1;

    case PROTOBUF_C_TYPE_ENUM:
      if (wire_type != PROTOBUF_C_WIRE_TYPE_VARINT)
        return 0;
      *(uint32_t*)member = parse_uint32 (len, data);
      return 1;

    case PROTOBUF_C_TYPE_STRING:
      if (wire_type != PROTOBUF_C_WIRE_TYPE_LENGTH_PREFIXED)
        return 0;
      {
        char **pstr = member;
        unsigned pref_len = scanned_member->length_prefix_len;
        if (maybe_clear && *pstr != NULL)
          {
            const char *def = scanned_member->field->default_value;
            if (*pstr != NULL && *pstr != def)
              FREE (allocator, *pstr);
          }
        DO_ALLOC (*pstr, allocator, len - pref_len + 1, return 0);
        memcpy (*pstr, data + pref_len, len - pref_len);
        (*pstr)[len-pref_len] = 0;
        return 1;
      }
    case PROTOBUF_C_TYPE_BYTES:
      if (wire_type != PROTOBUF_C_WIRE_TYPE_LENGTH_PREFIXED)
        return 0;
      {
        ProtobufCBinaryData *bd = member;
        const ProtobufCBinaryData *def_bd;
        unsigned pref_len = scanned_member->length_prefix_len;
        def_bd = scanned_member->field->default_value;
        if (maybe_clear && bd->data != NULL && bd->data != def_bd->data)
          FREE (allocator, bd->data);
        DO_ALLOC (bd->data, allocator, len - pref_len, return 0);
        memcpy (bd->data, data + pref_len, len - pref_len);
        bd->len = len - pref_len;
        return 1;
      }
    //case PROTOBUF_C_TYPE_GROUP,          // NOT SUPPORTED
    case PROTOBUF_C_TYPE_MESSAGE:
      if (wire_type != PROTOBUF_C_WIRE_TYPE_LENGTH_PREFIXED)
        return 0;
      {
        ProtobufCMessage **pmessage = member;
        ProtobufCMessage *subm;
        const ProtobufCMessage *def_mess;
        unsigned pref_len = scanned_member->length_prefix_len;
        def_mess = scanned_member->field->default_value;
        if (maybe_clear && *pmessage != NULL && *pmessage != def_mess)
          protobuf_c_message_free_unpacked (*pmessage, allocator);
        subm = protobuf_c_message_unpack (scanned_member->field->descriptor,
                                          allocator,
                                          len - pref_len, data + pref_len);
        *pmessage = subm;       /* since we freed the message we must clear the field, even if NULL */
        if (subm == NULL)
          return 0;
        return 1;
      }
    }
  return 0;
}

static protobuf_c_boolean
parse_optional_member (ScannedMember *scanned_member,
                       void *member,
                       ProtobufCMessage *message,
                       ProtobufCAllocator *allocator)
{
  if (!parse_required_member (scanned_member, member, allocator, TRUE))
    return 0;
  if (scanned_member->field->quantifier_offset != 0)
    STRUCT_MEMBER (protobuf_c_boolean,
                   message,
                   scanned_member->field->quantifier_offset) = 1;
  return 1;
}

static protobuf_c_boolean
parse_repeated_member (ScannedMember *scanned_member,
                       void *member,
                       ProtobufCMessage *message,
                       ProtobufCAllocator *allocator)
{
  const ProtobufCFieldDescriptor *field = scanned_member->field;
  size_t *p_n = STRUCT_MEMBER_PTR(size_t, message, field->quantifier_offset);
  size_t siz = sizeof_elt_in_repeated_array (field->type);
  char *array = *(char**)member;
  if (!parse_required_member (scanned_member,
                              array + siz * (*p_n),
                              allocator,
                              FALSE))
    return 0;
  *p_n += 1;
  return 1;
}

static unsigned scan_varint (unsigned len, const uint8_t *data)
{
  unsigned i;
  if (len > 10)
    len = 10;
  for (i = 0; i < len; i++)
    if ((data[i] & 0x80) == 0)
      break;
  if (i == len)
    return 0;
  return i + 1;
}

static protobuf_c_boolean
parse_packed_repeated_member (ScannedMember *scanned_member,
                              void *member,
                              ProtobufCMessage *message)
{
  const ProtobufCFieldDescriptor *field = scanned_member->field;
  size_t *p_n = STRUCT_MEMBER_PTR(size_t, message, field->quantifier_offset);
  size_t siz = sizeof_elt_in_repeated_array (field->type);
  char *array = *(char**)member + siz * (*p_n);
  const uint8_t *at = scanned_member->data + scanned_member->length_prefix_len;
  size_t rem = scanned_member->len - scanned_member->length_prefix_len;
  size_t count = 0;
  unsigned i;
  switch (field->type)
    {
      case PROTOBUF_C_TYPE_SFIXED32:
      case PROTOBUF_C_TYPE_FIXED32:
      case PROTOBUF_C_TYPE_FLOAT:
        count = (scanned_member->len - scanned_member->length_prefix_len) / 4;
#if IS_LITTLE_ENDIAN
        goto no_unpacking_needed;
#else
        for (i = 0; i < count; i++)
          {
            ((uint32_t*)array)[i] = parse_fixed_uint32 (at);
            at += 4;
          }
#endif
        break;
      case PROTOBUF_C_TYPE_SFIXED64:
      case PROTOBUF_C_TYPE_FIXED64:
      case PROTOBUF_C_TYPE_DOUBLE:
        count = (scanned_member->len - scanned_member->length_prefix_len) / 8;
#if IS_LITTLE_ENDIAN
        goto no_unpacking_needed;
#else
        for (i = 0; i < count; i++)
          {
            ((uint64_t*)array)[i] = parse_fixed_uint64 (at);
            at += 8;
          }
        break;
#endif
      case PROTOBUF_C_TYPE_INT32:
        while (rem > 0)
          {
            unsigned s = scan_varint (rem, at);
            if (s == 0)
              {
                UNPACK_ERROR (("bad packed-repeated int32 value"));
                return FALSE;
              }
            ((int32_t*)array)[count++] = parse_int32 (s, at);
            at += s;
            rem -= s;
          }
        break;

      case PROTOBUF_C_TYPE_SINT32:
        while (rem > 0)
          {
            unsigned s = scan_varint (rem, at);
            if (s == 0)
              {
                UNPACK_ERROR (("bad packed-repeated sint32 value"));
                return FALSE;
              }
            ((int32_t*)array)[count++] = unzigzag32 (parse_uint32 (s, at));
            at += s;
            rem -= s;
          }
        break;
      case PROTOBUF_C_TYPE_ENUM:
      case PROTOBUF_C_TYPE_UINT32:
        while (rem > 0)
          {
            unsigned s = scan_varint (rem, at);
            if (s == 0)
              {
                UNPACK_ERROR (("bad packed-repeated enum or uint32 value"));
                return FALSE;
              }
            ((uint32_t*)array)[count++] = parse_uint32 (s, at);
            at += s;
            rem -= s;
          }
        break;

      case PROTOBUF_C_TYPE_SINT64:
        while (rem > 0)
          {
            unsigned s = scan_varint (rem, at);
            if (s == 0)
              {
                UNPACK_ERROR (("bad packed-repeated sint64 value"));
                return FALSE;
              }
            ((int64_t*)array)[count++] = unzigzag64 (parse_uint64 (s, at));
            at += s;
            rem -= s;
          }
        break;
      case PROTOBUF_C_TYPE_INT64:
      case PROTOBUF_C_TYPE_UINT64:
        while (rem > 0)
          {
            unsigned s = scan_varint (rem, at);
            if (s == 0)
              {
                UNPACK_ERROR (("bad packed-repeated int64/uint64 value"));
                return FALSE;
              }
            ((int64_t*)array)[count++] = parse_uint64 (s, at);
            at += s;
            rem -= s;
          }
        break;
      case PROTOBUF_C_TYPE_BOOL:
        count = rem;
        for (i = 0; i < count; i++)
          {
            if (at[i] > 1)
              {
                UNPACK_ERROR (("bad packed-repeated boolean value"));
                return FALSE;
              }
            ((protobuf_c_boolean*)array)[i] = at[i];
          }
        break;
      default:
        assert(0);
    }
  *p_n += count;
  return TRUE;

no_unpacking_needed:
  memcpy (array, at, count * siz);
  *p_n += count;
  return TRUE;
}

static protobuf_c_boolean
parse_member (ScannedMember *scanned_member,
              ProtobufCMessage *message,
              ProtobufCAllocator *allocator)
{
  const ProtobufCFieldDescriptor *field = scanned_member->field;
  void *member;
  if (field == NULL)
    {
      ProtobufCMessageUnknownField *ufield = message->unknown_fields + (message->n_unknown_fields++);
      ufield->tag = scanned_member->tag;
      ufield->wire_type = scanned_member->wire_type;
      ufield->len = scanned_member->len;
      DO_UNALIGNED_ALLOC (ufield->data, allocator, scanned_member->len, return 0);
      memcpy (ufield->data, scanned_member->data, ufield->len);
      return 1;
    }
  member = (char*)message + field->offset;
  switch (field->label)
    {
    case PROTOBUF_C_LABEL_REQUIRED:
      return parse_required_member (scanned_member, member, allocator, TRUE);
    case PROTOBUF_C_LABEL_OPTIONAL:
      return parse_optional_member (scanned_member, member, message, allocator);
    case PROTOBUF_C_LABEL_REPEATED:
      if (field->packed
       && scanned_member->wire_type == PROTOBUF_C_WIRE_TYPE_LENGTH_PREFIXED)
        return parse_packed_repeated_member (scanned_member, member, message);
      else
        return parse_repeated_member (scanned_member, member, message, allocator);
    }
  PROTOBUF_C_ASSERT_NOT_REACHED ();
  return 0;
}


/* TODO: expose/use this function if desc->message_init==NULL 
   (which occurs for old code, and may be useful for certain
   programatic techniques for generating descriptors). */
void
protobuf_c_message_init_generic (const ProtobufCMessageDescriptor *desc,
                                 ProtobufCMessage *message)
{
  unsigned i;
  memset (message, 0, desc->sizeof_message);
  message->descriptor = desc;
  for (i = 0; i < desc->n_fields; i++)
    if (desc->fields[i].default_value != NULL
     && desc->fields[i].label != PROTOBUF_C_LABEL_REPEATED)
      {
        void *field = STRUCT_MEMBER_P (message, desc->fields[i].offset);
        const void *dv = desc->fields[i].default_value;
        switch (desc->fields[i].type)
        {
        case PROTOBUF_C_TYPE_INT32:
        case PROTOBUF_C_TYPE_SINT32:
        case PROTOBUF_C_TYPE_SFIXED32:
        case PROTOBUF_C_TYPE_UINT32:
        case PROTOBUF_C_TYPE_FIXED32:
        case PROTOBUF_C_TYPE_FLOAT:
        case PROTOBUF_C_TYPE_ENUM:
          memcpy (field, dv, 4);
          break;

        case PROTOBUF_C_TYPE_INT64:
        case PROTOBUF_C_TYPE_SINT64:
        case PROTOBUF_C_TYPE_SFIXED64:
        case PROTOBUF_C_TYPE_UINT64:
        case PROTOBUF_C_TYPE_FIXED64:
        case PROTOBUF_C_TYPE_DOUBLE:
          memcpy (field, dv, 8);
          break;

        case PROTOBUF_C_TYPE_BOOL:
          memcpy (field, dv, sizeof (protobuf_c_boolean));
          break;

        case PROTOBUF_C_TYPE_BYTES:
          memcpy (field, dv, sizeof (ProtobufCBinaryData));
          break;

        case PROTOBUF_C_TYPE_STRING:
        case PROTOBUF_C_TYPE_MESSAGE:
          /* the next line essentially implements a cast from const,
             which is totally unavoidable. */
          *(const void**)field = dv;
          break;
        }
      }
}

/* ScannedMember slabs (an unpacking implementation detail).
   Before doing real unpacking, we first scan through the
   elements to see how many there are (for repeated fields),
   and which field to use (for non-repeated fields given twice).

 * In order to avoid allocations for small messages,
   we keep a stack-allocated slab of ScannedMembers of
   size FIRST_SCANNED_MEMBER_SLAB_SIZE (16).
   After we fill that up, we allocate each slab twice
   as large as the previous one. */
#define FIRST_SCANNED_MEMBER_SLAB_SIZE_LOG2             4

/* The number of slabs, including the stack-allocated ones;
   choose the number so that we would overflow if we needed
   a slab larger than provided. */
#define MAX_SCANNED_MEMBER_SLAB                          \
  (sizeof(void*)*8 - 1                                   \
   - BOUND_SIZEOF_SCANNED_MEMBER_LOG2                    \
   - FIRST_SCANNED_MEMBER_SLAB_SIZE_LOG2)

ProtobufCMessage *
protobuf_c_message_unpack         (const ProtobufCMessageDescriptor *desc,
                                   ProtobufCAllocator  *allocator,
                                   size_t               len,
                                   const uint8_t       *data)
{
  ProtobufCMessage *rv;
  size_t rem = len;
  const uint8_t *at = data;
  const ProtobufCFieldDescriptor *last_field = desc->fields + 0;
  ScannedMember first_member_slab[1<<FIRST_SCANNED_MEMBER_SLAB_SIZE_LOG2];

  /* scanned_member_slabs[i] is an array of arrays of ScannedMember.
     The first slab (scanned_member_slabs[0] is just a pointer to
     first_member_slab), above.  All subsequent slabs will be allocated
     using the allocator. */
  ScannedMember *scanned_member_slabs[MAX_SCANNED_MEMBER_SLAB+1];
  unsigned which_slab = 0;       /* the slab we are currently populating */
  unsigned in_slab_index = 0;    /* number of members in the slab */
  size_t n_unknown = 0;
  unsigned f;
  unsigned i_slab;
  unsigned last_field_index = 0;
  unsigned long *required_fields_bitmap;
  unsigned required_fields_bitmap_len;
  static const unsigned word_bits = sizeof(long) * 8;

  ASSERT_IS_MESSAGE_DESCRIPTOR (desc);

  if (allocator == NULL)
    allocator = &protobuf_c_default_allocator;

  required_fields_bitmap_len = (desc->n_fields + word_bits - 1) / word_bits;
  required_fields_bitmap = alloca(required_fields_bitmap_len * sizeof(long));
  memset(required_fields_bitmap, 0, required_fields_bitmap_len * sizeof(long));

  DO_ALLOC (rv, allocator, desc->sizeof_message, return NULL);
  scanned_member_slabs[0] = first_member_slab;

  /* Generated code always defines "message_init".
     However, we provide a fallback for (1) users of old protobuf-c
     generated-code that do not provide the function,
     and (2) descriptors constructed from some other source
     (most likely, direct construction from the .proto file) */
  if (desc->message_init != NULL)
    protobuf_c_message_init (desc, rv);
  else
    protobuf_c_message_init_generic (desc, rv);

  while (rem > 0)
    {
      uint32_t tag;
      ProtobufCWireType wire_type;
      size_t used = parse_tag_and_wiretype (rem, at, &tag, &wire_type);
      const ProtobufCFieldDescriptor *field;
      ScannedMember tmp;
      if (used == 0)
        {
          UNPACK_ERROR (("error parsing tag/wiretype at offset %u",
                         (unsigned)(at-data)));
          goto error_cleanup_during_scan;
        }
      /* XXX: consider optimizing for field[1].id == tag, if field[1] exists! */
      if (last_field->id != tag)
        {
          /* lookup field */
          int field_index = int_range_lookup (desc->n_field_ranges,
                                              desc->field_ranges,
                                              tag);
          if (field_index < 0)
            {
              field = NULL;
              n_unknown++;
            }
          else
            {
              field = desc->fields + field_index;
              last_field = field;
              last_field_index = field_index;
            }
        }
      else
        field = last_field;

      if (field != NULL && field->label == PROTOBUF_C_LABEL_REQUIRED)
        required_fields_bitmap[last_field_index / word_bits] |= (1UL << (last_field_index % word_bits));

      at += used;
      rem -= used;
      tmp.tag = tag;
      tmp.wire_type = wire_type;
      tmp.field = field;
      tmp.data = at;
      switch (wire_type)
        {
        case PROTOBUF_C_WIRE_TYPE_VARINT:
          {
            unsigned max_len = rem < 10 ? rem : 10;
            unsigned i;
            for (i = 0; i < max_len; i++)
              if ((at[i] & 0x80) == 0)
                break;
            if (i == max_len)
              {
                UNPACK_ERROR (("unterminated varint at offset %u",
                               (unsigned)(at-data)));
                goto error_cleanup_during_scan;
              }
            tmp.len = i + 1;
          }
          break;
        case PROTOBUF_C_WIRE_TYPE_64BIT:
          if (rem < 8)
            {
              UNPACK_ERROR (("too short after 64bit wiretype at offset %u",
                             (unsigned)(at-data)));
              goto error_cleanup_during_scan;
            }
          tmp.len = 8;
          break;
        case PROTOBUF_C_WIRE_TYPE_LENGTH_PREFIXED:
          {
            size_t pref_len;
            tmp.len = scan_length_prefixed_data (rem, at, &pref_len);
            if (tmp.len == 0)
              {
                /* NOTE: scan_length_prefixed_data calls UNPACK_ERROR */
                goto error_cleanup_during_scan;
              }
            tmp.length_prefix_len = pref_len;
            break;
          }
        case PROTOBUF_C_WIRE_TYPE_32BIT:
          if (rem < 4)
            {
              UNPACK_ERROR (("too short after 32bit wiretype at offset %u",
                             (unsigned)(at-data)));
              goto error_cleanup_during_scan;
            }
          tmp.len = 4;
          break;
        default:
          UNPACK_ERROR (("unsupported tag %u at offset %u",
                         wire_type, (unsigned)(at-data))); 
          goto error_cleanup_during_scan;
        }
      if (in_slab_index == (1U<<(which_slab+FIRST_SCANNED_MEMBER_SLAB_SIZE_LOG2)))
        {
          size_t size;
          in_slab_index = 0;
          if (which_slab == MAX_SCANNED_MEMBER_SLAB)
            {
              UNPACK_ERROR (("too many fields"));
              goto error_cleanup_during_scan;
            }
          which_slab++;
          size = sizeof(ScannedMember) << (which_slab+FIRST_SCANNED_MEMBER_SLAB_SIZE_LOG2);
          /* TODO: consider using alloca() ! */
          if (allocator->tmp_alloc != NULL)
            scanned_member_slabs[which_slab] = TMPALLOC(allocator, size);
          else
            DO_ALLOC (scanned_member_slabs[which_slab], allocator, size, goto error_cleanup_during_scan);
        }
      scanned_member_slabs[which_slab][in_slab_index++] = tmp;

      if (field != NULL && field->label == PROTOBUF_C_LABEL_REPEATED)
        {
          size_t *n = STRUCT_MEMBER_PTR (size_t, rv, field->quantifier_offset);
          if (field->packed
           && wire_type == PROTOBUF_C_WIRE_TYPE_LENGTH_PREFIXED)
            {
              size_t count;
              if (!count_packed_elements (field->type,
                                          tmp.len - tmp.length_prefix_len,
                                          tmp.data + tmp.length_prefix_len,
                                          &count))
                {
                  UNPACK_ERROR (("counting packed elements"));
                  goto error_cleanup_during_scan;
                }
              *n += count;
            }
          else
           *n += 1;
        }

      at += tmp.len;
      rem -= tmp.len;
    }

  /* allocate space for repeated fields, also check that all required fields have been set */
  for (f = 0; f < desc->n_fields; f++)
  {
    const ProtobufCFieldDescriptor *field = desc->fields + f;
    if (field->label == PROTOBUF_C_LABEL_REPEATED)
    {
        size_t siz = sizeof_elt_in_repeated_array (field->type);
        size_t *n_ptr = STRUCT_MEMBER_PTR (size_t, rv, field->quantifier_offset);
        if (*n_ptr != 0)
          {
            unsigned n = *n_ptr;
            *n_ptr = 0;
            assert(rv->descriptor != NULL);
#define CLEAR_REMAINING_N_PTRS()                                            \
            for(f++;f < desc->n_fields; f++)                                \
              {                                                             \
                field = desc->fields + f;                                   \
                if (field->label == PROTOBUF_C_LABEL_REPEATED)              \
                  STRUCT_MEMBER (size_t, rv, field->quantifier_offset) = 0; \
              }
            DO_ALLOC (STRUCT_MEMBER (void *, rv, field->offset),
                      allocator, siz * n,
                      CLEAR_REMAINING_N_PTRS (); goto error_cleanup);
#undef CLEAR_REMAINING_N_PTRS
          }
    }
    else if (field->label == PROTOBUF_C_LABEL_REQUIRED)
    {
      if (field->default_value == NULL && 0 == (required_fields_bitmap[f / word_bits] & (1UL << (f % word_bits))))
      {
        UNPACK_ERROR (("message '%s': missing required field '%s'", desc->name, field->name));
        goto error_cleanup;
      }
    }
  }

  /* allocate space for unknown fields */
  if (n_unknown)
    {
      DO_ALLOC (rv->unknown_fields,
                allocator, n_unknown * sizeof (ProtobufCMessageUnknownField),
                goto error_cleanup);
    }

  /* do real parsing */
  for (i_slab = 0; i_slab <= which_slab; i_slab++)
    {
      unsigned max = (i_slab == which_slab) ? in_slab_index : (1U<<(i_slab+4));
      ScannedMember *slab = scanned_member_slabs[i_slab];
      unsigned j;
      for (j = 0; j < max; j++)
        {
          if (!parse_member (slab + j, rv, allocator))
            {
              UNPACK_ERROR (("error parsing member %s of %s",
                             slab->field ? slab->field->name : "*unknown-field*", desc->name));
              goto error_cleanup;
            }
        }
    }

  /* cleanup */
  if (allocator->tmp_alloc == NULL)
    {
      unsigned j;
      for (j = 1; j <= which_slab; j++)
        FREE (allocator, scanned_member_slabs[j]);
    }

  return rv;

error_cleanup:
  protobuf_c_message_free_unpacked (rv, allocator);
  if (allocator->tmp_alloc == NULL)
    {
      unsigned j;
      for (j = 1; j <= which_slab; j++)
        FREE (allocator, scanned_member_slabs[j]);
    }
  return NULL;

error_cleanup_during_scan:
  FREE (allocator, rv);
  if (allocator->tmp_alloc == NULL)
    {
      unsigned j;
      for (j = 1; j <= which_slab; j++)
        FREE (allocator, scanned_member_slabs[j]);
    }
  return NULL;
}

/* === free_unpacked === */
void     
protobuf_c_message_free_unpacked  (ProtobufCMessage    *message,
                                   ProtobufCAllocator  *allocator)
{
  const ProtobufCMessageDescriptor *desc = message->descriptor;
  unsigned f;
  ASSERT_IS_MESSAGE (message);
  if (allocator == NULL)
    allocator = &protobuf_c_default_allocator;
  message->descriptor = NULL;
  for (f = 0; f < desc->n_fields; f++)
    {
      if (desc->fields[f].label == PROTOBUF_C_LABEL_REPEATED)
        {
          size_t n = STRUCT_MEMBER (size_t, message, desc->fields[f].quantifier_offset);
          void * arr = STRUCT_MEMBER (void *, message, desc->fields[f].offset);
          if (desc->fields[f].type == PROTOBUF_C_TYPE_STRING)
            {
              unsigned i;
              for (i = 0; i < n; i++)
                FREE (allocator, ((char**)arr)[i]);
            }
          else if (desc->fields[f].type == PROTOBUF_C_TYPE_BYTES)
            {
              unsigned i;
              for (i = 0; i < n; i++)
                FREE (allocator, ((ProtobufCBinaryData*)arr)[i].data);
            }
          else if (desc->fields[f].type == PROTOBUF_C_TYPE_MESSAGE)
            {
              unsigned i;
              for (i = 0; i < n; i++)
                protobuf_c_message_free_unpacked (((ProtobufCMessage**)arr)[i], allocator);
            }
          if (arr != NULL)
            FREE (allocator, arr);
        }
      else if (desc->fields[f].type == PROTOBUF_C_TYPE_STRING)
        {
          char *str = STRUCT_MEMBER (char *, message, desc->fields[f].offset);
          if (str && str != desc->fields[f].default_value)
            FREE (allocator, str);
        }
      else if (desc->fields[f].type == PROTOBUF_C_TYPE_BYTES)
        {
          void *data = STRUCT_MEMBER (ProtobufCBinaryData, message, desc->fields[f].offset).data;
          const ProtobufCBinaryData *default_bd;
          default_bd = desc->fields[f].default_value;
          if (data != NULL
           && (default_bd == NULL || default_bd->data != data))
            FREE (allocator, data);
        }
      else if (desc->fields[f].type == PROTOBUF_C_TYPE_MESSAGE)
        {
          ProtobufCMessage *sm;
          sm = STRUCT_MEMBER (ProtobufCMessage *, message,desc->fields[f].offset);
          if (sm && sm != desc->fields[f].default_value)
            protobuf_c_message_free_unpacked (sm, allocator);
        }
    }

  for (f = 0; f < message->n_unknown_fields; f++)
    FREE (allocator, message->unknown_fields[f].data);
  if (message->unknown_fields != NULL)
    FREE (allocator, message->unknown_fields);

  FREE (allocator, message);
}

/* === services === */
typedef void (*GenericHandler)(void *service,
                               const ProtobufCMessage *input,
                               ProtobufCClosure  closure,
                               void             *closure_data);
void 
protobuf_c_service_invoke_internal(ProtobufCService *service,
                                  unsigned          method_index,
                                  const ProtobufCMessage *input,
                                  ProtobufCClosure  closure,
                                  void             *closure_data)
{
  GenericHandler *handlers;
  GenericHandler handler;

  /* Verify that method_index is within range.
     If this fails, you are likely invoking a newly added
     method on an old service.  (Although other memory corruption
     bugs can cause this assertion too) */
  PROTOBUF_C_ASSERT (method_index < service->descriptor->n_methods);

  /* Get the array of virtual methods (which are enumerated by 
     the generated code) */
  handlers = (GenericHandler *) (service + 1);

  /* get our method and invoke it */
  /* TODO: seems like handler==NULL is a situation that
     needs handling */
  handler = handlers[method_index];
  (*handler) (service, input, closure, closure_data);
}

void
protobuf_c_service_generated_init (ProtobufCService *service,
                                   const ProtobufCServiceDescriptor *descriptor,
                                   ProtobufCServiceDestroy destroy)
{
  ASSERT_IS_SERVICE_DESCRIPTOR(descriptor);
  service->descriptor = descriptor;
  service->destroy = destroy;
  service->invoke = protobuf_c_service_invoke_internal;
  memset (service + 1, 0, descriptor->n_methods * sizeof (GenericHandler));
}

void protobuf_c_service_destroy (ProtobufCService *service)
{
  service->destroy (service);
}

/* --- querying the descriptors --- */
const ProtobufCEnumValue *
protobuf_c_enum_descriptor_get_value_by_name 
                         (const ProtobufCEnumDescriptor    *desc,
                          const char                       *name)
{
  unsigned start = 0, count = desc->n_value_names;
  while (count > 1)
    {
      unsigned mid = start + count / 2;
      int rv = strcmp (desc->values_by_name[mid].name, name);
      if (rv == 0)
        return desc->values + desc->values_by_name[mid].index;
      else if (rv < 0)
        {
          count = start + count - (mid + 1);
          start = mid + 1;
        }
      else
        count = mid - start;
    }
  if (count == 0)
    return NULL;
  if (strcmp (desc->values_by_name[start].name, name) == 0)
    return desc->values + desc->values_by_name[start].index;
  return NULL;
}
const ProtobufCEnumValue *
protobuf_c_enum_descriptor_get_value        
                         (const ProtobufCEnumDescriptor    *desc,
                          int                               value)
{
  int rv = int_range_lookup (desc->n_value_ranges, desc->value_ranges, value);
  if (rv < 0)
    return NULL;
  return desc->values + rv;
}

const ProtobufCFieldDescriptor *
protobuf_c_message_descriptor_get_field_by_name
                         (const ProtobufCMessageDescriptor *desc,
                          const char                       *name)
{
  unsigned start = 0, count = desc->n_fields;
  const ProtobufCFieldDescriptor *field;
  while (count > 1)
    {
      unsigned mid = start + count / 2;
      int rv;
      field = desc->fields + desc->fields_sorted_by_name[mid];
      rv = strcmp (field->name, name);
      if (rv == 0)
        return field;
      else if (rv < 0)
        {
          count = start + count - (mid + 1);
          start = mid + 1;
        }
      else
        count = mid - start;
    }
  if (count == 0)
    return NULL;
  field = desc->fields + desc->fields_sorted_by_name[start];
  if (strcmp (field->name, name) == 0)
    return field;
  return NULL;
}

const ProtobufCFieldDescriptor *
protobuf_c_message_descriptor_get_field        
                         (const ProtobufCMessageDescriptor *desc,
                          unsigned                          value)
{
  int rv = int_range_lookup (desc->n_field_ranges,
                             desc->field_ranges,
                             value);
  if (rv < 0)
    return NULL;
  return desc->fields + rv;
}

const ProtobufCMethodDescriptor *
protobuf_c_service_descriptor_get_method_by_name
                         (const ProtobufCServiceDescriptor *desc,
                          const char                       *name)
{
  unsigned start = 0, count = desc->n_methods;
  while (count > 1)
    {
      unsigned mid = start + count / 2;
      unsigned mid_index = desc->method_indices_by_name[mid];
      const char *mid_name = desc->methods[mid_index].name;
      int rv = strcmp (mid_name, name);
      if (rv == 0)
        return desc->methods + desc->method_indices_by_name[mid];
      if (rv < 0)
        {
          count = start + count - (mid + 1);
          start = mid + 1;
        }
      else
        {
          count = mid - start;
        }
    }
  if (count == 0)
    return NULL;
  if (strcmp (desc->methods[desc->method_indices_by_name[start]].name, name) == 0)
    return desc->methods + desc->method_indices_by_name[start];
  return NULL;
}
