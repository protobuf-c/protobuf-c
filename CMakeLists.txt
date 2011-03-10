cmake_minimum_required(VERSION 2.8)
project(protobuf-c)
set(PROTOBUF_C_VERSION_MAJOR 0)
set(PROTOBUF_C_VERSION_MINOR 14)
set(PROTOBUF_C_VERSION_PATCH 1)
set(PROTOBUF_C_VERSION ${PROTOBUF_C_VERSION_MAJOR}.${PROTOBUF_C_VERSION_MINOR})

set(CMAKE_ROOT_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-${CMAKE_SYSTEM}
    CACHE PATH "root output directory in which to locate target files")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_ROOT_OUTPUT_DIRECTORY}/bin
    CACHE PATH "output directory in which to build runtime target files")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_ROOT_OUTPUT_DIRECTORY}/lib
    CACHE PATH "output directory in which to build library target files")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ROOT_OUTPUT_DIRECTORY}/lib
    CACHE PATH "output directory in which to build archive target files")

function(process_libprotobuf_c_pc)
    set(prefix ${PROTOBUF_C_INSTALL_PREFIX})
    set(exec_prefix ${PROTOBUF_C_INSTALL_EXEC_PREFIX})
    set(libdir ${PROTOBUF_C_INSTALL_LIBDIR})
    set(includedir ${PROTOBUF_C_INSTALL_INCLUDEDIR})
    set(VERSION ${PROTOBUF_C_VERSION})
    configure_file(libprotobuf-c.pc.in
        ${CMAKE_ROOT_OUTPUT_DIRECTORY}/pkgconfig/libprotobuf-c.pc @ONLY)
    install(DIRECTORY ${CMAKE_ROOT_OUTPUT_DIRECTORY}/pkgconfig DESTINATION
        ${PROTOBUF_C_INSTALL_LIBDIR})
endfunction()

set(PROTOBUF_C_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}
    CACHE PATH "install location for architecture-independent files")
set(PROTOBUF_C_INSTALL_EXEC_PREFIX ${CMAKE_INSTALL_PREFIX}
    CACHE PATH "install location for architecture-dependent files")
if ( UNIX )
    set(PROTOBUF_C_INSTALL_BINDIR ${PROTOBUF_C_INSTALL_EXEC_PREFIX}/bin
        CACHE PATH "install location for executable files")
    set(PROTOBUF_C_INSTALL_LIBDIR ${PROTOBUF_C_INSTALL_EXEC_PREFIX}/lib
        CACHE PATH "install location for object code libraries")
    set(PROTOBUF_C_INSTALL_INCLUDEDIR ${PROTOBUF_C_INSTALL_PREFIX}/include
        CACHE PATH "install location for public header files")
    process_libprotobuf_c_pc()
else()
    set(PROTOBUF_C_INSTALL_BINDIR bin
        CACHE PATH "install location for executable files")
    set(PROTOBUF_C_INSTALL_LIBDIR lib
        CACHE PATH "install location for object code libraries")
    set(PROTOBUF_C_INSTALL_INCLUDEDIR include
        CACHE PATH "install location for public header files")
endif()

enable_testing()
add_subdirectory(src)

# Use "make package" or something like that to create a protobuf-c package.
set(CPACK_PACKAGE_CONTACT "David Benson <daveb@ffem.org>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C bindings for protocol-buffers")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROTOBUF_C_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROTOBUF_C_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROTOBUF_C_VERSION_PATCH}")

if ( WIN32 )
    set(CPACK_GENERATOR ZIP)
else()
    # Look at http://public.kitware.com/Bug/view.php?id=7000
    set(CPACK_SET_DESTDIR "ON")

    set(CPACK_GENERATOR DEB)
    set(CPACK_DEBIAN_PACKAGE_SECTION  "libs")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "low")
    set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://code.google.com/p/protobuf-c")
endif()
include(CPack)

