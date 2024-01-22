load("//:base.bzl", "protobuf_c_library")

package(default_visibility = ["//visibility:public"])

protobuf_c_library(
    name = "issue375_proto",
    srcs = ["issue375.proto"],
)

cc_test(
    name = "issue375",
    srcs = ["issue375.c"],
    deps = [":issue375_proto"],
)
