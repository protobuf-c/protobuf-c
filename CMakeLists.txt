SET(PACKAGE protobuf-c)
SET(PACKAGE_NAME protobuf-c)
SET(PACKAGE_VERSION 1.0.0)


CMAKE_MINIMUM_REQUIRED(VERSION 2.8 FATAL_ERROR)

PROJECT(protobuf-c)

INCLUDE(TestBigEndian)
TEST_BIG_ENDIAN(WORDS_BIGENDIAN)
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
add_definitions(-DPACKAGE_VERSION="${PACKAGE_VERSION}")
add_definitions(-DPACKAGE_STRING="${PACKAGE_STRING}")
add_definitions(-DWORDS_BIGENDIAN=${WORDS_BIGENDIAN})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # using Visual Studio C++
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4267 /wd4244")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4267 /wd4244")
endif()


include_directories(${CMAKE_BINARY_DIR})

SET (PC_SOURCES 
        protobuf-c/protobuf-c.c 
        protobuf-c/protobuf-c.h)

add_library(protobuf-c ${PC_SOURCES})

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/protobuf-c)


find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})

file(GLOB PROTOC_SRC protoc-c/*.h protoc-c/*.cc )
add_executable(protoc-c ${PROTOC_SRC})

target_link_libraries(protoc-c ${PROTOBUF_PROTOC_LIBRARY} ${PROTOBUF_LIBRARY})

if(CMAKE_BUILD_TYPE MATCHES Debug)
  enable_testing()
  add_subdirectory(t)
endif()



install(TARGETS protoc-c protobuf-c RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(FILES protobuf-c/protobuf-c.h DESTINATION include)

INCLUDE(CPack)
