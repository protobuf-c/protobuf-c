#include <stdio.h>                      /* for occasional printf()s */
#include <stdlib.h>                     /* for abort(), malloc() etc */
#include <string.h>                     /* for strlen(), memcpy(), memmove() */

#include "protobuf-c.h"

#define MAX_UINT64_ENCODED_SIZE 10

/* convenience macros */
#define TMPALLOC(allocator, size) ((allocator)->tmp_alloc ((allocator)->allocator_data, (size)))
#define ALLOC(allocator, size) ((allocator)->alloc ((allocator)->allocator_data, (size)))
#define FREE(allocator, ptr)   ((allocator)->free ((allocator)->allocator_data, (ptr)))
#define UNALIGNED_ALLOC(allocator, size) ALLOC (allocator, size) /* placeholder */
#define STRUCT_MEMBER_P(struct_p, struct_offset)   \
    ((void *) ((BYTE*) (struct_p) + (struct_offset)))
#define STRUCT_MEMBER(member_type, struct_p, struct_offset)   \
    (*(member_type*) STRUCT_MEMBER_P ((struct_p), (struct_offset)))
#define STRUCT_MEMBER_PTR(member_type, struct_p, struct_offset)   \
    ((member_type*) STRUCT_MEMBER_P ((struct_p), (struct_offset)))
typedef unsigned char BYTE;
#define TRUE 1
#define FALSE 0

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

ProtobufCAllocator protobuf_c_default_allocator =
{
  system_alloc,
  system_free,
  NULL,
  8192,
  NULL
};

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
                                 const unsigned char *data)
{
  ProtobufCBufferSimple *simp = (ProtobufCBufferSimple *) buffer;
  size_t new_len = simp->len + len;
  if (new_len > simp->alloced)
    {
      size_t new_alloced = simp->alloced * 2;
      unsigned char *new_data;
      while (new_alloced < new_len)
        new_alloced += new_alloced;
      new_data = ALLOC (&protobuf_c_default_allocator, new_alloced);
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
static inline uint32_t
zigzag32 (int32_t v)
{
  if (v < 0)
    return ((uint32_t)(-v)) * 2 - 1;
  else
    return v * 2;
}
static inline size_t
sint32_size (int32_t v)
{
  return uint32_size(zigzag32(v));
}
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
static inline uint64_t
zigzag64 (int64_t v)
{
  if (v < 0)
    return ((uint64_t)(-v)) * 2 - 1;
  else
    return v * 2;
}
static inline size_t
sint64_size (int32_t v)
{
  return uint64_size(zigzag64(v));
}

static size_t
required_field_get_packed_size (const ProtobufCFieldDescriptor *field,
                                void *member)
{
  size_t rv = get_tag_size (field->id);
  switch (field->type)
    {
    case PROTOBUF_C_TYPE_SINT32:
      return rv + sint32_size (*(int32_t *) member);
    case PROTOBUF_C_TYPE_INT32:
    case PROTOBUF_C_TYPE_UINT32:
      return rv + uint32_size (*(uint32_t *) member);
    case PROTOBUF_C_TYPE_SINT64:
      return rv + sint64_size (*(int32_t *) member);
    case PROTOBUF_C_TYPE_INT64:
    case PROTOBUF_C_TYPE_UINT64:
      return rv + uint64_size (*(uint64_t *) member);
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
      return rv + uint32_size (*(uint32_t *) member);
    case PROTOBUF_C_TYPE_STRING:
      {
        size_t len = strlen (*(char**) member);
        return rv + uint32_size (len) + len;
      }
      
    case PROTOBUF_C_TYPE_BYTES:
      {
        size_t len = ((ProtobufCBinaryData*) member)->len;
        return rv + uint32_size (len) + len;
      }
    //case PROTOBUF_C_TYPE_GROUP:
    case PROTOBUF_C_TYPE_MESSAGE:
      {
        size_t subrv = protobuf_c_message_get_packed_size (*(ProtobufCMessage **) member);
        return rv + uint32_size (subrv) + subrv;
      }
    }
  PROTOBUF_C_ASSERT_NOT_REACHED ();
  return 0;
}
static size_t
optional_field_get_packed_size (const ProtobufCFieldDescriptor *field,
                                const protobuf_c_boolean *has,
                                void *member)
{
  if (field->type == PROTOBUF_C_TYPE_MESSAGE
   || field->type == PROTOBUF_C_TYPE_STRING)
    {
      if (*(void**)member == NULL)
        return 0;
    }
  else
    {
      if (!*has)
        return 0;
    }
  return required_field_get_packed_size (field, member);
}

static size_t
repeated_field_get_packed_size (const ProtobufCFieldDescriptor *field,
                                size_t count,
                                void *member)
{
  size_t rv = get_tag_size (field->id) * count;
  unsigned i;
  void *array = * (void **) member;
  switch (field->type)
    {
    case PROTOBUF_C_TYPE_SINT32:
      for (i = 0; i < count; i++)
        rv += sint32_size (((int32_t*)array)[i]);
      break;
    case PROTOBUF_C_TYPE_INT32:
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
        rv += protobuf_c_message_get_packed_size (((ProtobufCMessage **) member)[i]);
      break;
    //case PROTOBUF_C_TYPE_GROUP:          // NOT SUPPORTED
    }
  return rv;
}
size_t    protobuf_c_message_get_packed_size(const ProtobufCMessage *message)
{
  unsigned i;
  size_t rv = 0;
  for (i = 0; i < message->descriptor->n_fields; i++)
    {
      const ProtobufCFieldDescriptor *field = message->descriptor->fields + i;
      void *member = ((char *) message) + field->offset;
      void *qmember = ((char *) message) + field->quantifier_offset;

      if (field->label == PROTOBUF_C_LABEL_REQUIRED)
        rv += required_field_get_packed_size (field, member);
      else if (field->label == PROTOBUF_C_LABEL_OPTIONAL)
        rv += optional_field_get_packed_size (field, qmember, member);
      else
        rv += repeated_field_get_packed_size (field, * (size_t *) qmember, member);
    }

  return rv;
}
/* === pack() === */
static inline size_t
uint32_pack (uint32_t value, BYTE *out)
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
static inline size_t sint32_pack (int32_t value, BYTE *out)
{
  return uint32_pack (zigzag32 (value), out);
}
static size_t
uint64_pack (uint64_t value, BYTE *out)
{
  uint32_t hi = value>>32;
  uint32_t lo = value;
  unsigned rv;
  if (hi == 0)
    return uint32_pack ((uint32_t)lo, out);
  out[0] = (lo) & 0x7f;
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
static inline size_t sint64_pack (int64_t value, BYTE *out)
{
  return uint64_pack (zigzag64 (value), out);
}
static inline size_t fixed32_pack (uint32_t value, BYTE *out)
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
static inline size_t fixed64_pack (uint64_t value, BYTE *out)
{
#if IS_LITTLE_ENDIAN
  memcpy (out, &value, 8);
#else
  fixed32_pack (value, out);
  fixed32_pack (value>>32, out+4);
#endif
  return 8;
}
static inline size_t boolean_pack (protobuf_c_boolean value, BYTE *out)
{
  *out = value ? 1 : 0;
  return 1;
}
static inline size_t string_pack (const char * str, BYTE *out)
{
  size_t len = strlen (str);
  size_t rv = uint32_pack (len, out);
  memcpy (out + rv, str, len);
  return rv + len;
}
static inline size_t binary_data_pack (const ProtobufCBinaryData *bd, BYTE *out)
{
  size_t len = bd->len;
  size_t rv = uint32_pack (len, out);
  memcpy (out + rv, bd->data, len);
  return rv + len;
}

static inline size_t
prefixed_message_pack (const ProtobufCMessage *message, BYTE *out)
{
  size_t rv = protobuf_c_message_pack (message, out + 1);
  uint32_t rv_packed_size = uint32_size (rv);
  if (rv_packed_size != 1)
    memmove (out + rv_packed_size, out + 1, rv);
  return uint32_pack (rv, out) + rv;
}

/* wire-type will be added in required_field_pack() */
static size_t tag_pack (uint32_t id, BYTE *out)
{
  if (id < (1<<(32-3)))
    return uint32_pack (id<<3, out);
  else
    return uint64_pack (((uint64_t)id) << 3, out);
}
static size_t
required_field_pack (const ProtobufCFieldDescriptor *field,
                     void *member,
                     BYTE *out)
{
  size_t rv = tag_pack (field->id, out);
  switch (field->type)
    {
    case PROTOBUF_C_TYPE_SINT32:
      out[0] |= PROTOBUF_C_WIRE_TYPE_VARINT;
      return rv + sint32_pack (*(int32_t *) member, out + rv);
    case PROTOBUF_C_TYPE_INT32:
    case PROTOBUF_C_TYPE_UINT32:
    case PROTOBUF_C_TYPE_ENUM:
      out[0] |= PROTOBUF_C_WIRE_TYPE_VARINT;
      return rv + uint32_pack (*(uint32_t *) member, out + rv);
    case PROTOBUF_C_TYPE_SINT64:
      out[0] |= PROTOBUF_C_WIRE_TYPE_VARINT;
      return rv + sint64_pack (*(int32_t *) member, out + rv);
    case PROTOBUF_C_TYPE_INT64:
    case PROTOBUF_C_TYPE_UINT64:
      out[0] |= PROTOBUF_C_WIRE_TYPE_VARINT;
      return rv + uint64_pack (*(uint64_t *) member, out + rv);
    case PROTOBUF_C_TYPE_SFIXED32:
    case PROTOBUF_C_TYPE_FIXED32:
    case PROTOBUF_C_TYPE_FLOAT:
      out[0] |= PROTOBUF_C_WIRE_TYPE_32BIT;
      return rv + fixed32_pack (*(uint64_t *) member, out + rv);
    case PROTOBUF_C_TYPE_SFIXED64:
    case PROTOBUF_C_TYPE_FIXED64:
    case PROTOBUF_C_TYPE_DOUBLE:
      out[0] |= PROTOBUF_C_WIRE_TYPE_64BIT;
      return rv + fixed64_pack (*(uint64_t *) member, out + rv);
    case PROTOBUF_C_TYPE_BOOL:
      out[0] |= PROTOBUF_C_WIRE_TYPE_VARINT;
      return rv + boolean_pack (*(protobuf_c_boolean *) member, out + rv);
    case PROTOBUF_C_TYPE_STRING:
      {
        out[0] |= PROTOBUF_C_WIRE_TYPE_LENGTH_PREFIXED;
        return rv + string_pack (*(char**) member, out + rv);
      }
      
    case PROTOBUF_C_TYPE_BYTES:
      {
        const ProtobufCBinaryData * bd = ((ProtobufCBinaryData*) member);
        out[0] |= PROTOBUF_C_WIRE_TYPE_LENGTH_PREFIXED;
        return rv + binary_data_pack (bd, out + rv);
      }
    //case PROTOBUF_C_TYPE_GROUP:          // NOT SUPPORTED
    case PROTOBUF_C_TYPE_MESSAGE:
      {
        out[0] |= PROTOBUF_C_WIRE_TYPE_LENGTH_PREFIXED;
        return rv + prefixed_message_pack (*(ProtobufCMessage **) member,
                                           out + rv);
      }
    }
  PROTOBUF_C_ASSERT_NOT_REACHED ();
  return 0;
}
static size_t
optional_field_pack (const ProtobufCFieldDescriptor *field,
                     const protobuf_c_boolean *has,
                     void *member,
                     BYTE *out)
{
  if (field->type == PROTOBUF_C_TYPE_MESSAGE
   || field->type == PROTOBUF_C_TYPE_STRING)
    {
      if (*(void**)member == NULL)
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
static inline size_t sizeof_elt_in_repeated_array (ProtobufCType type)
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
static size_t
repeated_field_pack (const ProtobufCFieldDescriptor *field,
                     size_t count,
                     void *member,
                     BYTE *out)
{
  char *array = * (char **) member;
  size_t siz;
  unsigned i;
  size_t rv = 0;
  /* CONSIDER: optimize this case a bit (by putting the loop inside the switch) */
  siz = sizeof_elt_in_repeated_array (field->type);
  for (i = 0; i < count; i++)
    {
      rv += required_field_pack (field, array, out + rv);
      array += siz;
    }
  return rv;
}
size_t    protobuf_c_message_pack           (const ProtobufCMessage *message,
                                             unsigned char *out)
{
  unsigned i;
  size_t rv = 0;
  for (i = 0; i < message->descriptor->n_fields; i++)
    {
      const ProtobufCFieldDescriptor *field = message->descriptor->fields + i;
      void *member = ((char *) message) + field->offset;
      void *qmember = ((char *) message) + field->quantifier_offset;

      if (field->label == PROTOBUF_C_LABEL_REQUIRED)
        rv += required_field_pack (field, member, out + rv);
      else if (field->label == PROTOBUF_C_LABEL_OPTIONAL)
        rv += optional_field_pack (field, qmember, member, out + rv);
      else
        rv += repeated_field_pack (field, * (size_t *) qmember, member, out + rv);
    }

  return rv;
}

/* === pack_to_buffer() === */
static size_t
required_field_pack_to_buffer (const ProtobufCFieldDescriptor *field,
                               void *member,
                               ProtobufCBuffer *buffer)
{
  size_t rv;
  BYTE scratch[MAX_UINT64_ENCODED_SIZE * 2];
  rv = tag_pack (field->id, scratch);
  switch (field->type)
    {
    case PROTOBUF_C_TYPE_SINT32:
      scratch[0] |= PROTOBUF_C_WIRE_TYPE_VARINT;
      rv += sint32_pack (*(int32_t *) member, scratch + rv);
      buffer->append (buffer, rv, scratch);
      break;
    case PROTOBUF_C_TYPE_INT32:
    case PROTOBUF_C_TYPE_UINT32:
    case PROTOBUF_C_TYPE_ENUM:
      scratch[0] |= PROTOBUF_C_WIRE_TYPE_VARINT;
      rv += uint32_pack (*(uint32_t *) member, scratch + rv);
      buffer->append (buffer, rv, scratch);
      break;
    case PROTOBUF_C_TYPE_SINT64:
      scratch[0] |= PROTOBUF_C_WIRE_TYPE_VARINT;
      rv += sint64_pack (*(int32_t *) member, scratch + rv);
      buffer->append (buffer, rv, scratch);
      break;
    case PROTOBUF_C_TYPE_INT64:
    case PROTOBUF_C_TYPE_UINT64:
      scratch[0] |= PROTOBUF_C_WIRE_TYPE_VARINT;
      rv += uint64_pack (*(uint64_t *) member, scratch + rv);
      buffer->append (buffer, rv, scratch);
      break;
    case PROTOBUF_C_TYPE_SFIXED32:
    case PROTOBUF_C_TYPE_FIXED32:
    case PROTOBUF_C_TYPE_FLOAT:
      scratch[0] |= PROTOBUF_C_WIRE_TYPE_32BIT;
      rv += fixed32_pack (*(uint64_t *) member, scratch + rv);
      buffer->append (buffer, rv, scratch);
      break;
    case PROTOBUF_C_TYPE_SFIXED64:
    case PROTOBUF_C_TYPE_FIXED64:
    case PROTOBUF_C_TYPE_DOUBLE:
      scratch[0] |= PROTOBUF_C_WIRE_TYPE_64BIT;
      rv += fixed64_pack (*(uint64_t *) member, scratch + rv);
      buffer->append (buffer, rv, scratch);
      break;
    case PROTOBUF_C_TYPE_BOOL:
      scratch[0] |= PROTOBUF_C_WIRE_TYPE_VARINT;
      rv += boolean_pack (*(protobuf_c_boolean *) member, scratch + rv);
      buffer->append (buffer, rv, scratch);
      break;
    case PROTOBUF_C_TYPE_STRING:
      {
        size_t sublen = strlen (*(char **) member);
        scratch[0] |= PROTOBUF_C_WIRE_TYPE_LENGTH_PREFIXED;
        rv += uint32_pack (sublen, scratch + rv);
        buffer->append (buffer, rv, scratch);
        buffer->append (buffer, sublen, *(BYTE**)member);
        rv += sublen;
        break;
      }
      
    case PROTOBUF_C_TYPE_BYTES:
      {
        const ProtobufCBinaryData * bd = ((ProtobufCBinaryData*) member);
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
        BYTE simple_buffer_scratch[256];
        size_t sublen;
        ProtobufCBufferSimple simple_buffer
          = PROTOBUF_C_BUFFER_SIMPLE_INIT (simple_buffer_scratch);
        scratch[0] |= PROTOBUF_C_WIRE_TYPE_LENGTH_PREFIXED;
        sublen = protobuf_c_message_pack_to_buffer (*(ProtobufCMessage **) member,
                                           &simple_buffer.base);
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
                               void *member,
                               ProtobufCBuffer *buffer)
{
  if (field->type == PROTOBUF_C_TYPE_MESSAGE
   || field->type == PROTOBUF_C_TYPE_STRING)
    {
      if (*(void**)member == NULL)
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
repeated_field_pack_to_buffer (const ProtobufCFieldDescriptor *field,
                               unsigned count,
                               void *member,
                               ProtobufCBuffer *buffer)
{
  char *array = * (char **) member;
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

size_t
protobuf_c_message_pack_to_buffer (const ProtobufCMessage *message,
                                   ProtobufCBuffer  *buffer)
{
  unsigned i;
  size_t rv = 0;
  for (i = 0; i < message->descriptor->n_fields; i++)
    {
      const ProtobufCFieldDescriptor *field = message->descriptor->fields + i;
      void *member = ((char *) message) + field->offset;
      void *qmember = ((char *) message) + field->quantifier_offset;

      if (field->label == PROTOBUF_C_LABEL_REQUIRED)
        rv += required_field_pack_to_buffer (field, member, buffer);
      else if (field->label == PROTOBUF_C_LABEL_OPTIONAL)
        rv += optional_field_pack_to_buffer (field, qmember, member, buffer);
      else
        rv += repeated_field_pack_to_buffer (field, * (size_t *) qmember, member, buffer);
    }

  return rv;
}

/* === unpacking === */
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
                        const BYTE *data,
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

typedef struct _ScannedMember ScannedMember;
struct _ScannedMember
{
  uint32_t tag;
  const ProtobufCFieldDescriptor *field;
  BYTE wire_type;
  BYTE length_prefix_len;
  size_t len;
  const unsigned char *data;
};

#define MESSAGE_GET_UNKNOWNS(message) \
  STRUCT_MEMBER_PTR (ProtobufCMessageUnknownFieldArray, \
                     (message), (message)->descriptor->unknown_field_array_offset)

static inline uint32_t
scan_length_prefixed_data (size_t len, const BYTE *data, size_t *prefix_len_out)
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
    return 0;
  hdr_len = i + 1;
  *prefix_len_out = hdr_len;
  if (hdr_len + val > len)
    return 0;
  return hdr_len + val;
}

static inline uint32_t
parse_uint32 (unsigned len, const BYTE *data)
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
static inline int32_t
unzigzag32 (uint32_t v)
{
  if (v&1)
    return -(v>>1) - 1;
  else
    return v>>1;
}
static inline uint32_t
parse_fixed_uint32 (const BYTE *data)
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
parse_uint64 (unsigned len, const BYTE *data)
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
parse_fixed_uint64 (const BYTE *data)
{
#if IS_LITTLE_ENDIAN
  uint32_t t;
  memcpy (&t, data, 8);
  return t;
#else
  return (uint64_t)parse_fixed_uint32 (data)
      | (((uint64_t)parse_fixed_uint32(data+4)) << 32);
#endif
}
static protobuf_c_boolean
parse_boolean (unsigned len, const BYTE *data)
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
  const BYTE *data = scanned_member->data;
  ProtobufCWireType wire_type = scanned_member->wire_type;
  switch (scanned_member->field->type)
    {
    case PROTOBUF_C_TYPE_INT32:
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
      break;

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
          FREE (allocator, *pstr);
        *pstr = ALLOC (allocator, len - pref_len + 1);
        memcpy (*pstr, data + pref_len, len - pref_len);
        (*pstr)[len-pref_len] = 0;
        return 1;
      }
    case PROTOBUF_C_TYPE_BYTES:
      if (wire_type != PROTOBUF_C_WIRE_TYPE_LENGTH_PREFIXED)
        return 0;
      {
        ProtobufCBinaryData *bd = member;
        unsigned pref_len = scanned_member->length_prefix_len;
        if (maybe_clear && bd->data != NULL)
          FREE (allocator, bd->data);
        bd->data = ALLOC (allocator, len - pref_len);
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
        unsigned pref_len = scanned_member->length_prefix_len;
        if (maybe_clear && *pmessage != NULL)
          protobuf_c_message_free_unpacked (*pmessage, allocator);
        subm = protobuf_c_message_unpack (scanned_member->field->descriptor,
                                          allocator,
                                          len - pref_len, data + pref_len);
        if (subm == NULL)
          return 0;
        *pmessage = subm;
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
      ufield->data = UNALIGNED_ALLOC (allocator, scanned_member->len);
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
      return parse_repeated_member (scanned_member, member, message, allocator);
    }
  PROTOBUF_C_ASSERT_NOT_REACHED ();
  return 0;
}


ProtobufCMessage *
protobuf_c_message_unpack         (const ProtobufCMessageDescriptor *desc,
                                   ProtobufCAllocator  *allocator,
                                   size_t               len,
                                   const unsigned char *data)
{
  ProtobufCMessage *rv;
  size_t rem = len;
  const BYTE *at = data;
  const ProtobufCFieldDescriptor *last_field = desc->fields + 0;
  ScannedMember first_member_slab[16];
  ScannedMember *scanned_member_slabs[30];               /* size of member i is (1<<(i+4)) */
  unsigned which_slab = 0;
  unsigned in_slab_index = 0;
  size_t n_unknown = 0;
  unsigned f;
  unsigned i_slab;

  if (allocator == NULL)
    allocator = &protobuf_c_default_allocator;
  rv = ALLOC (allocator, desc->sizeof_message);
  scanned_member_slabs[0] = first_member_slab;

  memset (rv, 0, desc->sizeof_message);
  rv->descriptor = desc;

  while (rem > 0)
    {
      uint32_t tag;
      ProtobufCWireType wire_type;
      size_t used = parse_tag_and_wiretype (rem, at, &tag, &wire_type);
      const ProtobufCFieldDescriptor *field;
      ScannedMember tmp;
      if (used == 0)
        goto error_cleanup;
      /* XXX: consider optimizing for field[1].id == tag, if field[1] exists! */
      if (last_field->id != tag)
        {
          /* lookup field */
          int rv = int_range_lookup (desc->n_field_ranges,
                                     desc->field_ranges,
                                     tag);
          if (rv < 0)
            {
              field = NULL;
              n_unknown++;
            }
          else
            {
              field = desc->fields + rv;
              last_field = field;
            }
        }
      else
        field = last_field;

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
              goto error_cleanup;
            tmp.len = i + 1;
          }
          break;
        case PROTOBUF_C_WIRE_TYPE_64BIT:
          if (rem < 8)
            goto error_cleanup;
          tmp.len = 8;
          break;
        case PROTOBUF_C_WIRE_TYPE_LENGTH_PREFIXED:
          {
            size_t pref_len;
            tmp.len = scan_length_prefixed_data (rem, at, &pref_len);
            if (tmp.len == 0)
              goto error_cleanup;
            tmp.length_prefix_len = pref_len;
            break;
          }
        case PROTOBUF_C_WIRE_TYPE_START_GROUP:
        case PROTOBUF_C_WIRE_TYPE_END_GROUP:
          PROTOBUF_C_ASSERT_NOT_REACHED ();
        case PROTOBUF_C_WIRE_TYPE_32BIT:
          if (rem < 4)
            goto error_cleanup;
          tmp.len = 4;
          break;
        }
      if (in_slab_index == (1U<<(which_slab+4)))
        {
          size_t size;
          in_slab_index = 0;
          which_slab++;
          size = sizeof(ScannedMember) << (which_slab+4);
          /* TODO: consider using alloca() ! */
          if (allocator->tmp_alloc != NULL)
            scanned_member_slabs[which_slab] = TMPALLOC(allocator, size);
          else
            scanned_member_slabs[which_slab] = ALLOC(allocator, size);
        }
      scanned_member_slabs[which_slab][in_slab_index++] = tmp;

      if (field != NULL && field->label == PROTOBUF_C_LABEL_REPEATED)
        {
          size_t *n_ptr = (size_t*)((char*)rv + field->quantifier_offset);
          (*n_ptr) += 1;
        }

      at += tmp.len;
      rem -= tmp.len;
    }

  /* allocate space for repeated fields */
  for (f = 0; f < desc->n_fields; f++)
    if (desc->fields[f].label == PROTOBUF_C_LABEL_REPEATED)
      {
        const ProtobufCFieldDescriptor *field = desc->fields + f;
        size_t siz = sizeof_elt_in_repeated_array (field->type);
        size_t *n_ptr = STRUCT_MEMBER_PTR (size_t, rv, field->quantifier_offset);
        if (*n_ptr != 0)
          {
            STRUCT_MEMBER (void *, rv, field->offset) = ALLOC (allocator, siz * (*n_ptr));
            *n_ptr = 0;
          }
      }

  /* allocate space for unknown fields */
  if (n_unknown)
    {
      rv->unknown_fields = ALLOC (allocator, n_unknown * sizeof (ProtobufCMessageUnknownField));
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
            goto error_cleanup;
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
}

/* === free_unpacked === */
void     
protobuf_c_message_free_unpacked  (ProtobufCMessage    *message,
                                   ProtobufCAllocator  *allocator)
{
  const ProtobufCMessageDescriptor *desc = message->descriptor;
  unsigned f;
  if (allocator == NULL)
    allocator = &protobuf_c_default_allocator;
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
          FREE (allocator, STRUCT_MEMBER (char *, message, desc->fields[f].offset));
        }
      else if (desc->fields[f].type == PROTOBUF_C_TYPE_BYTES)
        {
          FREE (allocator, STRUCT_MEMBER (ProtobufCBinaryData, message, desc->fields[f].offset).data);
        }
      else if (desc->fields[f].type == PROTOBUF_C_TYPE_MESSAGE)
        {
          protobuf_c_message_free_unpacked (STRUCT_MEMBER (ProtobufCMessage *, message,
                                                           desc->fields[f].offset), allocator);

        }
    }
  FREE (allocator, message);
}

/* === services === */
typedef struct _ServiceMachgen ServiceMachgen;
struct _ServiceMachgen
{
  ProtobufCService base;
  void *service;
};

typedef void (*DestroyHandler)(void *service);
typedef void (*GenericHandler)(void *service,
                               const ProtobufCMessage *input,
                               ProtobufCClosure  closure,
                               void             *closure_data);
static void 
service_machgen_invoke(ProtobufCService *service,
                       unsigned          method_index,
                       const ProtobufCMessage *input,
                       ProtobufCClosure  closure,
                       void             *closure_data)
{
  GenericHandler *handlers;
  GenericHandler handler;
  ServiceMachgen *machgen = (ServiceMachgen *) service;
  PROTOBUF_C_ASSERT (method_index < service->descriptor->n_methods);
  handlers = (GenericHandler *) machgen->service;
  handler = handlers[method_index];
  (*handler) (machgen->service, input, closure, closure_data);
}

static void
service_machgen_destroy (ProtobufCService *service)
{
  /* destroy function always follows the methods.
     we assume these function pointers are the same size. */
  DestroyHandler *handlers;
  DestroyHandler handler;
  ServiceMachgen *machgen = (ServiceMachgen *) service;
  handlers = (DestroyHandler *) machgen->service;
  handler = handlers[service->descriptor->n_methods];
  (*handler) (machgen->service);
  FREE (&protobuf_c_default_allocator, service);
}
ProtobufCService *
protobuf_c_create_service_from_vfuncs
                               (const ProtobufCServiceDescriptor *descriptor,
                                void *service)
{
  ServiceMachgen *rv = ALLOC (&protobuf_c_default_allocator, sizeof (ServiceMachgen));
  rv->base.descriptor = descriptor;
  rv->base.invoke = service_machgen_invoke;
  rv->base.destroy = service_machgen_destroy;
  rv->service = service;
  return &rv->base;
}

void protobuf_c_service_destroy (ProtobufCService *service)
{
  service->destroy (service);
}
