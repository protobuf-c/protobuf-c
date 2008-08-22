check_PROGRAMS = test-generated-code
INCLUDES = -I$(srcdir)/..
test_generated_code_SOURCES = \
test-generated-code.c \
generated-code/test.pb-c.c
test_generated_code_LDADD = \
../libprotobuf-c.la

generated-code/test.pb-c.c generated-code/test.pb-c.h: ../protoc-c $(srcdir)/test.proto
	rm -rf generated-code
	mkdir generated-code
	../protoc-c -I$(srcdir) --c_out=generated-code $(srcdir)/test.proto

BUILT_SOURCES = generated-code/test.pb-c.c generated-code/test.pb-c.h
DISTCLEANFILES = $(BUILT_SOURCES)
TESTS = test-generated-code
EXTRA_DIST = test.proto
