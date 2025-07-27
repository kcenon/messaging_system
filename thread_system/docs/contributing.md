# Contributing to Thread System

Thank you for your interest in contributing to the Thread System project! This document provides guidelines and information for contributors.

## Getting Started

### Prerequisites

Before contributing, ensure you have:
- **C++20 compatible compiler**: GCC 9+, Clang 10+, or MSVC 2019+
- **CMake 3.16+**: For build system management
- **vcpkg**: Package manager for dependencies
- **Git**: Version control system

### Development Setup

```bash
# Fork and clone the repository
git clone https://github.com/YOUR_USERNAME/thread_system.git
cd thread_system

# Install dependencies
./dependency.sh  # Linux/macOS
./dependency.bat # Windows

# Build the project
./build.sh       # Linux/macOS
./build.bat      # Windows

# Run tests (Linux only)
cd build && ctest --verbose
```

## Contribution Workflow

### 1. Create a Feature Branch

```bash
git checkout -b feature/your-feature-name
```

### 2. Make Changes

- Follow the coding standards
- Add tests for new functionality
- Update documentation as needed
- Ensure all existing tests pass

### 3. Commit Changes

```bash
git commit -m "Add feature: Brief description of changes

- Detailed explanation of what was added/changed
- Why the change was necessary
- Any breaking changes or migration notes"
```

### 4. Submit Pull Request

- Push to your fork
- Create a pull request through GitHub
- Provide clear description of changes
- Link related issues

## Coding Standards

### C++ Style Guidelines

```cpp
// Classes: snake_case with _t suffix for templates
class thread_pool;
template<typename T> class typed_job_queue_t;

// Functions and variables: snake_case
auto process_job() -> result_void;
std::atomic<bool> is_running_;

// Constants: SCREAMING_SNAKE_CASE
constexpr size_t DEFAULT_THREAD_COUNT = 4;

// Prefer auto for type deduction
auto result = create_thread_pool();

// Use trailing return types for complex signatures
auto process_batch(std::vector<job>&& jobs) -> result_void;

// Use smart pointers for memory management
std::unique_ptr<thread_base> worker;
std::shared_ptr<job_queue> queue;
```

### Error Handling

```cpp
// Use result<T> pattern for error handling
auto create_worker() -> result<std::unique_ptr<thread_worker>> {
    if (invalid_configuration) {
        return error{error_code::configuration_invalid, "Invalid worker configuration"};
    }
    return std::make_unique<thread_worker>(config);
}

// Check results properly
auto worker_result = create_worker();
if (worker_result.has_error()) {
    return worker_result.get_error();
}
```

## Testing Guidelines

### Unit Tests

```cpp
#include <gtest/gtest.h>
#include "your_component.h"

class YourComponentTest : public ::testing::Test {
protected:
    void SetUp() override {
        component_ = std::make_unique<your_component>(test_config_);
    }
    
    std::unique_ptr<your_component> component_;
    configuration test_config_;
};

TEST_F(YourComponentTest, StartStopCycle) {
    ASSERT_TRUE(component_->start().has_value() == false);
    EXPECT_TRUE(component_->is_running());
    
    component_->stop();
    EXPECT_FALSE(component_->is_running());
}
```

### Performance Tests

- Benchmark critical performance paths
- Ensure no performance regressions
- Test under realistic load conditions
- Measure memory usage and efficiency

## Documentation Guidelines

### Code Documentation

```cpp
/**
 * @brief Creates a new thread pool with specified configuration.
 * 
 * @param config Configuration parameters for the thread pool
 * @return result<std::unique_ptr<thread_pool>> The created thread pool or an error
 * 
 * @note The thread pool must be stopped before destruction
 * 
 * @example
 * @code
 * auto config = thread_pool_configuration{}.with_worker_count(4);
 * auto pool_result = create_thread_pool(config);
 * if (!pool_result.has_error()) {
 *     auto pool = std::move(pool_result.get_value());
 *     pool->start();
 * }
 * @endcode
 */
auto create_thread_pool(const thread_pool_configuration& config) 
    -> result<std::unique_ptr<thread_pool>>;
```

## Review Process

### Pull Request Requirements

Before submitting:
- [ ] All tests pass locally
- [ ] Code follows established style guidelines
- [ ] New functionality includes appropriate tests
- [ ] Documentation is updated for public API changes
- [ ] Performance impact is measured and acceptable

### Review Criteria

Reviewers evaluate:
1. **Correctness**: Does the code solve the intended problem?
2. **Performance**: Are there any performance regressions?
3. **Maintainability**: Is the code readable and well-structured?
4. **Thread Safety**: Are concurrent access patterns safe?
5. **API Design**: Is the public interface intuitive and consistent?

## Getting Help

### Support Channels

- **GitHub Issues**: For bug reports and feature requests
- **GitHub Discussions**: For questions and general discussion
- **Code Review**: For implementation guidance during development

### Response Times

- **Bug reports**: 24-48 hours
- **Feature requests**: 3-5 business days
- **Security issues**: 12-24 hours

---

Thank you for contributing to Thread System!