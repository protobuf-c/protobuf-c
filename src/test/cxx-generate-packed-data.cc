/* generate byte arrays that match the constructs in test-generated-code2.c.
 * these are tested against eachother to make sure the c and c++ agree. */

#define __STDC_LIMIT_MACROS
#include "generated-code/test-full.pb.h"
#include <inttypes.h>

using namespace foo;

#define protobuf_c_boolean bool
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
static void dump_test_repeated_int32 (void)
{
  TestMess mess;

  mess.clear_test_int32 ();
  dump_message_bytes(&mess, "test_repeated_int32__empty");
  mess.add_test_int32(-1);
  dump_message_bytes(&mess, "test_repeated_int32_m1");
  mess.add_test_int32(1);
  dump_message_bytes(&mess, "test_repeated_int32_m1_1");
  mess.clear_test_int32 ();
  mess.add_test_int32(1);
  dump_message_bytes(&mess, "test_repeated_int32_1");
  mess.clear_test_int32 ();
  mess.add_test_int32(42);
  mess.add_test_int32(666);
  mess.add_test_int32(-1123123);
  mess.add_test_int32(0);
  mess.add_test_int32(47);
  dump_message_bytes(&mess, "test_repeated_int32_42_666_m1123123_0_47");
  mess.clear_test_int32 ();
  mess.add_test_int32(INT32_MIN);
  dump_message_bytes(&mess, "test_repeated_int32_min");
  mess.clear_test_int32 ();
  mess.add_test_int32(INT32_MAX);
  dump_message_bytes(&mess, "test_repeated_int32_max");
}
static void dump_test_repeated_sint32 (void)
{
  TestMess mess;

  mess.clear_test_sint32 ();
  dump_message_bytes(&mess, "test_repeated_sint32__empty");
  mess.add_test_sint32(-1);
  dump_message_bytes(&mess, "test_repeated_sint32_m1");
  mess.add_test_sint32(1);
  dump_message_bytes(&mess, "test_repeated_sint32_m1_1");
  mess.clear_test_sint32 ();
  mess.add_test_sint32(1);
  dump_message_bytes(&mess, "test_repeated_sint32_1");
  mess.clear_test_sint32 ();
  mess.add_test_sint32(42);
  mess.add_test_sint32(666);
  mess.add_test_sint32(-1123123);
  mess.add_test_sint32(0);
  mess.add_test_sint32(47);
  dump_message_bytes(&mess, "test_repeated_sint32_42_666_m1123123_0_47");
  mess.clear_test_sint32 ();
  mess.add_test_sint32(INT32_MIN);
  dump_message_bytes(&mess, "test_repeated_sint32_min");
  mess.clear_test_sint32 ();
  mess.add_test_sint32(INT32_MAX);
  dump_message_bytes(&mess, "test_repeated_sint32_max");
}
static void dump_test_repeated_uint32 (void)
{
  TestMess mess;
  mess.add_test_uint32(BILLION);
  mess.add_test_uint32(MILLION);
  mess.add_test_uint32(1);
  mess.add_test_uint32(0);
  dump_message_bytes (&mess, "test_repeated_uint32_bil_mil_1_0");
  mess.clear_test_uint32();
  mess.add_test_uint32(0);
  dump_message_bytes (&mess, "test_repeated_uint32_0");
  mess.clear_test_uint32();
  mess.add_test_uint32(UINT32_MAX);
  dump_message_bytes (&mess, "test_repeated_uint32_max");
}
static void dump_test_repeated_sfixed32 (void)
{
  TestMess mess;

  mess.clear_test_sfixed32 ();
  dump_message_bytes(&mess, "test_repeated_sfixed32__empty");
  mess.add_test_sfixed32(-1);
  dump_message_bytes(&mess, "test_repeated_sfixed32_m1");
  mess.add_test_sfixed32(1);
  dump_message_bytes(&mess, "test_repeated_sfixed32_m1_1");
  mess.clear_test_sfixed32 ();
  mess.add_test_sfixed32(1);
  dump_message_bytes(&mess, "test_repeated_sfixed32_1");
  mess.clear_test_sfixed32 ();
  mess.add_test_sfixed32(42);
  mess.add_test_sfixed32(666);
  mess.add_test_sfixed32(-1123123);
  mess.add_test_sfixed32(0);
  mess.add_test_sfixed32(47);
  dump_message_bytes(&mess, "test_repeated_sfixed32_42_666_m1123123_0_47");
  mess.clear_test_sfixed32 ();
  mess.add_test_sfixed32(INT32_MIN);
  dump_message_bytes(&mess, "test_repeated_sfixed32_min");
  mess.clear_test_sfixed32 ();
  mess.add_test_sfixed32(INT32_MAX);
  dump_message_bytes(&mess, "test_repeated_sfixed32_max");
}
static void dump_test_repeated_fixed32 (void)
{
  TestMess mess;
  mess.add_test_fixed32(BILLION);
  mess.add_test_fixed32(MILLION);
  mess.add_test_fixed32(1);
  mess.add_test_fixed32(0);
  dump_message_bytes (&mess, "test_repeated_fixed32_bil_mil_1_0");
  mess.clear_test_fixed32();
  mess.add_test_fixed32(0);
  dump_message_bytes (&mess, "test_repeated_fixed32_0");
  mess.clear_test_fixed32();
  mess.add_test_fixed32(UINT32_MAX);
  dump_message_bytes (&mess, "test_repeated_fixed32_max");
}
static void dump_test_repeated_int64 (void)
{
  TestMess mess;
  for (unsigned i = 0; i < N_ELEMENTS (int64_roundnumbers); i++)
    mess.add_test_int64(int64_roundnumbers[i]);
  dump_message_bytes(&mess, "test_repeated_int64_roundnumbers");
  mess.clear_test_int64();
  for (unsigned i = 0; i < N_ELEMENTS (int64_min_max); i++)
    mess.add_test_int64(int64_min_max[i]);
  dump_message_bytes(&mess, "test_repeated_int64_min_max");
}

static void dump_test_repeated_sint64 (void)
{
  TestMess mess;
  for (unsigned i = 0; i < N_ELEMENTS (int64_roundnumbers); i++)
    mess.add_test_sint64(int64_roundnumbers[i]);
  dump_message_bytes(&mess, "test_repeated_sint64_roundnumbers");
  mess.clear_test_sint64();
  for (unsigned i = 0; i < N_ELEMENTS (int64_min_max); i++)
    mess.add_test_sint64(int64_min_max[i]);
  dump_message_bytes(&mess, "test_repeated_sint64_min_max");
}

static void dump_test_repeated_sfixed64 (void)
{
  TestMess mess;
  for (unsigned i = 0; i < N_ELEMENTS (int64_roundnumbers); i++)
    mess.add_test_sfixed64(int64_roundnumbers[i]);
  dump_message_bytes(&mess, "test_repeated_sfixed64_roundnumbers");
  mess.clear_test_sfixed64();
  for (unsigned i = 0; i < N_ELEMENTS (int64_min_max); i++)
    mess.add_test_sfixed64(int64_min_max[i]);
  dump_message_bytes(&mess, "test_repeated_sfixed64_min_max");
}
static void dump_test_repeated_uint64 (void)
{
  TestMess mess;
#define DUMP_STATIC_ARRAY(static_array, output_array_name) \
  do{ \
  mess.clear_test_uint64(); \
  for (unsigned i = 0; i < N_ELEMENTS (static_array); i++) \
    mess.add_test_uint64(static_array[i]); \
  dump_message_bytes(&mess, output_array_name); \
  }while(0)

  DUMP_STATIC_ARRAY(uint64_roundnumbers, "test_repeated_uint64_roundnumbers");
  DUMP_STATIC_ARRAY(uint64_0_1_max, "test_repeated_uint64_0_1_max");
  DUMP_STATIC_ARRAY(uint64_random, "test_repeated_uint64_random");

#undef DUMP_STATIC_ARRAY
}
static void dump_test_repeated_fixed64 (void)
{
  TestMess mess;
#define DUMP_STATIC_ARRAY(static_array, output_array_name) \
  do{ \
  mess.clear_test_fixed64(); \
  for (unsigned i = 0; i < N_ELEMENTS (static_array); i++) \
    mess.add_test_fixed64(static_array[i]); \
  dump_message_bytes(&mess, output_array_name); \
  }while(0)

  DUMP_STATIC_ARRAY(uint64_roundnumbers, "test_repeated_fixed64_roundnumbers");
  DUMP_STATIC_ARRAY(uint64_0_1_max, "test_repeated_fixed64_0_1_max");
  DUMP_STATIC_ARRAY(uint64_random, "test_repeated_fixed64_random");

#undef DUMP_STATIC_ARRAY
}

static void dump_test_repeated_float (void)
{
  TestMess mess;
#define DUMP_STATIC_ARRAY(static_array, output_array_name) \
  do{ \
  mess.clear_test_float(); \
  for (unsigned i = 0; i < N_ELEMENTS (static_array); i++) \
    mess.add_test_float(static_array[i]); \
  dump_message_bytes(&mess, output_array_name); \
  }while(0)

  DUMP_STATIC_ARRAY(float_random, "test_repeated_float_random");

#undef DUMP_STATIC_ARRAY
}

static void dump_test_repeated_double (void)
{
  TestMess mess;
#define DUMP_STATIC_ARRAY(static_array, output_array_name) \
  do{ \
  mess.clear_test_double(); \
  for (unsigned i = 0; i < N_ELEMENTS (static_array); i++) \
    mess.add_test_double(static_array[i]); \
  dump_message_bytes(&mess, output_array_name); \
  }while(0)

  DUMP_STATIC_ARRAY(double_random, "test_repeated_double_random");

#undef DUMP_STATIC_ARRAY
}
static void dump_test_repeated_boolean (void)
{
  TestMess mess;
#define DUMP_STATIC_ARRAY(static_array, output_array_name) \
  do{ \
  mess.clear_test_boolean(); \
  for (unsigned i = 0; i < N_ELEMENTS (static_array); i++) \
    mess.add_test_boolean(static_array[i]); \
  dump_message_bytes(&mess, output_array_name); \
  }while(0)

  DUMP_STATIC_ARRAY(boolean_0, "test_repeated_boolean_0");
  DUMP_STATIC_ARRAY(boolean_1, "test_repeated_boolean_1");
  DUMP_STATIC_ARRAY(boolean_random, "test_repeated_boolean_random");

#undef DUMP_STATIC_ARRAY
}


int main(int argc, char **argv)
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
  return 0;
}
