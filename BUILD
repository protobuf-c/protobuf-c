package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "protoc-gen-c",
    srcs = glob(["protoc-c/*.cc"]),
    defines = ["PACKAGE_STRING=\\\"protobuf-c\\\""],
    includes = ["protoc-c"],
    deps = ["//protobuf-c:protobuf-c_proto"],
)

filegroup(
    name = "all",
    srcs = [
        ":protoc-gen-c",
        "//protobuf-c",
    ],
)

test_suite(
    name = "tests",
    tests = [
        "//t:test-generated-code",
        "//t:test-generated-code3",
        "//t:version",
        "//t/issue204",
        "//t/issue220",
        "//t/issue251",
        "//t/issue330",
        "//t/issue375",
        "//t/issue440",
    ],
)
