# Unit-Tests 
This directory contains the unit tests for the project, split into two categories:

- **verilator/** - RTL simulation tests using Verilator
- **software/** - Software component tests

## Requirements

### Verilator Tests
- Verilator installed on the host
- CMake 3.14 or higher

### Software Tests
- CMake 3.14 or higher
- Native C++ compiler

## Build and Run

### Verilator Tests (RTL simulation)

```bash
# Configure (from project root)
cmake --preset unittest_verilator

# Build all tests
cmake --build build/unittest-verilator -j8

# Run all tests
ctest --test-dir build/unittest-verilator
```

### Software Tests

```bash
# Configure (from project root)
cmake --preset unittest_software

# Build all tests
cmake --build build/unittest-software -j8

# Run all tests
ctest --test-dir build/unittest-software
```

### Running Individual Tests

```bash
# Build a single Verilator test
cmake --build build/unittest-verilator --target texEnv

# Run a single test
ctest --test-dir build/unittest-verilator -R texEnv

# Or run directly
./build/unittest-verilator/unittest/verilator/obj_dir_texEnv/VTexEnv
```

### Useful CTest Options

```bash
# Run tests in parallel
ctest --test-dir build/unittest-verilator -j8

# Verbose output
ctest --test-dir build/unittest-verilator -V

# Show output only on failure
ctest --test-dir build/unittest-verilator --output-on-failure
```