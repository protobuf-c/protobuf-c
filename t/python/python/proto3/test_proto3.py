import pytest
from google.protobuf import message as protobuf_message

import test_proto3_pb2 as pb
from common.utils import roundtrip_test
from hypothesis import given, strategies as st

@pytest.fixture
def empty_person():
    return pb.Person()

def test_person_defaults(empty_person):
    assert empty_person.name == ""
    assert empty_person.id == 0
    assert empty_person.email == ""
    assert empty_person.phone == []
    # Remove HasField for scalars (not supported in proto3)
    # Check submessage presence
    phone = empty_person.phone.add()
    assert phone.HasField('comment') is False

def test_person_roundtrip():
    def fill(msg):
        msg.name = "John"
        msg.id = 123
        msg.email = "john@example.com"
        phone1 = msg.phone.add()
        phone1.number = "123-456"
        phone1.type = pb.Person.PhoneType.HOME
        phone1.comment.comment = "home phone"
        phone2 = msg.phone.add()
        phone2.number = "789-012"
        phone2.type = pb.Person.PhoneType.WORK
    roundtrip_test(pb.Person(), fill)

def test_person_unicode_and_empty():
    def fill(msg):
        msg.name = "café"
        msg.email = ""
        phone = msg.phone.add()
        phone.number = ""
        phone.type = pb.Person.PhoneType.MOBILE
    roundtrip_test(pb.Person(), fill)

def test_person_submessage_has():
    msg = pb.Person()
    phone = msg.phone.add()
    assert not phone.HasField('comment')
    phone.comment.comment = "note"
    assert phone.HasField('comment')

def test_person_invalid_id():
    # Proto3 doesn't raise ValueError for invalid IDs; test serialization instead
    msg = pb.Person()
    msg.id = -2**31  # Large negative value
    serialized = msg.SerializeToString()  # Should not raise
    deserialized = pb.Person()
    deserialized.ParseFromString(serialized)
    assert deserialized.id == -2**31

def test_lookup_result():
    def fill(msg):
        msg.person.name = "found"
    roundtrip_test(pb.LookupResult(), fill)

def test_invalid_parse():
    with pytest.raises(protobuf_message.DecodeError):
        pb.Person().ParseFromString(b"\x99")

def test_csv_export():
    tests = [
        ("AC-PROTO3-001", "Test Person Defaults", "FR-PROTO3-001", "Unit", ["Initialize Person"], "Default values match"),
        ("AC-PROTO3-002", "Test Person Roundtrip", "FR-PROTO3-002", "Unit", ["Fill Person", "Serialize", "Deserialize"], "Messages equal")
    ]
    from common.utils import export_cases_to_csv
    export_cases_to_csv(tests, "automation-cases.csv")
    suites = [{
        "suite_id": "AS-PROTO3-001",
        "title": "Proto3 Tests",
        "description": "Tests for test-proto3.proto",
        "module": "Proto3",
        "test_case_ids": "AC-PROTO3-001,AC-PROTO3-002",
        "labels": "proto3,protobuf"
    }]
    from common.utils import export_suites_to_csv
    export_suites_to_csv(suites, "automation-suites.csv")

@given(
    name=st.text(max_size=100),  # Can be empty
    id=st.integers(min_value=-2**31, max_value=2**31-1),  # int32
    email=st.text(max_size=100),  # Can be empty
    phones=st.lists(
        st.tuples(
            st.text(max_size=20),  # Can be empty
            st.sampled_from([pb.Person.PhoneType.MOBILE, pb.Person.PhoneType.HOME, pb.Person.PhoneType.WORK]),
            st.text(max_size=50)  # Can be empty
        ),
        max_size=5
    )
)
@pytest.mark.slow
def test_person_hypothesis(name, id, email, phones):
    def fill(msg):
        msg.name = name
        msg.id = id
        msg.email = email
        for number, phone_type, comment in phones:
            phone = msg.phone.add()
            phone.number = number
            phone.type = phone_type
            phone.comment.comment = comment
    roundtrip_test(pb.Person(), fill)