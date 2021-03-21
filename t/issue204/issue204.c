#include <stdlib.h>

#include "t/issue251/issue251.pb-c.h"

int main(void)
{
	TwoOneofs msg = TWO_ONEOFS__INIT;
	const ProtobufCFieldDescriptor *field;
	unsigned off1, off2, off_name;
	field = protobuf_c_message_descriptor_get_field_by_name(
						msg.base.descriptor,
						"first_oneof");
	assert (field);
	off_name = field->offset;
	field = protobuf_c_message_descriptor_get_field(
						msg.base.descriptor,
						10);
	assert (field);
	off1 = field->offset;
	field = protobuf_c_message_descriptor_get_field(
						msg.base.descriptor,
						11);
	assert (field);
	off2 = field->offset;

	assert (off_name == off1);
	assert (off1 == off2);

	field = protobuf_c_message_descriptor_get_field_by_name(
						msg.base.descriptor,
						"second_oneof");
	assert (field);
	off_name = field->offset;
	field = protobuf_c_message_descriptor_get_field(
						msg.base.descriptor,
						20);
	assert (field);
	off1 = field->offset;
	field = protobuf_c_message_descriptor_get_field(
						msg.base.descriptor,
						21);
	assert (field);
	off2 = field->offset;

	assert (off_name == off1);
	assert (off1 == off2);
	return EXIT_SUCCESS;
}
