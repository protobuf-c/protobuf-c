/* generate byte arrays that match the constructs in test-generated-code2.c.
 * these are tested against eachother to make sure the c and c++ agree. */

#define __STDC_LIMIT_MACROS
#include "generated-code/test-full.pb.h"
#include <inttypes.h>

using namespace foo;

#define protobuf_c_boolean bool
#define TEST_ENUM_SMALL_TYPE_NAME  TestEnumSmall
#define TEST_ENUM_SMALL(NAME)      foo::NAME
#define TEST_ENUM_TYPE_NAME        TestEnum
#define TEST_ENUM(NAME)            foo::NAME
#include "common-test-arrays.h"
#define N_ELEMENTS(arr)   (sizeof(arr)/sizeof((arr)[0]))

static void
dump_message_bytes(google::protobuf::Message *message,
                   const char *label)
{
  std::string rv;
  unsigned char *bytes;
  unsigned len;
  if (!message->SerializeToString(&rv))
    assert(0);
  bytes = (unsigned char *) rv.data();
  len = rv.size();
  printf ("static const uint8_t %s[%u] = { ", label, len);
  for (unsigned i = 0; i < len; i++) {
    if (i)
      printf (", ");
    printf ("0x%02x", bytes[i]);
  }
  printf (" };\n");
}

static void
dump_test_enum_small (void)
{
  TestMessRequiredEnumSmall es;
  es.set_test(VALUE);
  dump_message_bytes(&es, "test_enum_small_VALUE");
  es.set_test(OTHER_VALUE);
  dump_message_bytes(&es, "test_enum_small_OTHER_VALUE");
}
static void
dump_test_enum_big (void)
{
  TestMessRequiredEnum eb;
  eb.set_test(VALUE0); dump_message_bytes(&eb, "test_enum_big_VALUE0");
  eb.set_test(VALUE127); dump_message_bytes(&eb, "test_enum_big_VALUE127");
  eb.set_test(VALUE128); dump_message_bytes(&eb, "test_enum_big_VALUE128");
  eb.set_test(VALUE16383); dump_message_bytes(&eb, "test_enum_big_VALUE16383");
  eb.set_test(VALUE16384); dump_message_bytes(&eb, "test_enum_big_VALUE16384");
  eb.set_test(VALUE2097151); dump_message_bytes(&eb, "test_enum_big_VALUE2097151");
  eb.set_test(VALUE2097152); dump_message_bytes(&eb, "test_enum_big_VALUE2097152");
  eb.set_test(VALUE268435455); dump_message_bytes(&eb, "test_enum_big_VALUE268435455");
  eb.set_test(VALUE268435456); dump_message_bytes(&eb, "test_enum_big_VALUE268435456");
}

static void
dump_test_field_numbers (void)
{
#define DUMP_ONE(num) \
  { TestFieldNo##num f; \
    f.set_test("tst"); \
    dump_message_bytes(&f, "test_field_number_" #num); }
  DUMP_ONE (15)
  DUMP_ONE (16)
  DUMP_ONE (2047)
  DUMP_ONE (2048)
  DUMP_ONE (262143)
  DUMP_ONE (262144)
  DUMP_ONE (33554431)
  DUMP_ONE (33554432)
#undef DUMP_ONE
} 

#define DUMP_STATIC_ARRAY_GENERIC(member, static_array, output_array_name) \
  do{ \
    TestMess mess; \
    for (unsigned i = 0; i < N_ELEMENTS (static_array); i++) \
      mess.add_##member(static_array[i]); \
    dump_message_bytes(&mess, output_array_name); \
  }while(0)
static void dump_test_repeated_int32 (void)
{
#define DUMP_STATIC_ARRAY(static_array, output_array_name) \
  DUMP_STATIC_ARRAY_GENERIC(test_int32, static_array, output_array_name)

  DUMP_STATIC_ARRAY (int32_arr0, "test_repeated_int32_arr0");
  DUMP_STATIC_ARRAY (int32_arr1, "test_repeated_int32_arr1");
  DUMP_STATIC_ARRAY (int32_arr_min_max, "test_repeated_int32_arr_min_max");

#undef DUMP_STATIC_ARRAY
}
static void dump_test_repeated_sint32 (void)
{
#define DUMP_STATIC_ARRAY(static_array, output_array_name) \
  DUMP_STATIC_ARRAY_GENERIC(test_sint32, static_array, output_array_name)

  DUMP_STATIC_ARRAY (int32_arr0, "test_repeated_sint32_arr0");
  DUMP_STATIC_ARRAY (int32_arr1, "test_repeated_sint32_arr1");
  DUMP_STATIC_ARRAY (int32_arr_min_max, "test_repeated_sint32_arr_min_max");

#undef DUMP_STATIC_ARRAY
}
static void dump_test_repeated_uint32 (void)
{
#define DUMP_STATIC_ARRAY(static_array, output_array_name) \
  DUMP_STATIC_ARRAY_GENERIC(test_uint32, static_array, output_array_name)

  DUMP_STATIC_ARRAY (uint32_roundnumbers, "test_repeated_uint32_roundnumbers");
  DUMP_STATIC_ARRAY (uint32_0_max, "test_repeated_uint32_0_max");

#undef DUMP_STATIC_ARRAY
}
static void dump_test_repeated_sfixed32 (void)
{
#define DUMP_STATIC_ARRAY(static_array, output_array_name) \
  DUMP_STATIC_ARRAY_GENERIC(test_sfixed32, static_array, output_array_name)

  DUMP_STATIC_ARRAY (int32_arr0, "test_repeated_sfixed32_arr0");
  DUMP_STATIC_ARRAY (int32_arr1, "test_repeated_sfixed32_arr1");
  DUMP_STATIC_ARRAY (int32_arr_min_max, "test_repeated_sfixed32_arr_min_max");

#undef DUMP_STATIC_ARRAY
}

static void dump_test_repeated_fixed32 (void)
{
#define DUMP_STATIC_ARRAY(static_array, output_array_name) \
  DUMP_STATIC_ARRAY_GENERIC(test_fixed32, static_array, output_array_name)

  DUMP_STATIC_ARRAY (uint32_roundnumbers, "test_repeated_fixed32_roundnumbers");
  DUMP_STATIC_ARRAY (uint32_0_max, "test_repeated_fixed32_0_max");

#undef DUMP_STATIC_ARRAY
}
static void dump_test_repeated_int64 (void)
{
#define DUMP_STATIC_ARRAY(static_array, output_array_name) \
  DUMP_STATIC_ARRAY_GENERIC(test_int64, static_array, output_array_name)

  DUMP_STATIC_ARRAY (int64_roundnumbers, "test_repeated_int64_roundnumbers");
  DUMP_STATIC_ARRAY (int64_min_max, "test_repeated_int64_min_max");

#undef DUMP_STATIC_ARRAY
}

static void dump_test_repeated_sint64 (void)
{
#define DUMP_STATIC_ARRAY(static_array, output_array_name) \
  DUMP_STATIC_ARRAY_GENERIC(test_sint64, static_array, output_array_name)

  DUMP_STATIC_ARRAY (int64_roundnumbers, "test_repeated_sint64_roundnumbers");
  DUMP_STATIC_ARRAY (int64_min_max, "test_repeated_sint64_min_max");

#undef DUMP_STATIC_ARRAY
}

static void dump_test_repeated_sfixed64 (void)
{
#define DUMP_STATIC_ARRAY(static_array, output_array_name) \
  DUMP_STATIC_ARRAY_GENERIC(test_sfixed64, static_array, output_array_name)

  DUMP_STATIC_ARRAY (int64_roundnumbers, "test_repeated_sfixed64_roundnumbers");
  DUMP_STATIC_ARRAY (int64_min_max, "test_repeated_sfixed64_min_max");

#undef DUMP_STATIC_ARRAY
}
static void dump_test_repeated_uint64 (void)
{
#define DUMP_STATIC_ARRAY(static_array, output_array_name) \
  DUMP_STATIC_ARRAY_GENERIC (test_uint64, static_array, output_array_name)

  DUMP_STATIC_ARRAY(uint64_roundnumbers, "test_repeated_uint64_roundnumbers");
  DUMP_STATIC_ARRAY(uint64_0_1_max, "test_repeated_uint64_0_1_max");
  DUMP_STATIC_ARRAY(uint64_random, "test_repeated_uint64_random");

#undef DUMP_STATIC_ARRAY
}
static void dump_test_repeated_fixed64 (void)
{
#define DUMP_STATIC_ARRAY(static_array, output_array_name) \
  DUMP_STATIC_ARRAY_GENERIC(test_fixed64, static_array, output_array_name)

  DUMP_STATIC_ARRAY(uint64_roundnumbers, "test_repeated_fixed64_roundnumbers");
  DUMP_STATIC_ARRAY(uint64_0_1_max, "test_repeated_fixed64_0_1_max");
  DUMP_STATIC_ARRAY(uint64_random, "test_repeated_fixed64_random");

#undef DUMP_STATIC_ARRAY
}

static void dump_test_repeated_float (void)
{
#define DUMP_STATIC_ARRAY(static_array, output_array_name) \
  DUMP_STATIC_ARRAY_GENERIC(test_float, static_array, output_array_name)

  DUMP_STATIC_ARRAY(float_random, "test_repeated_float_random");

#undef DUMP_STATIC_ARRAY
}

static void dump_test_repeated_double (void)
{
#define DUMP_STATIC_ARRAY(static_array, output_array_name) \
  DUMP_STATIC_ARRAY_GENERIC(test_double, static_array, output_array_name)

  DUMP_STATIC_ARRAY(double_random, "test_repeated_double_random");

#undef DUMP_STATIC_ARRAY
}
static void dump_test_repeated_boolean (void)
{
#define DUMP_STATIC_ARRAY(static_array, output_array_name) \
  DUMP_STATIC_ARRAY_GENERIC(test_boolean, static_array, output_array_name)

  DUMP_STATIC_ARRAY(boolean_0, "test_repeated_boolean_0");
  DUMP_STATIC_ARRAY(boolean_1, "test_repeated_boolean_1");
  DUMP_STATIC_ARRAY(boolean_random, "test_repeated_boolean_random");

#undef DUMP_STATIC_ARRAY
}
static void dump_test_repeated_enum_small (void)
{
#define DUMP_STATIC_ARRAY(static_array, output_array_name) \
  DUMP_STATIC_ARRAY_GENERIC (test_enum_small, static_array, output_array_name)

  DUMP_STATIC_ARRAY (enum_small_0, "test_repeated_enum_small_0");
  DUMP_STATIC_ARRAY (enum_small_1, "test_repeated_enum_small_1");
  DUMP_STATIC_ARRAY (enum_small_random, "test_repeated_enum_small_random");

#undef DUMP_STATIC_ARRAY
}

static void dump_test_repeated_enum (void)
{
#define DUMP_STATIC_ARRAY(static_array, output_array_name) \
  DUMP_STATIC_ARRAY_GENERIC (test_enum, static_array, output_array_name)

  DUMP_STATIC_ARRAY (enum_0, "test_repeated_enum_0");
  DUMP_STATIC_ARRAY (enum_1, "test_repeated_enum_1");
  DUMP_STATIC_ARRAY (enum_random, "test_repeated_enum_random");

#undef DUMP_STATIC_ARRAY
}

static void dump_test_repeated_string (void)
{
#define DUMP_STATIC_ARRAY(static_array, output_array_name) \
  DUMP_STATIC_ARRAY_GENERIC (test_string, static_array, output_array_name)

  DUMP_STATIC_ARRAY (repeated_strings_0, "test_repeated_strings_0");
  DUMP_STATIC_ARRAY (repeated_strings_1, "test_repeated_strings_1");
  DUMP_STATIC_ARRAY (repeated_strings_2, "test_repeated_strings_2");
  DUMP_STATIC_ARRAY (repeated_strings_3, "test_repeated_strings_3");

#undef DUMP_STATIC_ARRAY
}

static void dump_test_repeated_bytes (void)
{
  TestMess mess;
  mess.add_test_bytes(std::string("text"));
  mess.add_test_bytes(std::string("str\1\2\3\4\5") + '\0');
  mess.add_test_bytes(std::string("gobble") + '\0' + "foo");
  dump_message_bytes(&mess, "test_repeated_bytes_0");
}

static void dump_test_repeated_SubMess (void)
{
  TestMess mess;
  mess.add_test_message()->set_test(0);
  mess.add_test_message()->set_test(0);
  mess.add_test_message()->set_test(0);
  dump_message_bytes(&mess, "test_repeated_submess_0");

  mess.clear_test_message();
  mess.add_test_message()->set_test(42);
  mess.add_test_message()->set_test(-10000);
  mess.add_test_message()->set_test(667);
  dump_message_bytes(&mess, "test_repeated_submess_1");
}

static void dump_test_optional_int32 (void)
{
  TestMessOptional opt;
  opt.set_test_int32 (INT32_MIN);
  dump_message_bytes (&opt, "test_optional_int32_min");
  opt.set_test_int32 (-1);
  dump_message_bytes (&opt, "test_optional_int32_m1");
  opt.set_test_int32 (0);
  dump_message_bytes (&opt, "test_optional_int32_0");
  opt.set_test_int32 (666);
  dump_message_bytes (&opt, "test_optional_int32_666");
  opt.set_test_int32 (INT32_MAX);
  dump_message_bytes (&opt, "test_optional_int32_max");
}

static void dump_test_optional_sint32 (void)
{
  TestMessOptional opt;
  opt.set_test_sint32 (INT32_MIN);
  dump_message_bytes (&opt, "test_optional_sint32_min");
  opt.set_test_sint32 (-1);
  dump_message_bytes (&opt, "test_optional_sint32_m1");
  opt.set_test_sint32 (0);
  dump_message_bytes (&opt, "test_optional_sint32_0");
  opt.set_test_sint32 (666);
  dump_message_bytes (&opt, "test_optional_sint32_666");
  opt.set_test_sint32 (INT32_MAX);
  dump_message_bytes (&opt, "test_optional_sint32_max");
}

static void dump_test_optional_sfixed32 (void)
{
  TestMessOptional opt;
  opt.set_test_sfixed32 (INT32_MIN);
  dump_message_bytes (&opt, "test_optional_sfixed32_min");
  opt.set_test_sfixed32 (-1);
  dump_message_bytes (&opt, "test_optional_sfixed32_m1");
  opt.set_test_sfixed32 (0);
  dump_message_bytes (&opt, "test_optional_sfixed32_0");
  opt.set_test_sfixed32 (666);
  dump_message_bytes (&opt, "test_optional_sfixed32_666");
  opt.set_test_sfixed32 (INT32_MAX);
  dump_message_bytes (&opt, "test_optional_sfixed32_max");
}
static void dump_test_optional_int64 (void)
{
  TestMessOptional opt;
  opt.set_test_int64 (INT64_MIN);
  dump_message_bytes (&opt, "test_optional_int64_min");
  opt.set_test_int64 (-1111111111LL);
  dump_message_bytes (&opt, "test_optional_int64_m1111111111LL");
  opt.set_test_int64 (0);
  dump_message_bytes (&opt, "test_optional_int64_0");
  opt.set_test_int64 (QUINTILLION);
  dump_message_bytes (&opt, "test_optional_int64_quintillion");
  opt.set_test_int64 (INT64_MAX);
  dump_message_bytes (&opt, "test_optional_int64_max");
}
static void dump_test_optional_sint64 (void)
{
  TestMessOptional opt;
  opt.set_test_sint64 (INT64_MIN);
  dump_message_bytes (&opt, "test_optional_sint64_min");
  opt.set_test_sint64 (-1111111111LL);
  dump_message_bytes (&opt, "test_optional_sint64_m1111111111LL");
  opt.set_test_sint64 (0);
  dump_message_bytes (&opt, "test_optional_sint64_0");
  opt.set_test_sint64 (QUINTILLION);
  dump_message_bytes (&opt, "test_optional_sint64_quintillion");
  opt.set_test_sint64 (INT64_MAX);
  dump_message_bytes (&opt, "test_optional_sint64_max");
}
static void dump_test_optional_sfixed64 (void)
{
  TestMessOptional opt;
  opt.set_test_sfixed64 (INT64_MIN);
  dump_message_bytes (&opt, "test_optional_sfixed64_min");
  opt.set_test_sfixed64 (-1111111111LL);
  dump_message_bytes (&opt, "test_optional_sfixed64_m1111111111LL");
  opt.set_test_sfixed64 (0);
  dump_message_bytes (&opt, "test_optional_sfixed64_0");
  opt.set_test_sfixed64 (QUINTILLION);
  dump_message_bytes (&opt, "test_optional_sfixed64_quintillion");
  opt.set_test_sfixed64 (INT64_MAX);
  dump_message_bytes (&opt, "test_optional_sfixed64_max");
}
static void dump_test_optional_uint32 (void)
{
  TestMessOptional opt;
  opt.set_test_uint32(0);
  dump_message_bytes (&opt, "test_optional_uint32_0");
  opt.set_test_uint32(669);
  dump_message_bytes (&opt, "test_optional_uint32_669");
  opt.set_test_uint32(UINT32_MAX);
  dump_message_bytes (&opt, "test_optional_uint32_max");
}
static void dump_test_optional_fixed32 (void)
{
  TestMessOptional opt;
  opt.set_test_fixed32(0);
  dump_message_bytes (&opt, "test_optional_fixed32_0");
  opt.set_test_fixed32(669);
  dump_message_bytes (&opt, "test_optional_fixed32_669");
  opt.set_test_fixed32(UINT32_MAX);
  dump_message_bytes (&opt, "test_optional_fixed32_max");
}

static void dump_test_optional_uint64 (void)
{
  TestMessOptional opt;
  opt.set_test_uint64(0);
  dump_message_bytes (&opt, "test_optional_uint64_0");
  opt.set_test_uint64(669669669669669ULL);
  dump_message_bytes (&opt, "test_optional_uint64_669669669669669");
  opt.set_test_uint64(UINT64_MAX);
  dump_message_bytes (&opt, "test_optional_uint64_max");
}

static void dump_test_optional_float (void)
{
  TestMessOptional opt;
  opt.set_test_float (-100);
  dump_message_bytes (&opt, "test_optional_float_m100");
  opt.set_test_float (0);
  dump_message_bytes (&opt, "test_optional_float_0");
  opt.set_test_float (141243);
  dump_message_bytes (&opt, "test_optional_float_141243");
}

static void dump_test_optional_double (void)
{
  TestMessOptional opt;
  opt.set_test_double (-100);
  dump_message_bytes (&opt, "test_optional_double_m100");
  opt.set_test_double (0);
  dump_message_bytes (&opt, "test_optional_double_0");
  opt.set_test_double (141243);
  dump_message_bytes (&opt, "test_optional_double_141243");
}

static void dump_test_optional_fixed64 (void)
{
  TestMessOptional opt;
  opt.set_test_fixed64(0);
  dump_message_bytes (&opt, "test_optional_fixed64_0");
  opt.set_test_fixed64(669669669669669ULL);
  dump_message_bytes (&opt, "test_optional_fixed64_669669669669669");
  opt.set_test_fixed64(UINT64_MAX);
  dump_message_bytes (&opt, "test_optional_fixed64_max");
}
int main()
{
  dump_test_enum_small ();
  dump_test_enum_big ();
  dump_test_field_numbers ();
  dump_test_repeated_int32 ();
  dump_test_repeated_sint32 ();
  dump_test_repeated_uint32 ();
  dump_test_repeated_sfixed32 ();
  dump_test_repeated_fixed32 ();
  dump_test_repeated_int64 ();
  dump_test_repeated_sint64 ();
  dump_test_repeated_sfixed64 ();
  dump_test_repeated_uint64 ();
  dump_test_repeated_fixed64 ();
  dump_test_repeated_float ();
  dump_test_repeated_double ();
  dump_test_repeated_boolean ();
  dump_test_repeated_enum_small ();
  dump_test_repeated_enum ();
  dump_test_repeated_string ();
  dump_test_repeated_bytes ();
  dump_test_repeated_SubMess ();
  dump_test_optional_int32 ();
  dump_test_optional_sint32 ();
  dump_test_optional_sfixed32 ();
  dump_test_optional_int64 ();
  dump_test_optional_sint64 ();
  dump_test_optional_sfixed64 ();
  dump_test_optional_uint32 ();
  dump_test_optional_fixed32 ();
  dump_test_optional_uint64 ();
  dump_test_optional_fixed64 ();
  dump_test_optional_float ();
  dump_test_optional_double ();
  return 0;
}
