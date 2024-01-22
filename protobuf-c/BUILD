load("//:base.bzl", "protobuf_cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "protobuf-c",
    srcs = ["protobuf-c.c"],
    hdrs = ["protobuf-c.h"],
    includes = ["."],
)

protobuf_cc_library(
    name = "protobuf-c_proto",
    srcs = ["protobuf-c.proto"],
)
