#include <stdlib.h>
#include <string.h>

#include "t/issue440/issue440.pb-c.h"

int main(void)
{
	/* Output of $ echo "int: 1 int: -142342 int: 0 int: 423423222" | \
	 * protoc issue440.proto --encode=Int | xxd -i:
	 * 0x0a, 0x11, 0x01, 0xfa, 0xa7, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	 * 0x01, 0x00, 0xf6, 0xd9, 0xf3, 0xc9, 0x01
	 *
	 * Output of $ echo "int: 1 int: -142342 int: 0 int: 423423222" | \
	 * protoc issue440.proto --encode=Int | protoc issue440.proto \
	 * --decode=Boolean: boolean: true boolean: true boolean: false boolean: true
	 */
	uint8_t protoc[] = {0x0a, 0x11, 0x01, 0xfa, 0xa7, 0xf7, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0xf6, 0xd9, 0xf3, 0xc9,
		0x01};
	Boolean *msg = boolean__unpack (NULL, sizeof protoc, protoc);
	assert(msg);
	assert(msg->n_boolean == 4);
	assert(msg->boolean[0] == 1);
	assert(msg->boolean[1] == 1);
	assert(msg->boolean[2] == 0);
	assert(msg->boolean[3] == 1);
	boolean__free_unpacked (msg, NULL);

	return EXIT_SUCCESS;
}
