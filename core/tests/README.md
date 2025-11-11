# C++ Core Unit Tests

This directory contains unit tests for the haptech C++ core using Google Test (GTest).

## Setup

The testing framework uses:
- **Google Test (GTest)**: Main C++ testing framework
- **CMake**: Build system for tests
- **FetchContent**: Automatically downloads and builds GTest

## Running Tests

### Quick start
```bash
cd /home/peter/haptech/core/tests
./run_tests.sh
```

### Manual build and run
```bash
cd /home/peter/haptech/core/tests
mkdir -p build && cd build
cmake ..
make -j$(nproc)
./core_tests
```

### Test options
```bash
# Run with verbose output
./core_tests --gtest_output=verbose

# Run specific test
./core_tests --gtest_filter="BasicTest.SimpleAssertion"

# List all tests
./core_tests --gtest_list_tests
```

## Test Structure

### Current test files:
- **`test_main.cpp`**: Main entry point for test runner
- **`basic_test.cpp`**: Simple "1 == 1" test to verify setup
- **`CMakeLists.txt`**: CMake configuration for building tests
- **`run_tests.sh`**: Convenience script to build and run tests

### Adding new tests:
1. Create new `.cpp` file in tests directory
2. Add to `CMakeLists.txt` in the `add_executable` section
3. Include `<gtest/gtest.h>` and write TEST() macros

### Test example:
```cpp
#include <gtest/gtest.h>

TEST(TestSuiteName, TestName) {
    ASSERT_EQ(expected, actual);
    EXPECT_TRUE(condition);
}
```

## Project Structure

```
core/tests/
├── CMakeLists.txt          # CMake build configuration
├── test_main.cpp           # Test runner main()
├── basic_test.cpp          # Basic setup verification test
├── run_tests.sh            # Test runner script
├── README.md               # This file
└── build/                  # CMake build directory (created on first run)
    └── core_tests          # Compiled test executable
```

## GTest Features

### Assertions:
- `ASSERT_*`: Fatal assertions (stop on failure)
- `EXPECT_*`: Non-fatal assertions (continue on failure)

### Common assertions:
```cpp
ASSERT_EQ(expected, actual)     // Equal
ASSERT_NE(val1, val2)          // Not equal
ASSERT_TRUE(condition)         // True
ASSERT_FALSE(condition)        // False
ASSERT_LT(val1, val2)          // Less than
ASSERT_LE(val1, val2)          // Less or equal
ASSERT_GT(val1, val2)          // Greater than
ASSERT_GE(val1, val2)          // Greater or equal
```

## Integration with Core

To test core functionality:
1. Include relevant header files from `../src/`
2. Add necessary source files to CMakeLists.txt if needed
3. Link against required libraries (CHAI3D, protobuf, etc.)

## Status

✅ **Setup Complete**: Basic GTest framework with simple assertion test
📋 **TODO**: Add specific core functionality tests (ball_object, parser, etc.)
🔄 **Ready**: Framework ready for writing real unit tests