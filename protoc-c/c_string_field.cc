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

#include <google/protobuf/compiler/c/c_string_field.h>
#include <google/protobuf/compiler/c/c_helpers.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/wire_format.h>
#include <google/protobuf/descriptor.pb.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace c {

using internal::WireFormat;

void SetStringVariables(const FieldDescriptor* descriptor,
                        map<string, string>* variables) {
  (*variables)["name"] = FieldName(descriptor);
  (*variables)["default"] = FullNameToLower(descriptor->full_name())
	+ "__default_value";
  (*variables)["deprecated"] = FieldDeprecated(descriptor);
}

// ===================================================================

StringFieldGenerator::
StringFieldGenerator(const FieldDescriptor* descriptor)
  : FieldGenerator(descriptor) {
  SetStringVariables(descriptor, &variables_);
}

StringFieldGenerator::~StringFieldGenerator() {}

void StringFieldGenerator::GenerateStructMembers(io::Printer* printer) const
{
  switch (descriptor_->label()) {
    case FieldDescriptor::LABEL_REQUIRED:
    case FieldDescriptor::LABEL_OPTIONAL:
      printer->Print(variables_, "char *$name$$deprecated$;\n");
      break;
    case FieldDescriptor::LABEL_REPEATED:
      printer->Print(variables_, "size_t n_$name$$deprecated$;\n");
      printer->Print(variables_, "char **$name$$deprecated$;\n");
      break;
  }
}
void StringFieldGenerator::GenerateDefaultValueDeclarations(io::Printer* printer) const
{
  printer->Print(variables_, "extern char $default$[];\n");
}
void StringFieldGenerator::GenerateDefaultValueImplementations(io::Printer* printer) const
{
  std::map<string, string> vars;
  vars["default"] = variables_.find("default")->second;
  vars["escaped"] = CEscape(descriptor_->default_value_string());
  printer->Print(vars, "char $default$[] = \"$escaped$\";\n");
}

string StringFieldGenerator::GetDefaultValue(void) const
{
  return variables_.find("default")->second;
}
void StringFieldGenerator::GenerateStaticInit(io::Printer* printer) const
{
  std::map<string, string> vars;
  if (descriptor_->has_default_value()) {
    vars["default"] = GetDefaultValue();
  } else {
    vars["default"] = "NULL";
  }
  switch (descriptor_->label()) {
    case FieldDescriptor::LABEL_REQUIRED:
    case FieldDescriptor::LABEL_OPTIONAL:
      printer->Print(vars, "$default$");
      break;
    case FieldDescriptor::LABEL_REPEATED:
      printer->Print(vars, "0,NULL");
      break;
  }
}
void StringFieldGenerator::GenerateDescriptorInitializer(io::Printer* printer) const
{
  GenerateDescriptorInitializerGeneric(printer, false, "STRING", "NULL");
}

}  // namespace c
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
