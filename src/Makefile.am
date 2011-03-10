if BUILD_PROTOC_C
SUBDIRS = . test
bin_PROGRAMS = protoc-c
protoc_c_SOURCES = \
google/protobuf/compiler/c/c_service.cc \
google/protobuf/compiler/c/c_helpers.cc \
google/protobuf/compiler/c/c_enum.cc \
google/protobuf/compiler/c/c_enum_field.cc \
google/protobuf/compiler/c/c_string_field.cc \
google/protobuf/compiler/c/c_primitive_field.cc \
google/protobuf/compiler/c/c_extension.cc \
google/protobuf/compiler/c/c_file.cc \
google/protobuf/compiler/c/c_field.cc \
google/protobuf/compiler/c/c_message.cc \
google/protobuf/compiler/c/c_generator.cc \
google/protobuf/compiler/c/c_message_field.cc \
google/protobuf/compiler/c/main.cc \
google/protobuf/compiler/c/c_bytes_field.cc
protoc_c_LDADD = \
-lprotoc -lprotobuf -lpthread
endif

lib_LTLIBRARIES = libprotobuf-c.la
protobufcincludedir = $(includedir)/google/protobuf-c

EXTRA_DIST = CMakeLists.txt test/CMakeLists.txt

libprotobuf_c_la_SOURCES = \
google/protobuf-c/protobuf-c-dispatch.c \
google/protobuf-c/protobuf-c-data-buffer.c \
google/protobuf-c/protobuf-c-rpc.c \
google/protobuf-c/protobuf-c.c

# ah, -no-undefined, goofball windows hack.
# the documentation for this is rather confusing to me,
# but suffice it to say this is needed
# on a few garbage platforms (AIX, windows),
# and it compiles to a no-op on sensible platforms (elf)
libprotobuf_c_la_LDFLAGS = \
-no-undefined \
-version-info 0:0:0

noinst_HEADERS = \
google/protobuf/compiler/c/c_message_field.h \
google/protobuf/compiler/c/c_service.h \
google/protobuf/compiler/c/c_helpers.h \
google/protobuf/compiler/c/c_enum.h \
google/protobuf/compiler/c/c_enum_field.h \
google/protobuf/compiler/c/c_string_field.h \
google/protobuf/compiler/c/c_primitive_field.h \
google/protobuf/compiler/c/c_extension.h \
google/protobuf/compiler/c/c_file.h \
google/protobuf/compiler/c/c_field.h \
google/protobuf/compiler/c/c_message.h \
google/protobuf/compiler/c/c_generator.h \
google/protobuf/compiler/c/c_bytes_field.h \
google/protobuf-c/protobuf-c-data-buffer.h \
google/protobuf-c/gskrbtreemacros.h \
google/protobuf-c/gsklistmacros.h

protobufcinclude_HEADERS = \
google/protobuf-c/protobuf-c.h \
google/protobuf-c/protobuf-c-private.h \
google/protobuf-c/protobuf-c-dispatch.h \
google/protobuf-c/protobuf-c-rpc.h
