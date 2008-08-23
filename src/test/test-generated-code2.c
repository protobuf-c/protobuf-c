#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "generated-code/test-full.pb-c.h"
#include "generated-code/test-full-cxx-output.inc"

/* ==== helper functions ==== */
static protobuf_c_boolean
are_bytes_equal (unsigned actual_len,
                 const uint8_t *actual_data,
                 unsigned expected_len,
                 const uint8_t *expected_data)
{
  if (actual_len != expected_len)
    return 0;
  return memcmp (actual_data, expected_data, actual_len) == 0;
}
#define TEST_VERSUS_STATIC_ARRAY(actual_len, actual_data, buf) \
  assert(are_bytes_equal(actual_len,actual_data, \
                         sizeof(buf), buf))

/* rv is unpacked message */
static void *
test_compare_pack_methods (ProtobufCMessage *message,
                           size_t *packed_len_out,
                           uint8_t **packed_out)
{
  unsigned char scratch[16];
  ProtobufCBufferSimple bs = PROTOBUF_C_BUFFER_SIMPLE_INIT (scratch);
  size_t siz1 = protobuf_c_message_get_packed_size (message);
  size_t siz2;
  size_t siz3 = protobuf_c_message_pack_to_buffer (message, &bs.base);
  void *packed1 = malloc (siz1);
  void *rv;
  assert (packed1 != NULL);
  assert (siz1 == siz3);
  siz2 = protobuf_c_message_pack (message, packed1);
  assert (siz1 == siz2);
  assert (bs.len == siz1);
  assert (memcmp (bs.data, packed1, siz1) == 0);
  rv = protobuf_c_message_unpack (message->descriptor, NULL, siz1, packed1);
  assert (rv != NULL);
  PROTOBUF_C_BUFFER_SIMPLE_CLEAR (&bs);
  *packed_len_out = siz1;
  *packed_out = packed1;
  return rv;
}

/* === the actual tests === */

static void test_enum_small (void)
{
  Foo__TestMessRequiredEnumSmall small = FOO__TEST_MESS_REQUIRED_ENUM_SMALL__INIT;
  size_t len;
  uint8_t *data;
  Foo__TestMessRequiredEnumSmall *unpacked;

  small.test = FOO__TEST_ENUM_SMALL__VALUE;
  unpacked = test_compare_pack_methods ((ProtobufCMessage*)&small, &len, &data);
  assert (unpacked->test == FOO__TEST_ENUM_SMALL__VALUE);
  foo__test_mess_required_enum_small__free_unpacked (unpacked, NULL);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_enum_small_VALUE);
  free (data);

  small.test = FOO__TEST_ENUM_SMALL__OTHER_VALUE;
  unpacked = test_compare_pack_methods ((ProtobufCMessage*)&small, &len, &data);
  assert (unpacked->test == FOO__TEST_ENUM_SMALL__OTHER_VALUE);
  foo__test_mess_required_enum_small__free_unpacked (unpacked, NULL);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_enum_small_OTHER_VALUE);
  free (data);
}

static void test_enum_big (void)
{
  Foo__TestMessRequiredEnum big = FOO__TEST_MESS_REQUIRED_ENUM__INIT;
  size_t len;
  uint8_t *data;
  Foo__TestMessRequiredEnumSmall *unpacked;

#define DO_ONE_TEST(shortname, numeric_value, encoded_len) \
  do{ \
    big.test = FOO__TEST_ENUM__##shortname; \
    unpacked = test_compare_pack_methods ((ProtobufCMessage*)&big, &len, &data); \
    assert (unpacked->test == FOO__TEST_ENUM__##shortname); \
    foo__test_mess_required_enum__free_unpacked (unpacked, NULL); \
    TEST_VERSUS_STATIC_ARRAY (len, data, test_enum_big_##shortname); \
    assert (encoded_len + 1 == len); \
    assert (big.test == numeric_value); \
    free (data); \
  }while(0)

  DO_ONE_TEST(VALUE0, 0, 1);
  DO_ONE_TEST(VALUE127, 127, 1);
  DO_ONE_TEST(VALUE128, 128, 2);
  DO_ONE_TEST(VALUE16383, 16383, 2);
  DO_ONE_TEST(VALUE16384, 16384, 3);
  DO_ONE_TEST(VALUE2097151, 2097151, 3);
  DO_ONE_TEST(VALUE2097152, 2097152, 4);
  DO_ONE_TEST(VALUE268435455, 268435455, 4);
  DO_ONE_TEST(VALUE268435456, 268435456, 5);

#undef DO_ONE_TEST
}

/* === simple testing framework === */

typedef void (*TestFunc) (void);

typedef struct {
  const char *name;
  TestFunc func;
} Test;

static Test tests[] =
{
  { "small enums", test_enum_small },
  { "big enums", test_enum_big },
  //{ "test field numbers", test_field_numbers },
  //{ "test repeated int32" ,test_repeated_int32 },
  //{ "test repeated sint32" ,test_repeated_sint32 },
  //{ "test repeated sfixed32" ,test_repeated_sfixed32 },
  //{ "test repeated int64" ,test_repeated_int64 },
  //{ "test repeated sint64" ,test_repeated_sint64 },
  //{ "test repeated sfixed64" ,test_repeated_sfixed64 },
  //{ "test repeated uint32" ,test_repeated_uint32 },
  //{ "test repeated fixed32" ,test_repeated_fixed32 },
  //{ "test repeated uint64" ,test_repeated_uint64 },
  //{ "test repeated fixed64" ,test_repeated_fixed64 },
  //{ "test repeated float" ,test_repeated_float },
  //{ "test repeated double" ,test_repeated_double },
  //{ "test repeated bool" ,test_repeated_bool },
  //{ "test repeated TestEnumSmall" ,test_repeated_TestEnumSmall },
  //{ "test repeated TestEnum" ,test_repeated_TestEnum },
  //{ "test repeated string" ,test_repeated_string },
  //{ "test repeated bytes" ,test_repeated_bytes },
  //{ "test repeated SubMess" ,test_repeated_SubMess },
  //{ "test optional int32" ,test_optional_int32 },
  //{ "test optional sint32" ,test_optional_sint32 },
  //{ "test optional sfixed32" ,test_optional_sfixed32 },
  //{ "test optional int64" ,test_optional_int64 },
  //{ "test optional sint64" ,test_optional_sint64 },
  //{ "test optional sfixed64" ,test_optional_sfixed64 },
  //{ "test optional uint32" ,test_optional_uint32 },
  //{ "test optional fixed32" ,test_optional_fixed32 },
  //{ "test optional uint64" ,test_optional_uint64 },
  //{ "test optional fixed64" ,test_optional_fixed64 },
  //{ "test optional float" ,test_optional_float },
  //{ "test optional double" ,test_optional_double },
  //{ "test optional bool" ,test_optional_bool },
  //{ "test optional TestEnumSmall" ,test_optional_TestEnumSmall },
  //{ "test optional TestEnum" ,test_optional_TestEnum },
  //{ "test optional string" ,test_optional_string },
  //{ "test optional bytes" ,test_optional_bytes },
  //{ "test optional SubMess" ,test_optional_SubMess },
  //{ "test required int32" ,test_required_int32 },
  //{ "test required sint32" ,test_required_sint32 },
  //{ "test required sfixed32" ,test_required_sfixed32 },
  //{ "test required int64" ,test_required_int64 },
  //{ "test required sint64" ,test_required_sint64 },
  //{ "test required sfixed64" ,test_required_sfixed64 },
  //{ "test required uint32" ,test_required_uint32 },
  //{ "test required fixed32" ,test_required_fixed32 },
  //{ "test required uint64" ,test_required_uint64 },
  //{ "test required fixed64" ,test_required_fixed64 },
  //{ "test required float" ,test_required_float },
  //{ "test required double" ,test_required_double },
  //{ "test required bool" ,test_required_bool },
  //{ "test required TestEnumSmall" ,test_required_TestEnumSmall },
  //{ "test required TestEnum" ,test_required_TestEnum },
  //{ "test required string" ,test_required_string },
  //{ "test required bytes" ,test_required_bytes },
  //{ "test required SubMess" ,test_required_SubMess },
};
#define n_tests (sizeof(tests)/sizeof(Test))

int main (int argc, char **argv)
{
  unsigned i;
  for (i = 0; i < n_tests; i++)
    {
      fprintf (stderr, "Test: %s... ", tests[i].name);
      tests[i].func ();
      fprintf (stderr, " done.\n");
    }
  return 0;
}
