workspace(name = "protobuf-c")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "com_google_protobuf",
    commit = "a9b006bddd52e289029f16aa77b77e8e0033d9ee",
    remote = "https://github.com/protocolbuffers/protobuf.git",
    # tag = "v25.2",
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()
