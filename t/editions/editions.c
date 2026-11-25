/*
 * Tests for code generated from "edition = 2023" .proto files: the edition
 * features (field_presence, repeated_field_encoding) must select the same
 * wire behavior that proto2 and proto3 files get.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "t/editions/editions.pb-c.h"
#include "t/editions/editions-implicit.pb-c.h"
#include "t/editions/utf8-proto2.pb-c.h"
#include "t/editions/utf8-proto3.pb-c.h"

static void
check_bytes(const char *what, const uint8_t *got, size_t got_len,
	    const uint8_t *want, size_t want_len)
{
	if (got_len != want_len || memcmp(got, want, want_len) != 0) {
		size_t i;
		fprintf(stderr, "%s: wire mismatch\n  got: ", what);
		for (i = 0; i < got_len; i++)
			fprintf(stderr, "%02x ", got[i]);
		fprintf(stderr, "\n  want:");
		for (i = 0; i < want_len; i++)
			fprintf(stderr, " %02x", want[i]);
		fprintf(stderr, "\n");
		assert(0);
	}
}

#define CHECK_PACKED(what, msg, pack_fn, size_fn, ...) \
	do { \
		static const uint8_t want[] = { __VA_ARGS__ }; \
		uint8_t buf[256]; \
		size_t n = size_fn(msg); \
		assert(n <= sizeof(buf)); \
		assert(pack_fn(msg, buf) == n); \
		check_bytes(what, buf, n, want, sizeof(want)); \
	} while (0)

#define CHECK_MSG(what, msg, ...) \
	CHECK_PACKED(what, msg, editions__msg__pack, editions__msg__get_packed_size, __VA_ARGS__)
#define CHECK_IMPLICIT(what, msg, ...) \
	CHECK_PACKED(what, msg, editions__implicit__pack, editions__implicit__get_packed_size, __VA_ARGS__)

static void
test_explicit_presence(void)
{
	Editions__Msg m = EDITIONS__MSG__INIT;

	/* Defaults: only the legacy-required field is serialized. */
	assert(m.with_default == 42);
	CHECK_MSG("defaults", &m, 0x58, 0x00);

	/* Explicit presence: a zero that is present is sent. */
	m.has_explicit_int = 1;
	m.explicit_int = 0;
	CHECK_MSG("explicit zero", &m, 0x08, 0x00, 0x58, 0x00);

	/* Explicit-presence strings and bytes use NULL for "not present". */
	assert(m.explicit_str == NULL);
	m.explicit_str = (char *) "";
	CHECK_MSG("empty explicit string", &m, 0x08, 0x00, 0x12, 0x00, 0x58, 0x00);
}

static void
test_implicit_presence(void)
{
	Editions__Msg m = EDITIONS__MSG__INIT;

	/* Implicit presence: zero/empty values are never sent... */
	assert(m.implicit_str != NULL && m.implicit_str[0] == 0);
	CHECK_MSG("implicit zero", &m, 0x58, 0x00);

	/* ...but non-zero values are. */
	m.implicit_int = 5;
	CHECK_MSG("implicit nonzero", &m, 0x38, 0x05, 0x58, 0x00);
}

static void
test_repeated_encoding(void)
{
	Editions__Msg m = EDITIONS__MSG__INIT;
	int32_t v[2] = { 1, 2 };

	/* Edition 2023 default: PACKED. */
	m.n_packed_ints = 2;
	m.packed_ints = v;
	CHECK_MSG("packed", &m, 0x2a, 0x02, 0x01, 0x02, 0x58, 0x00);
	m.n_packed_ints = 0;
	m.packed_ints = NULL;

	/* features.repeated_field_encoding = EXPANDED. */
	m.n_expanded_ints = 2;
	m.expanded_ints = v;
	CHECK_MSG("expanded", &m, 0x30, 0x01, 0x30, 0x02, 0x58, 0x00);
}

static void
test_file_level_features(void)
{
	Editions__Implicit m = EDITIONS__IMPLICIT__INIT;
	int64_t v = 1;

	/* File-level IMPLICIT: nothing present, nothing sent. */
	CHECK_IMPLICIT("file implicit defaults", &m);

	/* A field may opt back in to explicit presence. */
	m.has_f = 1;
	CHECK_IMPLICIT("field explicit override", &m, 0x30, 0x00);
	m.has_f = 0;

	m.n_d = 1;
	m.d = &v;
	CHECK_IMPLICIT("packed repeated", &m, 0x22, 0x01, 0x01);
	m.n_d = 0;
	m.n_e = 1;
	m.e = &v;
	CHECK_IMPLICIT("expanded repeated", &m, 0x28, 0x01);
}

static void
test_roundtrip(void)
{
	Editions__Msg m = EDITIONS__MSG__INIT;
	Editions__Msg *u;
	int32_t v[2] = { 7, 8 };
	uint8_t buf[256];
	size_t n;

	m.has_explicit_int = 1;
	m.explicit_int = 0;
	m.implicit_int = 9;
	m.required_int = 3;
	m.n_packed_ints = 2;
	m.packed_ints = v;
	m.n_expanded_ints = 2;
	m.expanded_ints = v;
	m.choice_case = EDITIONS__MSG__CHOICE_CHOICE_INT;
	m.choice_int = 11;

	n = editions__msg__pack(&m, buf);
	u = editions__msg__unpack(NULL, n, buf);
	assert(u != NULL);
	assert(u->has_explicit_int && u->explicit_int == 0);
	assert(!u->has_explicit_bytes);
	assert(u->implicit_int == 9);
	assert(u->required_int == 3);
	assert(u->n_packed_ints == 2 && u->packed_ints[1] == 8);
	assert(u->n_expanded_ints == 2 && u->expanded_ints[0] == 7);
	assert(u->choice_case == EDITIONS__MSG__CHOICE_CHOICE_INT);
	assert(u->choice_int == 11);
	editions__msg__free_unpacked(u, NULL);
}

static const ProtobufCFieldDescriptor *
field(const ProtobufCMessageDescriptor *d, const char *name)
{
	const ProtobufCFieldDescriptor *f =
		protobuf_c_message_descriptor_get_field_by_name(d, name);
	assert(f != NULL);
	return f;
}

/* The resolved features are recorded in the descriptors. */
static void
test_features(void)
{
	const ProtobufCMessageDescriptor *md = &editions__msg__descriptor;
	const ProtobufCMessageDescriptor *id = &editions__implicit__descriptor;
	const ProtobufCFeatures *f;

	f = field(md, "explicit_int")->features;
	assert(f != NULL && f->struct_size >= sizeof(*f));
	assert(f->field_presence == PROTOBUF_C_FIELD_PRESENCE_EXPLICIT);
	assert(f->utf8_validation == PROTOBUF_C_UTF8_VALIDATION_VERIFY);
	assert(f->enum_type == PROTOBUF_C_ENUM_TYPE_OPEN);
	assert(f->message_encoding == PROTOBUF_C_MESSAGE_ENCODING_LENGTH_PREFIXED);

	f = field(md, "implicit_int")->features;
	assert(f->field_presence == PROTOBUF_C_FIELD_PRESENCE_IMPLICIT);

	f = field(md, "required_int")->features;
	assert(f->field_presence == PROTOBUF_C_FIELD_PRESENCE_LEGACY_REQUIRED);

	f = field(md, "packed_ints")->features;
	assert(f->repeated_field_encoding == PROTOBUF_C_REPEATED_FIELD_ENCODING_PACKED);
	f = field(md, "expanded_ints")->features;
	assert(f->repeated_field_encoding == PROTOBUF_C_REPEATED_FIELD_ENCODING_EXPANDED);

	/* Identical features share one object. */
	assert(field(md, "explicit_int")->features == field(md, "explicit_str")->features);
	assert(md->features != NULL);

	assert(editions__color__descriptor.features != NULL);
	assert(editions__color__descriptor.features->enum_type == PROTOBUF_C_ENUM_TYPE_OPEN);

	/* File-level options apply to every member; fields may override. */
	assert(field(id, "a")->features->field_presence == PROTOBUF_C_FIELD_PRESENCE_IMPLICIT);
	assert(field(id, "f")->features->field_presence == PROTOBUF_C_FIELD_PRESENCE_EXPLICIT);
}

/* ---- UTF-8 validation of string fields on unpack ---- */

static int warn_calls;
static protobuf_c_boolean
warn_validator(size_t len, const char *data)
{
	if (!protobuf_c_validate_utf8_strict(len, data))
		warn_calls++;	/* the example would print a warning */
	return 1;
}

static int custom_calls;
static protobuf_c_boolean
custom_validator(size_t len, const char *data)
{
	(void) len; (void) data;
	custom_calls++;
	return 0;	/* rejects everything */
}

static void
test_utf8_validator(void)
{
	/* well-formed */
	assert(protobuf_c_validate_utf8_strict(0, ""));
	assert(protobuf_c_validate_utf8_strict(5, "plain"));
	assert(protobuf_c_validate_utf8_strict(2, "\xc3\xa9"));		/* U+00E9 */
	assert(protobuf_c_validate_utf8_strict(3, "\xe2\x82\xac"));	/* U+20AC */
	assert(protobuf_c_validate_utf8_strict(4, "\xf0\x9f\x98\x80"));	/* U+1F600 */
	assert(protobuf_c_validate_utf8_strict(3, "a\0b"));		/* NUL is fine */
	assert(protobuf_c_validate_utf8_strict(3, "\xed\x9f\xbf"));	/* U+D7FF */
	assert(protobuf_c_validate_utf8_strict(4, "\xf4\x8f\xbf\xbf"));	/* U+10FFFF */
	/* malformed */
	assert(!protobuf_c_validate_utf8_strict(1, "\xff"));
	assert(!protobuf_c_validate_utf8_strict(1, "\x80"));		/* stray continuation */
	assert(!protobuf_c_validate_utf8_strict(1, "\xc3"));		/* truncated */
	assert(!protobuf_c_validate_utf8_strict(2, "\xc0\x80"));	/* overlong NUL */
	assert(!protobuf_c_validate_utf8_strict(3, "\xe0\x80\x80"));	/* overlong */
	assert(!protobuf_c_validate_utf8_strict(3, "\xed\xa0\x80"));	/* surrogate */
	assert(!protobuf_c_validate_utf8_strict(4, "\xf4\x90\x80\x80"));	/* > U+10FFFF */
	assert(!protobuf_c_validate_utf8_strict(4, "\xf0\x80\x80\x80"));	/* overlong */
	assert(!protobuf_c_validate_utf8_strict(2, "a\xc3"));
}

/* Unpack `wire` as `desc`; returns the message or NULL. */
static ProtobufCMessage *
unpack(const ProtobufCMessageDescriptor *desc, const char *wire, size_t len)
{
	return protobuf_c_message_unpack(desc, NULL, len, (const uint8_t *) wire);
}

static void
test_utf8(void)
{
	/* field 1 (len 2): ff fe, which is not UTF-8 */
	static const char bad1[] = "\x0a\x02\xff\xfe";
	static const char good1[] = "\x0a\x02\xc3\xa9";
	/* field 2 (repeated string) with a bad second element */
	static const char bad_rep[] = "\x12\x01" "a" "\x12\x01\xff";
	/* field 3 (oneof string) */
	static const char bad_oneof[] = "\x1a\x01\xff";
	/* field 5 (map<string,string>): entry { key: 0xff } */
	static const char bad_map[] = "\x2a\x03\x0a\x01\xff";
	/* field 6 (bytes) may hold anything */
	static const char raw[] = "\x32\x02\xff\xfe";
	ProtobufCMessage *m;

	assert(protobuf_c_validate_utf8 == protobuf_c_validate_utf8_strict);

	/* proto3: verified */
	m = unpack(&editions__utf8_proto3__descriptor, good1, 4);
	assert(m != NULL);
	protobuf_c_message_free_unpacked(m, NULL);
	assert(unpack(&editions__utf8_proto3__descriptor, bad1, 4) == NULL);
	assert(unpack(&editions__utf8_proto3__descriptor, bad_rep, 6) == NULL);
	assert(unpack(&editions__utf8_proto3__descriptor, bad_oneof, 3) == NULL);
	assert(unpack(&editions__utf8_proto3__descriptor, bad_map, 5) == NULL);
	m = unpack(&editions__utf8_proto3__descriptor, raw, 4);	/* bytes */
	assert(m != NULL);
	protobuf_c_message_free_unpacked(m, NULL);

	/* proto2: not verified */
	m = unpack(&editions__utf8_proto2__descriptor, bad1, 4);
	assert(m != NULL);
	protobuf_c_message_free_unpacked(m, NULL);

	/* edition 2023: verified by default, unless the field opts out */
	{
		/* Msg has a required field 11, so every wire string has it */
		static const char ok_explicit[] = "\x58\x01\x12\x01" "a";
		static const char bad_explicit[] = "\x58\x01\x12\x01\xff";	/* field 2 */
		static const char bad_lax[] = "\x58\x01\x7a\x01\xff";	/* field 15 */

		m = unpack(&editions__msg__descriptor, ok_explicit, 5);
		assert(m != NULL);
		protobuf_c_message_free_unpacked(m, NULL);
		assert(unpack(&editions__msg__descriptor, bad_explicit, 5) == NULL);
		m = unpack(&editions__msg__descriptor, bad_lax, 5);
		assert(m != NULL);
		protobuf_c_message_free_unpacked(m, NULL);
	}

	/* the escape hatch: a validator that warns and accepts (the example in
	 * protobuf-c.h) */
	protobuf_c_validate_utf8 = warn_validator;
	m = unpack(&editions__utf8_proto3__descriptor, bad1, 4);
	assert(m != NULL && warn_calls == 1);
	protobuf_c_message_free_unpacked(m, NULL);
	m = unpack(&editions__utf8_proto3__descriptor, good1, 4);
	assert(m != NULL && warn_calls == 1);	/* silent when it is valid */
	protobuf_c_message_free_unpacked(m, NULL);

	/* ... or one that rejects everything */
	protobuf_c_validate_utf8 = custom_validator;
	assert(unpack(&editions__utf8_proto3__descriptor, good1, 4) == NULL);
	assert(custom_calls == 1);
	m = unpack(&editions__utf8_proto2__descriptor, good1, 4);	/* not asked */
	assert(m != NULL && custom_calls == 1);
	protobuf_c_message_free_unpacked(m, NULL);

	protobuf_c_validate_utf8 = protobuf_c_validate_utf8_strict;
	puts("utf8 validation ok");
}

int
main(void)
{
	test_features();
	test_explicit_presence();
	test_implicit_presence();
	test_repeated_encoding();
	test_file_level_features();
	test_roundtrip();
	test_utf8_validator();
	test_utf8();
	puts("editions: ok");
	return 0;
}
