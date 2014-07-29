ADD_CUSTOM_COMMAND(OUTPUT test.pb-c.c test.pb-c.h
                   COMMAND protoc-c -I${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/test.proto --c_out=${CMAKE_BINARY_DIR}
                   DEPENDS protoc-c
                   )


ADD_EXECUTABLE(test-generated-code generated-code/test-generated-code.c test.pb-c.c test.pb-c.h )

TARGET_LINK_LIBRARIES(test-generated-code protobuf-c)


INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

PROTOBUF_GENERATE_CPP(CPPS HS ${CMAKE_SOURCE_DIR}/t/test-full.proto)

ADD_CUSTOM_COMMAND(OUTPUT test-full.pb-c.c test-full.pb-c.h
                   COMMAND protoc-c -I${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/t/test-full.proto --c_out=${CMAKE_BINARY_DIR}
                   DEPENDS protoc-c
                   )

ADD_EXECUTABLE(cxx-generate-packed-data generated-code2/cxx-generate-packed-data.cc test-full.pb.h  test-full.pb.cc)
TARGET_LINK_LIBRARIES(cxx-generate-packed-data ${PROTOBUF_LIBRARY})

FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/t/generated-code2)
ADD_CUSTOM_COMMAND(OUTPUT t/generated-code2/test-full-cxx-output.inc
                   COMMAND ${CMAKE_BINARY_DIR}/t/cxx-generate-packed-data ">generated-code2/test-full-cxx-output.inc"
                   DEPENDS cxx-generate-packed-data
                   )

ADD_EXECUTABLE(test-generated-code2 generated-code2/test-generated-code2.c t/generated-code2/test-full-cxx-output.inc test-full.pb-c.h test-full.pb-c.c)
TARGET_LINK_LIBRARIES(test-generated-code2  protobuf-c)

ADD_EXECUTABLE(test-version version/version.c)
TARGET_LINK_LIBRARIES(test-version protobuf-c)


INCLUDE(Dart)
SET(DART_TESTING_TIMEOUT 5)
ADD_TEST(test-generated-code test-generated-code)
ADD_TEST(test-generated-code2 test-generated-code2)
ADD_TEST(test-version test-version)
