""" Generate protobuf code """

load("@rules_cc//cc:defs.bzl", "cc_library")

def _impl(ctx):
    proto_path = ctx.label.package
    output_files = []

    gen = ctx.file.gen
    exe = ctx.file.exe

    args = ctx.actions.args()
    args.add("--plugin=protoc-gen-c=" + gen.path)
    if proto_path:
        offset = len(proto_path) + 1
        out_path = ctx.genfiles_dir.path + "/" + proto_path
        args.add("--proto_path=" + proto_path)
    else:
        offset = 0
        out_path = ctx.genfiles_dir.path
        args.add("--proto_path=.")

    args.add("--proto_path=.")
    args.add("--proto_path=external/com_google_protobuf/src")
    args.add("--c_out=" + out_path)

    for src in ctx.files.srcs:
        s = src.path[offset:]
        args.add(s)

        output = s.replace(".proto", ".pb-c.c")
        output_files.append(ctx.actions.declare_file(output))
        output = s.replace(".proto", ".pb-c.h")
        output_files.append(ctx.actions.declare_file(output))

    ctx.actions.run(
        inputs = ctx.files.srcs,
        outputs = output_files,
        executable = exe,
        tools = [gen],
        arguments = [args],
    )

    return [
        DefaultInfo(files = depset(output_files)),
    ]

gen_c = rule(
    implementation = _impl,
    attrs = {
        "srcs": attr.label_list(
            allow_files = [".proto"],
            mandatory = True,
        ),
        "gen": attr.label(allow_single_file = True, default = "@protobuf-c//:protoc-gen-c"),
        "exe": attr.label(allow_single_file = True, default = "@com_google_protobuf//:protoc"),
    },
    provides = [
        DefaultInfo,
    ],
)

def protobuf_c_library(name, srcs, deps = []):
    name_pb = name + "_pb"
    gen_c(
        name = name_pb,
        srcs = srcs,
    )

    cc_library(
        name = name,
        srcs = [name_pb],
        includes = [
            ".",
        ],
        deps = [
            "@protobuf-c//protobuf-c",
        ] + deps,
    )

# C++ -------------------------------------------------------------------------

def _cc_impl(ctx):
    proto_path = ctx.label.package
    output_files = []

    exe = ctx.file.exe

    args = ctx.actions.args()
    if proto_path:
        offset = len(proto_path) + 1
        out_path = ctx.genfiles_dir.path + "/" + proto_path
        args.add("--proto_path=" + proto_path)
    else:
        offset = 0
        out_path = ctx.genfiles_dir.path
        args.add("--proto_path=.")

    args.add("--proto_path=.")
    args.add("--proto_path=external/com_google_protobuf/src")
    args.add("--cpp_out=" + out_path)

    for src in ctx.files.srcs:
        s = src.path[offset:]
        args.add(s)

        output = s.replace(".proto", ".pb.cc")
        output_files.append(ctx.actions.declare_file(output))
        output = s.replace(".proto", ".pb.h")
        output_files.append(ctx.actions.declare_file(output))

    ctx.actions.run(
        inputs = ctx.files.srcs,
        outputs = output_files,
        executable = exe,
        arguments = [args],
    )

    return [
        DefaultInfo(files = depset(output_files)),
    ]

gen_cc = rule(
    implementation = _cc_impl,
    attrs = {
        "srcs": attr.label_list(
            allow_files = [".proto"],
            mandatory = True,
        ),
        "exe": attr.label(allow_single_file = True, default = "@com_google_protobuf//:protoc"),
    },
    provides = [
        DefaultInfo,
    ],
)

def protobuf_cc_library(name, srcs, deps = []):
    name_pb = name + "_pb"
    gen_cc(
        name = name_pb,
        srcs = srcs,
    )

    cc_library(
        name = name,
        srcs = [name_pb],
        includes = [
            ".",
        ],
        deps = [
            "@com_google_protobuf//:protoc_lib",
        ] + deps,
    )
