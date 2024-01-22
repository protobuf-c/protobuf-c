load("//:base.bzl", "protobuf_c_library")

package(default_visibility = ["//visibility:public"])

protobuf_c_library(
    name = "issue330_proto",
    srcs = ["issue330.proto"],
)

cc_test(
    name = "issue330",
    srcs = ["issue330.c"],
    deps = [":issue330_proto"],
)
