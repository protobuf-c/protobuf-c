/* --- protobuf-c-private.h: private structures and functions --- */
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


/* A little enum helper macro:  this will ensure that your
   enum's size is sizeof(int).  In protobuf, it need not
   be larger than 32-bits.
 
   This is written assuming it is appended to a list w/o a tail comma. */
#ifndef _PROTOBUF_C_FORCE_ENUM_TO_BE_INT_SIZE
  #define _PROTOBUF_C_FORCE_ENUM_TO_BE_INT_SIZE(enum_name) \
    , _##enum_name##_IS_INT_SIZE = INT_MAX
#endif

/* === needs to be declared for the PROTOBUF_C_BUFFER_SIMPLE_INIT macro === */

void protobuf_c_buffer_simple_append (ProtobufCBuffer *buffer,
                                      size_t           len,
                                      const unsigned char *data);

/* === stuff which needs to be declared for use in the generated code === */

struct _ProtobufCEnumValueIndex
{
  const char *name;
  unsigned index;               /* into values[] array */
};

/* IntRange: helper structure for optimizing
     int => index lookups
   in the case where the keys are mostly consecutive values,
   as they presumably are for enums and fields.

   The data structures assumes that the values in the original
   array are sorted */
struct _ProtobufCIntRange
{
  int start_value;
  unsigned orig_index;
  /* NOTE: the number of values in the range can
     be inferred by looking at the next element's orig_index.
     a dummy element is added to make this simple */
};


/* === declared for exposition on ProtobufCIntRange === */
/* note: ranges must have an extra sentinel IntRange at the end whose
   orig_index is set to the number of actual values in the original array */
/* returns -1 if no orig_index found */
int protobuf_c_int_ranges_lookup (unsigned n_ranges,
                                  ProtobufCIntRange *ranges);

#define PROTOBUF_C_SERVICE_DESCRIPTOR_MAGIC  0x14159bc3
#define PROTOBUF_C_MESSAGE_DESCRIPTOR_MAGIC  0x28aaeef9
#define PROTOBUF_C_ENUM_DESCRIPTOR_MAGIC     0x114315af

/* === behind the scenes on the generated service's __init functions */
typedef void (*ProtobufCServiceDestroy) (ProtobufCService *service);
void
protobuf_c_service_generated_init (ProtobufCService *service,
                                   const ProtobufCServiceDescriptor *descriptor,
                                   ProtobufCServiceDestroy destroy);

void 
protobuf_c_service_invoke_internal(ProtobufCService *service,
                                  unsigned          method_index,
                                  const ProtobufCMessage *input,
                                  ProtobufCClosure  closure,
                                  void             *closure_data);
