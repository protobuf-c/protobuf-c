#include <assert.h>
#include <stdlib.h>

#include "t/issue375/issue375.pb-c.h"

int main(void) {
	// This buffer represents some serialized bytes where we have a length
	// delimiter of 2^32 - 1 bytes for a particular repeated int32 field.
	// We want to make sure that parsing a length delimiter this large does
	// not cause a problematic integer overflow.
	uint8_t buffer[] = {
		// Field 1 with wire type 2 (length-delimited)
		0x0a,
		// Varint length delimiter: 2^32 - 1
		0xff, 0xff, 0xff, 0xff, 0x0f,
	};
	// The parser should detect that this message is malformed and return
	// null.
	Issue375__TestMessage* m =
		issue375__test_message__unpack(NULL, sizeof(buffer), buffer);
	assert(m == NULL);

	return EXIT_SUCCESS;
}
