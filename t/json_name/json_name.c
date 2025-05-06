#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "t/json_name/json_name.pb-c.h"

int main(void)
{
    JsonNames msg = JSON_NAMES__INIT;
    const ProtobufCFieldDescriptor *field;

    // Test regular field json_name
    field = protobuf_c_message_descriptor_get_field_by_name(
        &json_names__descriptor,
        "regular_field_a"
    );
    assert(field != NULL);
    assert(strcmp(field->json_name, "customNameA") == 0);

    // Test snake_case field json_name
    field = protobuf_c_message_descriptor_get_field_by_name(
        &json_names__descriptor,
        "regular_field_b"
    );
    assert(field != NULL);
    assert(strcmp(field->json_name, "customNameB") == 0);

    // Test no json_name field falls back to camelCase
    field = protobuf_c_message_descriptor_get_field_by_name(
        &json_names__descriptor,
        "no_json_name"
    );
    assert(field != NULL);
    assert(strcmp(field->json_name, "noJsonName") == 0);

    // Test oneof field json_names
    field = protobuf_c_message_descriptor_get_field_by_name(
        &json_names__descriptor,
        "oneof_field_a"
    );
    assert(field != NULL);
    assert(strcmp(field->json_name, "oneofCustomA") == 0);

    field = protobuf_c_message_descriptor_get_field_by_name(
        &json_names__descriptor,
        "oneof_field_b"
    );
    assert(field != NULL);
    assert(strcmp(field->json_name, "oneofCustomB") == 0);

    // Test map field json_name
    field = protobuf_c_message_descriptor_get_field_by_name(
        &json_names__descriptor,
        "map_field"
    );
    assert(field != NULL);
    assert(strcmp(field->json_name, "customMap") == 0);

    // Test repeated field json_name
    field = protobuf_c_message_descriptor_get_field_by_name(
        &json_names__descriptor,
        "numbers"
    );
    assert(field != NULL);
    assert(strcmp(field->json_name, "numberArray") == 0);

    // Test nested message field json_name
    field = protobuf_c_message_descriptor_get_field_by_name(
        &json_names__descriptor,
        "nested_message"
    );
    assert(field != NULL);
    assert(strcmp(field->json_name, "nestedMsg") == 0);

    return EXIT_SUCCESS;
}