# Messaging System Unit Tests

This directory contains unit tests for the messaging system modules.

## Test Structure

- `container_test.cpp` - Tests for the container module
- `database_test.cpp` - Tests for the database module  
- `network_test.cpp` - Tests for the network module
- `thread_system_test.cpp` - Tests for the thread system integration
- `integration_test.cpp` - Integration tests across modules

## Building and Running Tests

To build and run tests:

```bash
# Build with tests enabled
./build.sh --tests

# Run all tests
cd build
ctest --output-on-failure

# Run specific test
./tests/container_test
```

## Writing New Tests

Tests use Google Test framework. Example test structure:

```cpp
#include <gtest/gtest.h>
#include <messaging_system/container/container.h>

TEST(ContainerTest, BasicOperations) {
    container::container c;
    c.set("key", "value");
    EXPECT_EQ(c.get<std::string>("key"), "value");
}
```