import pytest
from google.protobuf import message as protobuf_message

import test_optimized_pb2 as pb
from common.utils import roundtrip_test
from hypothesis import given, strategies as st

@pytest.fixture
def empty_mess():
    return pb.TestMessLite()

def test_enum_lite():
    assert pb.TestEnumLite.LITE == 0
    assert pb.TestEnumLite.LITE1 == 1

def test_test_mess_lite_required(empty_mess):
    assert not empty_mess.IsInitialized()
    empty_mess.field1 = 42
    empty_mess.field2 = pb.TestEnumLite.LITE1
    assert empty_mess.IsInitialized()
    serialized = empty_mess.SerializeToString()
    msg2 = pb.TestMessLite()
    msg2.ParseFromString(serialized)
    assert empty_mess == msg2

def test_test_mess_lite_edge_cases():
    def fill(msg):
        msg.field1 = -2**31
        msg.field2 = pb.TestEnumLite.LITE
    roundtrip_test(pb.TestMessLite(), fill)

def test_test_mess_lite_invalid_parse():
    with pytest.raises(protobuf_message.DecodeError):
        pb.TestMessLite().ParseFromString(b"\x99")

def test_csv_export():
    tests = [
        ("AC-OPT-001", "Test EnumLite Values", "FR-OPT-001", "Unit", ["Check enum values"], "Correct values"),
        ("AC-OPT-002", "Test MessLite Required", "FR-OPT-002", "Unit", ["Fill required fields", "Check IsInitialized"], "Correct initialization")
    ]
    from common.utils import export_cases_to_csv
    export_cases_to_csv(tests, "automation-cases.csv")
    suites = [{
        "suite_id": "AS-OPT-001",
        "title": "Optimized Proto Tests",
        "description": "Tests for test-optimized.proto",
        "module": "Optimized",
        "test_case_ids": "AC-OPT-001,AC-OPT-002",
        "labels": "optimized,protobuf"
    }]
    from common.utils import export_suites_to_csv
    export_suites_to_csv(suites, "automation-suites.csv")

@given(
    field1=st.integers(min_value=-2**31, max_value=2**31-1),  # Required int32
    field2=st.sampled_from([pb.TestEnumLite.LITE, pb.TestEnumLite.LITE1])  # Required enum
)
def test_messlite_hypothesis(field1, field2):
    def fill(msg):
        msg.field1 = field1
        msg.field2 = field2
    roundtrip_test(pb.TestMessLite(), fill)