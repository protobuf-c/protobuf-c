#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "generated-code/test-full.pb-c.h"
#include "generated-code/test-full-cxx-output.inc"

#define TEST_ENUM_SMALL_TYPE_NAME   Foo__TestEnumSmall
#define TEST_ENUM_SMALL(shortname)   FOO__TEST_ENUM_SMALL__##shortname
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
#define DO_TEST_REPEATED(lc_member_name, static_array, example_packed_data, \
                         equals_macro) \
  do{ \
  Foo__TestMess mess = FOO__TEST_MESS__INIT; \
  Foo__TestMess *mess2; \
  size_t len; \
  uint8_t *data; \
  unsigned i; \
  mess.n_##lc_member_name = N_ELEMENTS (static_array); \
  mess.lc_member_name = static_array; \
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data); \
  assert(mess2->n_##lc_member_name == N_ELEMENTS (static_array)); \
  for (i = 0; i < N_ELEMENTS (static_array); i++) \
    assert(equals_macro(mess2->lc_member_name[i], static_array[i])); \
  TEST_VERSUS_STATIC_ARRAY (len, data, example_packed_data); \
  free (data); \
  foo__test_mess__free_unpacked (mess2, NULL); \
  }while(0)

/* === the actual tests === */

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
static void test_repeated_int32 (void)
{
  Foo__TestMess mess = FOO__TEST_MESS__INIT;
  Foo__TestMess *mess2;
  int32_t arr0[2] = { -1, 1 };
  int32_t arr1[5] = { 42, 666, -1123123, 0, 47 };
  int32_t arr_min[1] = { INT32_MIN };
  int32_t arr_max[1] = { INT32_MAX };
  size_t len;
  uint8_t *data;

  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_int32 == 0);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_int32__empty);
  free (data);
  foo__test_mess__free_unpacked (mess2, NULL);

  mess.n_test_int32 = 1;
  mess.test_int32 = arr0;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_int32 == 1);
  assert(mess2->test_int32[0] == -1);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_int32_m1);
  free (data);
  foo__test_mess__free_unpacked (mess2, NULL);

  mess.n_test_int32 = 1;
  mess.test_int32 = arr0+1;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_int32 == 1);
  assert(mess2->test_int32[0] == 1);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_int32_1);
  free (data);
  foo__test_mess__free_unpacked (mess2, NULL);

  mess.n_test_int32 = 2;
  mess.test_int32 = arr0;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_int32 == 2);
  assert(mess2->test_int32[0] == -1);
  assert(mess2->test_int32[1] == 1);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_int32_m1_1);
  free (data);
  foo__test_mess__free_unpacked (mess2, NULL);

  mess.n_test_int32 = 5;
  mess.test_int32 = arr1;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_int32 == 5);
  assert(memcmp (mess2->test_int32, arr1, 4*5)==0);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_int32_42_666_m1123123_0_47);
  free (data);
  foo__test_mess__free_unpacked (mess2, NULL);

  mess.n_test_int32 = 1;
  mess.test_int32 = arr_min;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_int32 == 1);
  assert(mess2->test_int32[0] == INT32_MIN);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_int32_min);
  free (data);

  mess.n_test_int32 = 1;
  mess.test_int32 = arr_max;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_int32 == 1);
  assert(mess2->test_int32[0] == INT32_MAX);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_int32_max);
  free (data);
}

static void test_repeated_sint32 (void)
{
  Foo__TestMess mess = FOO__TEST_MESS__INIT;
  Foo__TestMess *mess2;
  int32_t arr0[2] = { -1, 1 };
  int32_t arr1[5] = { 42, 666, -1123123, 0, 47 };
  int32_t arr_min[1] = { INT32_MIN };
  int32_t arr_max[1] = { INT32_MAX };
  size_t len;
  uint8_t *data;

  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_sint32 == 0);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_sint32__empty);
  free (data);
  foo__test_mess__free_unpacked (mess2, NULL);

  mess.n_test_sint32 = 1;
  mess.test_sint32 = arr0;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_sint32 == 1);
  assert(mess2->test_sint32[0] == -1);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_sint32_m1);
  free (data);
  foo__test_mess__free_unpacked (mess2, NULL);

  mess.n_test_sint32 = 1;
  mess.test_sint32 = arr0+1;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_sint32 == 1);
  assert(mess2->test_sint32[0] == 1);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_sint32_1);
  free (data);
  foo__test_mess__free_unpacked (mess2, NULL);

  mess.n_test_sint32 = 2;
  mess.test_sint32 = arr0;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_sint32 == 2);
  assert(mess2->test_sint32[0] == -1);
  assert(mess2->test_sint32[1] == 1);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_sint32_m1_1);
  free (data);
  foo__test_mess__free_unpacked (mess2, NULL);

  mess.n_test_sint32 = 5;
  mess.test_sint32 = arr1;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_sint32 == 5);
  assert(memcmp (mess2->test_sint32, arr1, 4*5)==0);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_sint32_42_666_m1123123_0_47);
  free (data);
  foo__test_mess__free_unpacked (mess2, NULL);

  mess.n_test_sint32 = 1;
  mess.test_sint32 = arr_min;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_sint32 == 1);
  assert(mess2->test_sint32[0] == INT32_MIN);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_sint32_min);
  free (data);

  mess.n_test_sint32 = 1;
  mess.test_sint32 = arr_max;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_sint32 == 1);
  assert(mess2->test_sint32[0] == INT32_MAX);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_sint32_max);
  free (data);
}

static void test_repeated_sfixed32 (void)
{
  Foo__TestMess mess = FOO__TEST_MESS__INIT;
  Foo__TestMess *mess2;
  int32_t arr0[2] = { -1, 1 };
  int32_t arr1[5] = { 42, 666, -1123123, 0, 47 };
  int32_t arr_min[1] = { INT32_MIN };
  int32_t arr_max[1] = { INT32_MAX };
  size_t len;
  uint8_t *data;

  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_sfixed32 == 0);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_sfixed32__empty);
  free (data);
  foo__test_mess__free_unpacked (mess2, NULL);

  mess.n_test_sfixed32 = 1;
  mess.test_sfixed32 = arr0;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_sfixed32 == 1);
  assert(mess2->test_sfixed32[0] == -1);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_sfixed32_m1);
  free (data);
  foo__test_mess__free_unpacked (mess2, NULL);

  mess.n_test_sfixed32 = 1;
  mess.test_sfixed32 = arr0+1;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_sfixed32 == 1);
  assert(mess2->test_sfixed32[0] == 1);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_sfixed32_1);
  free (data);
  foo__test_mess__free_unpacked (mess2, NULL);

  mess.n_test_sfixed32 = 2;
  mess.test_sfixed32 = arr0;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_sfixed32 == 2);
  assert(mess2->test_sfixed32[0] == -1);
  assert(mess2->test_sfixed32[1] == 1);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_sfixed32_m1_1);
  free (data);
  foo__test_mess__free_unpacked (mess2, NULL);

  mess.n_test_sfixed32 = 5;
  mess.test_sfixed32 = arr1;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_sfixed32 == 5);
  assert(memcmp (mess2->test_sfixed32, arr1, 4*5)==0);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_sfixed32_42_666_m1123123_0_47);
  free (data);
  foo__test_mess__free_unpacked (mess2, NULL);

  mess.n_test_sfixed32 = 1;
  mess.test_sfixed32 = arr_min;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_sfixed32 == 1);
  assert(mess2->test_sfixed32[0] == INT32_MIN);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_sfixed32_min);
  free (data);

  mess.n_test_sfixed32 = 1;
  mess.test_sfixed32 = arr_max;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_sfixed32 == 1);
  assert(mess2->test_sfixed32[0] == INT32_MAX);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_sfixed32_max);
  free (data);
}

static void test_repeated_uint32 (void)
{
  Foo__TestMess mess = FOO__TEST_MESS__INIT;
  Foo__TestMess *mess2;
  uint32_t arr0[4] = { BILLION, MILLION, 1, 0 };
  uint32_t arr_zero[1] = { 0 };
  uint32_t arr_max[1] = { UINT32_MAX };
  size_t len;
  uint8_t *data;

  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_uint32 == 0);
  assert(len == 0);
  free (data);
  foo__test_mess__free_unpacked (mess2, NULL);

  mess.n_test_uint32 = 4;
  mess.test_uint32 = arr0;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_uint32 == 4);
  assert(mess2->test_uint32[0] == BILLION);
  assert(mess2->test_uint32[1] == MILLION);
  assert(mess2->test_uint32[2] == 1);
  assert(mess2->test_uint32[3] == 0);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_uint32_bil_mil_1_0);
  free (data);
  foo__test_mess__free_unpacked (mess2, NULL);

  mess.n_test_uint32 = 1;
  mess.test_uint32 = arr_zero;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_uint32 == 1);
  assert(mess2->test_uint32[0] == 0);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_uint32_0);
  free (data);

  mess.n_test_uint32 = 1;
  mess.test_uint32 = arr_max;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_uint32 == 1);
  assert(mess2->test_uint32[0] == UINT32_MAX);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_uint32_max);
  free (data);
}

static void test_repeated_fixed32 (void)
{
  Foo__TestMess mess = FOO__TEST_MESS__INIT;
  Foo__TestMess *mess2;
  uint32_t arr0[4] = { BILLION, MILLION, 1, 0 };
  uint32_t arr_zero[1] = { 0 };
  uint32_t arr_max[1] = { UINT32_MAX };
  size_t len;
  uint8_t *data;

  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_fixed32 == 0);
  assert(len == 0);
  free (data);
  foo__test_mess__free_unpacked (mess2, NULL);

  mess.n_test_fixed32 = 4;
  mess.test_fixed32 = arr0;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_fixed32 == 4);
  assert(mess2->test_fixed32[0] == BILLION);
  assert(mess2->test_fixed32[1] == MILLION);
  assert(mess2->test_fixed32[2] == 1);
  assert(mess2->test_fixed32[3] == 0);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_fixed32_bil_mil_1_0);
  free (data);
  foo__test_mess__free_unpacked (mess2, NULL);

  mess.n_test_fixed32 = 1;
  mess.test_fixed32 = arr_zero;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_fixed32 == 1);
  assert(mess2->test_fixed32[0] == 0);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_fixed32_0);
  free (data);

  mess.n_test_fixed32 = 1;
  mess.test_fixed32 = arr_max;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_fixed32 == 1);
  assert(mess2->test_fixed32[0] == UINT32_MAX);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_fixed32_max);
  free (data);
}

static void test_repeated_int64 (void)
{
  Foo__TestMess mess = FOO__TEST_MESS__INIT;
  Foo__TestMess *mess2;
  size_t len;
  uint8_t *data;
  unsigned i;

  mess.n_test_int64 = N_ELEMENTS (int64_roundnumbers);
  mess.test_int64 = int64_roundnumbers;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_int64 == N_ELEMENTS (int64_roundnumbers));
  for (i = 0; i < N_ELEMENTS (int64_roundnumbers); i++)
    assert(mess2->test_int64[i] == int64_roundnumbers[i]);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_int64_roundnumbers);
  free (data);
  foo__test_mess__free_unpacked (mess2, NULL);

  mess.n_test_int64 = N_ELEMENTS (int64_min_max);
  mess.test_int64 = int64_min_max;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_int64 == N_ELEMENTS (int64_min_max));
  assert(mess2->test_int64[0] == INT64_MIN);
  assert(mess2->test_int64[1] == INT64_MAX);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_int64_min_max);
  free (data);
  foo__test_mess__free_unpacked (mess2, NULL);
}

static void test_repeated_sint64 (void)
{
  Foo__TestMess mess = FOO__TEST_MESS__INIT;
  Foo__TestMess *mess2;
  size_t len;
  uint8_t *data;
  unsigned i;

  mess.n_test_sint64 = N_ELEMENTS (int64_roundnumbers);
  mess.test_sint64 = int64_roundnumbers;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_sint64 == N_ELEMENTS (int64_roundnumbers));
  for (i = 0; i < N_ELEMENTS (int64_roundnumbers); i++)
    assert(mess2->test_sint64[i] == int64_roundnumbers[i]);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_sint64_roundnumbers);
  free (data);
  foo__test_mess__free_unpacked (mess2, NULL);

  mess.n_test_sint64 = N_ELEMENTS (int64_min_max);
  mess.test_sint64 = int64_min_max;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_sint64 == N_ELEMENTS (int64_min_max));
  assert(mess2->test_sint64[0] == INT64_MIN);
  assert(mess2->test_sint64[1] == INT64_MAX);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_sint64_min_max);
  free (data);
  foo__test_mess__free_unpacked (mess2, NULL);
}

static void test_repeated_sfixed64 (void)
{
  Foo__TestMess mess = FOO__TEST_MESS__INIT;
  Foo__TestMess *mess2;
  size_t len;
  uint8_t *data;
  unsigned i;

  mess.n_test_sfixed64 = N_ELEMENTS (int64_roundnumbers);
  mess.test_sfixed64 = int64_roundnumbers;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_sfixed64 == N_ELEMENTS (int64_roundnumbers));
  for (i = 0; i < N_ELEMENTS (int64_roundnumbers); i++)
    assert(mess2->test_sfixed64[i] == int64_roundnumbers[i]);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_sfixed64_roundnumbers);
  free (data);
  foo__test_mess__free_unpacked (mess2, NULL);

  mess.n_test_sfixed64 = N_ELEMENTS (int64_min_max);
  mess.test_sfixed64 = int64_min_max;
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data);
  assert(mess2->n_test_sfixed64 == N_ELEMENTS (int64_min_max));
  assert(mess2->test_sfixed64[0] == INT64_MIN);
  assert(mess2->test_sfixed64[1] == INT64_MAX);
  TEST_VERSUS_STATIC_ARRAY (len, data, test_repeated_sfixed64_min_max);
  free (data);
  foo__test_mess__free_unpacked (mess2, NULL);
}

static void test_repeated_uint64 (void)
{
  Foo__TestMess mess = FOO__TEST_MESS__INIT;
  Foo__TestMess *mess2;
  size_t len;
  uint8_t *data;
  unsigned i;

#define DO_TEST(static_array, example_packed_data) \
  do{ \
  mess.n_test_uint64 = N_ELEMENTS (static_array); \
  mess.test_uint64 = static_array; \
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data); \
  assert(mess2->n_test_uint64 == N_ELEMENTS (static_array)); \
  for (i = 0; i < N_ELEMENTS (static_array); i++) \
    assert(mess2->test_uint64[i] == static_array[i]); \
  TEST_VERSUS_STATIC_ARRAY (len, data, example_packed_data); \
  free (data); \
  foo__test_mess__free_unpacked (mess2, NULL); \
  }while(0)

  DO_TEST(uint64_roundnumbers, test_repeated_uint64_roundnumbers);
  DO_TEST(uint64_0_1_max, test_repeated_uint64_0_1_max);
  DO_TEST(uint64_random, test_repeated_uint64_random);

#undef DO_TEST
}

static void test_repeated_fixed64 (void)
{
  Foo__TestMess mess = FOO__TEST_MESS__INIT;
  Foo__TestMess *mess2;
  size_t len;
  uint8_t *data;
  unsigned i;

#define DO_TEST(static_array, example_packed_data) \
  do{ \
  mess.n_test_fixed64 = N_ELEMENTS (static_array); \
  mess.test_fixed64 = static_array; \
  mess2 = test_compare_pack_methods ((ProtobufCMessage*)(&mess), &len, &data); \
  assert(mess2->n_test_fixed64 == N_ELEMENTS (static_array)); \
  for (i = 0; i < N_ELEMENTS (static_array); i++) \
    assert(mess2->test_fixed64[i] == static_array[i]); \
  TEST_VERSUS_STATIC_ARRAY (len, data, example_packed_data); \
  free (data); \
  foo__test_mess__free_unpacked (mess2, NULL); \
  }while(0)

  DO_TEST(uint64_roundnumbers, test_repeated_fixed64_roundnumbers);
  DO_TEST(uint64_0_1_max, test_repeated_fixed64_0_1_max);
  DO_TEST(uint64_random, test_repeated_fixed64_random);

#undef DO_TEST
}

static void test_repeated_float (void)
{

#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_REPEATED(test_float, \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST(float_random, test_repeated_float_random);

#undef DO_TEST
}

static void test_repeated_double (void)
{

#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_REPEATED(test_double, \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST(double_random, test_repeated_double_random);

#undef DO_TEST
}

static void test_repeated_boolean (void)
{

#define DO_TEST(static_array, example_packed_data) \
  DO_TEST_REPEATED(test_boolean, \
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
  DO_TEST_REPEATED(test_enum_small, \
                   static_array, example_packed_data, \
                   NUMERIC_EQUALS)

  DO_TEST(enum_small_0, test_repeated_enum_small_0);
  DO_TEST(enum_small_1, test_repeated_enum_small_1);
  DO_TEST(enum_small_random, test_repeated_enum_small_random);

#undef DO_TEST
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
  //{ "test repeated TestEnum", test_repeated_TestEnum },
  //{ "test repeated string", test_repeated_string },
  //{ "test repeated bytes", test_repeated_bytes },
  //{ "test repeated SubMess", test_repeated_SubMess },
  //{ "test optional int32", test_optional_int32 },
  //{ "test optional sint32", test_optional_sint32 },
  //{ "test optional sfixed32", test_optional_sfixed32 },
  //{ "test optional int64", test_optional_int64 },
  //{ "test optional sint64", test_optional_sint64 },
  //{ "test optional sfixed64", test_optional_sfixed64 },
  //{ "test optional uint32", test_optional_uint32 },
  //{ "test optional fixed32", test_optional_fixed32 },
  //{ "test optional uint64", test_optional_uint64 },
  //{ "test optional fixed64", test_optional_fixed64 },
  //{ "test optional float", test_optional_float },
  //{ "test optional double", test_optional_double },
  //{ "test optional bool", test_optional_bool },
  //{ "test optional TestEnumSmall", test_optional_TestEnumSmall },
  //{ "test optional TestEnum", test_optional_TestEnum },
  //{ "test optional string", test_optional_string },
  //{ "test optional bytes", test_optional_bytes },
  //{ "test optional SubMess", test_optional_SubMess },
  //{ "test required int32", test_required_int32 },
  //{ "test required sint32", test_required_sint32 },
  //{ "test required sfixed32", test_required_sfixed32 },
  //{ "test required int64", test_required_int64 },
  //{ "test required sint64", test_required_sint64 },
  //{ "test required sfixed64", test_required_sfixed64 },
  //{ "test required uint32", test_required_uint32 },
  //{ "test required fixed32", test_required_fixed32 },
  //{ "test required uint64", test_required_uint64 },
  //{ "test required fixed64", test_required_fixed64 },
  //{ "test required float", test_required_float },
  //{ "test required double", test_required_double },
  //{ "test required bool", test_required_bool },
  //{ "test required TestEnumSmall", test_required_TestEnumSmall },
  //{ "test required TestEnum", test_required_TestEnum },
  //{ "test required string", test_required_string },
  //{ "test required bytes", test_required_bytes },
  //{ "test required SubMess", test_required_SubMess },
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
