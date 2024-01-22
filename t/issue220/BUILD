load("//:base.bzl", "protobuf_c_library")

package(default_visibility = ["//visibility:public"])

protobuf_c_library(
    name = "issue220_proto",
    srcs = ["issue220.proto"],
)

cc_test(
    name = "issue220",
    srcs = ["issue220.c"],
    deps = [":issue220_proto"],
)
