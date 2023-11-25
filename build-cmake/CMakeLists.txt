SET(PACKAGE protobuf-c)
SET(PACKAGE_NAME protobuf-c)
SET(PACKAGE_VERSION 1.5.0)
SET(PACKAGE_URL https://github.com/protobuf-c/protobuf-c)
SET(PACKAGE_DESCRIPTION "Protocol Buffers implementation in C")

CMAKE_MINIMUM_REQUIRED(VERSION 3.10 FATAL_ERROR)
cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0112 NEW)

PROJECT(protobuf-c C CXX)

if (MSVC AND NOT BUILD_SHARED_LIBS)
	SET(Protobuf_USE_STATIC_LIBS ON)
endif (MSVC AND NOT BUILD_SHARED_LIBS)

FIND_PACKAGE(Protobuf REQUIRED)
file(REAL_PATH "${PROTOBUF_INCLUDE_DIR}" PROTOBUF_INCLUDE_DIR)
INCLUDE_DIRECTORIES(${PROTOBUF_INCLUDE_DIR})

find_package(absl CONFIG)

# for static protobuf libraries include the dependencies
if (Protobuf_USE_STATIC_LIBS)
    get_property(protobuf_ABSL_USED_TARGETS DIRECTORY "${CMAKE_CURRENT_LIST_DIR}" PROPERTY IMPORTED_TARGETS)
    list(FILTER protobuf_ABSL_USED_TARGETS INCLUDE REGEX "absl::")

    find_package(utf8_range CONFIG)

    set(protobuf_UTF8_USED_TARGETS
        $<TARGET_NAME_IF_EXISTS:utf8_range::utf8_validity>
        $<TARGET_NAME_IF_EXISTS:utf8_range::utf8_range>
    )
elseif(WIN32)
    set(protobuf_ABSL_USED_TARGETS $<TARGET_NAME_IF_EXISTS:absl::abseil_dll>)
endif()

#options
option(BUILD_PROTOC "Build protoc-gen-c" ON)
if(CMAKE_BUILD_TYPE MATCHES Debug)
option(BUILD_TESTS "Build tests" ON)
else()
option(BUILD_TESTS "Build tests" OFF)
endif()

INCLUDE(TestBigEndian)
TEST_BIG_ENDIAN(WORDS_BIGENDIAN)

include(GNUInstallDirs)

set(PROTOBUF_C_TARGETS "protobuf-c")
SET(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
ADD_DEFINITIONS(-DPACKAGE_VERSION="${PACKAGE_VERSION}")
ADD_DEFINITIONS(-DPACKAGE_STRING="${PACKAGE_STRING}")
if (${WORDS_BIGENDIAN})
	ADD_DEFINITIONS(-DWORDS_BIGENDIAN)
endif()

IF (MSVC AND BUILD_SHARED_LIBS)
	ADD_DEFINITIONS(-DPROTOBUF_C_USE_SHARED_LIB)
ENDIF (MSVC AND BUILD_SHARED_LIBS)

if(MSVC)
  # using Visual Studio C++
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4267 /wd4244")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4267 /wd4244")

  # Allow matching protobuf runtime dependency
  if(NOT BUILD_SHARED_LIBS)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  endif(NOT BUILD_SHARED_LIBS)

ENDIF()

get_filename_component(MAIN_DIR ${CMAKE_CURRENT_SOURCE_DIR} PATH)
SET(TEST_DIR ${MAIN_DIR}/t)

MESSAGE(${MAIN_DIR})

SET (PC_SOURCES
        ${MAIN_DIR}/protobuf-c/protobuf-c.c
        ${MAIN_DIR}/protobuf-c/protobuf-c.h)

ADD_LIBRARY(protobuf-c ${PC_SOURCES})
set_target_properties(protobuf-c PROPERTIES COMPILE_PDB_NAME protobuf-c)
target_include_directories(protobuf-c
  PUBLIC
    $<BUILD_INTERFACE:${MAIN_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
IF (MSVC AND BUILD_SHARED_LIBS)
	TARGET_COMPILE_DEFINITIONS(protobuf-c PRIVATE -DPROTOBUF_C_EXPORT)
ENDIF (MSVC AND BUILD_SHARED_LIBS)
target_link_libraries(protobuf-c ${protobuf_ABSL_USED_TARGETS} ${protobuf_UTF8_USED_TARGETS})
target_compile_features(protobuf-c PRIVATE cxx_std_17)

INCLUDE_DIRECTORIES(${MAIN_DIR})
INCLUDE_DIRECTORIES(${MAIN_DIR}/protobuf-c)

IF(BUILD_PROTOC)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}) # for generated files

ENDIF()

if (MSVC AND NOT BUILD_SHARED_LIBS)
	# In case we are building static libraries, link also the runtime library statically
	# so that MSVCR*.DLL is not required at runtime.
	# https://msdn.microsoft.com/en-us/library/2kzt1wy3.aspx
	# This is achieved by replacing msvc option /MD with /MT and /MDd with /MTd
	# http://www.cmake.org/Wiki/CMake_FAQ#How_can_I_build_my_MSVC_application_with_a_static_runtime.3F
	foreach(flag_var
		CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
		CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
		CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE 
		CMAKE_C_FLAGS_MINSIZEREL CMAKE_FLAGS_RELWITHDEBINFO)
	  if(${flag_var} MATCHES "/MD")
		string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
	  endif(${flag_var} MATCHES "/MD")
	endforeach(flag_var)
endif (MSVC AND NOT BUILD_SHARED_LIBS)

if(WIN32)
    # Modify the environment to hint protoc where the plugin is
    # prepend to PATH because github host runners have abseil dll pre-installed.
    # Use %PATH% instead of actually inserting it. On Github runners the PATH is so long
    # it makes the NMake generated commands to fail.
    set(OS_PATH_VARIABLE "$<TARGET_FILE_DIR:protoc-gen-c>\\;%PATH%")

    if(BUILD_SHARED_LIBS)
        set(OS_PATH_VARIABLE "$<TARGET_FILE_DIR:protobuf::protoc>\\;${OS_PATH_VARIABLE}")
        if (TARGET absl::abseil_dll)
            set(OS_PATH_VARIABLE "$<TARGET_FILE_DIR:absl::abseil_dll>\\;${OS_PATH_VARIABLE}")
        endif()
    endif()

else(WIN32)

    set(OS_PATH_VARIABLE "$ENV{PATH}" )
    set(OS_PATH_VARIABLE "$<TARGET_FILE_DIR:protoc-gen-c>:${OS_PATH_VARIABLE}")

endif(WIN32)

IF(BUILD_PROTOC)
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)

add_custom_target(protoc-generated-files
       COMMAND ${CMAKE_COMMAND} -E env PATH="${OS_PATH_VARIABLE}" -- ${PROTOBUF_PROTOC_EXECUTABLE}
            --cpp_out ${CMAKE_CURRENT_BINARY_DIR} -I${PROTOBUF_INCLUDE_DIR}
            -I${MAIN_DIR} ${MAIN_DIR}/protobuf-c/protobuf-c.proto
       COMMENT Running protoc on ${MAIN_DIR}/protobuf-c/protobuf-c.proto
       BYPRODUCTS protobuf-c/protobuf-c.pb.cc protobuf-c/protobuf-c.pb.h
       SOURCES ${MAIN_DIR}/protobuf-c/protobuf-c.proto
)

file(GLOB PROTOC_GEN_C_SRC ${MAIN_DIR}/protoc-c/*.h ${MAIN_DIR}/protoc-c/*.cc )
add_executable(protoc-gen-c
    ${PROTOC_GEN_C_SRC}
    ${CMAKE_CURRENT_BINARY_DIR}/protobuf-c/protobuf-c.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/protobuf-c/protobuf-c.pb.h
)
add_dependencies(protoc-gen-c protoc-generated-files)
target_include_directories(protoc-gen-c
  PUBLIC
    $<BUILD_INTERFACE:${MAIN_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(protoc-gen-c
    protobuf::libprotoc
    protobuf::libprotobuf
    ${protobuf_ABSL_USED_TARGETS}
    ${protobuf_UTF8_USED_TARGETS}
)

target_compile_features(protoc-gen-c PRIVATE cxx_std_17)

IF (MSVC AND BUILD_SHARED_LIBS)
	TARGET_COMPILE_DEFINITIONS(protoc-gen-c PRIVATE -DPROTOBUF_USE_DLLS)
	GET_FILENAME_COMPONENT(PROTOBUF_DLL_DIR ${PROTOBUF_PROTOC_EXECUTABLE} DIRECTORY)
	FILE(GLOB PROTOBUF_DLLS ${PROTOBUF_DLL_DIR}/*.dll)
	FILE(COPY ${PROTOBUF_DLLS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
ENDIF (MSVC AND BUILD_SHARED_LIBS)

FUNCTION(GENERATE_TEST_SOURCES PROTO_FILE SRC HDR)
    ADD_CUSTOM_COMMAND(OUTPUT ${SRC} ${HDR}
       COMMAND ${CMAKE_COMMAND}
       ARGS -E env PATH="${OS_PATH_VARIABLE}" -- ${PROTOBUF_PROTOC_EXECUTABLE}
            --plugin=$<TARGET_FILE_NAME:protoc-gen-c> -I${MAIN_DIR} ${PROTO_FILE} --c_out=${CMAKE_CURRENT_BINARY_DIR}
       DEPENDS protoc-gen-c)
ENDFUNCTION()

IF(BUILD_TESTS)
ENABLE_TESTING()

GENERATE_TEST_SOURCES(${TEST_DIR}/test.proto t/test.pb-c.c t/test.pb-c.h)

ADD_EXECUTABLE(test-generated-code ${TEST_DIR}/generated-code/test-generated-code.c t/test.pb-c.c t/test.pb-c.h )
TARGET_LINK_LIBRARIES(test-generated-code protobuf-c)

ADD_CUSTOM_COMMAND(OUTPUT t/test-full.pb.cc t/test-full.pb.h
       COMMAND ${CMAKE_COMMAND}
       ARGS -E env PATH="${OS_PATH_VARIABLE}" -- ${PROTOBUF_PROTOC_EXECUTABLE}
            --cpp_out ${CMAKE_CURRENT_BINARY_DIR} -I${MAIN_DIR} ${TEST_DIR}/test-full.proto
)

GENERATE_TEST_SOURCES(${TEST_DIR}/test-full.proto t/test-full.pb-c.c t/test-full.pb-c.h)

ADD_EXECUTABLE(cxx-generate-packed-data ${TEST_DIR}/generated-code2/cxx-generate-packed-data.cc t/test-full.pb.h t/test-full.pb.cc protobuf-c/protobuf-c.pb.cc protobuf-c/protobuf-c.pb.h)
TARGET_LINK_LIBRARIES(cxx-generate-packed-data
    ${PROTOBUF_LIBRARY}
    ${protobuf_ABSL_USED_TARGETS}
    ${protobuf_UTF8_USED_TARGETS}
)
IF (MSVC AND BUILD_SHARED_LIBS)
	TARGET_COMPILE_DEFINITIONS(cxx-generate-packed-data PRIVATE -DPROTOBUF_USE_DLLS)
ENDIF (MSVC AND BUILD_SHARED_LIBS)

FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/t/generated-code2)
ADD_CUSTOM_COMMAND(OUTPUT t/generated-code2/test-full-cxx-output.inc
       COMMAND ${CMAKE_COMMAND}
       ARGS -E env PATH="${OS_PATH_VARIABLE}" -- cxx-generate-packed-data
            ">t/generated-code2/test-full-cxx-output.inc"
            DEPENDS cxx-generate-packed-data
)

GENERATE_TEST_SOURCES(${TEST_DIR}/test-optimized.proto t/test-optimized.pb-c.c t/test-optimized.pb-c.h)

ADD_EXECUTABLE(test-generated-code2 ${TEST_DIR}/generated-code2/test-generated-code2.c t/generated-code2/test-full-cxx-output.inc t/test-full.pb-c.h t/test-full.pb-c.c t/test-optimized.pb-c.h t/test-optimized.pb-c.c)
TARGET_LINK_LIBRARIES(test-generated-code2 protobuf-c)

GENERATE_TEST_SOURCES(${TEST_DIR}/issue220/issue220.proto t/issue220/issue220.pb-c.c t/issue220/issue220.pb-c.h)
ADD_EXECUTABLE(test-issue220 ${TEST_DIR}/issue220/issue220.c t/issue220/issue220.pb-c.c t/issue220/issue220.pb-c.h)
TARGET_LINK_LIBRARIES(test-issue220 protobuf-c)

GENERATE_TEST_SOURCES(${TEST_DIR}/issue251/issue251.proto t/issue251/issue251.pb-c.c t/issue251/issue251.pb-c.h)
ADD_EXECUTABLE(test-issue251 ${TEST_DIR}/issue251/issue251.c t/issue251/issue251.pb-c.c t/issue251/issue251.pb-c.h)
TARGET_LINK_LIBRARIES(test-issue251 protobuf-c)

ADD_EXECUTABLE(test-version ${TEST_DIR}/version/version.c)
TARGET_LINK_LIBRARIES(test-version protobuf-c)

GENERATE_TEST_SOURCES(${TEST_DIR}/test-proto3.proto t/test-proto3.pb-c.c t/test-proto3.pb-c.h)
ADD_EXECUTABLE(test-generated-code3 ${TEST_DIR}/generated-code/test-generated-code.c t/test-proto3.pb-c.c t/test-proto3.pb-c.h)
TARGET_COMPILE_DEFINITIONS(test-generated-code3 PUBLIC -DPROTO3)
TARGET_LINK_LIBRARIES(test-generated-code3 protobuf-c)

ENDIF() # BUILD_TESTS

# https://github.com/protocolbuffers/protobuf/issues/5107
IF(CMAKE_HOST_UNIX)
	FIND_PACKAGE(Threads REQUIRED)
	TARGET_LINK_LIBRARIES(protoc-gen-c ${CMAKE_THREAD_LIBS_INIT})
	IF(BUILD_TESTS)
		TARGET_LINK_LIBRARIES(cxx-generate-packed-data ${CMAKE_THREAD_LIBS_INIT})
	ENDIF()
ENDIF()

list(APPEND PROTOBUF_C_TARGETS "protoc-gen-c")
ENDIF() # BUILD_PROTOC

install(TARGETS ${PROTOBUF_C_TARGETS}
  EXPORT protobuf-c-targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)

INSTALL(FILES ${MAIN_DIR}/protobuf-c/protobuf-c.h ${MAIN_DIR}/protobuf-c/protobuf-c.proto DESTINATION include/protobuf-c)
INSTALL(FILES ${MAIN_DIR}/protobuf-c/protobuf-c.h DESTINATION include)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/protobuf-c.pdb DESTINATION lib OPTIONAL)

SET(prefix ${CMAKE_INSTALL_PREFIX})
SET(exec_prefix \${prefix})
SET(bindir \${exec_prefix}/${CMAKE_INSTALL_BINDIR})
SET(libdir \${exec_prefix}/${CMAKE_INSTALL_LIBDIR})
SET(includedir \${prefix}/${CMAKE_INSTALL_INCLUDEDIR})
CONFIGURE_FILE(${MAIN_DIR}/protobuf-c/libprotobuf-c.pc.in libprotobuf-c.pc @ONLY)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/libprotobuf-c.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

install(EXPORT protobuf-c-targets
  NAMESPACE protobuf-c::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/protobuf-c
)

include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/protobuf-c-config.cmake"
  INSTALL_DESTINATION "lib/cmake/protobuf-c"
)

write_basic_package_version_file(protobuf-c-config-version.cmake
  VERSION ${PACKAGE_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/protobuf-c-config.cmake ${CMAKE_CURRENT_BINARY_DIR}/protobuf-c-config-version.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/protobuf-c
)

export(EXPORT protobuf-c-targets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/protobuf-c-targets.cmake"
)

IF(BUILD_TESTS)
INCLUDE(Dart)

SET(DART_TESTING_TIMEOUT 5)
ADD_TEST(test-generated-code test-generated-code)
ADD_TEST(test-generated-code2 test-generated-code2)
ADD_TEST(test-generated-code3 test-generated-code3)
ADD_TEST(test-issue220 test-issue220)
ADD_TEST(test-issue251 test-issue251)
ADD_TEST(test-version test-version)

if(WIN32)
    set_tests_properties(
        test-generated-code 
        test-generated-code2 
        test-generated-code3 
        test-issue220 
        test-issue251 
        test-version 
        PROPERTIES ENVIRONMENT "PATH=${WINDOWS_PATH_VARIABLE}\\;$<TARGET_FILE_DIR:protoc-gen-c>" )
endif(WIN32)

ENDIF()

INCLUDE(CPack)
