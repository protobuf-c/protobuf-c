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

#include <google/protobuf/compiler/c/c_message_field.h>
#include <google/protobuf/compiler/c/c_helpers.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/wire_format.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace c {

using internal::WireFormat;

// ===================================================================

MessageFieldGenerator::
MessageFieldGenerator(const FieldDescriptor* descriptor)
  : FieldGenerator(descriptor) {
}

MessageFieldGenerator::~MessageFieldGenerator() {}

void MessageFieldGenerator::GenerateStructMembers(io::Printer* printer) const
{
  map<string, string> vars;
  vars["name"] = FieldName(descriptor_);
  vars["type"] = FullNameToC(descriptor_->message_type()->full_name());
  vars["deprecated"] = FieldDeprecated(descriptor_);
  switch (descriptor_->label()) {
    case FieldDescriptor::LABEL_REQUIRED:
    case FieldDescriptor::LABEL_OPTIONAL:
      printer->Print(vars, "$type$ *$name$$deprecated$;\n");
      break;
    case FieldDescriptor::LABEL_REPEATED:
      printer->Print(vars, "size_t n_$name$$deprecated$;\n");
      printer->Print(vars, "$type$ **$name$$deprecated$;\n");
      break;
  }
}
string MessageFieldGenerator::GetDefaultValue(void) const
{
  /* XXX: update when protobuf gets support
   *   for default-values of message fields.
   */
  return "NULL";
}
void MessageFieldGenerator::GenerateStaticInit(io::Printer* printer) const
{
  switch (descriptor_->label()) {
    case FieldDescriptor::LABEL_REQUIRED:
    case FieldDescriptor::LABEL_OPTIONAL:
      printer->Print("NULL");
      break;
    case FieldDescriptor::LABEL_REPEATED:
      printer->Print("0,NULL");
      break;
  }
}
void MessageFieldGenerator::GenerateDescriptorInitializer(io::Printer* printer) const
{
  string addr = "&" + FullNameToLower(descriptor_->message_type()->full_name()) + "__descriptor";
  GenerateDescriptorInitializerGeneric(printer, false, "MESSAGE", addr);
}

}  // namespace c
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
