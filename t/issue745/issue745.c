#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "t/issue745/issue745.pb-c.h"

int main(void)
{
	T t = T__INIT;

	size_t offset_to_union = offsetof(T, test_bool);
	size_t size_of_union = sizeof(T) - offset_to_union;
	unsigned char *ptr_to_union = ((unsigned char *)&t) + offset_to_union;

	assert(offsetof(T, test_bool) == offsetof(T, test_enum));
	assert(offsetof(T, test_enum) == offsetof(T, test_float));
	assert(offsetof(T, test_float) == offsetof(T, test_uint32));
	assert(offsetof(T, test_uint32) == offsetof(T, test_message));
	assert(offsetof(T, test_message) == offsetof(T, test_string));
	assert(offsetof(T, test_string) == offsetof(T, test_double));
	assert(offsetof(T, test_double) == offsetof(T, test_uint64));
	assert(offsetof(T, test_uint64) == offsetof(T, test_bytes));

	for (size_t i = 0; i < size_of_union; i++) {
		fprintf(stderr, "ptr_to_union[%zd] = %02x\n", i, ptr_to_union[i]);
	}

	// The following will probably crash on gcc >= 15 under its default
	// `-fzero-init-padding-bits=standard` behavior, if the ordering of oneof union
	// members in T are as performed by protobuf-c <= 1.5.0.
	//
	// The code generator in protobuf-c >= 1.5.1 should order union members from
	// largest to smallest, which should correctly zero all the bits used by the
	// object representations of the members of the oneof union even on gcc >= 15.

	for (size_t i = 0; i < size_of_union; i++) {
		assert(ptr_to_union[i] == 0);
	}

	return EXIT_SUCCESS;
}
