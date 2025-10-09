from google.protobuf.descriptor_pool import DescriptorPool
from google.protobuf import message_factory
from google.protobuf.internal import enum_type_wrapper
from google.protobuf import descriptor_pb2
import common.protobuf_c_pb2 as protobuf_c_pb  # Keep this; no conflict
from common.utils import roundtrip_test
import pytest
import google.protobuf.message as protobuf_message
from hypothesis import given, strategies as st

# Hardcode the serialized file descriptor from test_pb2.py (copy the b'...' here)
serialized_test_proto = b'\n\ntest.proto\x12\x03\x66oo\x1a\x10protobuf-c.proto\"\x9f\x02\n\x06Person\x12\x0c\n\x04name\x18\x01 \x02(\t\x12\n\n\x02id\x18\x02 \x02(\x05\x12\r\n\x05\x65mail\x18\x03 \x01(\t\x12&\n\x05phone\x18\x04 \x03(\x0b\x32\x17.foo.Person.PhoneNumber\x1a\x96\x01\n\x0bPhoneNumber\x12\x0e\n\x06number\x18\x01 \x02(\t\x12)\n\x04type\x18\x02 \x01(\x0e\x32\x15.foo.Person.PhoneType:\x04HOME\x12\x30\n\x07\x63omment\x18\x03 \x01(\x0b\x32\x1f.foo.Person.PhoneNumber.Comment\x1a\x1a\n\x07\x43omment\x12\x0f\n\x07\x63omment\x18\x01 \x02(\t\"+\n\tPhoneType\x12\n\n\x06MOBILE\x10\x00\x12\x08\n\x04HOME\x10\x01\x12\x08\n\x04WORK\x10\x02\"+\n\x0cLookupResult\x12\x1b\n\x06person\x18\x01 \x01(\x0b\x32\x0b.foo.Person\"\x14\n\x04Name\x12\x0c\n\x04name\x18\x01 \x01(\t23\n\tDirLookup\x12&\n\x06\x42yName\x12\t.foo.Name\x1a\x11.foo.LookupResultB\x08\xda?\x05\x32\x03\x66oo'  # Paste your exact b string here

# Create a custom descriptor pool
descriptor_pool = DescriptorPool()
# Add google/protobuf/descriptor.proto descriptor first
descriptor_pool.AddSerializedFile(descriptor_pb2.DESCRIPTOR.serialized_pb)
# Add protobuf-c.proto descriptor
descriptor_pool.AddSerializedFile(protobuf_c_pb.DESCRIPTOR.serialized_pb)
# Add test.proto serialized (hardcoded)
descriptor_pool.AddSerializedFile(serialized_test_proto)

# Define a namespace for pb (since no import test_pb2)
class PB:
    pass
pb = PB()

# Override message/enum classes with custom pool
pb.Person = message_factory.GetMessageClass(descriptor_pool.FindMessageTypeByName('foo.Person'))
pb.LookupResult = message_factory.GetMessageClass(descriptor_pool.FindMessageTypeByName('foo.LookupResult'))
pb.Name = message_factory.GetMessageClass(descriptor_pool.FindMessageTypeByName('foo.Name'))
pb.Person.PhoneNumber = message_factory.GetMessageClass(descriptor_pool.FindMessageTypeByName('foo.Person.PhoneNumber'))
pb.Person.PhoneNumber.Comment = message_factory.GetMessageClass(descriptor_pool.FindMessageTypeByName('foo.Person.PhoneNumber.Comment'))
pb.Person.PhoneType = enum_type_wrapper.EnumTypeWrapper(descriptor_pool.FindEnumTypeByName('foo.Person.PhoneType'))

@pytest.fixture
def empty_person():
    return pb.Person()

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

def test_lookup_result():
    def fill(msg):
        msg.person.name = "found"
        msg.person.id = 1
    roundtrip_test(pb.LookupResult(), fill)

def test_person_defaults():
    msg = pb.Person()
    assert msg.name == ""
    assert msg.id == 0

def test_person_has_fields():
    msg = pb.Person()
    assert msg.HasField('name') is False
    assert msg.HasField('id') is False

def test_person_edge_cases():
    def fill(msg):
        msg.name = ""
        msg.id = -2**31
        phone = msg.phone.add()
        phone.number = "\x00"
        phone.type = pb.Person.PhoneType.MOBILE
    roundtrip_test(pb.Person(), fill)

def test_invalid_parse():
    with pytest.raises(protobuf_message.DecodeError):
        pb.Person().ParseFromString(b"\x99")

def test_csv_export():
    tests = [
        ("AC-SIMPLE-001", "Test Person Defaults", "FR-SIMPLE-001", "Unit", ["Initialize Person"], "Default values match"),
        ("AC-SIMPLE-002", "Test Person Roundtrip", "FR-SIMPLE-002", "Unit", ["Fill Person", "Serialize", "Deserialize"], "Messages equal")
    ]
    from common.utils import export_cases_to_csv
    export_cases_to_csv(tests, "automation-cases.csv")
    suites = [{
        "suite_id": "AS-SIMPLE-001",
        "title": "Simple Proto Tests",
        "description": "Tests for test.proto",
        "module": "Simple",
        "test_case_ids": "AC-SIMPLE-001,AC-SIMPLE-002",
        "labels": "simple,protobuf"
    }]
    from common.utils import export_suites_to_csv
    export_suites_to_csv(suites, "automation-suites.csv")

@given(
    name=st.text(min_size=1, max_size=100),  # Required, non-empty
    id=st.integers(min_value=-2**31, max_value=2**31-1),  # int32
    email=st.one_of(st.none(), st.text(max_size=100)),  # Optional
    phones=st.lists(
        st.tuples(
            st.text(min_size=1, max_size=20),  # Required number
            st.sampled_from([pb.Person.PhoneType.MOBILE, pb.Person.PhoneType.HOME, pb.Person.PhoneType.WORK]),
            st.text(min_size=1, max_size=50)  # Required comment
        ),
        max_size=5
    )
)
def test_person_hypothesis(name, id, email, phones):
    def fill(msg):
        msg.name = name
        msg.id = id
        if email is not None:
            msg.email = email
        for number, phone_type, comment in phones:
            phone = msg.phone.add()
            phone.number = number
            phone.type = phone_type
            phone.comment.comment = comment
    roundtrip_test(pb.Person(), fill)