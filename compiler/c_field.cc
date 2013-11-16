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

#include <google/protobuf/compiler/c/c_field.h>
#include <google/protobuf/compiler/c/c_primitive_field.h>
#include <google/protobuf/compiler/c/c_string_field.h>
#include <google/protobuf/compiler/c/c_bytes_field.h>
#include <google/protobuf/compiler/c/c_enum_field.h>
#include <google/protobuf/compiler/c/c_message_field.h>
#include <google/protobuf/compiler/c/c_helpers.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/io/printer.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace c {

FieldGenerator::~FieldGenerator()
{
}
static bool is_packable_type(FieldDescriptor::Type type)
{
  return type == FieldDescriptor::TYPE_DOUBLE
      || type == FieldDescriptor::TYPE_FLOAT
      || type == FieldDescriptor::TYPE_INT64
      || type == FieldDescriptor::TYPE_UINT64
      || type == FieldDescriptor::TYPE_INT32
      || type == FieldDescriptor::TYPE_FIXED64
      || type == FieldDescriptor::TYPE_FIXED32
      || type == FieldDescriptor::TYPE_BOOL
      || type == FieldDescriptor::TYPE_UINT32
      || type == FieldDescriptor::TYPE_ENUM
      || type == FieldDescriptor::TYPE_SFIXED32
      || type == FieldDescriptor::TYPE_SFIXED64
      || type == FieldDescriptor::TYPE_SINT32
      || type == FieldDescriptor::TYPE_SINT64;
    //TYPE_BYTES
    //TYPE_STRING
    //TYPE_GROUP
    //TYPE_MESSAGE
}

void FieldGenerator::GenerateDescriptorInitializerGeneric(io::Printer* printer,
							  bool optional_uses_has,
							  const string &type_macro,
							  const string &descriptor_addr) const
{
  map<string, string> variables;
  variables["LABEL"] = CamelToUpper(GetLabelName(descriptor_->label()));
  variables["TYPE"] = type_macro;
  variables["classname"] = FullNameToC(FieldScope(descriptor_)->full_name());
  variables["name"] = FieldName(descriptor_);
  variables["proto_name"] = descriptor_->name();
  variables["descriptor_addr"] = descriptor_addr;
  variables["value"] = SimpleItoa(descriptor_->number());

  if (descriptor_->has_default_value()) {
    variables["default_value"] = string("&")
                               + FullNameToLower(descriptor_->full_name())
			       + "__default_value";
  } else {
    variables["default_value"] = "NULL";
  }

  if (descriptor_->label() == FieldDescriptor::LABEL_REPEATED
   && is_packable_type (descriptor_->type())
   && descriptor_->options().packed())
    variables["packed"] = "1";
  else
    variables["packed"] = "0";

  printer->Print(variables,
    "{\n"
    "  \"$proto_name$\",\n"
    "  $value$,\n"
    "  PROTOBUF_C_LABEL_$LABEL$,\n"
    "  PROTOBUF_C_TYPE_$TYPE$,\n");
  bool packed = false;
  switch (descriptor_->label()) {
    case FieldDescriptor::LABEL_REQUIRED:
      printer->Print(variables, "  0,   /* quantifier_offset */\n");
      break;
    case FieldDescriptor::LABEL_OPTIONAL:
      if (optional_uses_has) {
	printer->Print(variables, "  PROTOBUF_C_OFFSETOF($classname$, has_$name$),\n");
      } else {
	printer->Print(variables, "  0,   /* quantifier_offset */\n");
      }
      break;
    case FieldDescriptor::LABEL_REPEATED:
      printer->Print(variables, "  PROTOBUF_C_OFFSETOF($classname$, n_$name$),\n");
      break;
  }
  printer->Print(variables, "  PROTOBUF_C_OFFSETOF($classname$, $name$),\n");
  printer->Print(variables, "  $descriptor_addr$,\n");
  printer->Print(variables, "  $default_value$,\n");
  printer->Print(variables, "  $packed$,            /* packed */\n");
  printer->Print(variables, "  0,NULL,NULL    /* reserved1,reserved2, etc */\n");
  printer->Print("},\n");
}

FieldGeneratorMap::FieldGeneratorMap(const Descriptor* descriptor)
  : descriptor_(descriptor),
    field_generators_(
      new scoped_ptr<FieldGenerator>[descriptor->field_count()]) {
  // Construct all the FieldGenerators.
  for (int i = 0; i < descriptor->field_count(); i++) {
    field_generators_[i].reset(MakeGenerator(descriptor->field(i)));
  }
}

FieldGenerator* FieldGeneratorMap::MakeGenerator(const FieldDescriptor* field) {
  switch (field->type()) {
    case FieldDescriptor::TYPE_MESSAGE:
      return new MessageFieldGenerator(field);
    case FieldDescriptor::TYPE_STRING:
      return new StringFieldGenerator(field);
    case FieldDescriptor::TYPE_BYTES:
      return new BytesFieldGenerator(field);
    case FieldDescriptor::TYPE_ENUM:
      return new EnumFieldGenerator(field);
    case FieldDescriptor::TYPE_GROUP:
      return 0;			// XXX
    default:
      return new PrimitiveFieldGenerator(field);
  }
}

FieldGeneratorMap::~FieldGeneratorMap() {}

const FieldGenerator& FieldGeneratorMap::get(
    const FieldDescriptor* field) const {
  GOOGLE_CHECK_EQ(field->containing_type(), descriptor_);
  return *field_generators_[field->index()];
}

}  // namespace c
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
