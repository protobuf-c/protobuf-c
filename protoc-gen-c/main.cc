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

#include <string>

#include <google/protobuf/compiler/command_line_interface.h>
#include <google/protobuf/compiler/plugin.h>

#include "c_generator.h"
#include "c_helpers.h"

int main(int argc, char* argv[]) {
  protobuf_c::CGenerator c_generator;

  std::string invocation_name = argv[0];
  std::string invocation_basename = invocation_name.substr(invocation_name.find_last_of("/") + 1);
  const std::string standalone_name = "protoc-c";

  if (invocation_basename == standalone_name) {
    GOOGLE_LOG(WARNING) << "`protoc-c` is deprecated. Please use `protoc` instead!";
    google::protobuf::compiler::CommandLineInterface cli;
    cli.RegisterGenerator("--c_out", &c_generator, "Generate C/H files.");
    cli.SetVersionInfo(PACKAGE_STRING);
    return cli.Run(argc, argv);
  }

  return google::protobuf::compiler::PluginMain(argc, argv, &c_generator);
}
