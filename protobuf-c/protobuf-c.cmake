find_package(Protobuf REQUIRED)
find_program(PROTOC_GEN_C protoc-gen-c REQUIRED)

function(protobuf_generate_c SRCS HDRS)
  cmake_parse_arguments(protobuf_generate_c "" "EXPORT_MACRO;DESCRIPTORS" ""
      ${ARGN})

  set(_proto_files "${protobuf_generate_c_UNPARSED_ARGUMENTS}")
  if(NOT _proto_files)
    message(
        SEND_ERROR "Error: protobuf_generate_c() called without any proto files")
    return()
  endif()

  if(protobuf_generate_c_APPEND_PATH)
    set(_append_arg APPEND_PATH)
  endif()

  if(protobuf_generate_c_DESCRIPTORS)
    set(_descriptors DESCRIPTORS)
  endif()

  if(DEFINED Protobuf_IMPORT_DIRS)
    set(_import_arg IMPORT_DIRS ${Protobuf_IMPORT_DIRS})
  endif()

  set(_outvar)
  protobuf_generate(
      ${_append_arg}
      ${_descriptors}
      LANGUAGE
      c
      PLUGIN
      ${PROTOC_GEN_C}
      GENERATE_EXTENSIONS
      .pb-c.h
      .pb-c.c
      EXPORT_MACRO
      ${protobuf_generate_c_EXPORT_MACRO}
      OUT_VAR
      _outvar
      ${_import_arg}
      PROTOS
      ${_proto_files})

  set(${SRCS})
  set(${HDRS})
  if(protobuf_generate_c_DESCRIPTORS)
    set(${protobuf_generate_c_DESCRIPTORS})
  endif()

  foreach(_file ${_outvar})
    if(_file MATCHES "c$")
      list(APPEND ${SRCS} ${_file})
    elseif(_file MATCHES "desc$")
      list(APPEND ${protobuf_generate_c_DESCRIPTORS} ${_file})
    else()
      list(APPEND ${HDRS} ${_file})
    endif()
  endforeach()
  set(${SRCS}
      ${${SRCS}}
      PARENT_SCOPE)
  set(${HDRS}
      ${${HDRS}}
      PARENT_SCOPE)
  if(protobuf_generate_c_DESCRIPTORS)
    set(${protobuf_generate_c_DESCRIPTORS}
        "${${protobuf_generate_c_DESCRIPTORS}}"
        PARENT_SCOPE)
  endif()
endfunction()
