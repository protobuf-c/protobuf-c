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

#ifndef PROTOBUF_C_PROTOC_GEN_C_C_FIELD_H__
#define PROTOBUF_C_PROTOC_GEN_C_C_FIELD_H__

#include <memory>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/stubs/common.h>

namespace protobuf_c {

class FieldGenerator {
 public:
  explicit FieldGenerator(const google::protobuf::FieldDescriptor *descriptor) : descriptor_(descriptor) {}
  virtual ~FieldGenerator();

  // Generate definitions to be included in the structure.
  virtual void GenerateStructMembers(google::protobuf::io::Printer* printer) const = 0;

  // Generate a static initializer for this field.
  virtual void GenerateDescriptorInitializer(google::protobuf::io::Printer* printer) const = 0;

  virtual void GenerateDefaultValueDeclarations(google::protobuf::io::Printer* printer) const { }
  virtual void GenerateDefaultValueImplementations(google::protobuf::io::Printer* printer) const { }
  virtual std::string GetDefaultValue() const = 0;

  // Generate members to initialize this field from a static initializer
  virtual void GenerateStaticInit(google::protobuf::io::Printer* printer) const = 0;


 protected:
  void GenerateDescriptorInitializerGeneric(google::protobuf::io::Printer* printer,
                                            bool optional_uses_has,
                                            const std::string &type_macro,
                                            const std::string &descriptor_addr) const;
  const google::protobuf::FieldDescriptor *descriptor_;
};

// Convenience class which constructs FieldGenerators for a Descriptor.
class FieldGeneratorMap {
 public:
  explicit FieldGeneratorMap(const google::protobuf::Descriptor* descriptor);
  ~FieldGeneratorMap();

  const FieldGenerator& get(const google::protobuf::FieldDescriptor* field) const;

 private:
  const google::protobuf::Descriptor* descriptor_;
  std::unique_ptr<std::unique_ptr<FieldGenerator>[]> field_generators_;

  static FieldGenerator* MakeGenerator(const google::protobuf::FieldDescriptor* field);
};

}  // namespace protobuf_c

#endif  // PROTOBUF_C_PROTOC_GEN_C_C_FIELD_H__
