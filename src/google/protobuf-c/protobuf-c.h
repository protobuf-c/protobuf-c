/* --- protobuf-c.h: public protobuf c runtime api --- */

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

#ifndef __PROTOBUF_C_RUNTIME_H_
#define __PROTOBUF_C_RUNTIME_H_

#include <stddef.h>
#include <assert.h>

#ifdef __cplusplus
# define PROTOBUF_C_BEGIN_DECLS    extern "C" {
# define PROTOBUF_C_END_DECLS      }
#else
# define PROTOBUF_C_BEGIN_DECLS
# define PROTOBUF_C_END_DECLS
#endif

#if !defined(PROTOBUF_C_NO_DEPRECATED) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
#define PROTOBUF_C_DEPRECATED __attribute__((__deprecated__))
#else
#define PROTOBUF_C_DEPRECATED
#endif

/* Define int32_t, int64_t, uint32_t, uint64_t, uint8_t.

   Usually, just include <inttypes.h> to do the work.
   XXX: should we use stdint.h?
 */
#ifndef PROTOBUF_C_SKIP_INTTYPES_H
#  if defined(_MSC_VER)
     /* On windows, in ms visual studio, define the types ourselves */
#    define int32_t      signed __int32
#    define INT32_MIN    _I32_MIN
#    define INT32_MAX    _I32_MAX
#    define uint32_t     unsigned __int32
#    define UINT32_MIN   _UI32_MIN
#    define UINT32_MAX   _UI32_MAX
#    define int64_t      signed __int64
#    define INT64_MIN    _I64_MIN
#    define INT64_MAX    _I64_MAX
#    define uint64_t     unsigned __int64
#    define UINT64_MIN   _UI64_MIN
#    define UINT64_MAX   _UI64_MAX
#    define uint8_t      unsigned char
#  else
     /* Use the system inttypes.h */
#    include <inttypes.h>
#  endif
#endif

#if defined(_WIN32) && defined(PROTOBUF_C_USE_SHARED_LIB)

# ifdef PROTOBUF_C_EXPORT
# define PROTOBUF_C_API __declspec(dllexport)
# else
# define PROTOBUF_C_API __declspec(dllimport)
#endif

#else
# define PROTOBUF_C_API
#endif

PROTOBUF_C_BEGIN_DECLS

typedef enum
{
  PROTOBUF_C_LABEL_REQUIRED,
  PROTOBUF_C_LABEL_OPTIONAL,
  PROTOBUF_C_LABEL_REPEATED
} ProtobufCLabel;

typedef enum
{
  PROTOBUF_C_TYPE_INT32,
  PROTOBUF_C_TYPE_SINT32,
  PROTOBUF_C_TYPE_SFIXED32,
  PROTOBUF_C_TYPE_INT64,
  PROTOBUF_C_TYPE_SINT64,
  PROTOBUF_C_TYPE_SFIXED64,
  PROTOBUF_C_TYPE_UINT32,
  PROTOBUF_C_TYPE_FIXED32,
  PROTOBUF_C_TYPE_UINT64,
  PROTOBUF_C_TYPE_FIXED64,
  PROTOBUF_C_TYPE_FLOAT,
  PROTOBUF_C_TYPE_DOUBLE,
  PROTOBUF_C_TYPE_BOOL,
  PROTOBUF_C_TYPE_ENUM,
  PROTOBUF_C_TYPE_STRING,
  PROTOBUF_C_TYPE_BYTES,
  //PROTOBUF_C_TYPE_GROUP,          // NOT SUPPORTED
  PROTOBUF_C_TYPE_MESSAGE,
} ProtobufCType;


typedef int protobuf_c_boolean;
#define PROTOBUF_C_OFFSETOF(struct, member) offsetof(struct, member)

#define PROTOBUF_C_ASSERT(condition) assert(condition)
#define PROTOBUF_C_ASSERT_NOT_REACHED() assert(0)

typedef struct _ProtobufCBinaryData ProtobufCBinaryData;
struct _ProtobufCBinaryData
{
  size_t len;
  uint8_t *data;
};

typedef struct _ProtobufCIntRange ProtobufCIntRange; /* private */

/* --- memory management --- */
typedef struct _ProtobufCAllocator ProtobufCAllocator;
struct _ProtobufCAllocator
{
  void *(*alloc)(void *allocator_data, size_t size);
  void (*free)(void *allocator_data, void *pointer);
  void *(*tmp_alloc)(void *allocator_data, size_t size);
  unsigned max_alloca;
  void *allocator_data;
};

/* This is a configurable allocator.
 * By default, it uses the system allocator (meaning malloc() and free()).
 * This is typically changed to adapt to frameworks that provide
 * some nonstandard allocation functions.
 *
 * NOTE: you may modify this allocator.
 */
extern PROTOBUF_C_API ProtobufCAllocator protobuf_c_default_allocator; /* settable */

/* This is the system allocator, meaning it uses malloc() and free().
 *
 * NOTE: please do NOT modify this allocator.
 */
extern PROTOBUF_C_API ProtobufCAllocator protobuf_c_system_allocator;  /* use malloc, free etc */

/* This is the function that our default allocators call when they 
   run out-of-memory.  The default behavior of this function is to
   terminate your program. */
extern PROTOBUF_C_API void (*protobuf_c_out_of_memory) (void);

/* --- append-only data buffer --- */
typedef struct _ProtobufCBuffer ProtobufCBuffer;
struct _ProtobufCBuffer
{
  void (*append)(ProtobufCBuffer     *buffer,
                 size_t               len,
                 const uint8_t       *data);
};
/* --- enums --- */
typedef struct _ProtobufCEnumValue ProtobufCEnumValue;
typedef struct _ProtobufCEnumValueIndex ProtobufCEnumValueIndex;
typedef struct _ProtobufCEnumDescriptor ProtobufCEnumDescriptor;

/* ProtobufCEnumValue:  this represents a single value of
 * an enumeration.
 * 'name' is the string identifying this value, as given in the .proto file.
 * 'c_name' is the full name of the C enumeration value.
 * 'value' is the number assigned to this value, as given in the .proto file.
 */
struct _ProtobufCEnumValue
{
  const char *name;
  const char *c_name;
  int value;
};

/* ProtobufCEnumDescriptor: the represents the enum as a whole,
 * with all its values.
 * 'magic' is a code we check to ensure that the api is used correctly.
 * 'name' is the qualified name (e.g. "namespace.Type").
 * 'short_name' is the unqualified name ("Type"), as given in the .proto file.
 * 'package_name' is the '.'-separated namespace
 * 'n_values' is the number of distinct values.
 * 'values' is the array of distinct values.
 * 'n_value_names' number of named values (including aliases).
 * 'value_names' are the named values (including aliases).
 *
 * The rest of the values are private essentially.
 *
 * see also: Use protobuf_c_enum_descriptor_get_value_by_name()
 * and protobuf_c_enum_descriptor_get_value() to efficiently
 * lookup values in the descriptor.
 */
struct _ProtobufCEnumDescriptor
{
  uint32_t magic;

  const char *name;
  const char *short_name;
  const char *c_name;
  const char *package_name;

  /* sorted by value */
  unsigned n_values;
  const ProtobufCEnumValue *values;

  /* sorted by name */
  unsigned n_value_names;
  const ProtobufCEnumValueIndex *values_by_name;

  /* value-ranges, for faster lookups by number */
  unsigned n_value_ranges;
  const ProtobufCIntRange *value_ranges;

  void *reserved1;
  void *reserved2;
  void *reserved3;
  void *reserved4;
};

/* --- messages --- */
typedef struct _ProtobufCMessageDescriptor ProtobufCMessageDescriptor;
typedef struct _ProtobufCFieldDescriptor ProtobufCFieldDescriptor;
typedef struct _ProtobufCMessage ProtobufCMessage;
typedef void (*ProtobufCMessageInit)(ProtobufCMessage *);
/* ProtobufCFieldDescriptor: description of a single field
 * in a message.
 * 'name' is the name of the field, as given in the .proto file.
 * 'id' is the code representing the field, as given in the .proto file.
 * 'label' is one of PROTOBUF_C_LABEL_{REQUIRED,OPTIONAL,REPEATED}
 * 'type' is the type of field.
 * 'quantifier_offset' is the offset in bytes into the message's C structure
 *        for this member's "has_MEMBER" field (for optional members) or
 *        "n_MEMBER" field (for repeated members).
 * 'offset' is the offset in bytes into the message's C structure
 *        for the member itself.
 * 'descriptor' is a pointer to a ProtobufC{Enum,Message}Descriptor
 *        if type is PROTOBUF_C_TYPE_{ENUM,MESSAGE} respectively,
 *        otherwise NULL.
 * 'default_value' is a pointer to a default value for this field,
 *        where allowed.
 */
struct _ProtobufCFieldDescriptor
{
  const char *name;
  uint32_t id;
  ProtobufCLabel label;
  ProtobufCType type;
  unsigned quantifier_offset;
  unsigned offset;
  const void *descriptor;   /* for MESSAGE and ENUM types */
  const void *default_value;   /* or NULL if no default-value */
  protobuf_c_boolean packed;

  unsigned reserved_flags;
  void *reserved2;
  void *reserved3;
};
/* ProtobufCMessageDescriptor: description of a message.
 *
 * 'magic' is a code we check to ensure that the api is used correctly.
 * 'name' is the qualified name (e.g. "namespace.Type").
 * 'short_name' is the unqualified name ("Type"), as given in the .proto file.
 * 'c_name' is the c-formatted name of the structure
 * 'package_name' is the '.'-separated namespace
 * 'sizeof_message' is the size in bytes of the C structure
 *        representing an instance of this type of message.
 * 'n_fields' is the number of known fields in this message.
 * 'fields' is the fields sorted by id number.
 * 'fields_sorted_by_name', 'n_field_ranges' and 'field_ranges'
 *       are used for looking up fields by name and id. (private)
 */
struct _ProtobufCMessageDescriptor
{
  uint32_t magic;

  const char *name;
  const char *short_name;
  const char *c_name;
  const char *package_name;

  size_t sizeof_message;

  /* sorted by field-id */
  unsigned n_fields;
  const ProtobufCFieldDescriptor *fields;
  const unsigned *fields_sorted_by_name;

  /* ranges, optimization for looking up fields */
  unsigned n_field_ranges;
  const ProtobufCIntRange *field_ranges;

  ProtobufCMessageInit message_init;
  void *reserved1;
  void *reserved2;
  void *reserved3;
};


/* ProtobufCMessage: an instance of a message.
 *
 * ProtobufCMessage is sort-of a lightweight
 * base-class for all messages.
 * 
 * In particular, ProtobufCMessage doesn't have
 * any allocation policy associated with it.
 * That's because it is common to create ProtobufCMessage's
 * on the stack.  In fact, we that's what we recommend
 * for sending messages (because if you just allocate from the
 * stack, then you can't really have a memory leak).
 *
 * This means that functions like protobuf_c_message_unpack()
 * which return a ProtobufCMessage must be paired
 * with a free function, like protobuf_c_message_free_unpacked().
 *
 * 'descriptor' gives the locations and types of the members of message
 * 'n_unknown_fields' is the number of fields we didn't recognize.
 * 'unknown_fields' are fields we didn't recognize.
 */
typedef struct _ProtobufCMessageUnknownField ProtobufCMessageUnknownField;
struct _ProtobufCMessage
{
  const ProtobufCMessageDescriptor *descriptor;
  unsigned n_unknown_fields;
  ProtobufCMessageUnknownField *unknown_fields;
};
#define PROTOBUF_C_MESSAGE_INIT(descriptor) { descriptor, 0, NULL }

/* To pack a message: you have two options:
   (1) you can compute the size of the message
       using protobuf_c_message_get_packed_size() 
       then pass protobuf_c_message_pack() a buffer of
       that length.
   (2) Provide a virtual buffer (a ProtobufCBuffer) to
       accept data as we scan through it.
 */
PROTOBUF_C_API size_t    protobuf_c_message_get_packed_size(const ProtobufCMessage *message);
PROTOBUF_C_API size_t    protobuf_c_message_pack           (const ProtobufCMessage *message,
                                             uint8_t                *out);
PROTOBUF_C_API size_t    protobuf_c_message_pack_to_buffer (const ProtobufCMessage *message,
                                             ProtobufCBuffer  *buffer);

PROTOBUF_C_API ProtobufCMessage *
          protobuf_c_message_unpack         (const ProtobufCMessageDescriptor *,
                                             ProtobufCAllocator  *allocator,
                                             size_t               len,
                                             const uint8_t       *data);
PROTOBUF_C_API void      protobuf_c_message_free_unpacked  (ProtobufCMessage    *message,
                                             ProtobufCAllocator  *allocator);

/* WARNING: 'message' must be a block of memory 
   of size descriptor->sizeof_message. */
PROTOBUF_C_API void      protobuf_c_message_init           (const ProtobufCMessageDescriptor *,
                                             void                *message);

/* --- services --- */
typedef struct _ProtobufCMethodDescriptor ProtobufCMethodDescriptor;
typedef struct _ProtobufCServiceDescriptor ProtobufCServiceDescriptor;

struct _ProtobufCMethodDescriptor
{
  const char *name;
  const ProtobufCMessageDescriptor *input;
  const ProtobufCMessageDescriptor *output;
};
struct _ProtobufCServiceDescriptor
{
  uint32_t magic;

  const char *name;
  const char *short_name;
  const char *c_name;
  const char *package;
  unsigned n_methods;
  const ProtobufCMethodDescriptor *methods;	/* in order from .proto file */
  const unsigned *method_indices_by_name;
};

typedef struct _ProtobufCService ProtobufCService;
typedef void (*ProtobufCClosure)(const ProtobufCMessage *message,
                                 void                   *closure_data);
struct _ProtobufCService
{
  const ProtobufCServiceDescriptor *descriptor;
  void (*invoke)(ProtobufCService *service,
                 unsigned          method_index,
                 const ProtobufCMessage *input,
                 ProtobufCClosure  closure,
                 void             *closure_data);
  void (*destroy) (ProtobufCService *service);
};


void protobuf_c_service_destroy (ProtobufCService *);


/* --- querying the descriptors --- */
PROTOBUF_C_API const ProtobufCEnumValue *
protobuf_c_enum_descriptor_get_value_by_name 
                         (const ProtobufCEnumDescriptor    *desc,
                          const char                       *name);
PROTOBUF_C_API const ProtobufCEnumValue *
protobuf_c_enum_descriptor_get_value        
                         (const ProtobufCEnumDescriptor    *desc,
                          int                               value);
PROTOBUF_C_API const ProtobufCFieldDescriptor *
protobuf_c_message_descriptor_get_field_by_name
                         (const ProtobufCMessageDescriptor *desc,
                          const char                       *name);
PROTOBUF_C_API const ProtobufCFieldDescriptor *
protobuf_c_message_descriptor_get_field        
                         (const ProtobufCMessageDescriptor *desc,
                          unsigned                          value);
PROTOBUF_C_API const ProtobufCMethodDescriptor *
protobuf_c_service_descriptor_get_method_by_name
                         (const ProtobufCServiceDescriptor *desc,
                          const char                       *name);

/* --- wire format enums --- */
typedef enum
{
  PROTOBUF_C_WIRE_TYPE_VARINT,
  PROTOBUF_C_WIRE_TYPE_64BIT,
  PROTOBUF_C_WIRE_TYPE_LENGTH_PREFIXED,
  PROTOBUF_C_WIRE_TYPE_START_GROUP,     /* unsupported */
  PROTOBUF_C_WIRE_TYPE_END_GROUP,       /* unsupported */
  PROTOBUF_C_WIRE_TYPE_32BIT
} ProtobufCWireType;

/* --- unknown message fields --- */
struct _ProtobufCMessageUnknownField
{
  uint32_t tag;
  ProtobufCWireType wire_type;
  size_t len;
  uint8_t *data;
};

/* --- extra (superfluous) api:  trivial buffer --- */
typedef struct _ProtobufCBufferSimple ProtobufCBufferSimple;
struct _ProtobufCBufferSimple
{
  ProtobufCBuffer base;
  size_t alloced;
  size_t len;
  uint8_t *data;
  protobuf_c_boolean must_free_data;
};
#define PROTOBUF_C_BUFFER_SIMPLE_INIT(array_of_bytes) \
{ { protobuf_c_buffer_simple_append }, \
  sizeof(array_of_bytes), 0, (array_of_bytes), 0 }
#define PROTOBUF_C_BUFFER_SIMPLE_CLEAR(simp_buf) \
  do { if ((simp_buf)->must_free_data) \
         protobuf_c_default_allocator.free (&protobuf_c_default_allocator.allocator_data, (simp_buf)->data); } while (0)


typedef enum
{
  PROTOBUF_C_CTYPE_INT32,
  PROTOBUF_C_CTYPE_UINT32,
  PROTOBUF_C_CTYPE_INT64,
  PROTOBUF_C_CTYPE_UINT64,
  PROTOBUF_C_CTYPE_FLOAT,
  PROTOBUF_C_CTYPE_DOUBLE,
  PROTOBUF_C_CTYPE_BOOL,
  PROTOBUF_C_CTYPE_ENUM,
  PROTOBUF_C_CTYPE_STRING,
  PROTOBUF_C_CTYPE_BYTES,
  PROTOBUF_C_CTYPE_MESSAGE,
} ProtobufCCType;

extern ProtobufCCType protobuf_c_type_to_ctype (ProtobufCType type);
#define protobuf_c_type_to_ctype(type) \
  ((ProtobufCCType)(protobuf_c_type_to_ctype_array[(type)]))

/* ====== private ====== */
#include "protobuf-c-private.h"


PROTOBUF_C_END_DECLS

#endif /* __PROTOBUF_C_RUNTIME_H_ */
