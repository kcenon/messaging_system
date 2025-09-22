# Messaging System Unit Tests

Comprehensive test suite for the messaging system modules with 100% pass rate and extensive coverage.

## Test Structure

### Core Module Tests
- **`container_test.cpp`** - Container module tests (15 tests)
  - Serialization/deserialization
  - Value type operations  
  - Thread safety
  - Memory management
  
- **`database_test.cpp`** - Database module tests (14 tests)
  - Connection management
  - Query execution
  - Error handling
  - Singleton pattern
  
- **`network_test.cpp`** - Network module tests (14 tests)
  - Client/server communication
  - Session management
  - Connection handling
  - Protocol compliance
  
- **`integration_test.cpp`** - Cross-module integration tests (8 tests)
  - Container + Database integration
  - Container + Network integration
  - Multi-module workflows
  - End-to-end scenarios

## Test Results

### Latest Test Run (100% Pass Rate)
```
[==========] Running 51 tests from 4 test suites.
[----------] Global test environment set-up.

[----------] 15 tests from ContainerTest (0 ms total)
[  PASSED  ] 15 tests.

[----------] 14 tests from DatabaseTest (2 ms total)  
[  PASSED  ] 14 tests.

[----------] 14 tests from NetworkTest (2810 ms total)
[  PASSED  ] 14 tests.

[----------] 8 tests from IntegrationTest (1335 ms total)
[  PASSED  ] 8 tests.

[----------] Global test environment tear-down
[==========] 51 tests from 4 test suites ran. (5560 ms total)
[  PASSED  ] 51 tests.

100% tests passed, 0 tests failed out of 4 test suites
Total Test time (real) = 5.56 sec
```

## Building and Running Tests

### Quick Test Execution
```bash
# Build and run all tests
./build.sh --tests

# View detailed test output
cd build
ctest --output-on-failure --verbose

# Run specific test module
cd build/bin
./container_test
./database_test  
./network_test
./integration_test
```

### Advanced Test Options
```bash
# Run specific test cases with filters
./container_test --gtest_filter="ContainerTest.Serialization*"
./database_test --gtest_filter="DatabaseTest.SingletonAccess"
./network_test --gtest_filter="NetworkTest.ServerClient*"

# Run tests with detailed output
./container_test --gtest_list_tests
./database_test --gtest_print_time

# Performance testing
./network_test --gtest_filter="*Performance*" --gtest_repeat=100
```

## Test Coverage Details

### Container Module Tests (container_test.cpp)

| Test Case | Coverage | Description |
|-----------|----------|-------------|
| `DefaultConstruction` | Basic setup | Container initialization |
| `SetAndGetSourceTarget` | Header management | Source/target ID handling |
| `MessageType` | Type system | Message type assignment |
| `SwapHeader` | Header operations | Source/target swapping |
| `ConstructWithMessageType` | Constructors | Type-specific construction |
| `ConstructWithFullHeader` | Constructors | Full header construction |
| `Copy` | Memory management | Deep/shallow copying |
| `Serialization` | Data persistence | Binary serialization |
| `ArraySerialization` | Data persistence | Byte array format |
| `Initialize` | State management | Container reset |
| `ClearValue` | Memory management | Value cleanup |
| `MoveCopyConstructor` | Move semantics | Efficient transfers |
| `CopyConstructor` | Copy semantics | Value preservation |
| `XMLGeneration` | Output formats | XML serialization |
| `JSONGeneration` | Output formats | JSON serialization |

### Database Module Tests (database_test.cpp)

| Test Case | Coverage | Description |
|-----------|----------|-------------|
| `DefaultConstruction` | Initialization | Manager creation |
| `SetDatabaseMode` | Configuration | PostgreSQL mode setup |
| `SingletonAccess` | Pattern verification | Singleton behavior |
| `ConnectWithInvalidString` | Error handling | Invalid connections |
| `DisconnectWithoutConnection` | Error handling | Disconnection safety |
| `CreateQueryWithoutConnection` | Error handling | Query safety checks |
| `InsertQueryWithoutConnection` | Error handling | Insert safety |
| `UpdateQueryWithoutConnection` | Error handling | Update safety |
| `DeleteQueryWithoutConnection` | Error handling | Delete safety |
| `SelectQueryWithoutConnection` | Error handling | Select safety |
| `DatabaseTypeInitialization` | State management | Type tracking |
| `MultipleSetModeOperations` | State transitions | Mode switching |
| `EmptyQueryHandling` | Edge cases | Empty query processing |
| `SequentialOperations` | Workflow testing | Complete CRUD cycle |

### Network Module Tests (network_test.cpp)

| Test Case | Coverage | Description |
|-----------|----------|-------------|
| `ServerConstruction` | Object creation | Server initialization |
| `ClientConstruction` | Object creation | Client initialization |
| `ServerStartStop` | Lifecycle | Server state management |
| `ServerDoubleStart` | Error handling | Duplicate start prevention |
| `ServerStopWithoutStart` | Error handling | Invalid stop calls |
| `ServerWaitForStop` | Synchronization | Graceful shutdown |
| `ClientConnectToInvalidAddress` | Error handling | Invalid connections |
| `ClientConnectToValidButClosedPort` | Error handling | Connection failures |
| `ClientDisconnectWithoutConnection` | Error handling | Invalid disconnects |
| `ServerClientIntegration` | Communication | Full duplex messaging |
| `MultipleClients` | Scalability | Concurrent connections |
| `ServerPortBinding` | Resource management | Port allocation |
| `ServerQuickStartStop` | Performance | Rapid cycling |
| `ClientQuickConnectDisconnect` | Performance | Connection cycling |

### Integration Tests (integration_test.cpp)

| Test Case | Coverage | Description |
|-----------|----------|-------------|
| `ContainerAndDatabaseIntegration` | Cross-module | Container persistence |
| `ContainerAndNetworkIntegration` | Cross-module | Container transmission |
| `DatabaseAndNetworkIntegration` | Cross-module | Network + DB workflow |
| `AllModulesIntegration` | System-wide | Complete system test |
| `ContainerSerializationCycle` | Data integrity | Round-trip testing |
| `MultipleClientSessions` | Scalability | Multi-session handling |
| `DatabaseSingletonBehavior` | Pattern verification | Singleton consistency |
| `ContainerHeaderSwapAndSerialization` | Data operations | Header manipulation |

## Writing New Tests

### Basic Test Structure
```cpp
#include <gtest/gtest.h>
#include <container/container.h>

class MyModuleTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test objects
        container_ = std::make_shared<container_module::value_container>();
    }
    
    void TearDown() override {
        // Clean up resources
        container_.reset();
    }
    
    std::shared_ptr<container_module::value_container> container_;
};

TEST_F(MyModuleTest, BasicFunctionality) {
    // Arrange
    container_->set_message_type("test_message");
    
    // Act
    std::string result = container_->message_type();
    
    // Assert
    EXPECT_EQ(result, "test_message");
}
```

### Parameterized Tests
```cpp
class ValueTypeTest : public ::testing::TestWithParam<container_module::value_types> {
protected:
    void SetUp() override {
        type_ = GetParam();
    }
    
    container_module::value_types type_;
};

TEST_P(ValueTypeTest, ValueCreation) {
    auto value = container_module::value_factory::create("test_key", type_, "test_data");
    EXPECT_NE(value, nullptr);
    EXPECT_EQ(value->key(), "test_key");
}

INSTANTIATE_TEST_SUITE_P(
    AllValueTypes,
    ValueTypeTest,
    ::testing::Values(
        container_module::bool_value,
        container_module::string_value,
        container_module::int64_value
    )
);
```

### Performance Tests
```cpp
TEST(PerformanceTest, ContainerSerializationSpeed) {
    auto container = std::make_shared<container_module::value_container>();
    
    // Add test data
    for (int i = 0; i < 1000; ++i) {
        auto value = container_module::value_factory::create(
            "key_" + std::to_string(i), 
            container_module::string_value, 
            "test_value_" + std::to_string(i)
        );
        container->add_value(value);
    }
    
    // Measure serialization time
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 100; ++i) {
        std::string serialized = container->serialize();
        EXPECT_FALSE(serialized.empty());
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Serialization time: " << duration.count() << " microseconds" << std::endl;
    
    // Assert performance requirement (adjust as needed)
    EXPECT_LT(duration.count(), 10000); // Less than 10ms for 100 operations
}
```

### Threading Tests
```cpp
TEST(ThreadingTest, ConcurrentContainerAccess) {
    auto container = std::make_shared<container_module::value_container>();
    const int num_threads = 10;
    const int operations_per_thread = 100;
    
    std::vector<std::thread> threads;
    std::atomic<int> successful_operations{0};
    
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < operations_per_thread; ++i) {
                try {
                    std::string key = "thread_" + std::to_string(t) + "_key_" + std::to_string(i);
                    std::string value = "value_" + std::to_string(i);
                    
                    auto val = container_module::value_factory::create(
                        key, container_module::string_value, value
                    );
                    container->add_value(val);
                    
                    successful_operations++;
                } catch (const std::exception& e) {
                    // Log but don't fail the test for expected race conditions
                    std::cerr << "Thread " << t << " operation " << i 
                              << " failed: " << e.what() << std::endl;
                }
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Should have some successful operations
    EXPECT_GT(successful_operations.load(), 0);
    
    std::cout << "Successful operations: " << successful_operations.load() 
              << "/" << (num_threads * operations_per_thread) << std::endl;
}
```

## Test Guidelines

### Best Practices
1. **Test Independence**: Each test should be completely independent
2. **Clear Naming**: Use descriptive test names that explain the scenario
3. **AAA Pattern**: Arrange, Act, Assert structure
4. **Edge Cases**: Test boundary conditions and error cases
5. **Performance**: Include performance regression tests
6. **Documentation**: Comment complex test logic

### Test Categories
- **Unit Tests**: Test individual components in isolation
- **Integration Tests**: Test component interactions
- **Performance Tests**: Measure and verify performance characteristics
- **Stress Tests**: Test behavior under high load
- **Error Tests**: Verify proper error handling

### Continuous Integration
```bash
# CI test script
#!/bin/bash
set -e

echo "Building messaging system with tests..."
./build.sh --clean
./build.sh --tests

echo "Running all tests..."
cd build
ctest --output-on-failure

echo "Generating test coverage report..."
# Add coverage reporting if enabled

echo "All tests passed successfully!"
```

## Debugging Tests

### Debugging Failed Tests
```bash
# Run specific failing test with debug output
./container_test --gtest_filter="ContainerTest.FailingTest" --gtest_break_on_failure

# Use GDB for detailed debugging
gdb --args ./container_test --gtest_filter="ContainerTest.FailingTest"
(gdb) run
(gdb) bt  # backtrace when test fails
```

### Test Output Analysis
```bash
# Capture test output for analysis
./build.sh --tests 2>&1 | tee test_output.log

# Filter for failures only
grep -A 10 -B 2 "FAILED" test_output.log

# Performance analysis
grep "time" test_output.log
```

## Future Test Enhancements

### Planned Additions
- **Property-based Testing**: Use Hypothesis-style testing
- **Fuzzing Tests**: Input validation with random data
- **Memory Tests**: Valgrind integration for memory leak detection
- **Coverage Reports**: Automated test coverage reporting
- **Benchmark Tests**: Continuous performance monitoring

### Test Infrastructure Improvements
- **Parallel Test Execution**: Run tests concurrently for faster feedback
- **Test Data Management**: Shared test data and fixtures
- **Mock Services**: Database and network service mocking
- **Test Reporting**: Rich HTML test reports with metrics

## License

BSD 3-Clause License - see main project LICENSE file.