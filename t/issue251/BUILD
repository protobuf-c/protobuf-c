load("//:base.bzl", "protobuf_c_library")

package(default_visibility = ["//visibility:public"])

protobuf_c_library(
    name = "issue251_proto",
    srcs = ["issue251.proto"],
)

cc_test(
    name = "issue251",
    srcs = ["issue251.c"],
    deps = [":issue251_proto"],
)
