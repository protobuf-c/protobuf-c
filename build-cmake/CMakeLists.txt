SET(PACKAGE protobuf-c)
SET(PACKAGE_NAME protobuf-c)
SET(PACKAGE_VERSION 1.0.2)


CMAKE_MINIMUM_REQUIRED(VERSION 2.8 FATAL_ERROR)

PROJECT(protobuf-c)

INCLUDE(TestBigEndian)
TEST_BIG_ENDIAN(WORDS_BIGENDIAN)

SET(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
ADD_DEFINITIONS(-DPACKAGE_VERSION="${PACKAGE_VERSION}")
ADD_DEFINITIONS(-DPACKAGE_STRING="${PACKAGE_STRING}")
ADD_DEFINITIONS(-DWORDS_BIGENDIAN=${WORDS_BIGENDIAN})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # using Visual Studio C++
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4267 /wd4244")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4267 /wd4244")
ENDIF()

get_filename_component(MAIN_DIR ${CMAKE_SOURCE_DIR} PATH)
SET(TEST_DIR ${MAIN_DIR}/t)

MESSAGE(${MAIN_DIR})

SET (PC_SOURCES
        ${MAIN_DIR}/protobuf-c/protobuf-c.c
        ${MAIN_DIR}/protobuf-c/protobuf-c.h)

ADD_LIBRARY(protobuf-c ${PC_SOURCES})

INCLUDE_DIRECTORIES(${MAIN_DIR})
INCLUDE_DIRECTORIES(${MAIN_DIR}/protobuf-c)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}) # for generated files

FIND_PACKAGE(Protobuf REQUIRED)
INCLUDE_DIRECTORIES(${PROTOBUF_INCLUDE_DIR})

file(GLOB PROTOC_SRC ${MAIN_DIR}/protoc-c/*.h ${MAIN_DIR}/protoc-c/*.cc )
ADD_EXECUTABLE(protoc-c ${PROTOC_SRC})

TARGET_LINK_LIBRARIES(protoc-c ${PROTOBUF_PROTOC_LIBRARY} ${PROTOBUF_LIBRARY})

IF(CMAKE_BUILD_TYPE MATCHES Debug)
ENABLE_TESTING()
ADD_CUSTOM_COMMAND(OUTPUT t/test.pb-c.c t/test.pb-c.h
                   COMMAND protoc-c -I${MAIN_DIR} ${TEST_DIR}/test.proto --c_out=${CMAKE_BINARY_DIR}
                   DEPENDS protoc-c
                   )


ADD_EXECUTABLE(test-generated-code ${TEST_DIR}/generated-code/test-generated-code.c t/test.pb-c.c t/test.pb-c.h )
TARGET_LINK_LIBRARIES(test-generated-code protobuf-c)


ADD_CUSTOM_COMMAND(OUTPUT t/test-full.pb.cc t/test-full.pb.h
                   COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
		   ARGS --cpp_out ${CMAKE_BINARY_DIR} -I${MAIN_DIR} ${TEST_DIR}/test-full.proto)

ADD_CUSTOM_COMMAND(OUTPUT t/test-full.pb-c.c t/test-full.pb-c.h
                   COMMAND protoc-c -I${MAIN_DIR} ${TEST_DIR}/test-full.proto --c_out=${CMAKE_BINARY_DIR}
                   DEPENDS protoc-c
                   )

ADD_EXECUTABLE(cxx-generate-packed-data ${TEST_DIR}/generated-code2/cxx-generate-packed-data.cc t/test-full.pb.h t/test-full.pb.cc)
TARGET_LINK_LIBRARIES(cxx-generate-packed-data ${PROTOBUF_LIBRARY})

FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/t/generated-code2)
ADD_CUSTOM_COMMAND(OUTPUT t/generated-code2/test-full-cxx-output.inc
                   COMMAND ${CMAKE_BINARY_DIR}/cxx-generate-packed-data ">t/generated-code2/test-full-cxx-output.inc"
                   DEPENDS cxx-generate-packed-data
                   )

ADD_EXECUTABLE(test-generated-code2 ${TEST_DIR}/generated-code2/test-generated-code2.c t/generated-code2/test-full-cxx-output.inc t/test-full.pb-c.h t/test-full.pb-c.c)
TARGET_LINK_LIBRARIES(test-generated-code2  protobuf-c)

ADD_EXECUTABLE(test-version ${TEST_DIR}/version/version.c)
TARGET_LINK_LIBRARIES(test-version protobuf-c)


ENDIF()


INSTALL(TARGETS protoc-c protobuf-c RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(FILES ${MAIN_DIR}/protobuf-c/protobuf-c.h DESTINATION include/protobuf-c)
INSTALL(FILES ${MAIN_DIR}/protobuf-c/protobuf-c.h DESTINATION include)
INCLUDE(Dart)

SET(DART_TESTING_TIMEOUT 5)
ADD_TEST(test-generated-code test-generated-code)
ADD_TEST(test-generated-code2 test-generated-code2)
ADD_TEST(test-version test-version)


INCLUDE(CPack)
