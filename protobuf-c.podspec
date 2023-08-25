Pod::Spec.new do |spec|
  spec.name        = "protobuf-c"
  spec.version     = "1.4.1"
  spec.summary     = "C implementation of the Google Protocol Buffers data serialization format"
  spec.authors     = { "Dave Benson" => "" }
  spec.source      = { 
    :git => "https://github.com/protobuf-c/protobuf-c.git", 
    :tag => "v#{spec.version}" 
  }
  spec.homepage    = "https://github.com/protobuf-c/protobuf-c"
  spec.license     = { :type => "BSD", :file => "LICENSE" }

  spec.ios.deployment_target     = '12.0'
  # spec.tvos.deployment_target    = '12.0'
  spec.macos.deployment_target   = '10.13'
  # spec.watchos.deployment_target = '7.0'
  spec.requires_arc              = false

  spec.source_files        = 'protobuf-c/protobuf-c.h', 'protobuf-c/protobuf-c.c'
  spec.public_header_files = 'protobuf-c/protobuf-c.h'
  spec.header_dir          = 'protobuf-c'
  spec.xcconfig            = { "GCC_WARN_64_TO_32_BIT_CONVERSION" => "NO" }
end
