load("//:base.bzl", "protobuf_c_library")

package(default_visibility = ["//visibility:public"])

protobuf_c_library(
    name = "issue204_proto",
    srcs = ["issue204.proto"],
)

cc_test(
    name = "issue204",
    srcs = ["issue204.c"],
    deps = [":issue204_proto"],
)
