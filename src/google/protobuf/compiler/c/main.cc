#include <google/protobuf/compiler/command_line_interface.h>
#include <google/protobuf/compiler/c/c_generator.h>


int main(int argc, char* argv[]) {
  google::protobuf::compiler::CommandLineInterface cli;

  // Support generation of Foo code.
  google::protobuf::compiler::c::CGenerator c_generator;
  cli.RegisterGenerator("--c_out", &c_generator,
  "Generate C/H files.");
  
  return cli.Run(argc, argv);
}
