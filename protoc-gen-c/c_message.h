// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// http://code.google.com/p/protobuf/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Author: kenton@google.com (Kenton Varda)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.

// Copyright (c) 2008-2025, Dave Benson and the protobuf-c authors.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Modified to implement C code by Dave Benson.

#ifndef PROTOBUF_C_PROTOC_GEN_C_C_MESSAGE_H__
#define PROTOBUF_C_PROTOC_GEN_C_C_MESSAGE_H__

#include <memory>
#include <string>

#include <google/protobuf/io/printer.h>
#include <google/protobuf/stubs/common.h>

#include "c_enum.h"
#include "c_extension.h"
#include "c_field.h"

namespace protobuf_c {

class MessageGenerator {
 public:
  // See generator.cc for the meaning of dllexport_decl.
  explicit MessageGenerator(const google::protobuf::Descriptor* descriptor,
                            const std::string& dllexport_decl);
  ~MessageGenerator();

  // Header stuff.

  // Generate typedef.
  void GenerateStructTypedef(google::protobuf::io::Printer* printer);

  // Generate descriptor prototype
  void GenerateDescriptorDeclarations(google::protobuf::io::Printer* printer);

  // Generate descriptor prototype
  void GenerateClosureTypedef(google::protobuf::io::Printer* printer);

  // Generate definitions of all nested enums (must come before class
  // definitions because those classes use the enums definitions).
  void GenerateEnumDefinitions(google::protobuf::io::Printer* printer);

  // Generate definitions for this class and all its nested types.
  void GenerateStructDefinition(google::protobuf::io::Printer* printer);

  // Generate __INIT macro for populating this structure
  void GenerateStructStaticInitMacro(google::protobuf::io::Printer* printer);

  // Generate standard helper functions declarations for this message.
  void GenerateHelperFunctionDeclarations(google::protobuf::io::Printer* printer,
					  bool is_pack_deep,
					  bool gen_pack,
					  bool gen_init);

  // Source file stuff.

  // Generate code that initializes the global variable storing the message's
  // descriptor.
  void GenerateMessageDescriptor(google::protobuf::io::Printer* printer, bool gen_init);
  void GenerateHelperFunctionDefinitions(google::protobuf::io::Printer* printer,
					 bool is_pack_deep,
					 bool gen_pack,
					 bool gen_init);

 private:

  int GetOneofUnionOrder(const google::protobuf::FieldDescriptor *fd);

  const google::protobuf::Descriptor* descriptor_;
  std::string dllexport_decl_;
  FieldGeneratorMap field_generators_;
  std::unique_ptr<std::unique_ptr<MessageGenerator>[]> nested_generators_;
  std::unique_ptr<std::unique_ptr<EnumGenerator>[]> enum_generators_;
  std::unique_ptr<std::unique_ptr<ExtensionGenerator>[]> extension_generators_;
};

}  // namespace protobuf_c

#endif  // PROTOBUF_C_PROTOC_GEN_C_C_MESSAGE_H__
