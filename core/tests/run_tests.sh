#!/bin/bash

# Test runner script for C++ core tests using GTest

set -e  # Exit on any error

echo "Setting up C++ test environment..."

# Create build directory for tests
mkdir -p build
cd build

echo "Configuring CMake..."
cmake ..

echo "Building tests..."
make -j$(nproc)

echo "Running tests..."
./core_tests

echo ""
echo "✅ All C++ tests completed!"
echo ""
echo "To run tests again:"
echo "  cd tests/build && ./core_tests"
echo ""
echo "To run with verbose output:"
echo "  cd tests/build && ./core_tests --gtest_output=verbose"