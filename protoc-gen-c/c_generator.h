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

// Generates C code for a given .proto file.

#ifndef PROTOBUF_C_PROTOC_GEN_C_C_GENERATOR_H__
#define PROTOBUF_C_PROTOC_GEN_C_C_GENERATOR_H__

#include <string>

#include <google/protobuf/compiler/code_generator.h>

#if defined(_WIN32) && defined(PROTOBUF_C_USE_SHARED_LIB)
# define PROTOBUF_C_EXPORT __declspec(dllexport)
#else
# define PROTOBUF_C_EXPORT
#endif

namespace protobuf_c {

// CodeGenerator implementation which generates a C++ source file and
// header.  If you create your own protocol compiler binary and you want
// it to support C++ output, you can do so by registering an instance of this
// CodeGenerator with the CommandLineInterface in your main() function.
class PROTOBUF_C_EXPORT CGenerator : public google::protobuf::compiler::CodeGenerator {
 public:
  CGenerator();
  ~CGenerator();

  // implements CodeGenerator ----------------------------------------
  bool Generate(const google::protobuf::FileDescriptor* file,
                const std::string& parameter,
                google::protobuf::compiler::OutputDirectory* output_directory,
                std::string* error) const;

#if GOOGLE_PROTOBUF_VERSION >= 5026000
  uint64_t GetSupportedFeatures() const override {
    // Indicate that this code generator supports proto3 optional fields.
    return FEATURE_PROTO3_OPTIONAL;
  }
  google::protobuf::Edition GetMinimumEdition() const { return google::protobuf::Edition::EDITION_PROTO2; }
  google::protobuf::Edition GetMaximumEdition() const { return google::protobuf::Edition::EDITION_PROTO3; }
#endif
};

}  // namespace protobuf_c

#endif  // PROTOBUF_C_PROTOC_GEN_C_C_GENERATOR_H__
