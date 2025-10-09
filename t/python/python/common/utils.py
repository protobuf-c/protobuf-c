import csv
import math
from google.protobuf import message as protobuf_message

def roundtrip_test(msg, fill_func):
    """Serialize, deserialize, and assert equality, handling NaN floats."""
    fill_func(msg)
    serialized = msg.SerializeToString()
    msg2 = type(msg)()
    msg2.ParseFromString(serialized)
    for field in msg.DESCRIPTOR.fields:
        if field.type == field.TYPE_FLOAT or field.type == field.TYPE_DOUBLE:
            for v1, v2 in zip(getattr(msg, field.name), getattr(msg2, field.name)):
                if math.isnan(v1):
                    assert math.isnan(v2), f"NaN mismatch in {field.name}"
                else:
                    assert v1 == v2, f"Mismatch in {field.name}: {v1} != {v2}"
        else:
            assert getattr(msg, field.name) == getattr(msg2, field.name), f"Mismatch in {field.name}"
    return msg2

def log_diff(msg1, msg2):
    """Log field differences between two messages."""
    diffs = []
    for field in msg1.DESCRIPTOR.fields:
        v1 = getattr(msg1, field.name)
        v2 = getattr(msg2, field.name)
        if field.type == field.TYPE_FLOAT or field.type == field.TYPE_DOUBLE:
            if any(math.isnan(x) != math.isnan(y) for x, y in zip(v1, v2)):
                diffs.append(f"{field.name}: NaN mismatch")
            elif any(x != y for x, y in zip(v1, v2) if not (math.isnan(x) and math.isnan(y))):
                diffs.append(f"{field.name}: {v1} != {v2}")
        elif v1 != v2:
            diffs.append(f"{field.name}: {v1} != {v2}")
    return "\n".join(diffs) if diffs else "No differences"

def export_cases_to_csv(tests, filename):
    """Export test cases to automation-cases.csv."""
    with open(filename, 'w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=[
            'test_case_id', 'section_id', 'title', 'requirement_id', 'priority', 'type',
            'estimate', 'preconditions', 'steps', 'expected_results', 'labels'
        ])
        writer.writeheader()
        for i, (test_id, title, req_id, test_type, steps, expected) in enumerate(tests, 1):
            writer.writerow({
                'test_case_id': test_id,
                'section_id': f"AS-{test_id.split('-')[1]}-001",
                'title': title,
                'requirement_id': req_id,
                'priority': 'High',
                'type': test_type,
                'estimate': '1m',
                'preconditions': 'Protobuf message initialized',
                'steps': '|'.join(steps),
                'expected_results': expected,
                'labels': f"{test_type.lower()},protobuf"
            })

def export_suites_to_csv(suites, filename):
    """Export test suites to automation-suites.csv."""
    with open(filename, 'w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=[
            'suite_id', 'title', 'description', 'module', 'test_case_ids', 'labels'
        ])
        writer.writeheader()
        for suite in suites:
            writer.writerow(suite)
