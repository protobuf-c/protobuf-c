#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "generated-code/test-full.pb-c.h"
#include "generated-code/test-full-cxx-output.inc"

#define TEST_ENUM_SMALL_TYPE_NAME   Foo__TestEnumSmall
#define TEST_ENUM_SMALL(shortname)   FOO__TEST_ENUM_SMALL__##shortname
#define TEST_ENUM_TYPE_NAME   Foo__TestEnum
#include "common-test-arrays.h"
#define N_ELEMENTS(arr)   (sizeof(arr)/sizeof((arr)[0]))

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
static void
dump_bytes_stderr (size_t len, const uint8_t *data)
{
  size_t i;
  for (i = 0; i < len; i++)
    fprintf(stderr," %02x", data[i]);
  fprintf(stderr,"\n");
}
static void
test_versus_static_array(unsigned actual_len,
                         const uint8_t *actual_data,
                         unsigned expected_len,
                         const uint8_t *expected_data,
                         const char *static_buf_name,
                         const char *filename,
                         unsigned lineno)
{
  if (are_bytes_equal (actual_len, actual_data, expected_len, expected_data))
    return;

  fprintf (stderr, "test_versus_static_array failed:\n"
                   "actual: [length=%u]\n", actual_len);
  dump_bytes_stderr (actual_len, actual_data);
  fprintf (stderr, "expected: [length=%u] [buffer %s]\n",
           expected_len, static_buf_name);
  dump_bytes_stderr (expected_len, expected_data);
  fprintf (stderr, "at line %u of %s.\n", lineno, filename);
  abort();
}

#define TEST_VERSUS_STATIC_ARRAY(actual_len, actual_data, buf) \
  test_versus_static_array(actual_len,actual_data, \
                           sizeof(buf), buf, #buf, __FILE__, __LINE__)

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

#define NUMERIC_EQUALS(a,b)   ((a) == (b))
#define STRING_EQUALS(a,b)    (strcmp((a),(b))==0)

static protobuf_c_boolean
binary_data_equals (ProtobufCBinaryData a, ProtobufCBinaryData b)
{
  if (a.len != b.len)
    return 0;
  return memcmp (a.data, b.data, a.len) == 0;
}

static protobuf_c_boolean
submesses_equals (Foo__SubMess *a, Foo__SubMess *b)
{
  assert(a->base.descriptor == &foo__sub_mess__descriptor);
  assert(b->base.descriptor == &foo__sub_mess__descriptor);
  return a->test == b->test;
}

/* === the actual tests === */


/* === misc fencepost tests === */
static void test_enum_small (void)
{

#define DO_TEST(UC_VALUE) \
  do{ \
    Foo__TestMessRequiredEnumSmall small = FOO__TEST_MESS_REQUIRED_ENUM_SMALL__INIT; \
    size_t len; \
    uint8_t *data; \
    Foo__TestMessRequiredEnumSmall *unpacked; \
    small.test = FOO__TEST_ENUM_SMALL__##UC_VALUE; \
    unpacked = test_compare_pack_methods ((ProtobufCMessage*)&small, &len, &data); \
    assert (unpacked->test == FOO__TEST_ENUM_SMALL__##UC_VALUE); \
    foo__test_mess_required_enum_small__free_unpacked (unpacked, NULL); \
    TEST_VERSUS_STATIC_ARRAY (len, data, test_enum_small_##UC_VALUE); \
    free (data); \
  }while(0)

  assert (sizeof (Foo__TestEnumSmall) == 4);

  DO_TEST(VALUE);
  DO_TEST(OTHER_VALUE);

#undef DO_TEST
}

static void test_enum_big (void)
{
  Foo__TestMessRequiredEnum big = FOO__TEST_MESS_REQUIRED_ENUM__INIT;
  size_t len;
  uint8_t *data;
  Foo__TestMessRequiredEnum *unpacked;

  assert (sizeof (Foo__TestEnum) == 4);

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
static void test_field_numbers (void)
{
  size_t len;
  uint8_t *data;

#define DO_ONE_TEST(num, exp_len) \
  { \
    Foo__TestFieldNo##num t = FOO__TEST_FIELD_NO##num##__INIT; \
    Foo__TestFieldNo##num *t2; \
    t.test = "tst"; \
    t2 = test_compare_pack_methods ((ProtobufCMessage*)(&t), &len, &data); \
    assert (strcmp (t2->test, "tst") == 0); \
    TEST_VERSUS_STATIC_ARRAY (len, data, test_field_number_##num); \
    assert (len == exp_len); \
    free (data); \
    foo__test_field_no##num##__free_unpacked (t2, NULL); \
  }
  DO_ONE_TEST (15, 1 + 1 + 3);
  DO_ONE_TEST (16, 2 + 1 + 3);
  DO_ONE_TEST (2047, 2 + 1 + 3);
  DO_ONE_TEST (2048, 3 + 1 + 3);
  DO_ONE_TEST (262143, 3 + 1 + 3);
  DO_ONE_TEST (262144, 4 + 1 + 3);
  DO_ONE_TEST (33554431, 4 + 1 + 3);
  DO_ONE_TEST (33554432, 5 + 1 + 3);
#undef DO_ONE_TEST
}

/* === Required type fields === */

#define DO_TEST_REQUIRED(Type, TYPE, type, value, example_packed_data, equal_func) \
  do{ \
  Foo__TestMessRequired##Type opt = FOO__TEST_MESS_REQUIRED_##TYPE##__INIT; \
  Foo__TestMessRequired##Type *mess; \
  size_t len; uint8_t *data; \
  opt.test = value; \
  mess = test_compare_pack_methods (&opt.base, &len, &data); \
  TEST_VERSUS_STATIC_ARRAY (len, data, example_packed_data); \
  assert (equal_func (mess->test, value)); \
  foo__test_mess_required_##type##__free_unpacked (mess, NULL); \
  free (data); \
  }while(0)
static void test_required_int32 (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_REQUIRED(Int32, INT32, int32, value, example_packed_data, NUMERIC_EQUALS)
  DO_TEST(INT32_MIN, test_required_int32_min);
  DO_TEST(-1000, test_required_int32_m1000);
  DO_TEST(0, test_required_int32_0);
  DO_TEST(INT32_MAX, test_required_int32_max);
#undef DO_TEST
}

static void test_required_sint32 (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_REQUIRED(SInt32, SINT32, sint32, value, example_packed_data, NUMERIC_EQUALS)
  DO_TEST(INT32_MIN, test_required_sint32_min);
  DO_TEST(-1000, test_required_sint32_m1000);
  DO_TEST(0, test_required_sint32_0);
  DO_TEST(INT32_MAX, test_required_sint32_max);
#undef DO_TEST
}

static void test_required_sfixed32 (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_REQUIRED(SFixed32, SFIXED32, sfixed32, value, example_packed_data, NUMERIC_EQUALS)
  DO_TEST(INT32_MIN, test_required_sfixed32_min);
  DO_TEST(-1000, test_required_sfixed32_m1000);
  DO_TEST(0, test_required_sfixed32_0);
  DO_TEST(INT32_MAX, test_required_sfixed32_max);
#undef DO_TEST
}

static void test_required_uint32 (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_REQUIRED(UInt32, UINT32, uint32, value, example_packed_data, NUMERIC_EQUALS)
  DO_TEST(0, test_required_uint32_0);
  DO_TEST(MILLION, test_required_uint32_million);
  DO_TEST(UINT32_MAX, test_required_uint32_max);
#undef DO_TEST
}

static void test_required_fixed32 (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_REQUIRED(Fixed32, FIXED32, fixed32, value, example_packed_data, NUMERIC_EQUALS)
  DO_TEST(0, test_required_fixed32_0);
  DO_TEST(MILLION, test_required_fixed32_million);
  DO_TEST(UINT32_MAX, test_required_fixed32_max);
#undef DO_TEST
}

static void test_required_int64 (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_REQUIRED(Int64, INT64, int64, value, example_packed_data, NUMERIC_EQUALS)
  DO_TEST(INT64_MIN, test_required_int64_min);
  DO_TEST(-TRILLION, test_required_int64_mtril);
  DO_TEST(0, test_required_int64_0);
  DO_TEST(QUADRILLION, test_required_int64_quad);
  DO_TEST(INT64_MAX, test_required_int64_max);
#undef DO_TEST
}

static void test_required_sint64 (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_REQUIRED(SInt64, SINT64, sint64, value, example_packed_data, NUMERIC_EQUALS)
  DO_TEST(INT64_MIN, test_required_sint64_min);
  DO_TEST(-TRILLION, test_required_sint64_mtril);
  DO_TEST(0, test_required_sint64_0);
  DO_TEST(QUADRILLION, test_required_sint64_quad);
  DO_TEST(INT64_MAX, test_required_sint64_max);
#undef DO_TEST
}

static void test_required_sfixed64 (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_REQUIRED(SFixed64, SFIXED64, sfixed64, value, example_packed_data, NUMERIC_EQUALS)
  DO_TEST(INT64_MIN, test_required_sfixed64_min);
  DO_TEST(-TRILLION, test_required_sfixed64_mtril);
  DO_TEST(0, test_required_sfixed64_0);
  DO_TEST(QUADRILLION, test_required_sfixed64_quad);
  DO_TEST(INT64_MAX, test_required_sfixed64_max);
#undef DO_TEST
}

static void test_required_uint64 (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_REQUIRED(UInt64, UINT64, uint64, value, example_packed_data, NUMERIC_EQUALS)
  DO_TEST(0, test_required_uint64_0);
  DO_TEST(THOUSAND, test_required_uint64_thou);
  DO_TEST(MILLION, test_required_uint64_mill);
  DO_TEST(BILLION, test_required_uint64_bill);
  DO_TEST(TRILLION, test_required_uint64_tril);
  DO_TEST(QUADRILLION, test_required_uint64_quad);
  DO_TEST(QUINTILLION, test_required_uint64_quint);
  DO_TEST(UINT64_MAX, test_required_uint64_max);
#undef DO_TEST
}

static void test_required_fixed64 (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_REQUIRED(Fixed64, FIXED64, fixed64, value, example_packed_data, NUMERIC_EQUALS)
  DO_TEST(0, test_required_fixed64_0);
  DO_TEST(THOUSAND, test_required_fixed64_thou);
  DO_TEST(MILLION, test_required_fixed64_mill);
  DO_TEST(BILLION, test_required_fixed64_bill);
  DO_TEST(TRILLION, test_required_fixed64_tril);
  DO_TEST(QUADRILLION, test_required_fixed64_quad);
  DO_TEST(QUINTILLION, test_required_fixed64_quint);
  DO_TEST(UINT64_MAX, test_required_fixed64_max);
#undef DO_TEST
}

static void test_required_float (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_REQUIRED(Float, FLOAT, float, value, example_packed_data, NUMERIC_EQUALS)
  DO_TEST(-THOUSAND, test_required_float_mthou);
  DO_TEST(0, test_required_float_0);
  DO_TEST(420, test_required_float_420);
#undef DO_TEST
}

static void test_required_double (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_REQUIRED(Double, DOUBLE, double, value, example_packed_data, NUMERIC_EQUALS)
  DO_TEST(-THOUSAND, test_required_double_mthou);
  DO_TEST(0, test_required_double_0);
  DO_TEST(420, test_required_double_420);
#undef DO_TEST
}

static void test_required_bool (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_REQUIRED(Bool, BOOL, bool, value, example_packed_data, NUMERIC_EQUALS)
  DO_TEST(0, test_required_bool_0);
  DO_TEST(1, test_required_bool_1);
#undef DO_TEST
}

static void test_required_TestEnumSmall (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_REQUIRED(EnumSmall, ENUM_SMALL, enum_small, value, example_packed_data, NUMERIC_EQUALS)
  DO_TEST(FOO__TEST_ENUM_SMALL__VALUE, test_required_enum_small_VALUE);
  DO_TEST(FOO__TEST_ENUM_SMALL__OTHER_VALUE, test_required_enum_small_OTHER_VALUE);
#undef DO_TEST
}

static void test_required_TestEnum (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_REQUIRED(Enum, ENUM, enum, value, example_packed_data, NUMERIC_EQUALS)

  DO_TEST (FOO__TEST_ENUM__VALUE0, test_required_enum_0);
  DO_TEST (FOO__TEST_ENUM__VALUE1, test_required_enum_1);
  DO_TEST (FOO__TEST_ENUM__VALUE127, test_required_enum_127);
  DO_TEST (FOO__TEST_ENUM__VALUE128, test_required_enum_128);
  DO_TEST (FOO__TEST_ENUM__VALUE16383, test_required_enum_16383);
  DO_TEST (FOO__TEST_ENUM__VALUE16384, test_required_enum_16384);
  DO_TEST (FOO__TEST_ENUM__VALUE2097151, test_required_enum_2097151);
  DO_TEST (FOO__TEST_ENUM__VALUE2097152, test_required_enum_2097152);
  DO_TEST (FOO__TEST_ENUM__VALUE268435455, test_required_enum_268435455);
  DO_TEST (FOO__TEST_ENUM__VALUE268435456, test_required_enum_268435456);

#undef DO_TEST
}

static void test_required_string (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_REQUIRED(String, STRING, string, value, example_packed_data, STRING_EQUALS)

  DO_TEST("", test_required_string_empty);
  DO_TEST("hello", test_required_string_hello);
  DO_TEST("two hundred xs follow: xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", test_required_string_long);

#undef DO_TEST
}

static void test_required_bytes (void)
{
  static ProtobufCBinaryData bd_empty = { 0, (uint8_t*)"" };
  static ProtobufCBinaryData bd_hello = { 5, (uint8_t*)"hello" };
  static ProtobufCBinaryData bd_random = { 5, (uint8_t*)"\1\0\375\2\4" };
#define DO_TEST(value, example_packed_data) \
  DO_TEST_REQUIRED (Bytes, BYTES, bytes, value, example_packed_data, binary_data_equals)
  DO_TEST (bd_empty, test_required_bytes_empty);
  DO_TEST (bd_hello, test_required_bytes_hello);
  DO_TEST (bd_random, test_required_bytes_random);
#undef DO_TEST
}

static void test_required_SubMess (void)
{
  Foo__SubMess submess = FOO__SUB_MESS__INIT;
#define DO_TEST(value, example_packed_data) \
  DO_TEST_REQUIRED (Message, MESSAGE, message, value, example_packed_data, submesses_equals)
  submess.test = 0;
  DO_TEST (&submess, test_required_submess_0);
  submess.test = 42;
  DO_TEST (&submess, test_required_submess_42);
#undef DO_TEST
}

/* === Optional type fields === */
static void test_empty_optional (void)
{
  Foo__TestMessOptional mess = FOO__TEST_MESS_OPTIONAL__INIT;
  size_t len;
  uint8_t *data;
  Foo__TestMessOptional *mess2 = test_compare_pack_methods (&mess.base, &len, &data);
  assert (len == 0);
  free (data);
  foo__test_mess_optional__free_unpacked (mess2, NULL);
}


#define DO_TEST_OPTIONAL(base_member, value, example_packed_data, equal_func) \
  do{ \
  Foo__TestMessOptional opt = FOO__TEST_MESS_OPTIONAL__INIT; \
  Foo__TestMessOptional *mess; \
  size_t len; uint8_t *data; \
  opt.has_##base_member = 1; \
  opt.base_member = value; \
  mess = test_compare_pack_methods (&opt.base, &len, &data); \
  TEST_VERSUS_STATIC_ARRAY (len, data, example_packed_data); \
  assert (mess->has_##base_member); \
  assert (equal_func (mess->base_member, value)); \
  foo__test_mess_optional__free_unpacked (mess, NULL); \
  free (data); \
  }while(0)

static void test_optional_int32 (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_OPTIONAL(test_int32, value, example_packed_data, NUMERIC_EQUALS)

  DO_TEST (INT32_MIN, test_optional_int32_min);
  DO_TEST (-1, test_optional_int32_m1);
  DO_TEST (0, test_optional_int32_0);
  DO_TEST (666, test_optional_int32_666);
  DO_TEST (INT32_MAX, test_optional_int32_max);

#undef DO_TEST
}

static void test_optional_sint32 (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_OPTIONAL(test_sint32, value, example_packed_data, NUMERIC_EQUALS)

  DO_TEST (INT32_MIN, test_optional_sint32_min);
  DO_TEST (-1, test_optional_sint32_m1);
  DO_TEST (0, test_optional_sint32_0);
  DO_TEST (666, test_optional_sint32_666);
  DO_TEST (INT32_MAX, test_optional_sint32_max);

#undef DO_TEST
}
static void test_optional_sfixed32 (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_OPTIONAL(test_sfixed32, value, example_packed_data, NUMERIC_EQUALS)

  DO_TEST (INT32_MIN, test_optional_sfixed32_min);
  DO_TEST (-1, test_optional_sfixed32_m1);
  DO_TEST (0, test_optional_sfixed32_0);
  DO_TEST (666, test_optional_sfixed32_666);
  DO_TEST (INT32_MAX, test_optional_sfixed32_max);

#undef DO_TEST
}
static void test_optional_int64 (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_OPTIONAL(test_int64, value, example_packed_data, NUMERIC_EQUALS)

  DO_TEST (INT64_MIN, test_optional_int64_min);
  DO_TEST (-1111111111LL, test_optional_int64_m1111111111LL);
  DO_TEST (0, test_optional_int64_0);
  DO_TEST (QUINTILLION, test_optional_int64_quintillion);
  DO_TEST (INT64_MAX, test_optional_int64_max);

#undef DO_TEST
}
static void test_optional_sint64 (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_OPTIONAL(test_sint64, value, example_packed_data, NUMERIC_EQUALS)

  DO_TEST (INT64_MIN, test_optional_sint64_min);
  DO_TEST (-1111111111LL, test_optional_sint64_m1111111111LL);
  DO_TEST (0, test_optional_sint64_0);
  DO_TEST (QUINTILLION, test_optional_sint64_quintillion);
  DO_TEST (INT64_MAX, test_optional_sint64_max);

#undef DO_TEST
}
static void test_optional_sfixed64 (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_OPTIONAL(test_sfixed64, value, example_packed_data, NUMERIC_EQUALS)

  DO_TEST (INT64_MIN, test_optional_sfixed64_min);
  DO_TEST (-1111111111LL, test_optional_sfixed64_m1111111111LL);
  DO_TEST (0, test_optional_sfixed64_0);
  DO_TEST (QUINTILLION, test_optional_sfixed64_quintillion);
  DO_TEST (INT64_MAX, test_optional_sfixed64_max);

#undef DO_TEST
}

static void test_optional_uint32 (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_OPTIONAL(test_uint32, value, example_packed_data, NUMERIC_EQUALS)

  DO_TEST (0, test_optional_uint32_0);
  DO_TEST (669, test_optional_uint32_669);
  DO_TEST (UINT32_MAX, test_optional_uint32_max);

#undef DO_TEST
}

static void test_optional_fixed32 (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_OPTIONAL(test_fixed32, value, example_packed_data, NUMERIC_EQUALS)

  DO_TEST (0, test_optional_fixed32_0);
  DO_TEST (669, test_optional_fixed32_669);
  DO_TEST (UINT32_MAX, test_optional_fixed32_max);

#undef DO_TEST
}

static void test_optional_uint64 (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_OPTIONAL(test_uint64, value, example_packed_data, NUMERIC_EQUALS)

  DO_TEST (0, test_optional_uint64_0);
  DO_TEST (669669669669669ULL, test_optional_uint64_669669669669669);
  DO_TEST (UINT64_MAX, test_optional_uint64_max);

#undef DO_TEST
}

static void test_optional_fixed64 (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_OPTIONAL(test_fixed64, value, example_packed_data, NUMERIC_EQUALS)

  DO_TEST (0, test_optional_fixed64_0);
  DO_TEST (669669669669669ULL, test_optional_fixed64_669669669669669);
  DO_TEST (UINT64_MAX, test_optional_fixed64_max);

#undef DO_TEST
}

static void test_optional_float (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_OPTIONAL(test_float, value, example_packed_data, NUMERIC_EQUALS)

  DO_TEST (-100, test_optional_float_m100);
  DO_TEST (0, test_optional_float_0);
  DO_TEST (141243, test_optional_float_141243);

#undef DO_TEST
}
static void test_optional_double (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_OPTIONAL(test_double, value, example_packed_data, NUMERIC_EQUALS)

  DO_TEST (-100, test_optional_double_m100);
  DO_TEST (0, test_optional_double_0);
  DO_TEST (141243, test_optional_double_141243);

#undef DO_TEST
}
static void test_optional_bool (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_OPTIONAL(test_boolean, value, example_packed_data, NUMERIC_EQUALS)

  DO_TEST (0, test_optional_bool_0);
  DO_TEST (1, test_optional_bool_1);

#undef DO_TEST
}
static void test_optional_TestEnumSmall (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_OPTIONAL(test_enum_small, value, example_packed_data, NUMERIC_EQUALS)

  DO_TEST (0, test_optional_enum_small_0);
  DO_TEST (1, test_optional_enum_small_1);

#undef DO_TEST
}

static void test_optional_TestEnum (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_OPTIONAL(test_enum, value, example_packed_data, NUMERIC_EQUALS)

  DO_TEST (FOO__TEST_ENUM__VALUE0, test_optional_enum_0);
  DO_TEST (FOO__TEST_ENUM__VALUE1, test_optional_enum_1);
  DO_TEST (FOO__TEST_ENUM__VALUE127, test_optional_enum_127);
  DO_TEST (FOO__TEST_ENUM__VALUE128, test_optional_enum_128);
  DO_TEST (FOO__TEST_ENUM__VALUE16383, test_optional_enum_16383);
  DO_TEST (FOO__TEST_ENUM__VALUE16384, test_optional_enum_16384);
  DO_TEST (FOO__TEST_ENUM__VALUE2097151, test_optional_enum_2097151);
  DO_TEST (FOO__TEST_ENUM__VALUE2097152, test_optional_enum_2097152);
  DO_TEST (FOO__TEST_ENUM__VALUE268435455, test_optional_enum_268435455);
  DO_TEST (FOO__TEST_ENUM__VALUE268435456, test_optional_enum_268435456);

#undef DO_TEST
}

#define DO_TEST_OPTIONAL__NO_HAS(base_member, value, example_packed_data, equal_func) \
  do{ \
  Foo__TestMessOptional opt = FOO__TEST_MESS_OPTIONAL__INIT; \
  Foo__TestMessOptional *mess; \
  size_t len; uint8_t *data; \
  opt.base_member = value; \
  mess = test_compare_pack_methods (&opt.base, &len, &data); \
  TEST_VERSUS_STATIC_ARRAY (len, data, example_packed_data); \
  assert (mess->base_member != NULL); \
  assert (equal_func (mess->base_member, value)); \
  foo__test_mess_optional__free_unpacked (mess, NULL); \
  free (data); \
  }while(0)

static void test_optional_string (void)
{
#define DO_TEST(value, example_packed_data) \
  DO_TEST_OPTIONAL__NO_HAS (test_string, value, example_packed_data, STRING_EQUALS)
  DO_TEST ("", test_optional_string_empty);
  DO_TEST ("hello", test_optional_string_hello);
#undef DO_TEST
}
static void test_optional_bytes (void)
{
  static ProtobufCBinaryData bd_empty = { 0, (uint8_t*)"" };
  static ProtobufCBinaryData bd_hello = { 5, (uint8_t*)"hello" };
  static ProtobufCBinaryData bd_random = { 5, (uint8_t*)"\1\0\375\2\4" };
#define DO_TEST(value, example_packed_data) \
  DO_TEST_OPTIONAL (test_bytes, value, example_packed_data, binary_data_equals)
  DO_TEST (bd_empty, test_optional_bytes_empty);
  DO_TEST (bd_hello, test_optional_bytes_hello);
  DO_TEST (bd_random, test_optional_bytes_random);
#undef DO_TEST
}
static void test_optional_SubMess (void)
{
  Foo__SubMess submess = FOO__SUB_MESS__INIT;
#define DO_TEST(value, example_packed_data) \
  DO_TEST_OPTIONAL__NO_HAS (test_message, value, example_packed_data, submesses_equals)
  submess.test = 0;
  DO_TEST (&submess, test_optional_submess_0);
  submess.test = 42;
  DO_TEST (&submess, test_optional_submess_42);
#undef DO_TEST
}
/* === repeated type fields === */
#define DO_TEST_REPEATED(lc_member_name, cast, \
                         static_array, example_packed_data, \
                         equals_macro) \
  do{ \
  Foo__TestMess mess = FOO__TEST_MESS__INIT; \
  Foo__TestMess *mess2; \
  size_t len; \
  uint8_t *data; \
  unsigned i; \
  mess.n_##lc_member_name = N_ELEMENTS (static_array); \
  mess.lc_member_name = cast static_array; \
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data); \
  assert(mess2->n_##lc_member_name == N_ELEMENTS (static_array)); \
  for (i = 0; i < N_ELEMENTS (static_array); i++) \
    assert(equals_macro(mess2->lc_member_name[i], static_array[i])); \
  TEST_VERSUS_STATIC_ARRAY (len, data, example_packed_data); \
  free (data); \
  foo__test_mess__free_unpacked (mess2, NULL); \
  }while(0)

static void test_empty_repeated (void)
{
  Foo__TestMess mess = FOO__TEST_MESS__INIT;
  size_t len;
  uint8_t *data;
  Foo__TestMess *mess2 = test_compare_pack_methods (&mess.base, &len, &data);
  assert (len == 0);
  free (data);
  foo__test_mess__free_unpacked (mess2, NULL);
}

static void test_repeated_int32 (void)
{
#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_REPEATED(test_int32, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST (int32_arr0, test_repeated_int32_arr0);
  DO_TEST (int32_arr1, test_repeated_int32_arr1);
  DO_TEST (int32_arr_min_max, test_repeated_int32_arr_min_max);

#undef DO_TEST
}

static void test_repeated_sint32 (void)
{
#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_REPEATED(test_sint32, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST (int32_arr0, test_repeated_sint32_arr0);
  DO_TEST (int32_arr1, test_repeated_sint32_arr1);
  DO_TEST (int32_arr_min_max, test_repeated_sint32_arr_min_max);

#undef DO_TEST
}

static void test_repeated_sfixed32 (void)
{
#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_REPEATED(test_sfixed32, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST (int32_arr0, test_repeated_sfixed32_arr0);
  DO_TEST (int32_arr1, test_repeated_sfixed32_arr1);
  DO_TEST (int32_arr_min_max, test_repeated_sfixed32_arr_min_max);

#undef DO_TEST
}

static void test_repeated_uint32 (void)
{
#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_REPEATED(test_uint32, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST (uint32_roundnumbers, test_repeated_uint32_roundnumbers);
  DO_TEST (uint32_0_max, test_repeated_uint32_0_max);

#undef DO_TEST
}



static void test_repeated_fixed32 (void)
{
#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_REPEATED(test_fixed32, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST (uint32_roundnumbers, test_repeated_fixed32_roundnumbers);
  DO_TEST (uint32_0_max, test_repeated_fixed32_0_max);

#undef DO_TEST
}

static void test_repeated_int64 (void)
{
#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_REPEATED(test_int64, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST (int64_roundnumbers, test_repeated_int64_roundnumbers);
  DO_TEST (int64_min_max, test_repeated_int64_min_max);

#undef DO_TEST
}

static void test_repeated_sint64 (void)
{
#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_REPEATED(test_sint64, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST (int64_roundnumbers, test_repeated_sint64_roundnumbers);
  DO_TEST (int64_min_max, test_repeated_sint64_min_max);

#undef DO_TEST
}

static void test_repeated_sfixed64 (void)
{
#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_REPEATED(test_sfixed64, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST (int64_roundnumbers, test_repeated_sfixed64_roundnumbers);
  DO_TEST (int64_min_max, test_repeated_sfixed64_min_max);

#undef DO_TEST
}

static void test_repeated_uint64 (void)
{
#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_REPEATED(test_uint64, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST(uint64_roundnumbers, test_repeated_uint64_roundnumbers);
  DO_TEST(uint64_0_1_max, test_repeated_uint64_0_1_max);
  DO_TEST(uint64_random, test_repeated_uint64_random);

#undef DO_TEST
}

static void test_repeated_fixed64 (void)
{
#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_REPEATED(test_fixed64, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST(uint64_roundnumbers, test_repeated_fixed64_roundnumbers);
  DO_TEST(uint64_0_1_max, test_repeated_fixed64_0_1_max);
  DO_TEST(uint64_random, test_repeated_fixed64_random);

#undef DO_TEST
}

static void test_repeated_float (void)
{

#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_REPEATED(test_float, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST(float_random, test_repeated_float_random);

#undef DO_TEST
}

static void test_repeated_double (void)
{

#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_REPEATED(test_double, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST(double_random, test_repeated_double_random);

#undef DO_TEST
}

static void test_repeated_boolean (void)
{

#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_REPEATED(test_boolean, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST(boolean_0, test_repeated_boolean_0);
  DO_TEST(boolean_1, test_repeated_boolean_1);
  DO_TEST(boolean_random, test_repeated_boolean_random);

#undef DO_TEST
}

static void test_repeated_TestEnumSmall (void)
{

#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_REPEATED(test_enum_small, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST(enum_small_0, test_repeated_enum_small_0);
  DO_TEST(enum_small_1, test_repeated_enum_small_1);
  DO_TEST(enum_small_random, test_repeated_enum_small_random);

#undef DO_TEST
}

static void test_repeated_TestEnum (void)
{

#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_REPEATED(test_enum, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST(enum_0, test_repeated_enum_0);
  DO_TEST(enum_1, test_repeated_enum_1);
  DO_TEST(enum_random, test_repeated_enum_random);

#undef DO_TEST
}

static void test_repeated_string (void)
{

#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_REPEATED(test_string, (char **), \
                   static_array, example_packed_data, \
                   STRING_EQUALS)

  DO_TEST(repeated_strings_0, test_repeated_strings_0);
  DO_TEST(repeated_strings_1, test_repeated_strings_1);
  DO_TEST(repeated_strings_2, test_repeated_strings_2);
  DO_TEST(repeated_strings_3, test_repeated_strings_3);

#undef DO_TEST
}

static void test_repeated_bytes (void)
{
  static ProtobufCBinaryData test_binary_data_0[] = {
    { 4, (uint8_t *) "text" },
    { 9, (uint8_t *) "str\1\2\3\4\5\0" },
    { 10, (uint8_t *) "gobble\0foo" }
  };
#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_REPEATED(test_bytes, , \
                   static_array, example_packed_data, \
                   binary_data_equals)

  DO_TEST (test_binary_data_0, test_repeated_bytes_0);

#undef DO_TEST
}

static void test_repeated_SubMess (void)
{
  static Foo__SubMess submess0 = FOO__SUB_MESS__INIT;
  static Foo__SubMess submess1 = FOO__SUB_MESS__INIT;
  static Foo__SubMess submess2 = FOO__SUB_MESS__INIT;
  static Foo__SubMess *submesses[3] = { &submess0, &submess1, &submess2 };

#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_REPEATED(test_message, , \
                   static_array, example_packed_data, \
                   submesses_equals)

  DO_TEST (submesses, test_repeated_submess_0);
  submess0.test = 42;
  submess1.test = -10000;
  submess2.test = 667;
  DO_TEST (submesses, test_repeated_submess_1);

#undef DO_TEST
}

#define DO_TEST_PACKED_REPEATED(lc_member_name, cast, \
                         static_array, example_packed_data, \
                         equals_macro) \
  do{ \
  Foo__TestMessPacked mess = FOO__TEST_MESS_PACKED__INIT; \
  Foo__TestMessPacked *mess2; \
  size_t len; \
  uint8_t *data; \
  unsigned i; \
  mess.n_##lc_member_name = N_ELEMENTS (static_array); \
  mess.lc_member_name = cast static_array; \
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data); \
  TEST_VERSUS_STATIC_ARRAY (len, data, example_packed_data); \
  assert(mess2->n_##lc_member_name == N_ELEMENTS (static_array)); \
  for (i = 0; i < N_ELEMENTS (static_array); i++) \
    assert(equals_macro(mess2->lc_member_name[i], static_array[i])); \
  free (data); \
  foo__test_mess_packed__free_unpacked (mess2, NULL); \
  }while(0)

static void test_packed_repeated_int32 (void)
{
#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_PACKED_REPEATED(test_int32, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST (int32_arr0, test_packed_repeated_int32_arr0);
  DO_TEST (int32_arr1, test_packed_repeated_int32_arr1);
  DO_TEST (int32_arr_min_max, test_packed_repeated_int32_arr_min_max);

#undef DO_TEST
}

static void test_packed_repeated_sint32 (void)
{
#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_PACKED_REPEATED(test_sint32, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST (int32_arr0, test_packed_repeated_sint32_arr0);
  DO_TEST (int32_arr1, test_packed_repeated_sint32_arr1);
  DO_TEST (int32_arr_min_max, test_packed_repeated_sint32_arr_min_max);

#undef DO_TEST
}

static void test_packed_repeated_sfixed32 (void)
{
#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_PACKED_REPEATED(test_sfixed32, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST (int32_arr0, test_packed_repeated_sfixed32_arr0);
  DO_TEST (int32_arr1, test_packed_repeated_sfixed32_arr1);
  DO_TEST (int32_arr_min_max, test_packed_repeated_sfixed32_arr_min_max);

#undef DO_TEST
}

static void test_packed_repeated_uint32 (void)
{
#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_PACKED_REPEATED(test_uint32, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST (uint32_roundnumbers, test_packed_repeated_uint32_roundnumbers);
  DO_TEST (uint32_0_max, test_packed_repeated_uint32_0_max);

#undef DO_TEST
}



static void test_packed_repeated_fixed32 (void)
{
#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_PACKED_REPEATED(test_fixed32, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST (uint32_roundnumbers, test_packed_repeated_fixed32_roundnumbers);
  DO_TEST (uint32_0_max, test_packed_repeated_fixed32_0_max);

#undef DO_TEST
}

static void test_packed_repeated_int64 (void)
{
#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_PACKED_REPEATED(test_int64, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST (int64_roundnumbers, test_packed_repeated_int64_roundnumbers);
  DO_TEST (int64_min_max, test_packed_repeated_int64_min_max);

#undef DO_TEST
}

static void test_packed_repeated_sint64 (void)
{
#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_PACKED_REPEATED(test_sint64, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST (int64_roundnumbers, test_packed_repeated_sint64_roundnumbers);
  DO_TEST (int64_min_max, test_packed_repeated_sint64_min_max);

#undef DO_TEST
}

static void test_packed_repeated_sfixed64 (void)
{
#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_PACKED_REPEATED(test_sfixed64, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST (int64_roundnumbers, test_packed_repeated_sfixed64_roundnumbers);
  DO_TEST (int64_min_max, test_packed_repeated_sfixed64_min_max);

#undef DO_TEST
}

static void test_packed_repeated_uint64 (void)
{
#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_PACKED_REPEATED(test_uint64, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST(uint64_roundnumbers, test_packed_repeated_uint64_roundnumbers);
  DO_TEST(uint64_0_1_max, test_packed_repeated_uint64_0_1_max);
  DO_TEST(uint64_random, test_packed_repeated_uint64_random);

#undef DO_TEST
}

static void test_packed_repeated_fixed64 (void)
{
#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_PACKED_REPEATED(test_fixed64, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST(uint64_roundnumbers, test_packed_repeated_fixed64_roundnumbers);
  DO_TEST(uint64_0_1_max, test_packed_repeated_fixed64_0_1_max);
  DO_TEST(uint64_random, test_packed_repeated_fixed64_random);

#undef DO_TEST
}

static void test_packed_repeated_float (void)
{

#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_PACKED_REPEATED(test_float, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST(float_random, test_packed_repeated_float_random);

#undef DO_TEST
}

static void test_packed_repeated_double (void)
{

#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_PACKED_REPEATED(test_double, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST(double_random, test_packed_repeated_double_random);

#undef DO_TEST
}

static void test_packed_repeated_boolean (void)
{

#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_PACKED_REPEATED(test_boolean, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST(boolean_0, test_packed_repeated_boolean_0);
  DO_TEST(boolean_1, test_packed_repeated_boolean_1);
  DO_TEST(boolean_random, test_packed_repeated_boolean_random);

#undef DO_TEST
}

static void test_packed_repeated_TestEnumSmall (void)
{

#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_PACKED_REPEATED(test_enum_small, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST(enum_small_0, test_packed_repeated_enum_small_0);
  DO_TEST(enum_small_1, test_packed_repeated_enum_small_1);
  DO_TEST(enum_small_random, test_packed_repeated_enum_small_random);

#undef DO_TEST
}

static void test_packed_repeated_TestEnum (void)
{

#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_PACKED_REPEATED(test_enum, , \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST(enum_0, test_packed_repeated_enum_0);
  DO_TEST(enum_1, test_packed_repeated_enum_1);
  DO_TEST(enum_random, test_packed_repeated_enum_random);

#undef DO_TEST
}


static void test_unknown_fields (void)
{
  static Foo__EmptyMess mess = FOO__EMPTY_MESS__INIT;
  static Foo__EmptyMess *mess2;
  ProtobufCMessageUnknownField fields[2];
  size_t len; uint8_t *data;

  mess.base.n_unknown_fields = 2;
  mess.base.unknown_fields = fields;

  fields[0].tag = 5454;
  fields[0].wire_type = PROTOBUF_C_WIRE_TYPE_VARINT;
  fields[0].len = 2;
  fields[0].data = (uint8_t*)"\377\1";
  fields[1].tag = 5555;
  fields[1].wire_type = PROTOBUF_C_WIRE_TYPE_32BIT;
  fields[1].len = 4;
  fields[1].data = (uint8_t*)"\4\1\0\0";
  mess2 = test_compare_pack_methods (&mess.base, &len, &data);
  assert (mess2->base.n_unknown_fields == 2);
  assert (mess2->base.unknown_fields[0].tag == 5454);
  assert (mess2->base.unknown_fields[0].wire_type == PROTOBUF_C_WIRE_TYPE_VARINT);
  assert (mess2->base.unknown_fields[0].len == 2);
  assert (memcmp (mess2->base.unknown_fields[0].data, fields[0].data, 2) == 0);
  assert (mess2->base.unknown_fields[1].tag == 5555);
  assert (mess2->base.unknown_fields[1].wire_type == PROTOBUF_C_WIRE_TYPE_32BIT);
  assert (mess2->base.unknown_fields[1].len == 4);
  assert (memcmp (mess2->base.unknown_fields[1].data, fields[1].data, 4) == 0);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_unknown_fields_0);
  free (data);
  foo__empty_mess__free_unpacked (mess2, NULL);

  fields[0].tag = 6666;
  fields[0].wire_type = PROTOBUF_C_WIRE_TYPE_LENGTH_PREFIXED;
  fields[0].len = 9;
  fields[0].data = (uint8_t*)"\10xxxxxxxx";
  fields[1].tag = 7777;
  fields[1].wire_type = PROTOBUF_C_WIRE_TYPE_64BIT;
  fields[1].len = 8;
  fields[1].data = (uint8_t*)"\1\1\1\0\0\0\0\0";
  mess2 = test_compare_pack_methods (&mess.base, &len, &data);
  assert (mess2->base.n_unknown_fields == 2);
  assert (mess2->base.unknown_fields[0].tag == 6666);
  assert (mess2->base.unknown_fields[0].wire_type == PROTOBUF_C_WIRE_TYPE_LENGTH_PREFIXED);
  assert (mess2->base.unknown_fields[0].len == 9);
  assert (memcmp (mess2->base.unknown_fields[0].data, fields[0].data, 9) == 0);
  assert (mess2->base.unknown_fields[1].tag == 7777);
  assert (mess2->base.unknown_fields[1].wire_type == PROTOBUF_C_WIRE_TYPE_64BIT);
  assert (mess2->base.unknown_fields[1].len == 8);
  assert (memcmp (mess2->base.unknown_fields[1].data, fields[1].data, 8) == 0);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_unknown_fields_1);
  free (data);
  foo__empty_mess__free_unpacked (mess2, NULL);
}

static void
test_enum_descriptor (const ProtobufCEnumDescriptor *desc)
{
  unsigned i;
  for (i = 0; i < desc->n_values; i++)
    {
      const ProtobufCEnumValue *sv = desc->values + i;
      const ProtobufCEnumValue *vv;
      const ProtobufCEnumValue *vn;
      vv = protobuf_c_enum_descriptor_get_value (desc, sv->value);
      vn = protobuf_c_enum_descriptor_get_value_by_name (desc, sv->name);
      assert (sv == vv);
      assert (sv == vn);
    }
  for (i = 0; i < desc->n_value_names; i++)
    {
      const char *name = desc->values_by_name[i].name;
      const ProtobufCEnumValue *v;
      v = protobuf_c_enum_descriptor_get_value_by_name (desc, name);
      assert (v != NULL);
    }
}
static void
test_enum_by_name (const ProtobufCEnumDescriptor *desc,
                   const char *name,
                   int expected_value)
{
  const ProtobufCEnumValue *v;
  v = protobuf_c_enum_descriptor_get_value_by_name (desc, name);
  assert (v != NULL);
  assert (v->value == expected_value);
}

static void
test_enum_lookups (void)
{
  test_enum_descriptor (&foo__test_enum__descriptor);
  test_enum_descriptor (&foo__test_enum_small__descriptor);
  test_enum_descriptor (&foo__test_enum_dup_values__descriptor);
#define TEST_ENUM_DUP_VALUES(str, shortname) \
  test_enum_by_name (&foo__test_enum_dup_values__descriptor,  \
                     str, FOO__TEST_ENUM_DUP_VALUES__##shortname)
  TEST_ENUM_DUP_VALUES ("VALUE_A", VALUE_A);
  TEST_ENUM_DUP_VALUES ("VALUE_B", VALUE_B);
  TEST_ENUM_DUP_VALUES ("VALUE_C", VALUE_C);
  TEST_ENUM_DUP_VALUES ("VALUE_D", VALUE_D);
  TEST_ENUM_DUP_VALUES ("VALUE_E", VALUE_E);
  TEST_ENUM_DUP_VALUES ("VALUE_F", VALUE_F);
  TEST_ENUM_DUP_VALUES ("VALUE_AA", VALUE_AA);
  TEST_ENUM_DUP_VALUES ("VALUE_BB", VALUE_BB);
#undef TEST_ENUM_DUP_VALUES
}

static void
test_message_descriptor (const ProtobufCMessageDescriptor *desc)
{
  unsigned i;
  for (i = 0; i < desc->n_fields; i++)
    {
      const ProtobufCFieldDescriptor *f = desc->fields + i;
      const ProtobufCFieldDescriptor *fv;
      const ProtobufCFieldDescriptor *fn;
      fv = protobuf_c_message_descriptor_get_field (desc, f->id);
      fn = protobuf_c_message_descriptor_get_field_by_name (desc, f->name);
      assert (f == fv);
      assert (f == fn);
    }
}
static void
test_message_lookups (void)
{
  test_message_descriptor (&foo__test_mess__descriptor);
  test_message_descriptor (&foo__test_mess_optional__descriptor);
  test_message_descriptor (&foo__test_mess_required_enum__descriptor);
}

static void
assert_required_default_values_are_default (Foo__DefaultRequiredValues *mess)
{
  assert (mess->v_int32 == -42);
  assert (mess->v_uint32 == 666);
  assert (mess->v_int64 == 100000);
  assert (mess->v_uint64 == 100001);
  assert (mess->v_float == 2.5);
  assert (mess->v_double == 4.5);
  assert (strcmp (mess->v_string, "hi mom\n") == 0);
  assert (mess->v_bytes.len = /* a */ 1
                               + /* space */ 1
                               + /* NUL */ 1
                               + /* space */ 1
                               + /* "character" */ 9);
  assert (memcmp (mess->v_bytes.data, "a \0 character", 13) == 0);
}


static void
test_required_default_values (void)
{
  Foo__DefaultRequiredValues mess = FOO__DEFAULT_REQUIRED_VALUES__INIT;
  Foo__DefaultRequiredValues *mess2;
  size_t len; uint8_t *data;
  assert_required_default_values_are_default (&mess);
  mess2 = test_compare_pack_methods (&mess.base, &len, &data);
  free (data);
  assert_required_default_values_are_default (mess2);
  foo__default_required_values__free_unpacked (mess2, NULL);
}

static void
assert_optional_default_values_are_default (Foo__DefaultOptionalValues *mess)
{
  assert (!mess->has_v_int32);
  assert (mess->v_int32 == -42);
  assert (!mess->has_v_uint32);
  assert (mess->v_uint32 == 666);
  assert (!mess->has_v_int64);
  assert (mess->v_int64 == 100000);
  assert (!mess->has_v_uint64);
  assert (mess->v_uint64 == 100001);
  assert (!mess->has_v_float);
  assert (mess->v_float == 2.5);
  assert (!mess->has_v_double);
  assert (mess->v_double == 4.5);
  assert (strcmp (mess->v_string, "hi mom\n") == 0);
  assert (!mess->has_v_bytes);
  assert (mess->v_bytes.len = /* a */ 1
                               + /* space */ 1
                               + /* NUL */ 1
                               + /* space */ 1
                               + /* "character" */ 9);
  assert (memcmp (mess->v_bytes.data, "a \0 character", 13) == 0);
}

static void
test_optional_default_values (void)
{
  Foo__DefaultOptionalValues mess = FOO__DEFAULT_OPTIONAL_VALUES__INIT;
  Foo__DefaultOptionalValues *mess2;
  size_t len; uint8_t *data;
  assert_optional_default_values_are_default (&mess);
  mess2 = test_compare_pack_methods (&mess.base, &len, &data);
  assert (len == 0);            /* no non-default values */
  free (data);
  assert_optional_default_values_are_default (mess2);
  foo__default_optional_values__free_unpacked (mess2, NULL);
}

static struct alloc_data {
  uint32_t alloc_count;
  int32_t allocs_left;
} test_allocator_data;

static void *test_alloc(void *allocator_data, size_t size)
{
  struct alloc_data *ad = allocator_data;
  void *rv = NULL;
  if (ad->allocs_left-- > 0)
      rv = malloc (size);
  /* fprintf (stderr, "alloc %d = %p\n", size, rv); */
  if (rv)
    ad->alloc_count++;
  return rv;
}

static void test_free (void *allocator_data, void *data)
{
  struct alloc_data *ad = allocator_data;
  /* fprintf (stderr, "free %p\n", data); */
  free (data);
  if (data)
    ad->alloc_count--;
}

static ProtobufCAllocator test_allocator = {
  test_alloc, test_free, 0, 0, &test_allocator_data
};

#define SETUP_TEST_ALLOC_BUFFER(pbuf, len)					\
  uint8_t bytes[] = "some bytes", *pbuf;						\
  size_t len, _len2;                                        \
  Foo__DefaultRequiredValues _req = FOO__DEFAULT_REQUIRED_VALUES__INIT;		\
  Foo__AllocValues _mess = FOO__ALLOC_VALUES__INIT;				\
  _mess.a_string = "some string";						\
  _mess.r_string = repeated_strings_2;						\
  _mess.n_r_string = sizeof(repeated_strings_2) / sizeof(*repeated_strings_2);	\
  _mess.a_bytes.len = sizeof(bytes);						\
  _mess.a_bytes.data = bytes;							\
  _mess.a_mess = &_req;								\
  len = foo__alloc_values__get_packed_size (&_mess);			\
  pbuf = malloc (len);							\
  assert (pbuf);								\
  _len2 = foo__alloc_values__pack (&_mess, pbuf);			\
  assert (len == _len2);

static void
test_alloc_graceful_cleanup (uint8_t *packed, size_t len, int good_allocs)
{
  Foo__AllocValues *mess;
  test_allocator_data.alloc_count = 0;
  test_allocator_data.allocs_left = good_allocs;
  mess = foo__alloc_values__unpack (&test_allocator, len, packed);
  assert (test_allocator_data.allocs_left < 0 ? !mess : !!mess);
  if (mess)
    foo__alloc_values__free_unpacked (mess, &test_allocator);
  assert (0 == test_allocator_data.alloc_count);
}

static void
test_alloc_free_all (void)
{
  SETUP_TEST_ALLOC_BUFFER (packed, len);
  test_alloc_graceful_cleanup (packed, len, INT32_MAX);
  free (packed);
}

/* TODO: test alloc failure for slab, unknown fields */
static void
test_alloc_fail (void)
{
  int i = 0;
  SETUP_TEST_ALLOC_BUFFER (packed, len);
  do test_alloc_graceful_cleanup (packed, len, i++);
  while (test_allocator_data.allocs_left < 0);
  free (packed);
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
  { "test field numbers", test_field_numbers },

  { "test required int32", test_required_int32 },
  { "test required sint32", test_required_sint32 },
  { "test required sfixed32", test_required_sfixed32 },
  { "test required int64", test_required_int64 },
  { "test required sint64", test_required_sint64 },
  { "test required sfixed64", test_required_sfixed64 },
  { "test required uint32", test_required_uint32 },
  { "test required fixed32", test_required_fixed32 },
  { "test required uint64", test_required_uint64 },
  { "test required fixed64", test_required_fixed64 },
  { "test required float", test_required_float },
  { "test required double", test_required_double },
  { "test required bool", test_required_bool },
  { "test required TestEnumSmall", test_required_TestEnumSmall },
  { "test required TestEnum", test_required_TestEnum },
  { "test required string", test_required_string },
  { "test required bytes", test_required_bytes },
  { "test required SubMess", test_required_SubMess },

  { "test empty optional" ,test_empty_optional },
  { "test optional int32", test_optional_int32 },
  { "test optional sint32", test_optional_sint32 },
  { "test optional sfixed32", test_optional_sfixed32 },
  { "test optional int64", test_optional_int64 },
  { "test optional sint64", test_optional_sint64 },
  { "test optional sfixed64", test_optional_sfixed64 },
  { "test optional uint32", test_optional_uint32 },
  { "test optional fixed32", test_optional_fixed32 },
  { "test optional uint64", test_optional_uint64 },
  { "test optional fixed64", test_optional_fixed64 },
  { "test optional float", test_optional_float },
  { "test optional double", test_optional_double },
  { "test optional bool", test_optional_bool },
  { "test optional TestEnumSmall", test_optional_TestEnumSmall },
  { "test optional TestEnum", test_optional_TestEnum },
  { "test optional string", test_optional_string },
  { "test optional bytes", test_optional_bytes },
  { "test optional SubMess", test_optional_SubMess },

  { "test empty repeated" ,test_empty_repeated },
  { "test repeated int32" ,test_repeated_int32 },
  { "test repeated sint32" ,test_repeated_sint32 },
  { "test repeated sfixed32" ,test_repeated_sfixed32 },
  { "test repeated uint32", test_repeated_uint32 },
  { "test repeated int64", test_repeated_int64 },
  { "test repeated sint64", test_repeated_sint64 },
  { "test repeated sfixed64", test_repeated_sfixed64 },
  { "test repeated fixed32", test_repeated_fixed32 },
  { "test repeated uint64", test_repeated_uint64 },
  { "test repeated fixed64", test_repeated_fixed64 },
  { "test repeated float", test_repeated_float },
  { "test repeated double", test_repeated_double },
  { "test repeated boolean", test_repeated_boolean },
  { "test repeated TestEnumSmall", test_repeated_TestEnumSmall },
  { "test repeated TestEnum", test_repeated_TestEnum },
  { "test repeated string", test_repeated_string },
  { "test repeated bytes", test_repeated_bytes },
  { "test repeated SubMess", test_repeated_SubMess },

  { "test packed repeated int32", test_packed_repeated_int32 },
  { "test packed repeated sint32", test_packed_repeated_sint32 },
  { "test packed repeated sfixed32" ,test_packed_repeated_sfixed32 },
  { "test packed repeated uint32", test_packed_repeated_uint32 },
  { "test packed repeated int64", test_packed_repeated_int64 },
  { "test packed repeated sint64", test_packed_repeated_sint64 },
  { "test packed repeated sfixed64", test_packed_repeated_sfixed64 },
  { "test packed repeated fixed32", test_packed_repeated_fixed32 },
  { "test packed repeated uint64", test_packed_repeated_uint64 },
  { "test packed repeated fixed64", test_packed_repeated_fixed64 },
  { "test packed repeated float", test_packed_repeated_float },
  { "test packed repeated double", test_packed_repeated_double },
  { "test packed repeated boolean", test_packed_repeated_boolean },
  { "test packed repeated TestEnumSmall", test_packed_repeated_TestEnumSmall },
  { "test packed repeated TestEnum", test_packed_repeated_TestEnum },

  { "test unknown fields", test_unknown_fields },

  { "test enum lookups", test_enum_lookups },
  { "test message lookups", test_message_lookups },

  { "test required default values", test_required_default_values },
  { "test optional default values", test_optional_default_values },

  { "test free unpacked", test_alloc_free_all },
  { "test alloc failure", test_alloc_fail },
};
#define n_tests (sizeof(tests)/sizeof(Test))

int main ()
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
