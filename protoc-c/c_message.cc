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

#include <algorithm>
#include <map>
#include <google/protobuf/compiler/c/c_message.h>
#include <google/protobuf/compiler/c/c_enum.h>
#include <google/protobuf/compiler/c/c_extension.h>
#include <google/protobuf/compiler/c/c_helpers.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format.h>
#include <google/protobuf/descriptor.pb.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace c {

// ===================================================================

MessageGenerator::MessageGenerator(const Descriptor* descriptor,
                                   const string& dllexport_decl)
  : descriptor_(descriptor),
    dllexport_decl_(dllexport_decl),
    field_generators_(descriptor),
    nested_generators_(new scoped_ptr<MessageGenerator>[
      descriptor->nested_type_count()]),
    enum_generators_(new scoped_ptr<EnumGenerator>[
      descriptor->enum_type_count()]),
    extension_generators_(new scoped_ptr<ExtensionGenerator>[
      descriptor->extension_count()]) {

  for (int i = 0; i < descriptor->nested_type_count(); i++) {
    nested_generators_[i].reset(
      new MessageGenerator(descriptor->nested_type(i), dllexport_decl));
  }

  for (int i = 0; i < descriptor->enum_type_count(); i++) {
    enum_generators_[i].reset(
      new EnumGenerator(descriptor->enum_type(i), dllexport_decl));
  }

  for (int i = 0; i < descriptor->extension_count(); i++) {
    extension_generators_[i].reset(
      new ExtensionGenerator(descriptor->extension(i), dllexport_decl));
  }
}

MessageGenerator::~MessageGenerator() {}

void MessageGenerator::
GenerateStructTypedef(io::Printer* printer) {
  printer->Print("typedef struct _$classname$ $classname$;\n",
                 "classname", FullNameToC(descriptor_->full_name()));

  for (int i = 0; i < descriptor_->nested_type_count(); i++) {
    nested_generators_[i]->GenerateStructTypedef(printer);
  }
}

void MessageGenerator::
GenerateEnumDefinitions(io::Printer* printer) {
  for (int i = 0; i < descriptor_->nested_type_count(); i++) {
    nested_generators_[i]->GenerateEnumDefinitions(printer);
  }

  for (int i = 0; i < descriptor_->enum_type_count(); i++) {
    enum_generators_[i]->GenerateDefinition(printer);
  }
}


void MessageGenerator::
GenerateStructDefinition(io::Printer* printer) {
  for (int i = 0; i < descriptor_->nested_type_count(); i++) {
    nested_generators_[i]->GenerateStructDefinition(printer);
  }

  std::map<string, string> vars;
  vars["classname"] = FullNameToC(descriptor_->full_name());
  vars["lcclassname"] = FullNameToLower(descriptor_->full_name());
  vars["ucclassname"] = FullNameToUpper(descriptor_->full_name());
  vars["field_count"] = SimpleItoa(descriptor_->field_count());
  if (dllexport_decl_.empty()) {
    vars["dllexport"] = "";
  } else {
    vars["dllexport"] = dllexport_decl_ + " ";
  }

  printer->Print(vars,
    "struct $dllexport$ _$classname$\n"
    "{\n"
    "  ProtobufCMessage base;\n");

  // Generate fields.
  printer->Indent();
  for (int i = 0; i < descriptor_->field_count(); i++) {
    const FieldDescriptor *field = descriptor_->field(i);
    field_generators_.get(field).GenerateStructMembers(printer);
  }
  printer->Outdent();

  printer->Print(vars, "};\n");

  for (int i = 0; i < descriptor_->field_count(); i++) {
    const FieldDescriptor *field = descriptor_->field(i);
    if (field->has_default_value()) {
      field_generators_.get(field).GenerateDefaultValueDeclarations(printer);
    }
  }

  printer->Print(vars, "#define $ucclassname$__INIT \\\n"
		       " { PROTOBUF_C_MESSAGE_INIT (&$lcclassname$__descriptor) \\\n    ");
  for (int i = 0; i < descriptor_->field_count(); i++) {
    const FieldDescriptor *field = descriptor_->field(i);
    printer->Print(", ");
    field_generators_.get(field).GenerateStaticInit(printer);
  }
  printer->Print(" }\n\n\n");

}

void MessageGenerator::
GenerateHelperFunctionDeclarations(io::Printer* printer, bool is_submessage)
{
  for (int i = 0; i < descriptor_->nested_type_count(); i++) {
    nested_generators_[i]->GenerateHelperFunctionDeclarations(printer, true);
  }

  std::map<string, string> vars;
  vars["classname"] = FullNameToC(descriptor_->full_name());
  vars["lcclassname"] = FullNameToLower(descriptor_->full_name());
  printer->Print(vars,
		 "/* $classname$ methods */\n"
		 "void   $lcclassname$__init\n"
		 "                     ($classname$         *message);\n"
		);
  if (!is_submessage) {
    printer->Print(vars,
		 "size_t $lcclassname$__get_packed_size\n"
		 "                     (const $classname$   *message);\n"
		 "size_t $lcclassname$__pack\n"
		 "                     (const $classname$   *message,\n"
		 "                      uint8_t             *out);\n"
		 "size_t $lcclassname$__pack_to_buffer\n"
		 "                     (const $classname$   *message,\n"
		 "                      ProtobufCBuffer     *buffer);\n"
		 "$classname$ *\n"
		 "       $lcclassname$__unpack\n"
		 "                     (ProtobufCAllocator  *allocator,\n"
                 "                      size_t               len,\n"
                 "                      const uint8_t       *data);\n"
		 "void   $lcclassname$__free_unpacked\n"
		 "                     ($classname$ *message,\n"
		 "                      ProtobufCAllocator *allocator);\n"
		);
  }
}

void MessageGenerator::
GenerateDescriptorDeclarations(io::Printer* printer) {
  printer->Print("extern const ProtobufCMessageDescriptor $name$__descriptor;\n",
                 "name", FullNameToLower(descriptor_->full_name()));

  for (int i = 0; i < descriptor_->nested_type_count(); i++) {
    nested_generators_[i]->GenerateDescriptorDeclarations(printer);
  }

  for (int i = 0; i < descriptor_->enum_type_count(); i++) {
    enum_generators_[i]->GenerateDescriptorDeclarations(printer);
  }
}
void MessageGenerator::GenerateClosureTypedef(io::Printer* printer)
{
  for (int i = 0; i < descriptor_->nested_type_count(); i++) {
    nested_generators_[i]->GenerateClosureTypedef(printer);
  }
  std::map<string, string> vars;
  vars["name"] = FullNameToC(descriptor_->full_name());
  printer->Print(vars,
                 "typedef void (*$name$_Closure)\n"
		 "                 (const $name$ *message,\n"
		 "                  void *closure_data);\n");
}

static int
compare_pfields_by_number (const void *a, const void *b)
{
  const FieldDescriptor *pa = *(const FieldDescriptor **)a;
  const FieldDescriptor *pb = *(const FieldDescriptor **)b;
  if (pa->number() < pb->number()) return -1;
  if (pa->number() > pb->number()) return +1;
  return 0;
}

void MessageGenerator::
GenerateHelperFunctionDefinitions(io::Printer* printer, bool is_submessage)
{
  for (int i = 0; i < descriptor_->nested_type_count(); i++) {
    nested_generators_[i]->GenerateHelperFunctionDefinitions(printer, true);
  }

  std::map<string, string> vars;
  vars["classname"] = FullNameToC(descriptor_->full_name());
  vars["lcclassname"] = FullNameToLower(descriptor_->full_name());
  vars["ucclassname"] = FullNameToUpper(descriptor_->full_name());
  printer->Print(vars,
		 "void   $lcclassname$__init\n"
		 "                     ($classname$         *message)\n"
		 "{\n"
		 "  static $classname$ init_value = $ucclassname$__INIT;\n"
		 "  *message = init_value;\n"
		 "}\n");
  if (!is_submessage) {
    printer->Print(vars,
		 "size_t $lcclassname$__get_packed_size\n"
		 "                     (const $classname$ *message)\n"
		 "{\n"
		 "  PROTOBUF_C_ASSERT (message->base.descriptor == &$lcclassname$__descriptor);\n"
		 "  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));\n"
		 "}\n"
		 "size_t $lcclassname$__pack\n"
		 "                     (const $classname$ *message,\n"
		 "                      uint8_t       *out)\n"
		 "{\n"
		 "  PROTOBUF_C_ASSERT (message->base.descriptor == &$lcclassname$__descriptor);\n"
		 "  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);\n"
		 "}\n"
		 "size_t $lcclassname$__pack_to_buffer\n"
		 "                     (const $classname$ *message,\n"
		 "                      ProtobufCBuffer *buffer)\n"
		 "{\n"
		 "  PROTOBUF_C_ASSERT (message->base.descriptor == &$lcclassname$__descriptor);\n"
		 "  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);\n"
		 "}\n"
		 "$classname$ *\n"
		 "       $lcclassname$__unpack\n"
		 "                     (ProtobufCAllocator  *allocator,\n"
		 "                      size_t               len,\n"
                 "                      const uint8_t       *data)\n"
		 "{\n"
		 "  return ($classname$ *)\n"
		 "     protobuf_c_message_unpack (&$lcclassname$__descriptor,\n"
		 "                                allocator, len, data);\n"
		 "}\n"
		 "void   $lcclassname$__free_unpacked\n"
		 "                     ($classname$ *message,\n"
		 "                      ProtobufCAllocator *allocator)\n"
		 "{\n"
		 "  PROTOBUF_C_ASSERT (message->base.descriptor == &$lcclassname$__descriptor);\n"
		 "  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);\n"
		 "}\n"
		);
  }
}

void MessageGenerator::
GenerateMessageDescriptor(io::Printer* printer) {
    map<string, string> vars;
    vars["fullname"] = descriptor_->full_name();
    vars["classname"] = FullNameToC(descriptor_->full_name());
    vars["lcclassname"] = FullNameToLower(descriptor_->full_name());
    vars["shortname"] = ToCamel(descriptor_->name());
    vars["n_fields"] = SimpleItoa(descriptor_->field_count());
    vars["packagename"] = descriptor_->file()->package();

    for (int i = 0; i < descriptor_->nested_type_count(); i++) {
      nested_generators_[i]->GenerateMessageDescriptor(printer);
    }

    for (int i = 0; i < descriptor_->enum_type_count(); i++) {
      enum_generators_[i]->GenerateEnumDescriptor(printer);
    }

    for (int i = 0; i < descriptor_->field_count(); i++) {
      const FieldDescriptor *fd = descriptor_->field(i);
      if (fd->has_default_value()) {
	field_generators_.get(fd).GenerateDefaultValueImplementations(printer);
      }
    }

    for (int i = 0; i < descriptor_->field_count(); i++) {
      const FieldDescriptor *fd = descriptor_->field(i);
      if (fd->has_default_value()) {

	bool already_defined = false;
	vars["name"] = fd->name();
	vars["lcname"] = CamelToLower(fd->name());
	vars["maybe_static"] = "static ";
	vars["field_dv_ctype_suffix"] = "";
	vars["default_value"] = field_generators_.get(fd).GetDefaultValue();
	switch (fd->cpp_type()) {
	case FieldDescriptor::CPPTYPE_INT32:
	  vars["field_dv_ctype"] = "int32_t";
	  break;
	case FieldDescriptor::CPPTYPE_INT64:
	  vars["field_dv_ctype"] = "int64_t";
	  break;
	case FieldDescriptor::CPPTYPE_UINT32:
	  vars["field_dv_ctype"] = "uint32_t";
	  break;
	case FieldDescriptor::CPPTYPE_UINT64:
	  vars["field_dv_ctype"] = "uint64_t";
	  break;
	case FieldDescriptor::CPPTYPE_FLOAT:
	  vars["field_dv_ctype"] = "float";
	  break;
	case FieldDescriptor::CPPTYPE_DOUBLE:
	  vars["field_dv_ctype"] = "double";
	  break;
	case FieldDescriptor::CPPTYPE_BOOL:
	  vars["field_dv_ctype"] = "protobuf_c_boolean";
	  break;
	  
	case FieldDescriptor::CPPTYPE_MESSAGE:
	  // NOTE: not supported by protobuf
	  vars["maybe_static"] = "";
	  vars["field_dv_ctype"] = "{ ... }";
	  GOOGLE_LOG(DFATAL) << "Messages can't have default values!";
	  break;
	case FieldDescriptor::CPPTYPE_STRING:
	  if (fd->type() == FieldDescriptor::TYPE_BYTES)
	  {
	    vars["field_dv_ctype"] = "ProtobufCBinaryData";
	  }
	  else   /* STRING type */
	  {
	    already_defined = true;
	    vars["maybe_static"] = "";
	    vars["field_dv_ctype"] = "char";
	    vars["field_dv_ctype_suffix"] = "[]";
	  }
	  break;
	case FieldDescriptor::CPPTYPE_ENUM:
	  {
	    const EnumValueDescriptor *vd = fd->default_value_enum();
	    vars["field_dv_ctype"] = FullNameToC(vd->type()->full_name());
	    break;
	  }
	default:
	  GOOGLE_LOG(DFATAL) << "Unknown CPPTYPE";
	  break;
	}
	if (!already_defined)
	  printer->Print(vars, "$maybe_static$const $field_dv_ctype$ $lcclassname$__$lcname$__default_value$field_dv_ctype_suffix$ = $default_value$;\n");
      }
    }

    if ( descriptor_->field_count() ) {
  printer->Print(vars,
	"static const ProtobufCFieldDescriptor $lcclassname$__field_descriptors[$n_fields$] =\n"
	"{\n");
  printer->Indent();
  const FieldDescriptor **sorted_fields = new const FieldDescriptor *[descriptor_->field_count()];
  for (int i = 0; i < descriptor_->field_count(); i++) {
    sorted_fields[i] = descriptor_->field(i);
  }
  qsort (sorted_fields, descriptor_->field_count(),
       sizeof (const FieldDescriptor *), 
       compare_pfields_by_number);
  for (int i = 0; i < descriptor_->field_count(); i++) {
    const FieldDescriptor *field = sorted_fields[i];
    field_generators_.get(field).GenerateDescriptorInitializer(printer);
  }
  printer->Outdent();
  printer->Print(vars, "};\n");

  NameIndex *field_indices = new NameIndex [descriptor_->field_count()];
  for (int i = 0; i < descriptor_->field_count(); i++) {
    field_indices[i].name = sorted_fields[i]->name().c_str();
    field_indices[i].index = i;
  }
  qsort (field_indices, descriptor_->field_count(), sizeof (NameIndex),
         compare_name_indices_by_name);
  printer->Print(vars, "static const unsigned $lcclassname$__field_indices_by_name[] = {\n");
  for (int i = 0; i < descriptor_->field_count(); i++) {
    vars["index"] = SimpleItoa(field_indices[i].index);
    vars["name"] = field_indices[i].name;
    printer->Print(vars, "  $index$,   /* field[$index$] = $name$ */\n");
  }
  printer->Print("};\n");

  // create range initializers
  int *values = new int[descriptor_->field_count()];
  for (int i = 0; i < descriptor_->field_count(); i++) {
    values[i] = sorted_fields[i]->number();
  }
  int n_ranges = WriteIntRanges(printer,
				descriptor_->field_count(), values,
				vars["lcclassname"] + "__number_ranges");
  delete [] values;
  delete [] sorted_fields;

  vars["n_ranges"] = SimpleItoa(n_ranges);
    } else {
      /* MS compiler can't handle arrays with zero size and empty
       * initialization list. Furthermore it is an extension of GCC only but
       * not a standard. */
      vars["n_ranges"] = "0";
  printer->Print(vars,
        "#define $lcclassname$__field_descriptors NULL\n"
        "#define $lcclassname$__field_indices_by_name NULL\n"
        "#define $lcclassname$__number_ranges NULL\n");
    }
  
  printer->Print(vars,
  "const ProtobufCMessageDescriptor $lcclassname$__descriptor =\n"
  "{\n"
  "  PROTOBUF_C_MESSAGE_DESCRIPTOR_MAGIC,\n"
  "  \"$fullname$\",\n"
  "  \"$shortname$\",\n"
  "  \"$classname$\",\n"
  "  \"$packagename$\",\n"
  "  sizeof($classname$),\n"
  "  $n_fields$,\n"
  "  $lcclassname$__field_descriptors,\n"
  "  $lcclassname$__field_indices_by_name,\n"
  "  $n_ranges$,"
  "  $lcclassname$__number_ranges,\n"
  "  (ProtobufCMessageInit) $lcclassname$__init,\n"
  "  NULL,NULL,NULL    /* reserved[123] */\n"
  "};\n");
}

}  // namespace c
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
