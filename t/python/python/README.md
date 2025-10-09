# Protobuf Python Test Framework

## Overview

This framework tests Protocol Buffers definitions (`test-full.proto`, `test-optimized.proto`, `test-proto3.proto`, `test.proto`) using Python and pytest. It mirrors and extends the C tests in `../` (e.g., `test-generated-code.c`, `test-generated-code2.c`), covering happy path, negative, and corner cases. Tests are organized in subdirectories (`full/`, `optimized/`, `proto3/`, `simple/`) and output `automation-cases.csv` and `automation-suites.csv` for Test Management integration.

## Setup

1. **Install** `protoc`:

   - Ensure `protoc` &gt;= 3.20: `brew install protobuf` (macOS) or equivalent.
   - Verify: `protoc --version`.

2. **Set Up Virtual Environment**:

   ```bash
   python -m venv .venv
   source .venv/bin/activate
   ```

3. **Install Dependencies**:

   ```bash
   pip install -r requirements.txt
   ```

   Key packages: `protobuf==6.32.1`, `pytest==8.4.2`, `hypothesis==6.140.2`, `pytest-xdist==3.8.0`, `pytest-cov==7.0.0`.

4. **Generate Python Stubs**: From `t/python`:

   ```bash
   protoc --python_out=. --proto_path=../../../protobuf-c ../../../protobuf-c/protobuf-c.proto
   protoc --python_out=. --proto_path=. --proto_path=../../../protobuf-c test-full.proto
   protoc --python_out=. --proto_path=. --proto_path=../../../protobuf-c test-optimized.proto
   protoc --python_out=. --proto_path=. --proto_path=../../../protobuf-c test-proto3.proto
   protoc --python_out=. --proto_path=. --proto_path=../../../protobuf-c test.proto
   ```

   Note: `protobuf-c.proto` is required for `test.proto` due to custom options.

5. **Verify Setup**:

   ```bash
   pytest --version
   ```

## Running Tests

Run all tests from `t/python`:

```bash
pytest -v --cov
```

- **Output**: Verbose test results, HTML coverage report (`htmlcov/`), and CSVs (`automation-cases.csv`, `automation-suites.csv`) in each subdirectory.
- **Runtime**: \~2 seconds (46 tests).

Run specific tests:

- Single file: `pytest simple/test_simple.py -v`
- Slow tests: `pytest -m slow -v`
- Specific proto: `pytest full/ -v`

## Test Structure

- **common/**: Shared utilities (`utils.py` for roundtrip testing, CSV export, diff logging).
- **full/**: Tests for `test-full.proto` (`test_full.py`, `test_full_pb2.py`). Exhaustive coverage of field types, enums, boundaries, oneofs, packed fields, using `hypothesis` for generative testing.
- **simple/**: Tests for `test.proto` (`test_simple.py`, `test_pb2.py`). Proto2 `Person`, `LookupResult` with required fields, custom `DescriptorPool`, and `hypothesis` for generative testing.
- **proto3/**: Tests for `test-proto3.proto` (`test_proto3.py`, `test_proto3_pb2.py`). Proto3 `Person`, `LookupResult` with no required fields, implicit defaults, and `hypothesis` for generative testing.
- **optimized/**: Tests for `test-optimized.proto` (`test_optimized.py`, `test_optimized_pb2.py`). Code-size optimized `TestMessLite`, required fields, lite enums, and `hypothesis` for generative testing.
- **pytest.ini**: Configures test paths, coverage, markers (`slow`), and warnings.
- **conftest.py**: Ignores pb2 files during collection.
- **requirements.txt**: Lists all dependencies.

## CSV Usage

- **automation-cases.csv**: Maps test cases to IDs, requirements, steps, and expected results.
- **automation-suites.csv**: Groups tests by proto/module.
- Import CSVs into ClickUp for tracking and reporting.

## Coverage

- Targets 90%+ coverage of test logic (`common/utils.py`, test files). Note: Coverage on generated `*_pb2.py` files is included but less meaningful, as they’re auto-generated.
- View report: Open `htmlcov/index.html` after running `pytest -v --cov`.

## Troubleshooting

- **Duplicate Symbol Error (e.g.,** `foo.Person`**)**: Ensure `test_simple.py` uses a custom `DescriptorPool` with hardcoded `test.proto` descriptor to avoid conflicts with `test_proto3_pb2.py`.
- **Missing** `protoc`: Install via package manager or download from https://github.com/protocolbuffers/protobuf/releases.
- **Protobuf Version Mismatch**: Use `protobuf==6.32.1` (`pip show protobuf` to check).
- **Test Failures**: Check verbose output (`-v`) or logs in `common/utils.py` (`log_diff`).
- **Slow Tests**: Marked with `@pytest.mark.slow` (e.g., large message tests); run separately if needed.

## Notes

- Mirrors C tests (`../test-generated-code.c` for simple, `../test-generated-code2.c` for exhaustive), with extensions via negative/corner cases.