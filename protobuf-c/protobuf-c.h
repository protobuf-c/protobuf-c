/* --- protobuf-c.h: protobuf c runtime api --- */

/*
 * Copyright (c) 2008-2014, Dave Benson and the protobuf-c authors.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file
 * Support library definitions for \c protoc-c generated code.
 *
 * This file defines the public API used by the code generated
 * by \c protoc-c .
 *
 * \authors Dave Benson and the protobuf-c authors
 * \date 2008-2014
 *
 * \mainpage
 *
 * \section description Description
 *
 * Google protobufs are an efficient way to serialise and deserialise
 * data to send across the wire or to store on disk.  This library
 * supports the \c protoc-c generated C code.
 *
 * \sa
 *     - Google Protobufs: https://code.google.com/p/protobuf/
 *     - Protobuf docs:
 *       https://developers.google.com/protocol-buffers/docs/overview
 *     - Protobuf for C code: https://github.com/protobuf-c/protobuf-c
 */
#ifndef __PROTOBUF_C_RUNTIME_H_
#define __PROTOBUF_C_RUNTIME_H_

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>

#ifdef __cplusplus
# define PROTOBUF_C_BEGIN_DECLS	extern "C" {
# define PROTOBUF_C_END_DECLS	}
#else
# define PROTOBUF_C_BEGIN_DECLS
# define PROTOBUF_C_END_DECLS
#endif

#define PROTOBUF_C_ASSERT(condition) assert(condition)
#define PROTOBUF_C_ASSERT_NOT_REACHED() assert(0)

#if !defined(PROTOBUF_C_NO_DEPRECATED)
# if (__GNUC__ > 3) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
#  define PROTOBUF_C_DEPRECATED __attribute__((__deprecated__))
# endif
#else
# define PROTOBUF_C_DEPRECATED
#endif

#define PROTOBUF_C_OFFSETOF(struct, member) offsetof(struct, member)

#if defined(_WIN32) && defined(PROTOBUF_C_USE_SHARED_LIB)
# ifdef PROTOBUF_C_EXPORT
#  define PROTOBUF_C_API __declspec(dllexport)
# else
#  define PROTOBUF_C_API __declspec(dllimport)
# endif
#else
# define PROTOBUF_C_API
#endif

PROTOBUF_C_BEGIN_DECLS
/** \defgroup api Public API for libprotobuf-c
 *
 * The public API for C protobufs.  These interfaces are stable.
 *
 * <b>Packed Messages</b>
 *
 * To pack a message, you have two options:
 *
 *   -# you can compute the size of the message using
 *      protobuf_c_message_get_packed_size() then pass
 *      protobuf_c_message_pack() a buffer of that length.
 *   -# Provide a virtual buffer (a ProtobufCBuffer) to
 *      accept data as we scan through it.
 * @{ */

/** Get the version of the library.
 *
 * Get the version of the protobuf-c library. Note that this is the
 * version of the library linked against, not the version of the
 * headers compiled against.
 *
 * \return A string containing the version number of protobuf-c.
 */
PROTOBUF_C_API
const char *
protobuf_c_version(void);

/** Get numeric version of the library.
 *
 * Get the version of the protobuf-c library. Note that this is the
 * version of the library linked against, not the version of the
 * headers compiled against.
 *
 * \return A 32 bit unsigned integer containing the version number of
 *	   protobuf-c, represented in base-10 as
 *	   (MAJOR*1E6) + (MINOR*1E3) + PATCH.
 */
PROTOBUF_C_API
uint32_t
protobuf_c_version_number(void);

/** The version of the library as a #define.
 *
 * The version of the protobuf-c headers, represented as a string using
 * the same format as protobuf_c_version().
 */
#define PROTOBUF_C_VERSION		"1.0.0-pre"

/** The numeric version of the library as a #define.
 *
 * The version of the protobuf-c headers, represented as an integer
 * using the same format as protobuf_c_version_number().
 */
#define PROTOBUF_C_VERSION_NUMBER	1000000

/** Boolean type used in protobuf-c. */
typedef int protobuf_c_boolean;

/** Enumeration of the rules for this field.
 *
 * A field will be required, optional or repeated.  These enumerations
 * cover those options.
 *
 * \sa
 *   - Google protobuf docs on how to define a field:
 *     https://developers.google.com/protocol-buffers/docs/proto#simple
 *   - How to add new fields to a message:
 *     https://developers.google.com/protocol-buffers/docs/proto#updating
 */
typedef enum {
	PROTOBUF_C_LABEL_REQUIRED,
	PROTOBUF_C_LABEL_OPTIONAL,
	PROTOBUF_C_LABEL_REPEATED
} ProtobufCLabel;

/** Enumeration of the field types.
 *
 * This covers the list of all message types.
 *
 * \sa
 *   - Scalar types.  Further on enumeration and nested message types
 *     are covered.
 *     https://developers.google.com/protocol-buffers/docs/proto#scalar
 */
typedef enum {
	PROTOBUF_C_TYPE_INT32,    /**< int32 */
	PROTOBUF_C_TYPE_SINT32,   /**< signed int32 */
	PROTOBUF_C_TYPE_SFIXED32, /**< signed int32 */
	PROTOBUF_C_TYPE_INT64,    /**< int64 */
	PROTOBUF_C_TYPE_SINT64,   /**< signed int64 */
	PROTOBUF_C_TYPE_SFIXED64, /**< signed int64 */
	PROTOBUF_C_TYPE_UINT32,   /**< unsigned int32 */
	PROTOBUF_C_TYPE_FIXED32,  /**< unsigned int32 */
	PROTOBUF_C_TYPE_UINT64,   /**< unsigned int64 */
	PROTOBUF_C_TYPE_FIXED64,  /**< unsigned int64 */
	PROTOBUF_C_TYPE_FLOAT,    /**< float */
	PROTOBUF_C_TYPE_DOUBLE,   /**< double */
	PROTOBUF_C_TYPE_BOOL,     /**< boolean value */
	PROTOBUF_C_TYPE_ENUM,     /**< enumerated value */
	PROTOBUF_C_TYPE_STRING,   /**< string */
	PROTOBUF_C_TYPE_BYTES,    /**< array of bytes */
	/* PROTOBUF_C_TYPE_GROUP,	-- NOT SUPPORTED */
	PROTOBUF_C_TYPE_MESSAGE,  /**< Nested message. */
} ProtobufCType;

/** Data structure for \c bytes type.
 *
 * Byte fields might not be ASCII \c NUL terminated strings.  They might
 * have embedded \c NULs or they might not have a \c NUL at all.
 */
typedef struct _ProtobufCBinaryData ProtobufCBinaryData;
struct _ProtobufCBinaryData {
	size_t	len;
	uint8_t	*data;
};

typedef struct _ProtobufCIntRange ProtobufCIntRange; /* private */

/* --- memory management --- */
/** Functions for managing memory.
 *
 * Used by the protocol_c family of functions to manage memory.
 */
typedef struct _ProtobufCAllocator ProtobufCAllocator;
struct _ProtobufCAllocator
{
	void		*(*alloc)(void *allocator_data, size_t size);
	/**< Function to allocate memory. */
	void		(*free)(void *allocator_data, void *pointer);
	/**< Function to free memory. */
	void		*allocator_data;
	/**< Opaque buffer that the alloc and free functions
         * might use to save state. */
};

/** This is a configurable default allocator.
 *
 * By default, it uses the system allocator (meaning malloc() and free()).
 * This is typically changed to adapt to frameworks that provide
 * some nonstandard allocation functions.
 *
 * NOTE: you may modify this allocator.
 */
extern PROTOBUF_C_API ProtobufCAllocator protobuf_c_default_allocator; /* settable */

/* --- append-only data buffer --- */
/** Append-only data buffer.
 *
 * Used by the protobuf_c_message_pack_to_buffer() function to store
 * the packed message.
 */
typedef struct _ProtobufCBuffer ProtobufCBuffer;
struct _ProtobufCBuffer {
	void		(*append)(ProtobufCBuffer *buffer,
				  size_t len,
				  const uint8_t *data);
};

/* --- enums --- */

typedef struct _ProtobufCEnumValue ProtobufCEnumValue;
typedef struct _ProtobufCEnumValueIndex ProtobufCEnumValueIndex;
typedef struct _ProtobufCEnumDescriptor ProtobufCEnumDescriptor;

/** Represents a single value of an enumeration. */
struct _ProtobufCEnumValue {
	const char	*name;
	/**< is the string identifying this value, as given in the .proto
         * file. */
	const char	*c_name;
	/**< is the full name of the C enumeration value. */
	int		value;
	/**< is the number assigned to this value, as given in the .proto
         * file. */
};

/** Represents the enum as a whole, with all its values.
 *
 * The rest of the values are private essentially.
 *
 * See also: Use protobuf_c_enum_descriptor_get_value_by_name()
 * and protobuf_c_enum_descriptor_get_value() to efficiently
 * lookup values in the descriptor.
 */
struct _ProtobufCEnumDescriptor {
	uint32_t			magic;
	/**< Code we check to ensure that the api is used correctly. */

	const char			*name;
	/**< The qualified name (e.g. "namespace.Type"). */
	const char			*short_name;
	/**< The unqualified name ("Type"), as given in the .proto file. */
	const char			*c_name;
	const char			*package_name;
	/**< The '.'-separated namespace */

	/* sorted by value */
	unsigned			n_values;
	/**< The number of distinct values. */
	const ProtobufCEnumValue	*values;
	/**< The array of distinct values. */

	/* sorted by name */
	unsigned			n_value_names;
	/**< Number of named values (including aliases). */
	const ProtobufCEnumValueIndex	*values_by_name;
	/**< Are the named values (including aliases). */

	/* value-ranges, for faster lookups by number */
	unsigned			n_value_ranges;
	/**< Number of value ranges. */
	const ProtobufCIntRange		*value_ranges;
	/**< Value ranges. */

	void				*reserved1;
	/**< Reserved for future use in the implementation. */
	void				*reserved2;
	/**< Reserved for future use in the implementation. */
	void				*reserved3;
	/**< Reserved for future use in the implementation. */
	void				*reserved4;
	/**< Reserved for future use in the implementation. */
};

/* --- messages --- */

typedef struct _ProtobufCMessageDescriptor ProtobufCMessageDescriptor;
typedef struct _ProtobufCFieldDescriptor ProtobufCFieldDescriptor;
typedef struct _ProtobufCMessage ProtobufCMessage;
typedef void (*ProtobufCMessageInit)(ProtobufCMessage *);

/** Description of a single field in a message. */

struct _ProtobufCFieldDescriptor {
	const char		*name;
	/**< Name of the field, as given in the .proto file. */

	uint32_t		id;
	/**< Code representing the field, as given in the .proto file. */
	ProtobufCLabel		label;
	/**< One of \c PROTOBUF_C_LABEL_{REQUIRED,OPTIONAL,REPEATED} */
	ProtobufCType		type;
	/**< The type of field. */
	unsigned		quantifier_offset;
	/**< The offset in bytes into the message's C structure's
         * \c has_MEMBER field (for optional members) or
         * \c n_MEMBER field (for repeated members). */
	unsigned		offset;
	/**< The offset in bytes into the message's C structure
         * for the member itself. */
	const void		*descriptor; /* for MESSAGE and ENUM types */
	/**< Pointer to a \c ProtobufC{Enum,Message}Descriptor if type
         * is \c PROTOBUF_C_TYPE_{ENUM,MESSAGE} respectively, otherwise
         * NULL. */
	const void		*default_value; /* can be NULL */
	/**< pointer to a default value for this field, where allowed. */
	uint32_t		flags;
	/**< A flag word.  Zero or more of the bits defined in the
         * \c ProtobufCFieldFlag enum may be set. */

	unsigned		reserved_flags;
	/**< Reserved for future use in the implementation. */
	void			*reserved2;
	/**< Reserved for future use in the implementation. */
	void			*reserved3;
	/**< Reserved for future use in the implementation. */
};

/** Field flag types. */
typedef enum {
	PROTOBUF_C_FIELD_FLAG_PACKED		= (1 << 0),
	/**< Set if the field is repeated and marked with the
         * 'packed' option. */
	PROTOBUF_C_FIELD_FLAG_DEPRECATED	= (1 << 1),
	/**< Set if the field is marked with the 'deprecated' option. */
} ProtobufCFieldFlag;

/** Description of a message.
 *
 * Describes the components of a specific message.
 *
 */
struct _ProtobufCMessageDescriptor {
	uint32_t			magic;
	/**< Code we check to ensure that the api is used correctly. */

	const char			*name;
	/**< The qualified name (e.g. "namespace.Type"). */
	const char			*short_name;
	/**< The unqualified name ("Type"), as given in the .proto
         * file. */
	const char			*c_name;
	/**< The c-formatted name of the structure */
	const char			*package_name;
	/**< The '.'-separated namespace */

	size_t				sizeof_message;
	/**< The size in bytes of the C structure representing an
         * instance of this type of message. */

	/* sorted by field-id */
	unsigned			n_fields;
	/**< The number of known fields in this message. */
	const ProtobufCFieldDescriptor	*fields;
	/**< The fields sorted by id number. */
	const unsigned			*fields_sorted_by_name;
	/**< Used for looking up fields by name. (private) */

	/* ranges, optimization for looking up fields */
	unsigned			n_field_ranges;
	/**< Number of field ranges. (private) */
	const ProtobufCIntRange		*field_ranges;
	/**< Used for looking up fields by id. (private) */

	ProtobufCMessageInit		message_init;
	/** Function to initialise the message. */
	void				*reserved1;
	/**< Reserved for future use in the implementation. */
	void				*reserved2;
	/**< Reserved for future use in the implementation. */
	void				*reserved3;
	/**< Reserved for future use in the implementation. */
};

typedef struct _ProtobufCMessageUnknownField ProtobufCMessageUnknownField;
/** An instance of a message.
 *
 * \c ProtobufCMessage is sort of a light-weight base class for all
 * messages.
 *
 * In particular, \c ProtobufCMessage doesn't have any allocation policy
 * associated with it. That's because it's common to create
 * \c ProtobufCMessage's on the stack. In fact, that's what we recommend
 * for sending messages (because if you just allocate from the stack,
 * then you can't really have a memory leak).
 *
 * This means that functions like protobuf_c_message_unpack() which return
 * a \c ProtobufCMessage must be paired with a free function, like
 * protobuf_c_message_free_unpacked().
 */
struct _ProtobufCMessage {
	const ProtobufCMessageDescriptor	*descriptor;
	/**< The locations and types of the members of message. */
	unsigned				n_unknown_fields;
	/**< The number of fields we didn't recognize. */
	ProtobufCMessageUnknownField		*unknown_fields;
	/**< The fields we didn't recognize. */
};

/** Initialise message. */
#define PROTOBUF_C_MESSAGE_INIT(descriptor) { descriptor, 0, NULL }

/** Get the size of the serialised message.
 *
 * Determine the number of bytes required to store the serialised message.
 *
 * \param[in] message Message to serialise.
 * \return Bytes required to store the serialised message.
 */
PROTOBUF_C_API size_t
protobuf_c_message_get_packed_size(const ProtobufCMessage *message);

/** Serialise the message.
 *
 * Note: The out parameter must have enough space to store the packed
 * message.  Use protobuf_c_message_get_packed_size() to determine the
 * size.
 *
 * \param[in] message Message to serialise.
 * \param[in,out] out Buffer to store the serialised message in.
 * \return Bytes used for the serialised message.
 */
PROTOBUF_C_API size_t
protobuf_c_message_pack(const ProtobufCMessage *message, uint8_t *out);

/** Serialise the message.
 *
 * This function will allocate the space in a buffer required to store
 * the message.
 *
 * \param[in] message Message to serialise.
 * \param[in,out] out Buffer to store the serialised message in.
 * \return Bytes used for the serialised message.
 */
PROTOBUF_C_API size_t
protobuf_c_message_pack_to_buffer(const ProtobufCMessage *message,
    ProtobufCBuffer *out);

PROTOBUF_C_API ProtobufCMessage *
protobuf_c_message_unpack(
	const ProtobufCMessageDescriptor *,
	ProtobufCAllocator *,
	size_t len,
	const uint8_t *data);

PROTOBUF_C_API void
protobuf_c_message_free_unpacked(ProtobufCMessage *, ProtobufCAllocator *);

PROTOBUF_C_API protobuf_c_boolean
protobuf_c_message_check(const ProtobufCMessage *);

/** Initialise message.
 *
 * WARNING: 'message' must be a block of memory of size
 * descriptor->sizeof_message.
 *
 * \param[in] descriptor Message descriptor.
 * \param[in,out] message Buffer where the \c ProtobufCMessage will be
 *                        stored.
 */
PROTOBUF_C_API void
protobuf_c_message_init(const ProtobufCMessageDescriptor *descriptor,
    void *message);

/* --- services --- */

typedef struct _ProtobufCMethodDescriptor ProtobufCMethodDescriptor;
struct _ProtobufCMethodDescriptor {
	const char				*name;
	const ProtobufCMessageDescriptor	*input;
	const ProtobufCMessageDescriptor	*output;
};

typedef struct _ProtobufCServiceDescriptor ProtobufCServiceDescriptor;
struct _ProtobufCServiceDescriptor {
	uint32_t			magic;

	const char			*name;
	const char			*short_name;
	const char			*c_name;
	const char			*package;
	unsigned			n_methods;
	const ProtobufCMethodDescriptor	*methods; /* in order from .proto file */
	const unsigned			*method_indices_by_name;
};

typedef void (*ProtobufCClosure)(const ProtobufCMessage *, void *closure_data);

typedef struct _ProtobufCService ProtobufCService;
struct _ProtobufCService {
	const ProtobufCServiceDescriptor *descriptor;
	void (*invoke)(ProtobufCService *service,
		       unsigned method_index,
		       const ProtobufCMessage *input,
		       ProtobufCClosure closure,
		       void *closure_data);
	void (*destroy)(ProtobufCService *service);
};

PROTOBUF_C_API void
protobuf_c_service_destroy(ProtobufCService *);

/* --- querying the descriptors --- */

PROTOBUF_C_API const ProtobufCEnumValue *
protobuf_c_enum_descriptor_get_value_by_name(
	const ProtobufCEnumDescriptor *desc,
	const char *name);

PROTOBUF_C_API const ProtobufCEnumValue *
protobuf_c_enum_descriptor_get_value(
	const ProtobufCEnumDescriptor *desc,
	int value);

PROTOBUF_C_API const ProtobufCFieldDescriptor *
protobuf_c_message_descriptor_get_field_by_name(
	const ProtobufCMessageDescriptor *desc,
	const char *name);

PROTOBUF_C_API const ProtobufCFieldDescriptor *
protobuf_c_message_descriptor_get_field(
	const ProtobufCMessageDescriptor *desc,
	unsigned value);

PROTOBUF_C_API const ProtobufCMethodDescriptor *
protobuf_c_service_descriptor_get_method_by_name(
	const ProtobufCServiceDescriptor *desc,
	const char *name);

/* --- wire format enums --- */

typedef enum {
	PROTOBUF_C_WIRE_TYPE_VARINT,
	PROTOBUF_C_WIRE_TYPE_64BIT,
	PROTOBUF_C_WIRE_TYPE_LENGTH_PREFIXED,
	PROTOBUF_C_WIRE_TYPE_START_GROUP,	/* unsupported */
	PROTOBUF_C_WIRE_TYPE_END_GROUP,		/* unsupported */
	PROTOBUF_C_WIRE_TYPE_32BIT
} ProtobufCWireType;

/* --- unknown message fields --- */

struct _ProtobufCMessageUnknownField {
	uint32_t		tag;
	ProtobufCWireType	wire_type;
	size_t			len;
	uint8_t			*data;
};

/* --- extra (superfluous) api: trivial buffer --- */

typedef struct _ProtobufCBufferSimple ProtobufCBufferSimple;
struct _ProtobufCBufferSimple {
	ProtobufCBuffer		base;
	size_t			alloced;
	size_t			len;
	uint8_t			*data;
	protobuf_c_boolean	must_free_data;
};

#define PROTOBUF_C_BUFFER_SIMPLE_INIT(array_of_bytes)			\
{									\
	{ protobuf_c_buffer_simple_append },				\
	sizeof(array_of_bytes),						\
	0,								\
	(array_of_bytes),						\
	0								\
}

#define PROTOBUF_C_BUFFER_SIMPLE_CLEAR(simp_buf)			\
do {									\
	if ((simp_buf)->must_free_data) {				\
		protobuf_c_default_allocator.free(			\
			&protobuf_c_default_allocator,			\
			(simp_buf)->data);				\
	}								\
} while (0)

/** @} */  /* End of api group. */

/** \defgroup cgapi Codegen API
 *
 * Available for users of \c libprotobuf-c but tied to the current
 * implementation of the compiler; might change in the future.
 * @{ */
#define PROTOBUF_C_SERVICE_DESCRIPTOR_MAGIC	0x14159bc3
#define PROTOBUF_C_MESSAGE_DESCRIPTOR_MAGIC	0x28aaeef9
#define PROTOBUF_C_ENUM_DESCRIPTOR_MAGIC	0x114315af

#ifndef _PROTOBUF_C_FORCE_ENUM_TO_BE_INT_SIZE
/** Make sure enum isn't too big.
 *
 * A little enum helper macro: this will ensure that your enum's size
 * is * sizeof(int). In protobuf, it needs to not be larger than
 * 32-bits. This is written assuming it is appended to a list w/o a
 * tail comma.
 */
 #define _PROTOBUF_C_FORCE_ENUM_TO_BE_INT_SIZE(enum_name) \
  , _##enum_name##_IS_INT_SIZE = INT_MAX
#endif

/* === needs to be declared for the PROTOBUF_C_BUFFER_SIMPLE_INIT macro === */

void
protobuf_c_buffer_simple_append(
	ProtobufCBuffer *buffer,
	size_t len,
	const unsigned char *data);

/* === stuff which needs to be declared for use in the generated code === */

struct _ProtobufCEnumValueIndex {
	const char	*name;
	unsigned	index; /* into values[] array */
};

/** Helper structure for optimizing int => index lookups.
 *
 * Helper structure for optimizing int => index lookups in the case
 * where the keys are mostly consecutive values, as they presumably are for
 * enums and fields.
 *
 * The data structures assumes that the values in the original array are
 * sorted.
 *
 * NOTE: the number of values in the range can be inferred by looking
 * at the next element's orig_index. A dummy element is added to make
 * this simple.
 */
struct _ProtobufCIntRange {
	int		start_value;
        /**< TODO */
	unsigned	orig_index;
        /**< TODO */
};

/* === declared for exposition on ProtobufCIntRange === */

/** Lookup int ranges.
 *
 * Note: ranges must have an extra sentinel IntRange at the end whose
 * orig_index is set to the number of actual values in the original array.
 * Returns -1 if no orig_index found.
 */
int
protobuf_c_int_ranges_lookup(unsigned n_ranges, ProtobufCIntRange *ranges);

/* === behind the scenes on the generated service's __init functions */

typedef void (*ProtobufCServiceDestroy)(ProtobufCService *);

void
protobuf_c_service_generated_init(
	ProtobufCService *service,
	const ProtobufCServiceDescriptor *descriptor,
	ProtobufCServiceDestroy destroy);

void 
protobuf_c_service_invoke_internal(
	ProtobufCService *service,
	unsigned method_index,
	const ProtobufCMessage *input,
	ProtobufCClosure closure,
	void *closure_data);

PROTOBUF_C_END_DECLS

/** @} */  /* End of cgapi group. */

#endif /* __PROTOBUF_C_RUNTIME_H_ */
