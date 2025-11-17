# Tests for messaging_system

# Find GTest
find_package(GTest REQUIRED)

enable_testing()

# Add unit tests
add_subdirectory(unit)

# Add benchmarks
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/CMakeLists.txt)
    add_subdirectory(benchmarks)
endif()
