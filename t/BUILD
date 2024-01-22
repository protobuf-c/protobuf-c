load("//:base.bzl", "protobuf_c_library")

package(default_visibility = ["//visibility:public"])

protobuf_c_library(
    name = "test-proto3_proto",
    srcs = ["test-proto3.proto"],
)

protobuf_c_library(
    name = "test_proto",
    srcs = ["test.proto"],
)

protobuf_c_library(
    name = "test-optimized_proto",
    srcs = ["test-optimized.proto"],
)

protobuf_c_library(
    name = "test-full_proto",
    srcs = ["test-full.proto"],
)

cc_test(
    name = "test-generated-code3",
    srcs = ["generated-code/test-generated-code.c"],
    defines = ["PROTO3"],
    deps = [":test-proto3_proto"],
)

cc_test(
    name = "test-generated-code",
    srcs = ["generated-code/test-generated-code.c"],
    deps = [":test_proto"],
)

cc_test(
    name = "test-generated-code2",
    srcs = ["generated-code2/test-generated-code2.c"],
    deps = [
        ":test-full_proto",
        ":test-optimized_proto",
    ],
)

cc_test(
    name = "version",
    srcs = ["version/version.c"],
    deps = [
        "//protobuf-c",
    ],
    defines = ["PACKAGE_VERSION=\\\"1.5.0\\\""],
)
