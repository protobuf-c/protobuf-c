import pytest
import math
from google.protobuf import message as protobuf_message
from hypothesis import given, strategies as st

import test_full_pb2 as pb
from common.utils import roundtrip_test, log_diff

@pytest.fixture
def empty_sub_sub_mess():
    return pb.SubMess.SubSubMess()

@pytest.fixture
def empty_sub_mess():
    return pb.SubMess()

def test_sub_sub_mess_defaults(empty_sub_sub_mess):
    assert empty_sub_sub_mess.val1 == 100
    assert empty_sub_sub_mess.rep == []
    assert empty_sub_sub_mess.bytes1 == b"a \x00 char"
    assert empty_sub_sub_mess.str1 == "hello world\n"
    assert empty_sub_sub_mess.str2 == "hello\x00world\n"

def test_sub_sub_mess_roundtrip():
    def fill(msg):
        msg.val1 = -100
        msg.rep.extend([-2**31, 0, 2**31 - 1])
        msg.bytes1 = b"\x00\xff"
        msg.str1 = "café\n"
        msg.str2 = "world\x00unicode"
    msg = roundtrip_test(pb.SubMess.SubSubMess(), fill)
    assert msg.val1 == -100

def test_sub_sub_mess_empty_roundtrip(empty_sub_sub_mess):
    serialized = empty_sub_sub_mess.SerializeToString()
    msg2 = pb.SubMess.SubSubMess()
    msg2.ParseFromString(serialized)
    assert empty_sub_sub_mess == msg2

def test_sub_sub_mess_unicode_and_null():
    def fill(msg):
        msg.str1 = "hello\u1234world"
        msg.str2 = "\x00null\x00"
        msg.bytes1 = b""
    roundtrip_test(pb.SubMess.SubSubMess(), fill)

def test_sub_sub_mess_invalid_parse():
    with pytest.raises(protobuf_message.DecodeError):
        pb.SubMess.SubSubMess().ParseFromString(b"\x99\x99")

@given(val1=st.integers(min_value=-2**31, max_value=2**31-1),
       rep=st.lists(st.integers(min_value=-2**31, max_value=2**31-1), max_size=10),
       bytes1=st.binary(max_size=100),
       str1=st.text(max_size=100),
       str2=st.text(max_size=100))
def test_sub_sub_mess_hypothesis(val1, rep, bytes1, str1, str2):
    def fill(msg):
        msg.val1 = val1
        msg.rep.extend(rep)
        msg.bytes1 = bytes1
        msg.str1 = str1
        msg.str2 = str2
    roundtrip_test(pb.SubMess.SubSubMess(), fill)

def test_sub_mess_required(empty_sub_mess):
    assert not empty_sub_mess.IsInitialized()
    empty_sub_mess.test = 42
    assert empty_sub_mess.IsInitialized()

def test_sub_mess_roundtrip():
    def fill(msg):
        msg.test = -42
        msg.val1 = 1
        msg.val2 = 2
        msg.rep.extend([3, 4])
        sub1 = msg.sub1
        sub1.val1 = 101
        sub1.rep.extend([5])
        sub1.bytes1 = b"test\x00"
        sub1.str1 = "test"
        sub1.str2 = "test\x00"
        sub2 = msg.sub2
        sub2.val1 = 102
    roundtrip_test(pb.SubMess(), fill)

def test_sub_mess_negative_uint32():
    msg = pb.SubMess()
    msg.val1 = -1  # Use existing field
    assert msg.val1 == -1

def test_sub_mess_large_repeated():
    msg = pb.SubMess()
    msg.test = 42
    msg.rep.extend([0] * 1000)  # Large but reasonable
    serialized = msg.SerializeToString()
    msg2 = pb.SubMess()
    msg2.ParseFromString(serialized)
    assert msg == msg2

def test_enum_small():
    assert pb.TestEnumSmall.NEG_VALUE == -1
    assert pb.TestEnumSmall.VALUE == 0
    assert pb.TestEnumSmall.OTHER_VALUE == 1

def test_enum_boundaries():
    assert pb.TestEnum.VALUENEG123456 == -123456
    assert pb.TestEnum.VALUE16383 == 16383
    assert pb.TestEnum.VALUE268435456 == 268435456

def test_enum_dup_values():
    assert pb.TestEnumDupValues.VALUE_A == 42
    assert pb.TestEnumDupValues.VALUE_D == 666
    assert pb.TestEnumDupValues.VALUE_AA == 1000

def test_field_no_high_roundtrip():
    def fill(msg):
        msg.test = "high field number test"
    msg = roundtrip_test(pb.TestFieldNo33554432(), fill)
    assert msg.IsInitialized()

def test_test_mess_roundtrip():
    def fill(msg):
        msg.test_int32.extend([-2**31, -1, 0, 1, 2**31 - 1])
        msg.test_sint32.extend([-2**31, -1, 0, 1, 2**31 - 1])
        msg.test_sfixed32.extend([-2**31, -1, 0, 1, 2**31 - 1])
        msg.test_int64.extend([-2**63, -1, 0, 1, 2**63 - 1])
        msg.test_sint64.extend([-2**63, -1, 0, 1, 2**63 - 1])
        msg.test_sfixed64.extend([-2**63, -1, 0, 1, 2**63 - 1])
        msg.test_uint32.extend([0, 1, 2**32 - 1])
        msg.test_fixed32.extend([0, 1, 2**32 - 1])
        msg.test_uint64.extend([0, 1, 2**64 - 1])
        msg.test_fixed64.extend([0, 1, 2**64 - 1])
        msg.test_float.extend([0.0, -1.5, math.inf, -math.inf, math.nan])
        msg.test_double.extend([0.0, -1.5, math.inf, -math.inf, math.nan])
        msg.test_boolean.extend([True, False])
        msg.test_enum_small.extend([pb.TestEnumSmall.NEG_VALUE, pb.TestEnumSmall.VALUE])
        msg.test_enum.extend([pb.TestEnum.VALUENEG123456, pb.TestEnum.VALUE268435456])
        msg.test_string.extend(["", "hello", "café", "with\x00null"])
        msg.test_bytes.extend([b"", b"hello", b"\x00\xff"])
        sub = msg.test_message.add()
        sub.test = 42
    roundtrip_test(pb.TestMess(), fill)

def test_test_mess_packed_roundtrip():
    def fill(msg):
        msg.test_int32.extend([-1, 0, 1])
        msg.test_float.extend([math.nan, math.inf])
    roundtrip_test(pb.TestMessPacked(), fill)

def test_test_mess_optional():
    msg = pb.TestMessOptional()
    assert msg.IsInitialized()
    assert msg.test_int32 == 0
    assert not msg.HasField('test_int32')
    msg.test_int32 = -42
    assert msg.HasField('test_int32')
    msg.ClearField('test_int32')
    assert not msg.HasField('test_int32')

def test_test_mess_oneof():
    msg = pb.TestMessOneof()
    assert msg.WhichOneof('test_oneof') is None
    msg.test_int32 = -42
    assert msg.WhichOneof('test_oneof') == 'test_int32'
    msg.test_string = "hello"
    assert msg.WhichOneof('test_oneof') == 'test_string'
    assert msg.test_int32 == 0

def test_required_invalid_assignment():
    with pytest.raises(ValueError):
        msg = pb.TestMessRequiredUInt32()
        msg.test = -1

def test_default_required_values():
    msg = pb.DefaultRequiredValues()
    assert not msg.IsInitialized()
    msg.v_int32 = -42
    msg.v_uint32 = 666
    msg.v_int64 = 100000
    msg.v_uint64 = 100001
    msg.v_float = 2.5
    msg.v_double = 4.5
    msg.v_string = "hi mom\n"
    msg.v_bytes = b"a \x00 character"
    assert msg.IsInitialized()

def test_default_optional_values():
    msg = pb.DefaultOptionalValues()
    assert msg.IsInitialized()
    assert msg.v_int32 == -42
    assert not msg.HasField('v_int32')

def test_empty_mess():
    msg = pb.EmptyMess()
    assert msg.IsInitialized()
    serialized = msg.SerializeToString()
    assert serialized == b""
    msg2 = pb.EmptyMess()
    msg2.ParseFromString(serialized)
    assert msg == msg2

def test_required_fields_bitmap():
    msg = pb.TestRequiredFieldsBitmap()
    assert not msg.IsInitialized()
    msg.field1 = "set1"
    msg.field129 = "set129"
    assert msg.IsInitialized()
    msg.field65 = "optional"
    serialized = msg.SerializeToString()
    msg2 = pb.TestRequiredFieldsBitmap()
    msg2.ParseFromString(serialized)
    assert msg2.field1 == "set1"
    assert msg2.field129 == "set129"
    assert msg2.field65 == "optional"

def test_field_flags_deprecated():
    msg = pb.TestFieldFlags()
    msg.deprecated.extend([1, 2])
    msg.no_flags2 = 0
    serialized = msg.SerializeToString()
    assert len(serialized) > 0

def test_large_message():
    msg = pb.TestMess()
    msg.test_int32.extend([0] * 10000)  # Stress test
    serialized = msg.SerializeToString()
    msg2 = pb.TestMess()
    msg2.ParseFromString(serialized)
    assert len(msg2.test_int32) == 10000

def test_csv_export():
    tests = [
        ("AC-FULL-001", "Test SubSubMess Defaults", "FR-FULL-001", "Unit", ["Initialize SubSubMess"], "Default values match"),
        ("AC-FULL-002", "Test SubMess Roundtrip", "FR-FULL-002", "Unit", ["Fill SubMess", "Serialize", "Deserialize"], "Messages equal")
    ]
    from common.utils import export_cases_to_csv
    export_cases_to_csv(tests, "automation-cases.csv")
    suites = [{
        "suite_id": "AS-FULL-001",
        "title": "Full Proto Tests",
        "description": "Tests for test-full.proto",
        "module": "Full",
        "test_case_ids": "AC-FULL-001,AC-FULL-002",
        "labels": "full,protobuf"
    }]
    from common.utils import export_suites_to_csv
    export_suites_to_csv(suites, "automation-suites.csv")
