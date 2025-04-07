set(PACKAGE protobuf-c)
set(PACKAGE_NAME protobuf-c)
set(PACKAGE_VERSION 1.5.2)
set(PACKAGE_URL https://github.com/protobuf-c/protobuf-c)
set(PACKAGE_DESCRIPTION "Protocol Buffers implementation in C")

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0112 NEW)

project(protobuf-c C CXX)

if(MSVC AND NOT BUILD_SHARED_LIBS)
  set(Protobuf_USE_STATIC_LIBS ON)
endif()

find_package(Protobuf CONFIG)
if(Protobuf_FOUND)
  # Keep compatibility with FindProtobuf CMake module
  set(PROTOBUF_PROTOC_EXECUTABLE $<TARGET_FILE:protobuf::protoc>)
  get_target_property(PROTOBUF_INCLUDE_DIR protobuf::libprotobuf INTERFACE_INCLUDE_DIRECTORIES)
else()
  message(STATUS "Protobuf CMake config not found fallback to Cmake Module")
  find_package(Protobuf REQUIRED)
endif()

find_package(absl CONFIG)

# for static protobuf libraries include the dependencies
if(Protobuf_USE_STATIC_LIBS)
  get_property(
    protobuf_ABSL_USED_TARGETS
    DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
    PROPERTY IMPORTED_TARGETS)
  list(FILTER protobuf_ABSL_USED_TARGETS INCLUDE REGEX "absl::")

  find_package(utf8_range CONFIG)

  set(protobuf_UTF8_USED_TARGETS
      $<TARGET_NAME_IF_EXISTS:utf8_range::utf8_validity>
      $<TARGET_NAME_IF_EXISTS:utf8_range::utf8_range>)
elseif(WIN32)
  set(protobuf_ABSL_USED_TARGETS $<TARGET_NAME_IF_EXISTS:absl::abseil_dll>)
endif()

# options
option(BUILD_PROTOC "Build protoc-gen-c" ON)
if(CMAKE_BUILD_TYPE MATCHES Debug)
  option(BUILD_TESTS "Build tests" ON)
else()
  option(BUILD_TESTS "Build tests" OFF)
endif()

include(TestBigEndian)
test_big_endian(WORDS_BIGENDIAN)

include(GNUInstallDirs)

set(PROTOBUF_C_TARGETS "protobuf-c")
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
add_definitions(-DPACKAGE_VERSION="${PACKAGE_VERSION}")
add_definitions(-DPACKAGE_STRING="${PACKAGE_STRING}")
if(${WORDS_BIGENDIAN})
  add_definitions(-DWORDS_BIGENDIAN)
endif()

if(MSVC AND BUILD_SHARED_LIBS)
  add_definitions(-DPROTOBUF_C_USE_SHARED_LIB)
endif(MSVC AND BUILD_SHARED_LIBS)

if(MSVC)
  # using Visual Studio C++
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4267 /wd4244")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4267 /wd4244")

  # Allow matching protobuf runtime dependency
  if(NOT BUILD_SHARED_LIBS)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  endif(NOT BUILD_SHARED_LIBS)

endif()

get_filename_component(MAIN_DIR ${CMAKE_CURRENT_SOURCE_DIR} PATH)
set(TEST_DIR ${MAIN_DIR}/t)

add_library(protobuf-c ${MAIN_DIR}/protobuf-c/protobuf-c.c)
set_target_properties(protobuf-c PROPERTIES COMPILE_PDB_NAME protobuf-c)
# Both <protobuf-c/protobuf-c.h> and "protobuf-c.h" are used
target_include_directories(
  protobuf-c PUBLIC $<BUILD_INTERFACE:${MAIN_DIR}>
                    $<BUILD_INTERFACE:${MAIN_DIR}/protobuf-c>
                    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                    ${PROTOBUF_INCLUDE_DIR})
if(MSVC AND BUILD_SHARED_LIBS)
  target_compile_definitions(protobuf-c PRIVATE -DPROTOBUF_C_EXPORT)
endif()
target_link_libraries(protobuf-c ${protobuf_ABSL_USED_TARGETS}
                      ${protobuf_UTF8_USED_TARGETS})
target_compile_features(protobuf-c PRIVATE cxx_std_17)

if(BUILD_PROTOC)
  include_directories(${CMAKE_CURRENT_BINARY_DIR}) # for generated files
endif()

if(MSVC AND NOT BUILD_SHARED_LIBS)
  # In case we are building static libraries, link also the runtime library
  # statically so that MSVCR*.DLL is not required at runtime.
  # https://msdn.microsoft.com/en-us/library/2kzt1wy3.aspx This is achieved by
  # replacing msvc option /MD with /MT and /MDd with /MTd
  # http://www.cmake.org/Wiki/CMake_FAQ#How_can_I_build_my_MSVC_application_with_a_static_runtime.3F
  foreach(
    flag_var
    CMAKE_CXX_FLAGS
    CMAKE_CXX_FLAGS_DEBUG
    CMAKE_CXX_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_MINSIZEREL
    CMAKE_CXX_FLAGS_RELWITHDEBINFO
    CMAKE_C_FLAGS
    CMAKE_C_FLAGS_DEBUG
    CMAKE_C_FLAGS_RELEASE
    CMAKE_C_FLAGS_MINSIZEREL
    CMAKE_FLAGS_RELWITHDEBINFO)
    if(${flag_var} MATCHES "/MD")
      string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif(${flag_var} MATCHES "/MD")
  endforeach(flag_var)
endif()

if(WIN32)
  # Modify the environment to hint protoc where the plugin is prepend to PATH
  # because github host runners have abseil dll pre-installed. Use %PATH%
  # instead of actually inserting it. On Github runners the PATH is so long it
  # makes the NMake generated commands to fail.
  set(OS_PATH_VARIABLE "$<TARGET_FILE_DIR:protoc-gen-c>\\;%PATH%")

  if(BUILD_SHARED_LIBS)
    set(OS_PATH_VARIABLE
        "$<TARGET_FILE_DIR:protobuf::protoc>\\;${OS_PATH_VARIABLE}")
    if(TARGET absl::abseil_dll)
      set(OS_PATH_VARIABLE
          "$<TARGET_FILE_DIR:absl::abseil_dll>\\;${OS_PATH_VARIABLE}")
    endif()
  endif()
else(WIN32)
  set(OS_PATH_VARIABLE "$ENV{PATH}")
  set(OS_PATH_VARIABLE "$<TARGET_FILE_DIR:protoc-gen-c>:${OS_PATH_VARIABLE}")
endif()

if(BUILD_PROTOC)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)

  add_custom_target(
    protoc-generated-files
    COMMAND
      ${CMAKE_COMMAND} -E env PATH="${OS_PATH_VARIABLE}"
      ${PROTOBUF_PROTOC_EXECUTABLE} --cpp_out ${CMAKE_CURRENT_BINARY_DIR}
      -I${MAIN_DIR} -I${PROTOBUF_INCLUDE_DIR}
      ${MAIN_DIR}/protobuf-c/protobuf-c.proto
    COMMENT Running
    protoc on ${MAIN_DIR}/protobuf-c/protobuf-c.proto
    BYPRODUCTS protobuf-c/protobuf-c.pb.cc protobuf-c/protobuf-c.pb.h
    SOURCES ${MAIN_DIR}/protobuf-c/protobuf-c.proto)

  file(GLOB PROTOC_GEN_C_SRC ${MAIN_DIR}/protoc-gen-c/*.cc)
  add_executable(
    protoc-gen-c
    ${PROTOC_GEN_C_SRC} ${CMAKE_CURRENT_BINARY_DIR}/protobuf-c/protobuf-c.pb.cc)
  add_dependencies(protoc-gen-c protoc-generated-files)
  target_include_directories(
    protoc-gen-c PUBLIC $<BUILD_INTERFACE:${MAIN_DIR}>
                        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
  target_link_libraries(
    protoc-gen-c protobuf-c
    protobuf::libprotoc protobuf::libprotobuf
    ${protobuf_ABSL_USED_TARGETS} ${protobuf_UTF8_USED_TARGETS})

  target_compile_features(protoc-gen-c PRIVATE cxx_std_17)

  if(MSVC AND BUILD_SHARED_LIBS)
    target_compile_definitions(protoc-gen-c PRIVATE -DPROTOBUF_USE_DLLS)
    get_filename_component(PROTOBUF_DLL_DIR ${PROTOBUF_PROTOC_EXECUTABLE}
                           DIRECTORY)
    file(GLOB PROTOBUF_DLLS ${PROTOBUF_DLL_DIR}/*.dll)
    file(COPY ${PROTOBUF_DLLS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
  endif()

  function(GENERATE_TEST_SOURCES PROTO_FILE SRC HDR)
    add_custom_command(
      OUTPUT ${SRC} ${HDR}
      COMMAND
        ${CMAKE_COMMAND} ARGS -E env PATH="${OS_PATH_VARIABLE}"
        ${PROTOBUF_PROTOC_EXECUTABLE} --plugin=$<TARGET_FILE_NAME:protoc-gen-c>
        -I${MAIN_DIR} ${PROTO_FILE} --c_out=${CMAKE_CURRENT_BINARY_DIR}
      DEPENDS protoc-gen-c)
  endfunction()

  if(BUILD_TESTS)
    enable_testing()

    generate_test_sources(${TEST_DIR}/test.proto t/test.pb-c.c t/test.pb-c.h)

    add_executable(
      test-generated-code ${TEST_DIR}/generated-code/test-generated-code.c
                          t/test.pb-c.c t/test.pb-c.h)
    target_link_libraries(test-generated-code protobuf-c)

    add_custom_command(
      OUTPUT t/test-full.pb.cc t/test-full.pb.h
      COMMAND
        ${CMAKE_COMMAND} ARGS -E env PATH="${OS_PATH_VARIABLE}"
        ${PROTOBUF_PROTOC_EXECUTABLE} --cpp_out ${CMAKE_CURRENT_BINARY_DIR}
        -I${MAIN_DIR} ${TEST_DIR}/test-full.proto)

    generate_test_sources(${TEST_DIR}/test-full.proto t/test-full.pb-c.c
                          t/test-full.pb-c.h)

    add_executable(
      cxx-generate-packed-data
      ${TEST_DIR}/generated-code2/cxx-generate-packed-data.cc t/test-full.pb.h
      t/test-full.pb.cc protobuf-c/protobuf-c.pb.cc protobuf-c/protobuf-c.pb.h)
    target_link_libraries(
      cxx-generate-packed-data protobuf::libprotobuf
      ${protobuf_ABSL_USED_TARGETS} ${protobuf_UTF8_USED_TARGETS})
    if(MSVC AND BUILD_SHARED_LIBS)
      target_compile_definitions(cxx-generate-packed-data
                                 PRIVATE -DPROTOBUF_USE_DLLS)
    endif()

    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/t/generated-code2)
    add_custom_command(
      OUTPUT t/generated-code2/test-full-cxx-output.inc
      COMMAND
        ${CMAKE_COMMAND} ARGS -E env PATH="${OS_PATH_VARIABLE}"
        cxx-generate-packed-data ">t/generated-code2/test-full-cxx-output.inc"
      DEPENDS cxx-generate-packed-data)

    generate_test_sources(${TEST_DIR}/test-optimized.proto
                          t/test-optimized.pb-c.c t/test-optimized.pb-c.h)

    add_executable(
      test-generated-code2
      ${TEST_DIR}/generated-code2/test-generated-code2.c
      t/generated-code2/test-full-cxx-output.inc t/test-full.pb-c.h
      t/test-full.pb-c.c t/test-optimized.pb-c.h t/test-optimized.pb-c.c)
    target_link_libraries(test-generated-code2 protobuf-c)

    generate_test_sources(${TEST_DIR}/issue220/issue220.proto
                          t/issue220/issue220.pb-c.c t/issue220/issue220.pb-c.h)
    add_executable(
      test-issue220 ${TEST_DIR}/issue220/issue220.c t/issue220/issue220.pb-c.c
                    t/issue220/issue220.pb-c.h)
    target_link_libraries(test-issue220 protobuf-c)

    generate_test_sources(${TEST_DIR}/issue251/issue251.proto
                          t/issue251/issue251.pb-c.c t/issue251/issue251.pb-c.h)
    add_executable(
      test-issue251 ${TEST_DIR}/issue251/issue251.c t/issue251/issue251.pb-c.c
                    t/issue251/issue251.pb-c.h)
    target_link_libraries(test-issue251 protobuf-c)

    add_executable(test-version ${TEST_DIR}/version/version.c)
    target_link_libraries(test-version protobuf-c)

    generate_test_sources(${TEST_DIR}/test-proto3.proto t/test-proto3.pb-c.c
                          t/test-proto3.pb-c.h)
    add_executable(
      test-generated-code3 ${TEST_DIR}/generated-code/test-generated-code.c
                           t/test-proto3.pb-c.c t/test-proto3.pb-c.h)
    target_compile_definitions(test-generated-code3 PUBLIC -DPROTO3)
    target_link_libraries(test-generated-code3 protobuf-c)

  endif()

  # https://github.com/protocolbuffers/protobuf/issues/5107
  if(CMAKE_HOST_UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries(protoc-gen-c ${CMAKE_THREAD_LIBS_INIT})
    if(BUILD_TESTS)
      target_link_libraries(cxx-generate-packed-data ${CMAKE_THREAD_LIBS_INIT})
    endif()
  endif()

  list(APPEND PROTOBUF_C_TARGETS "protoc-gen-c")
endif() # BUILD_PROTOC

install(
  TARGETS ${PROTOBUF_C_TARGETS}
  EXPORT protobuf-c-targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin)

install(FILES ${MAIN_DIR}/protobuf-c/protobuf-c.h
              ${MAIN_DIR}/protobuf-c/protobuf-c.proto
        DESTINATION include/protobuf-c)
install(FILES ${MAIN_DIR}/protobuf-c/protobuf-c.h DESTINATION include)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/protobuf-c.pdb
  DESTINATION lib
  OPTIONAL)

set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix \${prefix})
set(bindir \${exec_prefix}/${CMAKE_INSTALL_BINDIR})
set(libdir \${exec_prefix}/${CMAKE_INSTALL_LIBDIR})
set(includedir \${prefix}/${CMAKE_INSTALL_INCLUDEDIR})
configure_file(${MAIN_DIR}/protobuf-c/libprotobuf-c.pc.in libprotobuf-c.pc
               @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libprotobuf-c.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

install(
  EXPORT protobuf-c-targets
  NAMESPACE protobuf-c::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/protobuf-c)

include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/protobuf-c-config.cmake"
  INSTALL_DESTINATION "lib/cmake/protobuf-c")

write_basic_package_version_file(
  protobuf-c-config-version.cmake
  VERSION ${PACKAGE_VERSION}
  COMPATIBILITY SameMajorVersion)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/protobuf-c-config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/protobuf-c-config-version.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/protobuf-c)

export(EXPORT protobuf-c-targets
       FILE "${CMAKE_CURRENT_BINARY_DIR}/protobuf-c-targets.cmake")

if(BUILD_TESTS)
  enable_testing()

  set(CTEST_TEST_TIMEOUT 5)
  add_test(test-generated-code test-generated-code)
  add_test(test-generated-code2 test-generated-code2)
  add_test(test-generated-code3 test-generated-code3)
  add_test(test-issue220 test-issue220)
  add_test(test-issue251 test-issue251)
  add_test(test-version test-version)

  if(WIN32)
    set_tests_properties(
      test-generated-code test-generated-code2 test-generated-code3
      test-issue220 test-issue251 test-version
      PROPERTIES
        ENVIRONMENT
        "PATH=${WINDOWS_PATH_VARIABLE}\\;$<TARGET_FILE_DIR:protoc-gen-c>")
  endif()

endif()

include(CPack)
