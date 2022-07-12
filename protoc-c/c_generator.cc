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

// Copyright (c) 2008-2013, Dave Benson.  All rights reserved.
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

#include <protoc-c/c_generator.h>
#include <protoc-c/options.h>

#include <memory>
#include <vector>
#include <utility>

#include <protoc-c/c_file.h>
#include <protoc-c/c_helpers.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <protobuf-c/protobuf-c.pb.h>
#include <protoc-c/options.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace c {

GeneratorOptions g_generator_options;

// Parses a set of comma-delimited name/value pairs, e.g.:
//   "foo=bar,baz,qux=corge"
// parses to the pairs:
//   ("foo", "bar"), ("baz", ""), ("qux", "corge")
void ParseOptions(const std::string& text, GeneratorOptions& output) {
  std::vector<std::string> parts;
  SplitStringUsing(text, ",", &parts);

  for (unsigned i = 0; i < parts.size(); i++) {
    std::string::size_type equals_pos = parts[i].find_first_of('=');
    if (equals_pos == std::string::npos) {
      output[parts[i]] = "";
    } else {
      output[parts[i].substr(0, equals_pos)] = parts[i].substr(equals_pos + 1);
    }
  }
}

CGenerator::CGenerator() {}
CGenerator::~CGenerator() {}

bool CGenerator::Generate(const FileDescriptor* file,
                            const std::string& parameter,
                            OutputDirectory* output_directory,
                            std::string* error) const {
  if (file->options().GetExtension(pb_c_file).no_generate())
    return true;

  // parse generator options
  ParseOptions(parameter, g_generator_options);

  // For option --c_opt=dllexport_decl=FOO_EXPORT:
  // If the dllexport_decl option is passed to the compiler, we need to write
  // it in front of every symbol that should be exported if this .proto is
  // compiled into a Windows DLL.  E.g., if the user invokes the protocol
  // compiler as:
  //   protoc-c --c_opt=outdir --c_opt=dllexport_decl=FOO_EXPORT foo.proto
  // then we'll define classes like this:
  //   struct FOO_EXPORT Foo {
  //     ...
  //   }
  // FOO_EXPORT is a macro which should expand to __declspec(dllexport) or
  // __declspec(dllimport) depending on what is being compiled.
  //
  // for option --c_opt=disable_lowercase_name:
  // Identifier case will remain the same.

  for (auto &it : g_generator_options) {
    if (it.first != "dllexport_decl" &&
        it.first != "disable_lowercase_name") {
      *error = "Unknown generator option: " + it.first;
      return false;
    }
  }

  std::string basename = StripProto(file->name());
  basename.append(".pb-c");

  FileGenerator file_generator(file);

  // Generate header.
  {
    std::unique_ptr<io::ZeroCopyOutputStream> output(
      output_directory->Open(basename + ".h"));
    io::Printer printer(output.get(), '$');
    file_generator.GenerateHeader(&printer);
  }

  // Generate cc file.
  {
    std::unique_ptr<io::ZeroCopyOutputStream> output(
      output_directory->Open(basename + ".c"));
    io::Printer printer(output.get(), '$');
    file_generator.GenerateSource(&printer);
  }

  return true;
}

}  // namespace c
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
