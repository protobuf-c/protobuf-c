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

#ifndef GOOGLE_PROTOBUF_COMPILER_C_FIELD_H__
#define GOOGLE_PROTOBUF_COMPILER_C_FIELD_H__

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/descriptor.h>

namespace google {
namespace protobuf {
  namespace io {
    class Printer;             // printer.h
  }
}

namespace protobuf {
namespace compiler {
namespace c {

class FieldGenerator {
 public:
  explicit FieldGenerator(const FieldDescriptor *descriptor) : descriptor_(descriptor) {}
  virtual ~FieldGenerator();

  // Generate definitions to be included in the structure.
  virtual void GenerateStructMembers(io::Printer* printer) const = 0;

  // Generate a static initializer for this field.
  virtual void GenerateDescriptorInitializer(io::Printer* printer) const = 0;

  virtual void GenerateDefaultValueDeclarations(io::Printer* printer) const { }
  virtual void GenerateDefaultValueImplementations(io::Printer* printer) const { }
  virtual string GetDefaultValue() const = 0;

  // Generate members to initialize this field from a static initializer
  virtual void GenerateStaticInit(io::Printer* printer) const = 0;


 protected:
  void GenerateDescriptorInitializerGeneric(io::Printer* printer,
                                            bool optional_uses_has,
                                            const string &type_macro,
                                            const string &descriptor_addr) const;
  const FieldDescriptor *descriptor_;

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(FieldGenerator);
};

// Convenience class which constructs FieldGenerators for a Descriptor.
class FieldGeneratorMap {
 public:
  explicit FieldGeneratorMap(const Descriptor* descriptor);
  ~FieldGeneratorMap();

  const FieldGenerator& get(const FieldDescriptor* field) const;

 private:
  const Descriptor* descriptor_;
  scoped_array<scoped_ptr<FieldGenerator> > field_generators_;

  static FieldGenerator* MakeGenerator(const FieldDescriptor* field);

  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(FieldGeneratorMap);
};

}  // namespace c
}  // namespace compiler
}  // namespace protobuf

}  // namespace google
#endif  // GOOGLE_PROTOBUF_COMPILER_C_FIELD_H__
