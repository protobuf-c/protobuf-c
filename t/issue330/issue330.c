#include <stdlib.h>
#include <string.h>

#include "t/issue330/issue330.pb-c.h"

int main(void)
{
	/* Output of $ echo acl_id: 2 acl_id: 3 | protoc issue330.proto \
	 * --encode=pbr_route | xxd -i:  0x52, 0x02, 0x02, 0x03 
	 */
	uint8_t protoc[] = {0x52, 0x02, 0x02, 0x03};
	PbrRoute msg = PBR_ROUTE__INIT;
	int ids[] = {2, 3};
	uint8_t buf[16] = {0};
	size_t sz = 0;

	msg.n_acl_id = 2;
	msg.acl_id = ids;
	sz = pbr_route__pack(&msg, buf);

	assert (sz == sizeof protoc);
	assert (memcmp (protoc, buf, sz) == 0);

	return EXIT_SUCCESS;
}
