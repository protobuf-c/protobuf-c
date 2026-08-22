#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "t/issue804/issue804.pb-c.h"

int main(void)
{
	// Two members of the oneof are set. This is legal at the wire format level:
	//
	// "Note that if multiple values are set, the last set value as determined by
	// the order in the proto will overwrite all previous ones."
	//
	// https://protobuf.dev/programming-guides/proto3/#oneof
	static const uint8_t wire[] = {
		// M.a = 1
		0x08, 0x01,

		// M.b = "A"
		0x12, 0x01, 0x41,
	};

	M *m = m__unpack(NULL, sizeof(wire), wire);
	assert(m != NULL);

	m__free_unpacked(m, NULL);

	return EXIT_SUCCESS;
}
