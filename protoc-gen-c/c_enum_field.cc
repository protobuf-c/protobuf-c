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

#include <google/protobuf/io/printer.h>
#include <google/protobuf/wire_format.h>

#include "c_enum_field.h"
#include "c_helpers.h"

namespace protobuf_c {

// TODO(kenton):  Factor out a "SetCommonFieldVariables()" to get rid of
//   repeat code between this and the other field types.
void SetEnumVariables(const google::protobuf::FieldDescriptor* descriptor,
                      std::map<std::string, std::string>* variables) {

  (*variables)["name"] = FieldName(descriptor);
  (*variables)["type"] = FullNameToC(descriptor->enum_type()->full_name(), descriptor->enum_type()->file());
  const google::protobuf::EnumValueDescriptor* default_value = descriptor->default_value_enum();
  (*variables)["default"] = FullNameToUpper(default_value->type()->full_name(), default_value->type()->file())
                          + "__" + default_value->name();
  (*variables)["deprecated"] = FieldDeprecated(descriptor);
}

// ===================================================================

EnumFieldGenerator::
EnumFieldGenerator(const google::protobuf::FieldDescriptor* descriptor)
  : FieldGenerator(descriptor)
{
  SetEnumVariables(descriptor, &variables_);
}

EnumFieldGenerator::~EnumFieldGenerator() {}

void EnumFieldGenerator::GenerateStructMembers(google::protobuf::io::Printer* printer) const
{
  switch (descriptor_->label()) {
    case google::protobuf::FieldDescriptor::LABEL_REQUIRED:
      printer->Print(variables_, "$type$ $name$$deprecated$;\n");
      break;
    case google::protobuf::FieldDescriptor::LABEL_OPTIONAL:
      if (descriptor_->containing_oneof() == NULL && FieldSyntax(descriptor_) == 2)
        printer->Print(variables_, "protobuf_c_boolean has_$name$$deprecated$;\n");
      printer->Print(variables_, "$type$ $name$$deprecated$;\n");
      break;
    case google::protobuf::FieldDescriptor::LABEL_REPEATED:
      printer->Print(variables_, "size_t n_$name$$deprecated$;\n");
      printer->Print(variables_, "$type$ *$name$$deprecated$;\n");
      break;
  }
}

std::string EnumFieldGenerator::GetDefaultValue(void) const
{
  return variables_.find("default")->second;
}
void EnumFieldGenerator::GenerateStaticInit(google::protobuf::io::Printer* printer) const
{
  switch (descriptor_->label()) {
    case google::protobuf::FieldDescriptor::LABEL_REQUIRED:
      printer->Print(variables_, "$default$");
      break;
    case google::protobuf::FieldDescriptor::LABEL_OPTIONAL:
      if (FieldSyntax(descriptor_) == 2)
        printer->Print(variables_, "0, ");
      printer->Print(variables_, "$default$");
      break;
    case google::protobuf::FieldDescriptor::LABEL_REPEATED:
      // no support for default?
      printer->Print("0,NULL");
      break;
  }
}

void EnumFieldGenerator::GenerateDescriptorInitializer(google::protobuf::io::Printer* printer) const
{
  std::string addr = "&" + FullNameToLower(descriptor_->enum_type()->full_name(), descriptor_->enum_type()->file()) + "__descriptor";
  GenerateDescriptorInitializerGeneric(printer, true, "ENUM", addr);
}

}  // namespace protobuf_c
