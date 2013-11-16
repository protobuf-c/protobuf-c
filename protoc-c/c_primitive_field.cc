// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.
// http://code.google.com/p/protobuf/
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Author: kenton@google.com (Kenton Varda)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.

// Modified to implement C code by Dave Benson.

#include <google/protobuf/compiler/c/c_primitive_field.h>
#include <google/protobuf/compiler/c/c_helpers.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/wire_format.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace c {

PrimitiveFieldGenerator::
PrimitiveFieldGenerator(const FieldDescriptor* descriptor)
  : FieldGenerator(descriptor) {
}

PrimitiveFieldGenerator::~PrimitiveFieldGenerator() {}

void PrimitiveFieldGenerator::GenerateStructMembers(io::Printer* printer) const
{
  string c_type;
  map<string, string> vars;
  switch (descriptor_->type()) {
    case FieldDescriptor::TYPE_SINT32  : 
    case FieldDescriptor::TYPE_SFIXED32: 
    case FieldDescriptor::TYPE_INT32   : c_type = "int32_t"; break;
    case FieldDescriptor::TYPE_SINT64  : 
    case FieldDescriptor::TYPE_SFIXED64:
    case FieldDescriptor::TYPE_INT64   : c_type = "int64_t"; break;
    case FieldDescriptor::TYPE_UINT32  : 
    case FieldDescriptor::TYPE_FIXED32 : c_type = "uint32_t"; break;
    case FieldDescriptor::TYPE_UINT64  : 
    case FieldDescriptor::TYPE_FIXED64 : c_type = "uint64_t"; break;
    case FieldDescriptor::TYPE_FLOAT   : c_type = "float"; break;
    case FieldDescriptor::TYPE_DOUBLE  : c_type = "double"; break;
    case FieldDescriptor::TYPE_BOOL    : c_type = "protobuf_c_boolean"; break;
    case FieldDescriptor::TYPE_ENUM    : 
    case FieldDescriptor::TYPE_STRING  :
    case FieldDescriptor::TYPE_BYTES   :
    case FieldDescriptor::TYPE_GROUP   :
    case FieldDescriptor::TYPE_MESSAGE : GOOGLE_LOG(FATAL) << "not a primitive type"; break;

    // No default because we want the compiler to complain if any new
    // types are added.
  }
  vars["c_type"] = c_type;
  vars["name"] = FieldName(descriptor_);
  vars["deprecated"] = FieldDeprecated(descriptor_);

  switch (descriptor_->label()) {
    case FieldDescriptor::LABEL_REQUIRED:
      printer->Print(vars, "$c_type$ $name$$deprecated$;\n");
      break;
    case FieldDescriptor::LABEL_OPTIONAL:
      printer->Print(vars, "protobuf_c_boolean has_$name$$deprecated$;\n");
      printer->Print(vars, "$c_type$ $name$$deprecated$;\n");
      break;
    case FieldDescriptor::LABEL_REPEATED:
      printer->Print(vars, "size_t n_$name$$deprecated$;\n");
      printer->Print(vars, "$c_type$ *$name$$deprecated$;\n");
      break;
  }
}
string PrimitiveFieldGenerator::GetDefaultValue() const
{
  /* XXX: SimpleItoa seems woefully inadequate for anything but int32,
   * but that's what protobuf uses. */
  switch (descriptor_->cpp_type()) {
    case FieldDescriptor::CPPTYPE_INT32:
      return SimpleItoa(descriptor_->default_value_int32());
    case FieldDescriptor::CPPTYPE_INT64:
      return SimpleItoa(descriptor_->default_value_int64());
    case FieldDescriptor::CPPTYPE_UINT32:
      return SimpleItoa(descriptor_->default_value_uint32());
    case FieldDescriptor::CPPTYPE_UINT64:
      return SimpleItoa(descriptor_->default_value_uint64());
    case FieldDescriptor::CPPTYPE_FLOAT:
      return SimpleFtoa(descriptor_->default_value_float());
    case FieldDescriptor::CPPTYPE_DOUBLE:
      return SimpleDtoa(descriptor_->default_value_double());
    case FieldDescriptor::CPPTYPE_BOOL:
      return descriptor_->default_value_bool() ? "1" : "0";
    default:
      GOOGLE_LOG(DFATAL) << "unexpected CPPTYPE in c_primitive_field";
      return "UNEXPECTED_CPPTYPE";
  }
}
void PrimitiveFieldGenerator::GenerateStaticInit(io::Printer* printer) const
{
  map<string, string> vars;
  if (descriptor_->has_default_value()) {
    vars["default_value"] = GetDefaultValue();
  } else {
    vars["default_value"] = "0";
  }
  switch (descriptor_->label()) {
    case FieldDescriptor::LABEL_REQUIRED:
      printer->Print(vars, "$default_value$");
      break;
    case FieldDescriptor::LABEL_OPTIONAL:
      printer->Print(vars, "0,$default_value$");
      break;
    case FieldDescriptor::LABEL_REPEATED:
      printer->Print("0,NULL");
      break;
  }
}

void PrimitiveFieldGenerator::GenerateDescriptorInitializer(io::Printer* printer) const
{
  string c_type_macro;
  switch (descriptor_->type()) {
  #define WRITE_CASE(shortname) case FieldDescriptor::TYPE_##shortname: c_type_macro = #shortname; break;
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

    case FieldDescriptor::TYPE_ENUM    : 
    case FieldDescriptor::TYPE_STRING  :
    case FieldDescriptor::TYPE_BYTES   :
    case FieldDescriptor::TYPE_GROUP   :
    case FieldDescriptor::TYPE_MESSAGE : GOOGLE_LOG(FATAL) << "not a primitive type"; break;

    // No default because we want the compiler to complain if any new
    // types are added.
  }
  GenerateDescriptorInitializerGeneric(printer, true, c_type_macro, "NULL");
}

}  // namespace c
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
