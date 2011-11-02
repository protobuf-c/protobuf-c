check_PROGRAMS = test-generated-code test-generated-code2 test-rpc
noinst_PROGRAMS = cxx-generate-packed-data example-server example-client
INCLUDES = -I$(srcdir)/..
test_generated_code_SOURCES = \
test-generated-code.c \
generated-code/test.pb-c.c
test_generated_code_LDADD = \
../libprotobuf-c.la
test_generated_code2_SOURCES = \
test-generated-code2.c \
generated-code/test-full.pb-c.c
test_generated_code2_LDADD = \
../libprotobuf-c.la
test_rpc_SOURCES = \
test-rpc.c \
generated-code/test.pb-c.c
test_rpc_LDADD = \
../libprotobuf-c.la
cxx_generate_packed_data_SOURCES = \
cxx-generate-packed-data.cc generated-code/test-full.pb.cc
cxx-generate-packed-data.$(OBJEXT): generated-code/test-full.pb.h
cxx_generate_packed_data_LDADD = -lprotobuf
cxx_generate_packed_data_LDFLAGS = -pthread
example_server_SOURCES = \
example-server.c generated-code/test.pb-c.c
example_server_LDADD = ../libprotobuf-c.la
example_client_SOURCES = \
example-client.c generated-code/test.pb-c.c
example_client_LDADD = ../libprotobuf-c.la

generated-code/test.pb-c.c generated-code/test.pb-c.h: ../protoc-c$(EXEEXT) $(srcdir)/test.proto
	mkdir -p generated-code
	../protoc-c -I$(srcdir) --c_out=generated-code $(srcdir)/test.proto
generated-code/test-full.pb-c.c generated-code/test-full.pb-c.h: ../protoc-c $(srcdir)/test-full.proto
	mkdir -p generated-code
	../protoc-c -I$(srcdir) --c_out=generated-code $(srcdir)/test-full.proto
generated-code/test-full.pb.cc generated-code/test-full.pb.h: $(srcdir)/test-full.proto @PROTOC@
	mkdir -p generated-code
	@PROTOC@ -I$(srcdir) --cpp_out=generated-code $(srcdir)/test-full.proto
generated-code/test-full-cxx-output.inc: cxx-generate-packed-data$(EXEEXT)
	./cxx-generate-packed-data > generated-code/test-full-cxx-output.inc

BUILT_SOURCES = generated-code/test.pb-c.c generated-code/test.pb-c.h \
                generated-code/test-full.pb-c.c generated-code/test-full.pb-c.h \
                generated-code/test-full.pb.cc generated-code/test-full.pb.h \
                generated-code/test-full-cxx-output.inc
DISTCLEANFILES = $(BUILT_SOURCES)
TESTS = test-generated-code test-generated-code2 test-rpc
EXTRA_DIST = test.proto test-full.proto common-test-arrays.h
