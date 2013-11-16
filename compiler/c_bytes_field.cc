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

#include <google/protobuf/compiler/c/c_bytes_field.h>
#include <google/protobuf/compiler/c/c_helpers.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/wire_format.h>
#include <google/protobuf/descriptor.pb.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace c {

using internal::WireFormat;

void SetBytesVariables(const FieldDescriptor* descriptor,
                        map<string, string>* variables) {
  (*variables)["name"] = FieldName(descriptor);
  (*variables)["default"] =
    "\"" + CEscape(descriptor->default_value_string()) + "\"";
  (*variables)["deprecated"] = FieldDeprecated(descriptor);
}

// ===================================================================

BytesFieldGenerator::
BytesFieldGenerator(const FieldDescriptor* descriptor)
  : FieldGenerator(descriptor) {
  SetBytesVariables(descriptor, &variables_);
  variables_["default_value"] = descriptor->has_default_value()
                              ? GetDefaultValue() 
			      : string("{0,NULL}");
}

BytesFieldGenerator::~BytesFieldGenerator() {}

void BytesFieldGenerator::GenerateStructMembers(io::Printer* printer) const
{
  switch (descriptor_->label()) {
    case FieldDescriptor::LABEL_REQUIRED:
      printer->Print(variables_, "ProtobufCBinaryData $name$$deprecated$;\n");
      break;
    case FieldDescriptor::LABEL_OPTIONAL:
      printer->Print(variables_, "protobuf_c_boolean has_$name$$deprecated$;\n");
      printer->Print(variables_, "ProtobufCBinaryData $name$$deprecated$;\n");
      break;
    case FieldDescriptor::LABEL_REPEATED:
      printer->Print(variables_, "size_t n_$name$$deprecated$;\n");
      printer->Print(variables_, "ProtobufCBinaryData *$name$$deprecated$;\n");
      break;
  }
}
void BytesFieldGenerator::GenerateDefaultValueDeclarations(io::Printer* printer) const
{
  std::map<string, string> vars;
  vars["default_value_data"] = FullNameToLower(descriptor_->full_name())
	                     + "__default_value_data";
  printer->Print(vars, "extern uint8_t $default_value_data$[];\n");
}

void BytesFieldGenerator::GenerateDefaultValueImplementations(io::Printer* printer) const
{
  std::map<string, string> vars;
  vars["default_value_data"] = FullNameToLower(descriptor_->full_name())
	                     + "__default_value_data";
  vars["escaped"] = CEscape(descriptor_->default_value_string());
  printer->Print(vars, "uint8_t $default_value_data$[] = \"$escaped$\";\n");
}
string BytesFieldGenerator::GetDefaultValue(void) const
{
  return "{ "
	+ SimpleItoa(descriptor_->default_value_string().size())
	+ ", "
	+ FullNameToLower(descriptor_->full_name())
	+ "__default_value_data }";
}
void BytesFieldGenerator::GenerateStaticInit(io::Printer* printer) const
{
  switch (descriptor_->label()) {
    case FieldDescriptor::LABEL_REQUIRED:
      printer->Print(variables_, "$default_value$");
      break;
    case FieldDescriptor::LABEL_OPTIONAL:
      printer->Print(variables_, "0,$default_value$");
      break;
    case FieldDescriptor::LABEL_REPEATED:
      // no support for default?
      printer->Print("0,NULL");
      break;
  }
}
void BytesFieldGenerator::GenerateDescriptorInitializer(io::Printer* printer) const
{
  GenerateDescriptorInitializerGeneric(printer, true, "BYTES", "NULL");
}

}  // namespace c
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
