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

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/wire_format.h>

#include "c_helpers.h"
#include "c_primitive_field.h"
#include "compat.h"

namespace protobuf_c {

PrimitiveFieldGenerator::
PrimitiveFieldGenerator(const google::protobuf::FieldDescriptor* descriptor)
  : FieldGenerator(descriptor) {
}

PrimitiveFieldGenerator::~PrimitiveFieldGenerator() {}

void PrimitiveFieldGenerator::GenerateStructMembers(google::protobuf::io::Printer* printer) const
{
  std::string c_type;
  std::map<std::string, std::string> vars;
  switch (descriptor_->type()) {
    case google::protobuf::FieldDescriptor::TYPE_SINT32  :
    case google::protobuf::FieldDescriptor::TYPE_SFIXED32:
    case google::protobuf::FieldDescriptor::TYPE_INT32   : c_type = "int32_t"; break;
    case google::protobuf::FieldDescriptor::TYPE_SINT64  :
    case google::protobuf::FieldDescriptor::TYPE_SFIXED64:
    case google::protobuf::FieldDescriptor::TYPE_INT64   : c_type = "int64_t"; break;
    case google::protobuf::FieldDescriptor::TYPE_UINT32  :
    case google::protobuf::FieldDescriptor::TYPE_FIXED32 : c_type = "uint32_t"; break;
    case google::protobuf::FieldDescriptor::TYPE_UINT64  :
    case google::protobuf::FieldDescriptor::TYPE_FIXED64 : c_type = "uint64_t"; break;
    case google::protobuf::FieldDescriptor::TYPE_FLOAT   : c_type = "float"; break;
    case google::protobuf::FieldDescriptor::TYPE_DOUBLE  : c_type = "double"; break;
    case google::protobuf::FieldDescriptor::TYPE_BOOL    : c_type = "protobuf_c_boolean"; break;
    case google::protobuf::FieldDescriptor::TYPE_ENUM    :
    case google::protobuf::FieldDescriptor::TYPE_STRING  :
    case google::protobuf::FieldDescriptor::TYPE_BYTES   :
    case google::protobuf::FieldDescriptor::TYPE_GROUP   :
    case google::protobuf::FieldDescriptor::TYPE_MESSAGE : GOOGLE_LOG(FATAL) << "not a primitive type"; break;

    // No default because we want the compiler to complain if any new
    // types are added.
  }
  vars["c_type"] = c_type;
  vars["name"] = FieldName(descriptor_);
  vars["deprecated"] = FieldDeprecated(descriptor_);

  switch (descriptor_->label()) {
    case google::protobuf::FieldDescriptor::LABEL_REQUIRED:
      printer->Print(vars, "$c_type$ $name$$deprecated$;\n");
      break;
    case google::protobuf::FieldDescriptor::LABEL_OPTIONAL:
      if (descriptor_->containing_oneof() == NULL && FieldSyntax(descriptor_) == 2)
        printer->Print(vars, "protobuf_c_boolean has_$name$$deprecated$;\n");
      printer->Print(vars, "$c_type$ $name$$deprecated$;\n");
      break;
    case google::protobuf::FieldDescriptor::LABEL_REPEATED:
      printer->Print(vars, "size_t n_$name$$deprecated$;\n");
      printer->Print(vars, "$c_type$ *$name$$deprecated$;\n");
      break;
  }
}
std::string PrimitiveFieldGenerator::GetDefaultValue() const
{
  /* XXX: SimpleItoa seems woefully inadequate for anything but int32,
   * but that's what protobuf uses. */
  switch (descriptor_->cpp_type()) {
    case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
      return SimpleItoa(descriptor_->default_value_int32());
    case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
      return SimpleItoa(descriptor_->default_value_int64()) + "ll";
    case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
      return SimpleItoa(descriptor_->default_value_uint32()) + "u";
    case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
      return SimpleItoa(descriptor_->default_value_uint64()) + "ull";
    case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
      return SimpleFtoa(descriptor_->default_value_float());
    case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
      return SimpleDtoa(descriptor_->default_value_double());
    case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
      return descriptor_->default_value_bool() ? "1" : "0";
    default:
      GOOGLE_LOG(FATAL) << "unexpected CPPTYPE in c_primitive_field";
      return "UNEXPECTED_CPPTYPE";
  }
}
void PrimitiveFieldGenerator::GenerateStaticInit(google::protobuf::io::Printer* printer) const
{
  std::map<std::string, std::string> vars;
  if (descriptor_->has_default_value()) {
    vars["default_value"] = GetDefaultValue();
  } else {
    vars["default_value"] = "0";
  }
  switch (descriptor_->label()) {
    case google::protobuf::FieldDescriptor::LABEL_REQUIRED:
      printer->Print(vars, "$default_value$");
      break;
    case google::protobuf::FieldDescriptor::LABEL_OPTIONAL:
      if (FieldSyntax(descriptor_) == 2)
        printer->Print(vars, "0, ");
      printer->Print(vars, "$default_value$");
      break;
    case google::protobuf::FieldDescriptor::LABEL_REPEATED:
      printer->Print("0,NULL");
      break;
  }
}

void PrimitiveFieldGenerator::GenerateDescriptorInitializer(google::protobuf::io::Printer* printer) const
{
  std::string c_type_macro;
  switch (descriptor_->type()) {
  #define WRITE_CASE(shortname) case google::protobuf::FieldDescriptor::TYPE_##shortname: c_type_macro = #shortname; break;
    WRITE_CASE(INT32)
    WRITE_CASE(SINT32)
    WRITE_CASE(UINT32)
    WRITE_CASE(SFIXED32)
    WRITE_CASE(FIXED32)

    WRITE_CASE(INT64)
    WRITE_CASE(SINT64)
    WRITE_CASE(UINT64)
    WRITE_CASE(FIXED64)
    WRITE_CASE(SFIXED64)

    WRITE_CASE(FLOAT)
    WRITE_CASE(DOUBLE)

    WRITE_CASE(BOOL)
  #undef WRITE_CASE

    case google::protobuf::FieldDescriptor::TYPE_ENUM    :
    case google::protobuf::FieldDescriptor::TYPE_STRING  :
    case google::protobuf::FieldDescriptor::TYPE_BYTES   :
    case google::protobuf::FieldDescriptor::TYPE_GROUP   :
    case google::protobuf::FieldDescriptor::TYPE_MESSAGE : GOOGLE_LOG(FATAL) << "not a primitive type"; break;

    // No default because we want the compiler to complain if any new
    // types are added.
  }
  GenerateDescriptorInitializerGeneric(printer, true, c_type_macro, "NULL");
}

}  // namespace protobuf_c
