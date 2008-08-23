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

#include <google/protobuf/compiler/c/c_service.h>
#include <google/protobuf/compiler/c/c_helpers.h>
#include <google/protobuf/io/printer.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace c {

ServiceGenerator::ServiceGenerator(const ServiceDescriptor* descriptor,
                                   const string& dllexport_decl)
  : descriptor_(descriptor) {
  vars_["name"] = descriptor_->name();
  vars_["fullname"] = descriptor_->full_name();
  vars_["cname"] = FullNameToC(descriptor_->full_name());
  vars_["lcfullname"] = FullNameToLower(descriptor_->full_name());
  vars_["package"] = descriptor_->file()->package();
  if (dllexport_decl.empty()) {
    vars_["dllexport"] = "";
  } else {
    vars_["dllexport"] = dllexport_decl + " ";
  }
}

ServiceGenerator::~ServiceGenerator() {}

// Header stuff.
void ServiceGenerator::GenerateMainHFile(io::Printer* printer)
{
  GenerateVfuncs(printer);
  GenerateCreateServiceDeclaration(printer);
  GenerateCallersDeclarations(printer);
}
void ServiceGenerator::GenerateVfuncs(io::Printer* printer)
{
  printer->Print(vars_,
		 "typedef struct _$cname$_Service $cname$_Service;\n"
		 "struct _$cname$_Service\n"
		 "{\n");
  for (int i = 0; i < descriptor_->method_count(); i++) {
    const MethodDescriptor *method = descriptor_->method(i);
    string lcname = CamelToLower(method->name());
    vars_["method"] = lcname;
    vars_["metpad"] = ConvertToSpaces(lcname);
    vars_["input_typename"] = FullNameToC(method->input_type()->full_name());
    vars_["output_typename"] = FullNameToC(method->output_type()->full_name());
    printer->Print(vars_,
                   "  void (*$method$)($cname$_Service *service,\n"
                   "         $metpad$  const $input_typename$ *input,\n"
                   "         $metpad$  $output_typename$_Closure closure,\n"
                   "         $metpad$  void *closure_data);\n");
  }
  printer->Print(vars_,
		 "  void (*destroy) ($cname$_Service *service);\n");
  printer->Print(vars_,
		 "};\n");
}
void ServiceGenerator::GenerateCallersDeclarations(io::Printer* printer)
{
  for (int i = 0; i < descriptor_->method_count(); i++) {
    const MethodDescriptor *method = descriptor_->method(i);
    string lcname = CamelToLower(method->name());
    string lcfullname = FullNameToLower(descriptor_->full_name());
    vars_["method"] = lcname;
    vars_["metpad"] = ConvertToSpaces(lcname);
    vars_["input_typename"] = FullNameToC(method->input_type()->full_name());
    vars_["output_typename"] = FullNameToC(method->output_type()->full_name());
    vars_["padddddddddddddddddd"] = ConvertToSpaces(lcfullname + "__" + lcname);
    printer->Print(vars_,
                   "void $lcfullname$__$method$(ProtobufCService *service,\n"
                   "     $padddddddddddddddddd$ const $input_typename$ *input,\n"
                   "     $padddddddddddddddddd$ $output_typename$_Closure closure,\n"
                   "     $padddddddddddddddddd$ void *closure_data);\n");
  }
}

void ServiceGenerator::GenerateCreateServiceDeclaration(io::Printer* printer)
{
  printer->Print(vars_, "ProtobufCService *$lcfullname$__create_service ($cname$_Service *service);\n");
}

void ServiceGenerator::GenerateDescriptorDeclarations(io::Printer* printer)
{
  printer->Print(vars_, "extern const ProtobufCServiceDescriptor $lcfullname$__descriptor;\n");
}


// Source file stuff.
void ServiceGenerator::GenerateCFile(io::Printer* printer)
{
  GenerateServiceDescriptor(printer);
  GenerateCreateService(printer);
  GenerateCallersImplementations(printer);
}
void ServiceGenerator::GenerateServiceDescriptor(io::Printer* printer)
{
  int n_methods = descriptor_->method_count();
  vars_["n_methods"] = SimpleItoa(n_methods);
  printer->Print(vars_, "static const ProtobufCMethodDescriptor $lcfullname$__method_descriptors[$n_methods$] =\n"
                       "{\n");
  for (int i = 0; i < n_methods; i++) {
    const MethodDescriptor *method = descriptor_->method(i);
    vars_["method"] = method->name();
    vars_["input_descriptor"] = "&" + FullNameToLower(method->input_type()->full_name()) + "__descriptor";
    vars_["output_descriptor"] = "&" + FullNameToLower(method->output_type()->full_name()) + "__descriptor";
    printer->Print(vars_,
             "  { \"$method$\", $input_descriptor$, $output_descriptor$ },\n");
  }
  printer->Print(vars_, "};\n");
  printer->Print(vars_, "const ProtobufCServiceDescriptor $lcfullname$__descriptor =\n"
                       "{\n"
		       "  PROTOBUF_C_SERVICE_DESCRIPTOR_MAGIC,\n"
		       "  \"$fullname$\",\n"
		       "  \"$name$\",\n"
		       "  \"$cname$\",\n"
		       "  \"$package$\",\n"
		       "  $n_methods$,\n"
		       "  $lcfullname$__method_descriptors\n"
		       "};\n");
}

void ServiceGenerator::GenerateCreateService(io::Printer* printer)
{
  printer->Print(vars_, "ProtobufCService *\n"
                       "$lcfullname$__create_service ($cname$_Service *service)\n"
		       "{\n"
		       "  return protobuf_c_create_service_from_vfuncs (&$lcfullname$__descriptor, service);\n"
		       "}\n");
}
void ServiceGenerator::GenerateCallersImplementations(io::Printer* printer)
{
  for (int i = 0; i < descriptor_->method_count(); i++) {
    const MethodDescriptor *method = descriptor_->method(i);
    string lcname = CamelToLower(method->name());
    string lcfullname = FullNameToLower(descriptor_->full_name());
    vars_["method"] = lcname;
    vars_["metpad"] = ConvertToSpaces(lcname);
    vars_["input_typename"] = FullNameToC(method->input_type()->full_name());
    vars_["output_typename"] = FullNameToC(method->output_type()->full_name());
    vars_["padddddddddddddddddd"] = ConvertToSpaces(lcfullname + "__" + lcname);
    vars_["index"] = SimpleItoa(i);
     
    printer->Print(vars_,
                   "void $lcfullname$__$method$(ProtobufCService *service,\n"
                   "     $padddddddddddddddddd$ const $input_typename$ *input,\n"
                   "     $padddddddddddddddddd$ $output_typename$_Closure closure,\n"
                   "     $padddddddddddddddddd$ void *closure_data)\n"
		   "{\n"
		   "  PROTOBUF_C_ASSERT (service->descriptor == &$lcfullname$__descriptor);\n"
		   "  service->invoke(service, $index$, (const ProtobufCMessage *) input, (ProtobufCClosure) closure, closure_data);\n"
		   "}\n");
  }
}


}  // namespace c
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
