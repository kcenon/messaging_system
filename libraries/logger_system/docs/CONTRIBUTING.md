# Contributing to Logger System

Thank you for your interest in contributing to the Logger System! This document provides guidelines and information for contributors.

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
   git clone https://github.com/kcenon/logger_system.git
   cd logger_system
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
feat(writer): add rotating file writer

Implement a file writer that automatically rotates files based on size.
Includes configuration for max file size and number of backups.

Closes #123
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
clang-format -i sources/logger/logger.cpp

# Format all files
find . -name "*.cpp" -o -name "*.h" | xargs clang-format -i
```

### Naming Conventions

```cpp
namespace logger_module {

class my_writer : public base_writer {
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

} // namespace logger_module
```

## Testing

### Unit Tests

All new code should include unit tests:

```cpp
TEST(LoggerTest, AsyncLogging) {
    auto logger = std::make_shared<logger>(true);
    auto mock_writer = std::make_shared<mock_writer>();
    logger->add_writer(mock_writer);
    
    logger->start();
    logger->log(log_level::info, "Test message");
    logger->stop();
    
    EXPECT_EQ(mock_writer->message_count(), 1);
}
```

### Running Tests

```bash
# Run all tests
cd build
ctest

# Run specific test
ctest -R LoggerTest

# Run with verbose output
ctest -V
```

### Benchmarks

Performance-critical code should include benchmarks:

```cpp
static void BM_AsyncLogging(benchmark::State& state) {
    auto logger = std::make_shared<logger>(true, state.range(0));
    logger->add_writer(std::make_unique<null_writer>());
    logger->start();
    
    for (auto _ : state) {
        logger->log(log_level::info, "Benchmark message");
    }
    
    logger->stop();
}
BENCHMARK(BM_AsyncLogging)->Range(1024, 65536);
```

## Documentation

### Code Documentation

Use Doxygen-style comments:

```cpp
/**
 * @brief High-performance asynchronous logger
 * 
 * This logger provides thread-safe logging with multiple output
 * destinations and configurable filtering.
 * 
 * @see base_writer for creating custom outputs
 */
class logger : public logger_interface {
public:
    /**
     * @brief Construct a new logger
     * 
     * @param async Enable asynchronous mode
     * @param buffer_size Size of the log buffer (for async mode)
     * @throws std::invalid_argument if buffer_size is 0
     */
    explicit logger(bool async = true, std::size_t buffer_size = 8192);
```

### Documentation Updates

When adding features:
1. Update relevant documentation files
2. Add examples to the docs
3. Update the API reference if needed
4. Add performance implications to PERFORMANCE.md

## Submitting Changes

### Pull Request Process

1. **Update Your Fork**
   ```bash
   git remote add upstream https://github.com/kcenon/logger_system.git
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
   - Include screenshots for UI changes
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
   - [ ] Manual testing completed
   
   ## Checklist
   - [ ] Code follows style guidelines
   - [ ] Self-review completed
   - [ ] Documentation updated
   - [ ] No new warnings
   ```

### Review Process

- At least one maintainer review required
- All CI checks must pass
- Address review feedback promptly
- Squash commits before merging

## Questions?

If you have questions:
- Check the documentation
- Open an issue for clarification
- Join discussions in existing issues

Thank you for contributing!