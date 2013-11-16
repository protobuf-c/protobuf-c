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

#include <google/protobuf/compiler/c/c_extension.h>
#include <google/protobuf/compiler/c/c_helpers.h>
#include <google/protobuf/io/printer.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace c {

ExtensionGenerator::ExtensionGenerator(const FieldDescriptor* descriptor,
                                       const string& dllexport_decl)
  : descriptor_(descriptor),
    dllexport_decl_(dllexport_decl) {
}

ExtensionGenerator::~ExtensionGenerator() {}

void ExtensionGenerator::GenerateDeclaration(io::Printer* printer) {
}

void ExtensionGenerator::GenerateDefinition(io::Printer* printer) {
}

}  // namespace c
}  // namespace compiler
}  // namespace protobuf

}  // namespace google
