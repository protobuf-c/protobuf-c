SET(PACKAGE protobuf-c)
SET(PACKAGE_NAME protobuf-c)
SET(PACKAGE_VERSION 1.4.1)
SET(PACKAGE_URL https://github.com/protobuf-c/protobuf-c)
SET(PACKAGE_DESCRIPTION "Protocol Buffers implementation in C")

CMAKE_MINIMUM_REQUIRED(VERSION 3.10 FATAL_ERROR)
cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0091 NEW)

PROJECT(protobuf-c C CXX)

if (MSVC AND NOT BUILD_SHARED_LIBS)
	SET(Protobuf_USE_STATIC_LIBS ON)
endif (MSVC AND NOT BUILD_SHARED_LIBS)

FIND_PACKAGE(Protobuf REQUIRED)
INCLUDE_DIRECTORIES(${PROTOBUF_INCLUDE_DIR})

# for static protobuf libraries include the dependencies
if (Protobuf_USE_STATIC_LIBS)
    find_package(absl REQUIRED)

    set(protobuf_ABSL_USED_TARGETS
        absl::log_severity
        absl::raw_logging_internal
        absl::spinlock_wait
        absl::malloc_internal
        absl::base
        absl::throw_delegate
        absl::scoped_set_env
        absl::strerror
        absl::hashtablez_sampler
        absl::raw_hash_set
        absl::crc_cpu_detect
        absl::crc_internal
        absl::crc32c
        absl::crc_cord_state
        absl::stacktrace
        absl::symbolize
        absl::examine_stack
        absl::failure_signal_handler
        absl::debugging_internal
        absl::demangle_internal
        absl::leak_check
        absl::flags_program_name
        absl::flags_config
        absl::flags_marshalling
        absl::flags_commandlineflag_internal
        absl::flags_commandlineflag
        absl::flags_private_handle_accessor
        absl::flags_reflection
        absl::flags_internal
        absl::flags
        absl::flags_usage_internal
        absl::flags_usage
        absl::flags_parse
        absl::hash
        absl::city
        absl::low_level_hash
        absl::log_internal_check_op
        absl::log_internal_conditions
        absl::log_internal_format
        absl::log_internal_globals
        absl::log_internal_proto
        absl::log_internal_message
        absl::log_internal_log_sink_set
        absl::log_internal_nullguard
        absl::die_if_null
        absl::log_flags
        absl::log_globals
        absl::log_initialize
        absl::log_entry
        absl::log_sink
        absl::log_internal_fnmatch
        absl::int128
        absl::exponential_biased
        absl::periodic_sampler
        absl::random_distributions
        absl::random_seed_gen_exception
        absl::random_seed_sequences
        absl::random_internal_seed_material
        absl::random_internal_pool_urbg
        absl::random_internal_platform
        absl::random_internal_randen
        absl::random_internal_randen_slow
        absl::random_internal_randen_hwaes
        absl::random_internal_randen_hwaes_impl
        absl::random_internal_distribution_test_util
        absl::status
        absl::statusor
        absl::string_view
        absl::strings
        absl::strings_internal
        absl::str_format_internal
        absl::cord_internal
        absl::cordz_functions
        absl::cordz_handle
        absl::cordz_info
        absl::cordz_sample_token
        absl::cord
        absl::graphcycles_internal
        absl::kernel_timeout_internal
        absl::synchronization
        absl::time
        absl::civil_time
        absl::time_zone
        absl::bad_any_cast_impl
        absl::bad_optional_access
        absl::bad_variant_access
    )

    find_package(utf8_range REQUIRED)

    set(protobuf_UTF8_USED_TARGETS
        utf8_range::utf8_validity
        utf8_range::utf8_range
    )
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

IF(BUILD_PROTOC)
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)
ADD_CUSTOM_COMMAND(OUTPUT protobuf-c/protobuf-c.pb.cc protobuf-c/protobuf-c.pb.h
                   COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
                   ARGS --cpp_out ${CMAKE_CURRENT_BINARY_DIR} -I${PROTOBUF_INCLUDE_DIR} -I${MAIN_DIR} ${MAIN_DIR}/protobuf-c/protobuf-c.proto)
FILE(GLOB PROTOC_GEN_C_SRC ${MAIN_DIR}/protoc-c/*.h ${MAIN_DIR}/protoc-c/*.cc )
ADD_EXECUTABLE(protoc-gen-c ${PROTOC_GEN_C_SRC} protobuf-c/protobuf-c.pb.cc protobuf-c/protobuf-c.pb.h)
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

if(WIN32)

    # modify the environment to hint protoc where the plugin is
    string(REGEX MATCHALL  "[A-Z]:[^;:]+;" PATH_VARIABLE "$ENV{PATH}" )
    foreach (itvar ${PATH_VARIABLE})
            string(REPLACE ";" "" itvar ${itvar})
            set(WINDOWS_PATH_VARIABLE "${WINDOWS_PATH_VARIABLE}\\;${itvar}" )
    endforeach( itvar )

    FUNCTION(GENERATE_TEST_SOURCES PROTO_FILE SRC HDR)
        ADD_CUSTOM_COMMAND(OUTPUT ${SRC} ${HDR}
                       COMMAND ${CMAKE_COMMAND}
                       ARGS -E env PATH="${WINDOWS_PATH_VARIABLE}\\;$<TARGET_FILE_DIR:protoc-gen-c>" -- ${PROTOBUF_PROTOC_EXECUTABLE}
                            --plugin=$<TARGET_FILE_NAME:protoc-gen-c> -I${MAIN_DIR} ${PROTO_FILE} --c_out=${CMAKE_CURRENT_BINARY_DIR}
                       DEPENDS protoc-gen-c)
    ENDFUNCTION()

else(WIN32)

    FUNCTION(GENERATE_TEST_SOURCES PROTO_FILE SRC HDR)
        ADD_CUSTOM_COMMAND(OUTPUT ${SRC} ${HDR}
                       COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
                       ARGS --plugin=$<TARGET_FILE:protoc-gen-c> -I${MAIN_DIR} ${PROTO_FILE} --c_out=${CMAKE_CURRENT_BINARY_DIR}
                       DEPENDS protoc-gen-c)
    ENDFUNCTION()

endif(WIN32)

IF(BUILD_TESTS)
ENABLE_TESTING()

GENERATE_TEST_SOURCES(${TEST_DIR}/test.proto t/test.pb-c.c t/test.pb-c.h)

ADD_EXECUTABLE(test-generated-code ${TEST_DIR}/generated-code/test-generated-code.c t/test.pb-c.c t/test.pb-c.h )
TARGET_LINK_LIBRARIES(test-generated-code protobuf-c)

ADD_CUSTOM_COMMAND(OUTPUT t/test-full.pb.cc t/test-full.pb.h
                   COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
                   ARGS --cpp_out ${CMAKE_CURRENT_BINARY_DIR} -I${MAIN_DIR} ${TEST_DIR}/test-full.proto)

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
                   COMMAND cxx-generate-packed-data ">t/generated-code2/test-full-cxx-output.inc"
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
