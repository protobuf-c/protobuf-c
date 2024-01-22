load("//:base.bzl", "protobuf_c_library")

package(default_visibility = ["//visibility:public"])

protobuf_c_library(
    name = "issue440_proto",
    srcs = ["issue440.proto"],
)

cc_test(
    name = "issue440",
    srcs = ["issue440.c"],
    deps = [":issue440_proto"],
)
