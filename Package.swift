// swift-tools-version: 5.4

import PackageDescription

let package = Package(
    name: "protobuf-c",
    products: [
        .library(name: "protobuf-c", targets: ["protobuf-c"]),
    ],
    targets: [
        .target(
            name: "protobuf-c",
            path: "./protobuf-c",
            sources: [
                "protobuf-c.c"
            ]
        )
    ]
)
