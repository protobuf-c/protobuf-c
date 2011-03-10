set(BUILD_PROTOBUF_C_COMPILER ON
    CACHE BOOL "build protobuf-c compiler (aka protoc-c)")

if ( BUILD_PROTOBUF_C_COMPILER )
    find_package(Protobuf REQUIRED)
    include_directories(${PROTOBUF_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
    set(PROTOC_C_SOURCES
        google/protobuf/compiler/c/c_service.cc
        google/protobuf/compiler/c/c_helpers.cc
        google/protobuf/compiler/c/c_enum.cc
        google/protobuf/compiler/c/c_enum_field.cc
        google/protobuf/compiler/c/c_string_field.cc
        google/protobuf/compiler/c/c_primitive_field.cc
        google/protobuf/compiler/c/c_extension.cc
        google/protobuf/compiler/c/c_file.cc
        google/protobuf/compiler/c/c_field.cc
        google/protobuf/compiler/c/c_message.cc
        google/protobuf/compiler/c/c_generator.cc
        google/protobuf/compiler/c/c_message_field.cc
        google/protobuf/compiler/c/main.cc
        google/protobuf/compiler/c/c_bytes_field.cc)
    add_executable(protoc-c ${PROTOC_C_SOURCES})
    target_link_libraries(protoc-c ${PROTOBUF_LIBRARY}
        ${PROTOBUF_PROTOC_LIBRARY})
    if ( UNIX )
        find_package(Threads REQUIRED)
        target_link_libraries(protoc-c ${CMAKE_THREAD_LIBS_INIT})
    endif()
    install(TARGETS protoc-c DESTINATION ${PROTOBUF_C_INSTALL_BINDIR})
endif()

include(CheckIncludeFiles)
check_include_files("alloca.h" HAVE_ALLOCA_H)
check_include_files("malloc.h" HAVE_MALLOC_H)
check_include_files("sys/poll.h" HAVE_SYS_POLL_H)
check_include_files("sys/select.h" HAVE_SYS_SELECT_H)
check_include_files("inttypes.h" HAVE_INTTYPES_H)
check_include_files("sys/uio.h" HAVE_SYS_UIO_H)
check_include_files("unistd.h" HAVE_UNISTD_H)
check_include_files("io.h" HAVE_IO_H)
include(TestBigEndian)
test_big_endian(IS_BIG_ENDIAN)
if ( NOT IS_BIG_ENDIAN )
    set(IS_LITTLE_ENDIAN 1)
endif()
add_definitions(-DHAVE_PROTOBUF_C_CONFIG_H=1)
configure_file(google/protobuf-c/protobuf-c-config.h.in
    google/protobuf-c/protobuf-c-config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/google/protobuf-c)

set(BUILD_PROTOBUF_C_STATIC_LIB ON
    CACHE BOOL "build static protobuf-c library")
set(BUILD_PROTOBUF_C_SHARED_LIB ON
    CACHE BOOL "build shared protobuf-c library")

if ( WIN32 )
    set(PROTOBUF_C_SOURCES
        google/protobuf-c/protobuf-c-data-buffer.c
        google/protobuf-c/protobuf-c.c)
else()
    set(PROTOBUF_C_SOURCES
        google/protobuf-c/protobuf-c-dispatch.c
        google/protobuf-c/protobuf-c-data-buffer.c
        google/protobuf-c/protobuf-c-rpc.c
        google/protobuf-c/protobuf-c.c)
endif()

if ( BUILD_PROTOBUF_C_STATIC_LIB )
    add_library(protobuf-c-static STATIC ${PROTOBUF_C_SOURCES})
    install(TARGETS protobuf-c-static
        ARCHIVE DESTINATION ${PROTOBUF_C_INSTALL_LIBDIR})
    set_target_properties(protobuf-c-static PROPERTIES OUTPUT_NAME protobuf-c)

    get_target_property(DIRECTORY protobuf-c-static ARCHIVE_OUTPUT_DIRECTORY) 
    set_target_properties(protobuf-c-static PROPERTIES ARCHIVE_OUTPUT_DIRECTORY
        ${DIRECTORY}/static)
endif()

if ( BUILD_PROTOBUF_C_SHARED_LIB )
    add_library(protobuf-c-shared SHARED ${PROTOBUF_C_SOURCES})
    install(TARGETS protobuf-c-shared
        RUNTIME DESTINATION ${PROTOBUF_C_INSTALL_BINDIR}
        LIBRARY DESTINATION ${PROTOBUF_C_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${PROTOBUF_C_INSTALL_LIBDIR}/dllimport)
    set_target_properties(protobuf-c-shared PROPERTIES OUTPUT_NAME protobuf-c)

    set_target_properties(protobuf-c-shared PROPERTIES VERSION
        ${PROTOBUF_C_VERSION})
    get_target_property(DIRECTORY protobuf-c-shared ARCHIVE_OUTPUT_DIRECTORY) 
    set_target_properties(protobuf-c-shared PROPERTIES ARCHIVE_OUTPUT_DIRECTORY
        ${DIRECTORY}/shared)
    get_target_property(DIRECTORY protobuf-c-shared LIBRARY_OUTPUT_DIRECTORY) 
    set_target_properties(protobuf-c-shared PROPERTIES LIBRARY_OUTPUT_DIRECTORY
        ${DIRECTORY}/shared)
    if ( WIN32 )
        set_target_properties(protobuf-c-shared PROPERTIES COMPILE_DEFINITIONS
            "PROTOBUF_C_USE_SHARED_LIB;PROTOBUF_C_EXPORT")
    endif()
endif()

if ( BUILD_PROTOBUF_C_STATIC_LIB OR BUILD_PROTOBUF_C_SHARED_LIB )
    if ( WIN32 )
        set(PROTOBUF_C_PUBLIC_HEADERS
            google/protobuf-c/protobuf-c.h
            google/protobuf-c/protobuf-c-private.h)
    else()
        set(PROTOBUF_C_PUBLIC_HEADERS
            google/protobuf-c/protobuf-c.h
            google/protobuf-c/protobuf-c-private.h
            google/protobuf-c/protobuf-c-dispatch.h
            google/protobuf-c/protobuf-c-rpc.h)
    endif()
    install(FILES ${PROTOBUF_C_PUBLIC_HEADERS} DESTINATION
        ${PROTOBUF_C_INSTALL_INCLUDEDIR})
endif()

add_subdirectory(test)

