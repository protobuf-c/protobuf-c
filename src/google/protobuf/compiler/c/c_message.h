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

#ifndef GOOGLE_PROTOBUF_COMPILER_C_MESSAGE_H__
#define GOOGLE_PROTOBUF_COMPILER_C_MESSAGE_H__

#include <string>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/compiler/c/c_field.h>

namespace google {
namespace protobuf {
  namespace io {
    class Printer;             // printer.h
  }
}

namespace protobuf {
namespace compiler {
namespace c {

class EnumGenerator;           // enum.h
class ExtensionGenerator;      // extension.h

class MessageGenerator {
 public:
  // See generator.cc for the meaning of dllexport_decl.
  explicit MessageGenerator(const Descriptor* descriptor,
                            const string& dllexport_decl);
  ~MessageGenerator();

  // Header stuff.

  // Generate typedef.
  void GenerateStructTypedef(io::Printer* printer);

  // Generate descriptor prototype
  void GenerateDescriptorDeclarations(io::Printer* printer);

  // Generate descriptor prototype
  void GenerateClosureTypedef(io::Printer* printer);

  // Generate definitions of all nested enums (must come before class
  // definitions because those classes use the enums definitions).
  void GenerateEnumDefinitions(io::Printer* printer);

  // Generate definitions for this class and all its nested types.
  void GenerateStructDefinition(io::Printer* printer);

  // Generate __INIT macro for populating this structure
  void GenerateStructStaticInitMacro(io::Printer* printer);

  // Generate standard helper functions declarations for this message.
  void GenerateHelperFunctionDeclarations(io::Printer* printer, bool is_submessage);

  // Source file stuff.

  // Generate code that initializes the global variable storing the message's
  // descriptor.
  void GenerateMessageDescriptor(io::Printer* printer);
  void GenerateHelperFunctionDefinitions(io::Printer* printer, bool is_submessage);

 private:

  string GetDefaultValueC(const FieldDescriptor *fd);

  const Descriptor* descriptor_;
  string dllexport_decl_;
  FieldGeneratorMap field_generators_;
  scoped_array<scoped_ptr<MessageGenerator> > nested_generators_;
  scoped_array<scoped_ptr<EnumGenerator> > enum_generators_;
  scoped_array<scoped_ptr<ExtensionGenerator> > extension_generators_;

  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(MessageGenerator);
};

}  // namespace c
}  // namespace compiler
}  // namespace protobuf

}  // namespace google
#endif  // GOOGLE_PROTOBUF_COMPILER_C_MESSAGE_H__
