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

#include "c_helpers.h"
#include "c_service.h"

namespace protobuf_c {

ServiceGenerator::ServiceGenerator(const google::protobuf::ServiceDescriptor* descriptor,
                                   const std::string& dllexport_decl)
  : descriptor_(descriptor) {
  vars_["name"] = std::string(descriptor_->name());
  vars_["fullname"] = std::string(descriptor_->full_name());
  vars_["cname"] = FullNameToC(descriptor_->full_name(), descriptor_->file());
  vars_["lcfullname"] = FullNameToLower(descriptor_->full_name(), descriptor_->file());
  vars_["ucfullname"] = FullNameToUpper(descriptor_->full_name(), descriptor_->file());
  vars_["lcfullpadd"] = ConvertToSpaces(vars_["lcfullname"]);
  vars_["package"] = std::string(descriptor_->file()->package());
  if (dllexport_decl.empty()) {
    vars_["dllexport"] = "";
  } else {
    vars_["dllexport"] = dllexport_decl + " ";
  }
}

ServiceGenerator::~ServiceGenerator() {}

// Header stuff.
void ServiceGenerator::GenerateMainHFile(google::protobuf::io::Printer* printer)
{
  GenerateVfuncs(printer);
  GenerateInitMacros(printer);
  GenerateCallersDeclarations(printer);
}
void ServiceGenerator::GenerateVfuncs(google::protobuf::io::Printer* printer)
{
  printer->Print(vars_,
		 "typedef struct $cname$_Service $cname$_Service;\n"
		 "struct $cname$_Service\n"
		 "{\n"
		 "  ProtobufCService base;\n");
  for (int i = 0; i < descriptor_->method_count(); i++) {
    const google::protobuf::MethodDescriptor* method = descriptor_->method(i);
    std::string lcname = CamelToLower(method->name());
    vars_["method"] = lcname;
    vars_["metpad"] = ConvertToSpaces(lcname);
    vars_["input_typename"] = FullNameToC(method->input_type()->full_name(), method->input_type()->file());
    vars_["output_typename"] = FullNameToC(method->output_type()->full_name(), method->output_type()->file());
    printer->Print(vars_,
                   "  void (*$method$)($cname$_Service *service,\n"
                   "         $metpad$  const $input_typename$ *input,\n"
                   "         $metpad$  $output_typename$_Closure closure,\n"
                   "         $metpad$  void *closure_data);\n");
  }
  printer->Print(vars_,
		 "};\n");
  printer->Print(vars_,
		 "typedef void (*$cname$_ServiceDestroy)($cname$_Service *);\n"
		 "void $lcfullname$__init ($cname$_Service *service,\n"
		 "     $lcfullpadd$        $cname$_ServiceDestroy destroy);\n");
}
void ServiceGenerator::GenerateInitMacros(google::protobuf::io::Printer* printer)
{
  printer->Print(vars_,
		 "#define $ucfullname$__BASE_INIT \\\n"
		 "    { &$lcfullname$__descriptor, protobuf_c_service_invoke_internal, NULL }\n"
		 "#define $ucfullname$__INIT(function_prefix__) \\\n"
		 "    { $ucfullname$__BASE_INIT");
  for (int i = 0; i < descriptor_->method_count(); i++) {
    const google::protobuf::MethodDescriptor* method = descriptor_->method(i);
    std::string lcname = CamelToLower(method->name());
    vars_["method"] = lcname;
    vars_["metpad"] = ConvertToSpaces(lcname);
    printer->Print(vars_,
                   ",\\\n      function_prefix__ ## $method$");
  }
  printer->Print(vars_,
		 "  }\n");
}
void ServiceGenerator::GenerateCallersDeclarations(google::protobuf::io::Printer* printer)
{
  for (int i = 0; i < descriptor_->method_count(); i++) {
    const google::protobuf::MethodDescriptor* method = descriptor_->method(i);
    std::string lcname = CamelToLower(method->name());
    std::string lcfullname = FullNameToLower(descriptor_->full_name(), descriptor_->file());
    vars_["method"] = lcname;
    vars_["metpad"] = ConvertToSpaces(lcname);
    vars_["input_typename"] = FullNameToC(method->input_type()->full_name(), method->input_type()->file());
    vars_["output_typename"] = FullNameToC(method->output_type()->full_name(), method->output_type()->file());
    vars_["padddddddddddddddddd"] = ConvertToSpaces(lcfullname + "__" + lcname);
    printer->Print(vars_,
                   "void $lcfullname$__$method$(ProtobufCService *service,\n"
                   "     $padddddddddddddddddd$ const $input_typename$ *input,\n"
                   "     $padddddddddddddddddd$ $output_typename$_Closure closure,\n"
                   "     $padddddddddddddddddd$ void *closure_data);\n");
  }
}

void ServiceGenerator::GenerateDescriptorDeclarations(google::protobuf::io::Printer* printer)
{
  printer->Print(vars_, "extern const ProtobufCServiceDescriptor $lcfullname$__descriptor;\n");
}


// Source file stuff.
void ServiceGenerator::GenerateCFile(google::protobuf::io::Printer* printer)
{
  GenerateServiceDescriptor(printer);
  GenerateCallersImplementations(printer);
  GenerateInit(printer);
}
void ServiceGenerator::GenerateInit(google::protobuf::io::Printer* printer)
{
  printer->Print(vars_,
		 "void $lcfullname$__init ($cname$_Service *service,\n"
		 "     $lcfullpadd$        $cname$_ServiceDestroy destroy)\n"
		 "{\n"
		 "  protobuf_c_service_generated_init (&service->base,\n"
		 "                                     &$lcfullname$__descriptor,\n"
		 "                                     (ProtobufCServiceDestroy) destroy);\n"
		 "}\n");
}

struct MethodIndexAndName { unsigned i; compat::StringView name; };
static int
compare_method_index_and_name_by_name (const void *a, const void *b)
{
  const MethodIndexAndName *ma = (const MethodIndexAndName *) a;
  const MethodIndexAndName *mb = (const MethodIndexAndName *) b;
  return ma->name.compare(mb->name);
}

void ServiceGenerator::GenerateServiceDescriptor(google::protobuf::io::Printer* printer)
{
  int n_methods = descriptor_->method_count();
  std::vector<MethodIndexAndName> mi_array;

  bool optimize_code_size = descriptor_->file()->options().has_optimize_for() &&
    descriptor_->file()->options().optimize_for() ==
    google::protobuf::FileOptions_OptimizeMode_CODE_SIZE;

  vars_["n_methods"] = SimpleItoa(n_methods);
  printer->Print(vars_, "static const ProtobufCMethodDescriptor $lcfullname$__method_descriptors[$n_methods$] =\n"
                       "{\n");
  for (unsigned i = 0; i < n_methods; i++) {
    const google::protobuf::MethodDescriptor* method = descriptor_->method(i);
    vars_["method"] = std::string(method->name());
    vars_["input_descriptor"] = "&" + FullNameToLower(method->input_type()->full_name(), method->input_type()->file()) + "__descriptor";
    vars_["output_descriptor"] = "&" + FullNameToLower(method->output_type()->full_name(), method->output_type()->file()) + "__descriptor";
    if (optimize_code_size) {
      printer->Print(vars_,
          "  { NULL, $input_descriptor$, $output_descriptor$ }, /* CODE_SIZE */\n");
    } else {
      printer->Print(vars_,
          "  { \"$method$\", $input_descriptor$, $output_descriptor$ },\n");
    }
    mi_array.push_back({i, method->name()});
  }
  printer->Print(vars_, "};\n");

  if (!optimize_code_size) {
    qsort(&mi_array[0],
          mi_array.size(),
          sizeof(MethodIndexAndName),
          compare_method_index_and_name_by_name);
    printer->Print(vars_, "const unsigned $lcfullname$__method_indices_by_name[] = {\n");
    for (int i = 0; i < n_methods; i++) {
      vars_["i"] = SimpleItoa(mi_array[i].i);
      vars_["name"] = std::string(mi_array[i].name);
      vars_["comma"] = (i + 1 < n_methods) ? "," : " ";
      printer->Print(vars_, "  $i$$comma$        /* $name$ */\n");
    }
    printer->Print(vars_, "};\n");
    vars_["name"] = std::string(descriptor_->name());
  }

  if (optimize_code_size) {
    printer->Print(vars_, "const ProtobufCServiceDescriptor $lcfullname$__descriptor =\n"
        "{\n"
        "  PROTOBUF_C__SERVICE_DESCRIPTOR_MAGIC,\n"
        "  NULL,NULL,NULL,NULL, /* CODE_SIZE */\n"
        "  $n_methods$,\n"
        "  $lcfullname$__method_descriptors,\n"
        "  NULL /* CODE_SIZE */\n"
        "};\n");
  } else {
    printer->Print(vars_, "const ProtobufCServiceDescriptor $lcfullname$__descriptor =\n"
        "{\n"
        "  PROTOBUF_C__SERVICE_DESCRIPTOR_MAGIC,\n"
        "  \"$fullname$\",\n"
        "  \"$name$\",\n"
        "  \"$cname$\",\n"
        "  \"$package$\",\n"
        "  $n_methods$,\n"
        "  $lcfullname$__method_descriptors,\n"
        "  $lcfullname$__method_indices_by_name\n"
        "};\n");
  }
}

void ServiceGenerator::GenerateCallersImplementations(google::protobuf::io::Printer* printer)
{
  for (int i = 0; i < descriptor_->method_count(); i++) {
    const google::protobuf::MethodDescriptor* method = descriptor_->method(i);
    std::string lcname = CamelToLower(method->name());
    std::string lcfullname = FullNameToLower(descriptor_->full_name(), descriptor_->file());
    vars_["method"] = lcname;
    vars_["metpad"] = ConvertToSpaces(lcname);
    vars_["input_typename"] = FullNameToC(method->input_type()->full_name(), method->input_type()->file());
    vars_["output_typename"] = FullNameToC(method->output_type()->full_name(), method->output_type()->file());
    vars_["padddddddddddddddddd"] = ConvertToSpaces(lcfullname + "__" + lcname);
    vars_["index"] = SimpleItoa(i);

    printer->Print(vars_,
                   "void $lcfullname$__$method$(ProtobufCService *service,\n"
                   "     $padddddddddddddddddd$ const $input_typename$ *input,\n"
                   "     $padddddddddddddddddd$ $output_typename$_Closure closure,\n"
                   "     $padddddddddddddddddd$ void *closure_data)\n"
		   "{\n"
		   "  assert(service->descriptor == &$lcfullname$__descriptor);\n"
		   "  service->invoke(service, $index$, (const ProtobufCMessage *) input, (ProtobufCClosure) closure, closure_data);\n"
		   "}\n");
  }
}

}  // namespace protobuf_c
