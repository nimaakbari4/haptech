cmake_minimum_required(VERSION 3.10)
project(CoreTests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Coverage option
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)

# Find required packages
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

# CHAI3D directories (matching the Makefile)
set(CHAI3D_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../external/chai3d-Franka-integration)
set(CHAI3D_BUILD_DIR ${CHAI3D_DIR}/build)
set(GLFW_BUILD_DIR ${CHAI3D_BUILD_DIR}/extras/GLFW)
set(FRANKA_BUILD_DIR ${CHAI3D_BUILD_DIR}/external/libfranka/src)

# Include directories (matching the Makefile)
include_directories(
    ${CHAI3D_DIR}/src
    ${CHAI3D_DIR}/external/Eigen
    ${CHAI3D_DIR}/external/glew/include
    ${CHAI3D_DIR}/external/DHD/include
    ${CHAI3D_DIR}/extras/GLFW/include
    ${CHAI3D_DIR}/external/libfranka/include
    ${CHAI3D_DIR}/external/libfranka/common/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/objects
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/trial_conditions
    /usr/include
    /usr/local/include
    /usr/include/eigen3
)

# Compiler definitions
add_definitions(-DLINUX -Wno-deprecated)

# Coverage flags
if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

# Build CHAI3D first (like cmake_chai3d_build in Makefile)
include(ExternalProject)
ExternalProject_Add(
    CHAI3D_BUILD
    SOURCE_DIR ${CHAI3D_DIR}
    BINARY_DIR ${CHAI3D_BUILD_DIR}
    CMAKE_ARGS
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
        -DCMAKE_CXX_FLAGS="-DTCP_KEEPIDLE=TCP_KEEPALIVE"
        -DCMAKE_PREFIX_PATH="/usr"
        -DPoco_DIR="/usr/lib/x86_64-linux-gnu/cmake/Poco"
    BUILD_COMMAND ${CMAKE_COMMAND} --build .
    INSTALL_COMMAND ""
)

# Core source files that can be tested
set(CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/trial_conditions/trial_condition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/trial_conditions/time_condition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/trial_conditions/position_condition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/trial_conditions/velocity_condition.cpp
)

# Add core sources library
add_library(core_sources ${CORE_SOURCES})
add_dependencies(core_sources CHAI3D_BUILD)

# Link directories
link_directories(
    ${CHAI3D_BUILD_DIR}
    ${GLFW_BUILD_DIR}
    ${FRANKA_BUILD_DIR}
    /usr/lib/x86_64-linux-gnu
)

# Add test executable
add_executable(
  core_tests
  test_main.cpp
  test_trial_conditions.cpp
  simulation_manager_stubs.cpp
)

# Link libraries (matching the Makefile LDLIBS)
target_link_libraries(
  core_tests
  core_sources
  ${CHAI3D_BUILD_DIR}/libchai3d.a
  ${GLFW_BUILD_DIR}/libglfw.a
  ${CHAI3D_BUILD_DIR}/external/libfranka/libfranka.so.0.13.3
  grpc++ grpc gpr protobuf
  cares z ssl crypto
  pthread rt dl GL GLU usb-1.0
  X11 Xcursor Xrandr Xinerama
  PocoFoundation PocoNet PocoNetSSL PocoCrypto
  GTest::gtest_main
  GTest::gtest
  GTest::gmock_main
  GTest::gmock
  Threads::Threads
)

# Add tests to CTest
include(GoogleTest)
gtest_discover_tests(core_tests)

# Coverage targets
if(ENABLE_COVERAGE)
    # Find required coverage tools
    find_program(GCOV_PATH gcov)
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)
    
    if(NOT GCOV_PATH)
        message(FATAL_ERROR "gcov not found! Aborting...")
    endif()
    
    if(NOT LCOV_PATH)
        message(FATAL_ERROR "lcov not found! Aborting...")
    endif()
    
    if(NOT GENHTML_PATH)
        message(FATAL_ERROR "genhtml not found! Aborting...")
    endif()
    
    # Setup target
    add_custom_target(coverage
        # Cleanup lcov
        COMMAND ${LCOV_PATH} --directory . --zerocounters
        
        # Create baseline coverage data file
        COMMAND ${LCOV_PATH} --capture --initial --directory . --output-file coverage.base
        
        # Run tests
        COMMAND ${CMAKE_CTEST_COMMAND} --force-new-ctest-process --verbose
        
        # Capturing lcov counters and generating report
        COMMAND ${LCOV_PATH} --capture --directory . --output-file coverage.capture
        
        # Combine baseline and test coverage data
        COMMAND ${LCOV_PATH} --add-tracefile coverage.base --add-tracefile coverage.capture --output-file coverage.total
        
        # Remove unwanted files from coverage (external libraries, system headers, test files)
        COMMAND ${LCOV_PATH} --remove coverage.total '/usr/*' '*/external/*' '*/build/*' '*/test_*' '*/gtest/*' '*/gmock/*' --output-file coverage.info
        
        # Generate HTML coverage report
        COMMAND ${GENHTML_PATH} coverage.info --output-directory coverage_html --title "Core Tests Coverage" --show-details --legend
        
        COMMAND ${CMAKE_COMMAND} -E echo "Coverage report generated in coverage_html/index.html"
        
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS core_tests
        COMMENT "Generating coverage report"
    )
    
    # Clean coverage files
    add_custom_target(coverage-clean
        COMMAND ${CMAKE_COMMAND} -E remove -f coverage.base coverage.capture coverage.total coverage.info
        COMMAND ${CMAKE_COMMAND} -E remove_directory coverage_html
        COMMAND find ${CMAKE_BINARY_DIR} -name "*.gcda" -delete
        COMMAND find ${CMAKE_BINARY_DIR} -name "*.gcno" -delete
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Cleaning coverage files"
    )
endif()