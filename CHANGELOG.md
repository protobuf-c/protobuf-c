# [1.5.2] - 2025-04-06

## What's Changed
* Chase compatibility issues with Google protobuf 30.0-rc1 by @edmonds in https://github.com/protobuf-c/protobuf-c/pull/762
* protoc-gen-c: Explicitly construct strings where needed for protobuf 30.x by @edmonds in https://github.com/protobuf-c/protobuf-c/pull/768

# [1.5.1] - 2025-02-01

## What's Changed
* CMakeList.txt: Remove double hyphens by @AlessandroBono in https://github.com/protobuf-c/protobuf-c/pull/699
* Makefile.am: Distribute missing Config.cmake.in by @AlessandroBono in https://github.com/protobuf-c/protobuf-c/pull/700
* protobuf_c_message_unpack(): Fix memory corruption by initializing unknown_fields pointer by @smuellerDD in https://github.com/protobuf-c/protobuf-c/pull/703
* Fix CI issues with CMake by @clementperon in https://github.com/protobuf-c/protobuf-c/pull/714
* build.yml: Install libtool on OS X by @edmonds in https://github.com/protobuf-c/protobuf-c/pull/717
* build.yml: Set "fail-fast: false" so we can tell which jobs are failing by @edmonds in https://github.com/protobuf-c/protobuf-c/pull/718
* Update actions by @AndrewQuijano in https://github.com/protobuf-c/protobuf-c/pull/740
* Miscellaneous CI updates by @edmonds in https://github.com/protobuf-c/protobuf-c/pull/747
* build.yml: Build on more pull request activity types by @edmonds in https://github.com/protobuf-c/protobuf-c/pull/751
* Chase compatibility issues with Google protobuf >= 26.0 by @edmonds in https://github.com/protobuf-c/protobuf-c/pull/711
* Clean CMake by @clementperon in https://github.com/protobuf-c/protobuf-c/pull/719
* build.yml: Update Windows dependencies (abseil, protobuf) by @edmonds in https://github.com/protobuf-c/protobuf-c/pull/753
* build.yml: Ubuntu: Add 22.04, 24.04 by @edmonds in https://github.com/protobuf-c/protobuf-c/pull/754
* Order oneof union members from largest to smallest by @edmonds in https://github.com/protobuf-c/protobuf-c/pull/755
* More renaming of `protoc-c` to `protoc-gen-c` by @edmonds in https://github.com/protobuf-c/protobuf-c/pull/756
* cmake: Fix build when using ninja and protobuf-c already installed by @edmonds in https://github.com/protobuf-c/protobuf-c/pull/757
* protoc-gen-c: Log a deprecation warning when invoked as `protoc-c` by @edmonds in https://github.com/protobuf-c/protobuf-c/pull/758
* build.yml: Try running multiarch builds on Debian bookworm by @edmonds in https://github.com/protobuf-c/protobuf-c/pull/759

# [1.5.0] - 2023-11-25

## What's Changed
* Makefile.am: change link order by @franksinankaya in https://github.com/protobuf-c/protobuf-c/pull/486
* GitHub actions fail on Windows due to missing unzip command by @britzl in https://github.com/protobuf-c/protobuf-c/pull/525
* Export and install CMake targets by @morrisonlevi in https://github.com/protobuf-c/protobuf-c/pull/472
* Use CMAKE_CURRENT_BINARY_DIR instead of CMAKE_BINARY_DIR by @KivApple in https://github.com/protobuf-c/protobuf-c/pull/482
* remove deprecated functionality by @aviborg in https://github.com/protobuf-c/protobuf-c/pull/542
* Avoid "unused variable" compiler warning by @rgriege in https://github.com/protobuf-c/protobuf-c/pull/545
* Update autotools by @AtariDreams in https://github.com/protobuf-c/protobuf-c/pull/550
* Support for new Google protobuf 22.x, 23.x releases by @edmonds in https://github.com/protobuf-c/protobuf-c/pull/673
* Miscellaneous fixes by @edmonds in https://github.com/protobuf-c/protobuf-c/pull/675
* Remove protobuf 2.x support by @edmonds in https://github.com/protobuf-c/protobuf-c/pull/676
* Silence some compiler diagnostics by @edmonds in https://github.com/protobuf-c/protobuf-c/pull/677
* Fixing MSVC build for Msbuild and Makefile generators by @MiguelBarro in https://github.com/protobuf-c/protobuf-c/pull/685
