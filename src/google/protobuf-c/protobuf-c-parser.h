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


typedef struct _ProtobufC_ParseResult ProtobufC_ParseResult;
struct _ProtobufC_ParseResult
{
  const char *message;
  const char *filename;
  unsigned line_no;
};

struct _ProtobufC_ParserPackage
{
  char *name;
  unsigned n_messages;
  ProtobufCMessageDescriptor *message_descriptors;
  unsigned n_enums;
  ProtobufCEnumDescriptor *enum_descriptors;
};

ProtobufC_Parser *protobuf_c_parser_new (void);

/* If non-NULL, return-value should be freed with default allocator */
typedef char *(*ProtobufC_ImportHandlerFunc) (const char *package_name,
                                              void       *func_data);

void protobuf_c_parser_set_import_handler (ProtobufC_Parser *parser,
                                           ProtobufC_ImportHandlerFunc func,
                                           void *func_data);

ProtobufC_ParserPackage *
               protobuf_c_parser_parse_file(ProtobufC_Parser *parser,
                                            const char       *filename,
                                            ProtobufC_ParseResult *error);
ProtobufC_ParserPackage *
               protobuf_c_parser_import    (ProtobufC_Parser *parser,
                                            const char       *package_name,
                                            ProtobufC_ParseResult *error);

void           protobuf_c_parser_free      (ProtobufC_Parser *parser);


typedef enum
{
  PROTOBUF_C_CODEGEN_MODE_H,
  PROTOBUF_C_CODEGEN_MODE_C
} ProtobufC_CodegenMode;

struct _ProtobufC_CodegenInfo
{
  const char *package_name;
  ProtobufC_CodegenMode mode;
  const char *output_filename;
};
#define PROTOBUF_C_CODEGEN_INFO_INIT { NULL, PROTOBUF_C_CODEGEN_MODE_H, NULL }

protobuf_c_boolean protobuf_c_parser_codegen(ProtobufC_Parser *parser,
                                             ProtobufC_CodegenInfo *codegen);
