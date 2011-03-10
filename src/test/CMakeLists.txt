if ( BUILD_PROTOBUF_C_COMPILER AND BUILD_PROTOBUF_C_STATIC_LIB )
    function(BUILD_C_TEST target_name proto_name)
        set(TMP_OUTPUT_DIR ${CMAKE_BINARY_DIR}/${target_name}/generated-code)
        file(MAKE_DIRECTORY ${TMP_OUTPUT_DIR})

        set(GENERATED_SOURCES ${TMP_OUTPUT_DIR}/${proto_name}.pb-c.h
            ${TMP_OUTPUT_DIR}/${proto_name}.pb-c.c)
        add_executable(${target_name} ${target_name}.c ${GENERATED_SOURCES}
            ${ARGN})
        set_target_properties(${target_name} PROPERTIES COMPILE_FLAGS
            "-I${CMAKE_BINARY_DIR}/${target_name}")
        target_link_libraries(${target_name} protobuf-c-static)

        add_custom_command(OUTPUT ${GENERATED_SOURCES}
            COMMAND protoc-c -I${CMAKE_CURRENT_SOURCE_DIR}
            --c_out=${TMP_OUTPUT_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/${proto_name}.proto
            DEPENDS protoc-c ${CMAKE_CURRENT_SOURCE_DIR}/${proto_name}.proto
            COMMENT "Running C protocol buffer compiler on ${proto_name}"
            VERBATIM)
    endfunction()

    function(BUILD_CPP_TEST target_name proto_name)
        set(TMP_OUTPUT_DIR ${CMAKE_BINARY_DIR}/${target_name}/generated-code)
        file(MAKE_DIRECTORY ${TMP_OUTPUT_DIR})

        set(GENERATED_SOURCES ${TMP_OUTPUT_DIR}/${proto_name}.pb.h
            ${TMP_OUTPUT_DIR}/${proto_name}.pb.cc)
        add_executable(${target_name} ${target_name}.cc ${GENERATED_SOURCES}
            ${ARGN})
        set_target_properties(${target_name} PROPERTIES COMPILE_FLAGS
            "-I${CMAKE_BINARY_DIR}/${target_name} -I${PROTOBUF_INCLUDE_DIRS}")
        target_link_libraries(${target_name} ${PROTOBUF_LIBRARIES})
        if ( UNIX )
            find_package(Threads REQUIRED)
            target_link_libraries(${target_name} ${CMAKE_THREAD_LIBS_INIT})
        endif()

        add_custom_command(OUTPUT ${GENERATED_SOURCES}
            COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} -I${CMAKE_CURRENT_SOURCE_DIR}
            --cpp_out=${TMP_OUTPUT_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/${proto_name}.proto
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${proto_name}.proto
            COMMENT "Running C++ protocol buffer compiler on ${proto_name}"
            VERBATIM)
    endfunction()

    build_cpp_test(cxx-generate-packed-data test-full)
    set(TMP_OUTPUT_DIR ${CMAKE_BINARY_DIR}/cxx-generate-packed-data)
    file(MAKE_DIRECTORY ${TMP_OUTPUT_DIR}/generated-code)
    set(GENERATED_INC_FILE ${TMP_OUTPUT_DIR}/generated-code/test-full-cxx-output.inc)
    add_custom_command(OUTPUT ${GENERATED_INC_FILE}
        COMMAND cxx-generate-packed-data > ${GENERATED_INC_FILE}
        DEPENDS cxx-generate-packed-data)
    build_c_test(test-generated-code2 test-full ${GENERATED_INC_FILE})
    get_target_property(EXTRA_COMPILE_FLAGS test-generated-code2
        COMPILE_FLAGS)
    set(EXTRA_COMPILE_FLAGS "${EXTRA_COMPILE_FLAGS} -I${TMP_OUTPUT_DIR}")
    set_target_properties(test-generated-code2 PROPERTIES COMPILE_FLAGS
        ${EXTRA_COMPILE_FLAGS})
    add_test(NAME test-generated-code2 COMMAND test-generated-code2) 

    build_c_test(test-generated-code test)
    add_test(NAME test-generated-code COMMAND test-generated-code)

    if ( UNIX )
        build_c_test(test-rpc test)
        build_c_test(example-client test)
        build_c_test(example-server test)
        add_test(NAME test-rpc COMMAND test-rpc)
    endif()
endif()

