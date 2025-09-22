# Contributing to Monitoring System

Thank you for your interest in contributing to the Monitoring System! This document provides guidelines and information for contributors.

## Table of Contents
- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Process](#development-process)
- [Code Style](#code-style)
- [Testing](#testing)
- [Documentation](#documentation)
- [Submitting Changes](#submitting-changes)

## Code of Conduct

This project adheres to a code of conduct. By participating, you are expected to:
- Use welcoming and inclusive language
- Be respectful of differing viewpoints and experiences
- Gracefully accept constructive criticism
- Focus on what is best for the community
- Show empathy towards other community members

## Getting Started

1. **Fork the Repository**
   ```bash
   git clone https://github.com/kcenon/monitoring_system.git
   cd monitoring_system
   ```

2. **Create a Branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

3. **Set Up Development Environment**
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -DBUILD_BENCHMARKS=ON
   cmake --build .
   ```

4. **Run Tests**
   ```bash
   ctest --output-on-failure
   ```

## Development Process

### 1. Before You Start

- Check existing issues and pull requests
- For major changes, open an issue first to discuss
- Ensure your idea aligns with the project's direction

### 2. Making Changes

- Follow the existing code style
- Write meaningful commit messages
- Add tests for new functionality
- Update documentation as needed
- Ensure all tests pass

### 3. Commit Message Format

Follow the conventional commits format:

```
type(scope): description

[optional body]

[optional footer(s)]
```

Types:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes
- `refactor`: Code refactoring
- `perf`: Performance improvements
- `test`: Test additions/changes
- `build`: Build system changes
- `ci`: CI configuration changes
- `chore`: Other changes

Example:
```
feat(collector): add network metrics collector

Implement a collector that gathers network interface statistics
including bytes sent/received and packet counts.

Closes #45
```

## Code Style

### C++ Guidelines

We follow the Google C++ Style Guide with modifications:
- Use 4 spaces for indentation (not 2)
- Use snake_case for all names (not CamelCase)
- Place opening braces on the same line
- Always use braces for control structures

### Code Formatting

Use clang-format with the provided `.clang-format` file:

```bash
# Format a file
clang-format -i sources/monitoring/monitoring.cpp

# Format all files
find . -name "*.cpp" -o -name "*.h" | xargs clang-format -i
```

### Naming Conventions

```cpp
namespace monitoring_module {

class my_collector : public metrics_collector {
private:
    int member_variable_;
    static constexpr int CONSTANT_VALUE = 42;
    
public:
    void public_method();
    
private:
    void private_method();
};

void free_function();
int global_variable;

} // namespace monitoring_module
```

## Testing

### Unit Tests

All new code should include unit tests:

```cpp
TEST(MonitoringTest, MetricsUpdate) {
    auto monitor = std::make_shared<monitoring>();
    
    system_metrics metrics;
    metrics.cpu_usage_percent = 50;
    monitor->update_system_metrics(metrics);
    
    auto snapshot = monitor->get_current_snapshot();
    EXPECT_EQ(snapshot.system.cpu_usage_percent, 50);
}
```

### Performance Tests

Performance-critical code should include benchmarks:

```cpp
static void BM_MetricsUpdate(benchmark::State& state) {
    auto monitor = std::make_shared<monitoring>();
    monitor->start();
    
    system_metrics metrics;
    for (auto _ : state) {
        metrics.cpu_usage_percent = rand() % 100;
        monitor->update_system_metrics(metrics);
    }
}
BENCHMARK(BM_MetricsUpdate);
```

### Integration Tests

Test interaction with Thread System:

```cpp
TEST(IntegrationTest, ThreadSystemMonitoring) {
    auto monitor = std::make_shared<monitoring>();
    monitor->start();
    
    // Register in service container
    service_container::global()
        .register_singleton<monitoring_interface>(monitor);
    
    // Create thread pool with monitoring
    thread_context context;
    auto pool = std::make_shared<thread_pool>("TestPool", context);
    
    // Verify metrics are collected
    pool->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto snapshot = monitor->get_current_snapshot();
    EXPECT_GT(snapshot.thread_pool.worker_threads, 0);
}
```

## Documentation

### Code Documentation

Use Doxygen-style comments:

```cpp
/**
 * @brief Real-time performance monitoring system
 * 
 * This class provides low-overhead metrics collection with
 * configurable history storage and extensible collectors.
 * 
 * @see metrics_collector for custom metric sources
 */
class monitoring : public monitoring_interface {
public:
    /**
     * @brief Construct a new monitoring instance
     * 
     * @param history_size Number of snapshots to retain
     * @param collection_interval_ms Collection frequency in milliseconds
     * @throws std::invalid_argument if history_size is 0
     */
    explicit monitoring(std::size_t history_size = 1000,
                       std::uint32_t collection_interval_ms = 1000);
```

### Documentation Updates

When adding features:
1. Update relevant documentation files
2. Add examples to the docs
3. Update the API reference if needed
4. Document performance implications

## Submitting Changes

### Pull Request Process

1. **Update Your Fork**
   ```bash
   git remote add upstream https://github.com/kcenon/monitoring_system.git
   git fetch upstream
   git rebase upstream/main
   ```

2. **Push Your Changes**
   ```bash
   git push origin feature/your-feature-name
   ```

3. **Create Pull Request**
   - Use a clear, descriptive title
   - Reference any related issues
   - Describe what changes you made and why
   - Include benchmark results for performance changes
   - List any breaking changes

4. **PR Template**
   ```markdown
   ## Description
   Brief description of changes
   
   ## Type of Change
   - [ ] Bug fix
   - [ ] New feature
   - [ ] Breaking change
   - [ ] Documentation update
   
   ## Testing
   - [ ] Unit tests pass
   - [ ] New tests added
   - [ ] Performance tests pass
   - [ ] Integration tests pass
   
   ## Performance Impact
   - [ ] No performance impact
   - [ ] Performance improved (include benchmarks)
   - [ ] Performance degraded (justify why)
   
   ## Checklist
   - [ ] Code follows style guidelines
   - [ ] Self-review completed
   - [ ] Documentation updated
   - [ ] No new warnings
   ```

### Code Review Process

- At least one maintainer review required
- All CI checks must pass
- Performance regression tests must pass
- Address review feedback promptly

## Performance Considerations

When contributing, consider:

1. **Overhead**: Monitoring should have minimal impact
2. **Allocations**: Avoid allocations in hot paths
3. **Lock Contention**: Minimize mutex usage
4. **Cache Efficiency**: Keep data structures compact
5. **Benchmarks**: Provide before/after comparisons

## Questions?

If you have questions:
- Check the documentation
- Open an issue for clarification
- Join discussions in existing issues

Thank you for contributing!