/* generate byte arrays that match the constructs in test-generated-code2.c.
 * these are tested against eachother to make sure the c and c++ agree. */

#include "generated-code/test-full.pb.h"

using namespace foo;

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

int main(int argc, char **argv)
{
  dump_test_enum_small ();
  dump_test_enum_big ();
  return 0;
}
