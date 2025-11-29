# Contributing to Messaging System

Thank you for your interest in contributing to messaging_system!

## Table of Contents

1. [Code of Conduct](#code-of-conduct)
2. [Getting Started](#getting-started)
3. [Development Workflow](#development-workflow)
4. [Coding Standards](#coding-standards)
5. [Testing Requirements](#testing-requirements)
6. [Pull Request Process](#pull-request-process)

---

## Code of Conduct

Please be respectful and constructive in all interactions.

---

## Getting Started

### Prerequisites

- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- CMake 3.16 or higher
- Git

### Building for Development

```bash
git clone https://github.com/kcenon/messaging_system.git
cd messaging_system

# Debug build with tests
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build

# Run tests
ctest --test-dir build --output-on-failure
```

---

## Development Workflow

1. **Fork** the repository
2. **Create** a feature branch
3. **Make** your changes
4. **Test** thoroughly
5. **Commit** with clear messages
6. **Push** and open a PR

---

## Coding Standards

- Use clang-format with the provided configuration
- Follow existing code patterns
- Use meaningful names
- Comment complex logic

---

## Testing Requirements

- Unit tests for new functionality
- Integration tests for pattern implementations
- Thread safety tests for concurrent code

---

## Pull Request Process

1. Ensure all tests pass
2. Update documentation if needed
3. Add changelog entry
4. Request review

### PR Title Format

```
feat: add new messaging pattern
fix: resolve race condition in topic router
docs: update API documentation
test: add pub/sub integration tests
```

---

## Questions?

- Open an [issue](https://github.com/kcenon/messaging_system/issues)
- Email: kcenon@naver.com
