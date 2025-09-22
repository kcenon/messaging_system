# Contributing to Monitoring System

We welcome contributions to the Monitoring System! This document provides guidelines for contributing to the project.

## Table of Contents

- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [Making Changes](#making-changes)
- [Coding Standards](#coding-standards)
- [Testing](#testing)
- [Submitting Changes](#submitting-changes)
- [Code Review Process](#code-review-process)
- [Community Guidelines](#community-guidelines)

## Getting Started

### Prerequisites

- C++20 capable compiler (GCC 11+, Clang 14+, MSVC 2019+)
- CMake 3.16 or later
- Git
- Basic understanding of monitoring systems and observability

### Fork and Clone

1. Fork the repository on GitHub
2. Clone your fork locally:

```bash
git clone https://github.com/yourusername/monitoring_system.git
cd monitoring_system
```

## Development Setup

### Build Environment

```bash
# Create build directory
mkdir build && cd build

# Configure with development options
cmake .. \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTS=ON \
  -DBUILD_BENCHMARKS=ON \
  -DENABLE_COVERAGE=ON

# Build
cmake --build . --parallel $(nproc)
```

### Running Tests

```bash
# Run all tests
ctest --output-on-failure

# Run specific test categories
./tests/monitoring_system_tests --gtest_filter="MetricsTest.*"
./tests/monitoring_system_tests --gtest_filter="AlertingTest.*"
./tests/monitoring_system_tests --gtest_filter="DashboardTest.*"

# Run benchmarks
./benchmarks/monitoring_benchmarks

# Run stress tests
./tests/stress_tests --duration=60 --threads=4
```

## Making Changes

### Branching Strategy

- Create feature branches from `main`
- Use descriptive branch names: `feature/add-metrics-aggregation`, `fix/memory-leak-collector`
- Keep branches focused on a single feature or fix

```bash
git checkout -b feature/your-feature-name
```

### Commit Messages

Follow the conventional commit format:

```
type(scope): description

body (optional)

footer (optional)
```

**Types:**
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Formatting changes
- `refactor`: Code refactoring
- `test`: Adding/modifying tests
- `perf`: Performance improvements
- `build`: Build system changes

**Examples:**
```
feat(metrics): add histogram metric type support

Add support for histogram metrics with configurable buckets
and automatic percentile calculation.

Closes #123
```

```
fix(alerting): resolve memory leak in alert processor

Fix memory leak caused by unreleased alert rule objects
in the alert processing pipeline.

Fixes #456
```

## Coding Standards

### C++ Guidelines

- Follow modern C++20 standards
- Use RAII principles consistently
- Prefer stack allocation over heap allocation
- Use smart pointers when heap allocation is necessary
- Avoid raw pointers except for non-owning references

### Code Style

- Use 4 spaces for indentation (no tabs)
- Maximum line length: 100 characters
- Use snake_case for variables and functions
- Use PascalCase for classes and types
- Use UPPER_CASE for constants and macros

### Naming Conventions

```cpp
// Classes and types
class MetricsCollector;
using MetricValue = double;
enum class AlertSeverity;

// Functions and variables
void collect_metrics();
auto metric_value = get_current_value();
const size_t buffer_size = 1024;

// Constants and macros
constexpr int MAX_METRICS = 10000;
#define MONITORING_VERSION_MAJOR 2
```

### Documentation

- Use Doxygen-style comments for public APIs
- Include usage examples in API documentation
- Document complex algorithms and design decisions
- Keep comments up-to-date with code changes

```cpp
/**
 * @brief Collects metrics from registered collectors
 * 
 * This function iterates through all registered metric collectors
 * and aggregates their values into a single metrics snapshot.
 * 
 * @param timestamp The timestamp for the metrics snapshot
 * @return MetricsSnapshot containing all collected metrics
 * 
 * @example
 * auto snapshot = metrics_manager.collect_metrics(
 *     std::chrono::system_clock::now());
 */
MetricsSnapshot collect_metrics(TimePoint timestamp);
```

## Testing

### Test Categories

1. **Unit Tests**: Test individual components in isolation
2. **Integration Tests**: Test component interactions
3. **Performance Tests**: Verify performance characteristics
4. **Stress Tests**: Test system under load

### Writing Tests

- Use Google Test framework
- Write tests for all public APIs
- Include both positive and negative test cases
- Test error conditions and edge cases
- Maintain high test coverage (>90%)

```cpp
TEST(MetricsCollectorTest, CollectBasicMetrics) {
    MetricsCollector collector;
    collector.register_metric("cpu_usage", MetricType::Gauge);
    
    auto snapshot = collector.collect();
    
    EXPECT_FALSE(snapshot.empty());
    EXPECT_TRUE(snapshot.contains("cpu_usage"));
}

TEST(MetricsCollectorTest, HandlesInvalidMetricName) {
    MetricsCollector collector;
    
    EXPECT_THROW(
        collector.register_metric("", MetricType::Counter),
        std::invalid_argument
    );
}
```

### Performance Considerations

- Profile performance-critical code paths
- Use benchmarks to prevent performance regressions
- Consider memory allocation patterns
- Optimize for common use cases

## Submitting Changes

### Pre-submission Checklist

- [ ] Code compiles without warnings
- [ ] All tests pass
- [ ] New code is covered by tests
- [ ] Documentation is updated
- [ ] Code follows style guidelines
- [ ] Commit messages are properly formatted

### Pull Request Process

1. **Update your branch**:
```bash
git checkout main
git pull upstream main
git checkout your-feature-branch
git rebase main
```

2. **Push your changes**:
```bash
git push origin your-feature-branch
```

3. **Create Pull Request**:
   - Use descriptive title and description
   - Reference related issues
   - Include testing instructions
   - Add screenshots for UI changes

4. **Address Review Feedback**:
   - Respond to all comments
   - Make requested changes
   - Update tests as needed

## Code Review Process

### Review Criteria

- **Correctness**: Does the code work as intended?
- **Design**: Is the code well-designed and maintainable?
- **Functionality**: Does it meet the requirements?
- **Complexity**: Is the code easy to understand?
- **Tests**: Are there appropriate tests?
- **Naming**: Are names clear and descriptive?
- **Comments**: Are comments helpful and accurate?
- **Style**: Does it follow the style guide?
- **Documentation**: Is the documentation adequate?

### Review Timeline

- Initial response: Within 2 business days
- Complete review: Within 5 business days
- Re-review after changes: Within 2 business days

## Community Guidelines

### Code of Conduct

- Be respectful and inclusive
- Welcome newcomers and help them learn
- Focus on constructive feedback
- Assume positive intent
- Respect different perspectives and experiences

### Communication

- **Issues**: Use GitHub Issues for bug reports and feature requests
- **Discussions**: Use GitHub Discussions for questions and general discussion
- **Email**: kcenon@naver.com for sensitive matters

### Getting Help

- Check existing documentation and examples
- Search existing issues and discussions
- Ask specific, detailed questions
- Provide minimal reproducible examples

## Development Best Practices

### Performance Guidelines

- Minimize memory allocations in hot paths
- Use lock-free data structures where appropriate
- Prefer batch operations over individual calls
- Cache frequently accessed data
- Profile before optimizing

### Error Handling

- Use result types for error handling
- Provide meaningful error messages
- Log errors with appropriate context
- Fail fast for programming errors
- Gracefully handle runtime errors

### Security Considerations

- Validate all input data
- Use secure coding practices
- Avoid buffer overflows
- Handle sensitive data appropriately
- Follow principle of least privilege

## Recognition

Contributors will be recognized in:
- CHANGELOG.md for significant contributions
- README.md contributors section
- GitHub contributors graph
- Annual contributor appreciation posts

Thank you for contributing to the Monitoring System! Your efforts help make observability better for everyone.