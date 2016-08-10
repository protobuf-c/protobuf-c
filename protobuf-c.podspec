Pod::Spec.new do |s|
  s.name = "protobuf-c"
  s.version = "0.16-beta1"
  s.summary = "C bindings for Google's Protocol Buffers"
  s.authors = { "Dave Benson" => "",
                "Ilya Lipnitskiy" => "",
                "edmonds" => "",
                "Nick Galbreath" => "",
                "Maximilian Szengel" => "m@maxsz.de" }
  s.source = { :git => "https://github.com/protobuf-c/protobuf-c.git.git", :tag => "v#{s.version}" }
  s.homepage = "https://github.com/protobuf-c/protobuf-c"
  s.license = { :type => "BSD", :file => "LICENSE"}
  s.source_files = "protobuf-c/*.{h,c}"
  s.header_mappings_dir = "./"
  s.prefix_header_contents = "#define HAVE_ALLOCA_H 1",
                             "#define HAVE_MALLOC_H 1",
                             "#define HAVE_SYS_POLL_H 1",
                             "#define HAVE_SYS_SELECT_H 1",
                             "#define HAVE_INTTYPES_H 1",
                             "#define HAVE_SYS_UIO_H 1",
                             "#define HAVE_UNISTD_H 1",
                             "#define HAVE_IO_H 1"
  s.requires_arc = false
  s.xcconfig = { "GCC_WARN_64_TO_32_BIT_CONVERSION" => "NO" }
end
