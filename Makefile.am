bin_PROGRAMS =
check_PROGRAMS =
noinst_PROGRAMS =
lib_LTLIBRARIES =
nobase_include_HEADERS =
pkgconfig_DATA =
BUILT_SOURCES =
TESTS =
CLEANFILES =
DISTCLEANFILES =
EXTRA_DIST =
ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

AM_CPPFLAGS = \
	-include $(top_builddir)/config.h \
	-I${top_srcdir}/protobuf-c \
	-I${top_builddir} \
	-I${top_srcdir}
AM_CFLAGS = ${my_CFLAGS}
AM_LDFLAGS =

#
# libprotobuf-c
#

LIBPROTOBUF_C_CURRENT=0
LIBPROTOBUF_C_REVISION=0
LIBPROTOBUF_C_AGE=0

lib_LTLIBRARIES += \
	protobuf-c/libprotobuf-c.la

nobase_include_HEADERS += \
	protobuf-c/protobuf-c.h \
	protobuf-c/protobuf-c-private.h

protobuf_c_libprotobuf_c_la_SOURCES = \
	protobuf-c/protobuf-c.c \
	protobuf-c/protobuf-c.h \
	protobuf-c/protobuf-c-private.h

protobuf_c_libprotobuf_c_la_LDFLAGS = $(AM_LDFLAGS) \
	-version-info $(LIBPROTOBUF_C_CURRENT):$(LIBPROTOBUF_C_REVISION):$(LIBPROTOBUF_C_AGE) \
	-export-symbols-regex "^(protobuf_c_[a-z].*)" \
	-no-undefined

pkgconfig_DATA += protobuf-c/libprotobuf-c.pc
CLEANFILES += protobuf-c/libprotobuf-c.pc
EXTRA_DIST += protobuf-c/libprotobuf-c.pc.in

#
# libprotobuf-c-rpc
#

if BUILD_RPC

LIBPROTOBUF_C_RPC_CURRENT=0
LIBPROTOBUF_C_RPC_REVISION=0
LIBPROTOBUF_C_RPC_AGE=0

lib_LTLIBRARIES += \
	protobuf-c-rpc/libprotobuf-c-rpc.la

nobase_include_HEADERS += \
	protobuf-c-rpc/protobuf-c-data-buffer.h \
	protobuf-c-rpc/protobuf-c-dispatch.h \
	protobuf-c-rpc/protobuf-c-rpc.h

protobuf_c_rpc_libprotobuf_c_rpc_la_SOURCES = \
	protobuf-c-rpc/gsklistmacros.h \
	protobuf-c-rpc/gskrbtreemacros.h \
	protobuf-c-rpc/protobuf-c-data-buffer.c \
	protobuf-c-rpc/protobuf-c-dispatch.c \
	protobuf-c-rpc/protobuf-c-rpc.c

protobuf_c_rpc_libprotobuf_c_rpc_la_LDFLAGS = $(AM_LDFLAGS) \
	-version-info $(LIBPROTOBUF_C_RPC_CURRENT):$(LIBPROTOBUF_C_RPC_REVISION):$(LIBPROTOBUF_C_RPC_AGE) \
	-export-symbols-regex "^(protobuf_c_[a-z].*)" \
	-no-undefined

pkgconfig_DATA += protobuf-c-rpc/libprotobuf-c-rpc.pc
CLEANFILES += protobuf-c-rpc/libprotobuf-c-rpc.pc
EXTRA_DIST += protobuf-c-rpc/libprotobuf-c-rpc.pc.in

endif

#
# protoc-c
#

bin_PROGRAMS += protoc-c/protoc-c
protoc_c_protoc_c_SOURCES = \
	protoc-c/c_bytes_field.cc \
	protoc-c/c_bytes_field.h \
	protoc-c/c_enum.cc \
	protoc-c/c_enum.h \
	protoc-c/c_enum_field.cc \
	protoc-c/c_enum_field.h \
	protoc-c/c_extension.cc \
	protoc-c/c_extension.h \
	protoc-c/c_field.cc \
	protoc-c/c_field.h \
	protoc-c/c_file.cc \
	protoc-c/c_file.h \
	protoc-c/c_generator.cc \
	protoc-c/c_generator.h \
	protoc-c/c_helpers.cc \
	protoc-c/c_helpers.h \
	protoc-c/c_message.cc \
	protoc-c/c_message.h \
	protoc-c/c_message_field.cc \
	protoc-c/c_message_field.h \
	protoc-c/c_primitive_field.cc \
	protoc-c/c_primitive_field.h \
	protoc-c/c_service.cc \
	protoc-c/c_service.h \
	protoc-c/c_string_field.cc \
	protoc-c/c_string_field.h \
	protoc-c/main.cc
protoc_c_protoc_c_CFLAGS = \
	$(AM_CFLAGS) \
	$(protobuf_CFLAGS)
protoc_c_protoc_c_LDADD = \
	$(protobuf_LIBS) \
	-lprotoc

#
# protobuf-c tests
#

check_PROGRAMS += \
	t/generated-code/test-generated-code \
	t/generated-code2/test-generated-code2

TESTS += \
	t/generated-code/test-generated-code \
	t/generated-code2/test-generated-code2

t_generated_code_test_generated_code_SOURCES = \
	t/generated-code/test-generated-code.c \
	t/test.pb-c.c
t_generated_code_test_generated_code_LDADD = \
	protobuf-c/libprotobuf-c.la

t_generated_code2_test_generated_code2_SOURCES = \
	t/generated-code2/test-generated-code2.c \
	t/test-full.pb-c.c
t_generated_code2_test_generated_code2_LDADD = \
	protobuf-c/libprotobuf-c.la

noinst_PROGRAMS += \
	t/generated-code2/cxx-generate-packed-data

t_generated_code2_cxx_generate_packed_data_SOURCES = \
	t/generated-code2/cxx-generate-packed-data.cc \
	t/test-full.pb.cc
t/generated-code2/cxx-generate-packed-data.$(OBJEXT): t/test-full.pb.h
t_generated_code2_cxx_generate_packed_data_CFLAGS = \
	$(AM_CFLAGS) \
	$(protobuf_CFLAGS)
t_generated_code2_cxx_generate_packed_data_LDADD = \
	$(protobuf_LIBS)

t/test.pb-c.c t/test.pb-c.h: $(top_builddir)/protoc-c/protoc-c$(EXEEXT) $(top_srcdir)/t/test.proto
	$(AM_V_GEN)$(top_builddir)/protoc-c/protoc-c$(EXEEXT) -I$(top_srcdir) --c_out=$(top_builddir) $(top_srcdir)/t/test.proto

t/test-full.pb-c.c t/test-full.pb-c.h: $(top_builddir)/protoc-c/protoc-c$(EXEEXT) $(top_srcdir)/t/test-full.proto
	$(AM_V_GEN)$(top_builddir)/protoc-c/protoc-c$(EXEEXT) -I$(top_srcdir) --c_out=$(top_builddir) $(top_srcdir)/t/test-full.proto

t/test-full.pb.cc t/test-full.pb.h: @PROTOC@ $(top_srcdir)/t/test-full.proto
	$(AM_V_GEN)@PROTOC@ -I$(top_srcdir) --cpp_out=$(top_builddir) $(top_srcdir)/t/test-full.proto

t/generated-code2/test-full-cxx-output.inc: t/generated-code2/cxx-generate-packed-data$(EXEEXT)
	$(AM_V_GEN)$(top_builddir)/t/generated-code2/cxx-generate-packed-data$(EXEEXT) > $(top_builddir)/t/generated-code2/test-full-cxx-output.inc

BUILT_SOURCES += \
	t/test.pb-c.c t/test.pb-c.h \
	t/test-full.pb-c.c t/test-full.pb-c.h \
	t/test-full.pb.cc t/test-full.pb.h \
	t/generated-code2/test-full-cxx-output.inc

EXTRA_DIST += \
	t/test.proto \
	t/test-full.proto \
	t/generated-code2/common-test-arrays.h

#
# protobuf-c-rpc tests
#

if BUILD_RPC

check_PROGRAMS += \
	protobuf-c-rpc/t/test-rpc

TESTS += \
	protobuf-c-rpc/t/test-rpc

protobuf_c_rpc_t_test_rpc_SOURCES = \
	protobuf-c-rpc/t/test-rpc.c \
	t/test.pb-c.c
protobuf_c_rpc_t_test_rpc_LDADD = \
	protobuf-c/libprotobuf-c.la \
	protobuf-c-rpc/libprotobuf-c-rpc.la

endif

#
#
#

CLEANFILES += $(BUILT_SOURCES)

dist-hook:
	rm -vf `find $(top_distdir) -name '*.pb-c.[ch]' -o -name '*.pb.cc' -o -name '*.pb.h'`
