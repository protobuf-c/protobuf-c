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

#include <google/protobuf/compiler/c/c_enum_field.h>
#include <google/protobuf/compiler/c/c_helpers.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/wire_format.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace c {

using internal::WireFormat;

// TODO(kenton):  Factor out a "SetCommonFieldVariables()" to get rid of
//   repeat code between this and the other field types.
void SetEnumVariables(const FieldDescriptor* descriptor,
                      map<string, string>* variables) {

  (*variables)["name"] = FieldName(descriptor);
  (*variables)["type"] = FullNameToC(descriptor->enum_type()->full_name());
  if (descriptor->has_default_value()) {
    const EnumValueDescriptor* default_value = descriptor->default_value_enum();
    (*variables)["default"] = FullNameToUpper(default_value->type()->full_name())
			    + "__" + ToUpper(default_value->name());
  } else
    (*variables)["default"] = "0";
  (*variables)["deprecated"] = FieldDeprecated(descriptor);
}

// ===================================================================

EnumFieldGenerator::
EnumFieldGenerator(const FieldDescriptor* descriptor)
  : FieldGenerator(descriptor)
{
  SetEnumVariables(descriptor, &variables_);
}

EnumFieldGenerator::~EnumFieldGenerator() {}

void EnumFieldGenerator::GenerateStructMembers(io::Printer* printer) const
{
  switch (descriptor_->label()) {
    case FieldDescriptor::LABEL_REQUIRED:
      printer->Print(variables_, "$type$ $name$$deprecated$;\n");
      break;
    case FieldDescriptor::LABEL_OPTIONAL:
      printer->Print(variables_, "protobuf_c_boolean has_$name$$deprecated$;\n");
      printer->Print(variables_, "$type$ $name$$deprecated$;\n");
      break;
    case FieldDescriptor::LABEL_REPEATED:
      printer->Print(variables_, "size_t n_$name$$deprecated$;\n");
      printer->Print(variables_, "$type$ *$name$$deprecated$;\n");
      break;
  }
}

string EnumFieldGenerator::GetDefaultValue(void) const
{
  return variables_.find("default")->second;
}
void EnumFieldGenerator::GenerateStaticInit(io::Printer* printer) const
{
  switch (descriptor_->label()) {
    case FieldDescriptor::LABEL_REQUIRED:
      printer->Print(variables_, "$default$");
      break;
    case FieldDescriptor::LABEL_OPTIONAL:
      printer->Print(variables_, "0,$default$");
      break;
    case FieldDescriptor::LABEL_REPEATED:
      // no support for default?
      printer->Print("0,NULL");
      break;
  }
}

void EnumFieldGenerator::GenerateDescriptorInitializer(io::Printer* printer) const
{
  string addr = "&" + FullNameToLower(descriptor_->enum_type()->full_name()) + "__descriptor";
  GenerateDescriptorInitializerGeneric(printer, true, "ENUM", addr);
}


}  // namespace c
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
