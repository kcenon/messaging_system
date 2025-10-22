# Messaging System v2.0 - Build Guide

## Quick Start

The messaging system v2.0 integrates 7 external systems and provides multiple build modes. Due to CMake target conflicts between local installations and FetchContent, **choose one of the recommended approaches below**.

---

## Recommended Build Approaches

### Option 1: Clean Environment Build (Recommended for First-Time Users)

If you have local external system directories that conflict with FetchContent:

```bash
# Use the automated build script
./scripts/build_with_fetchcontent.sh --clean

# This script automatically:
# - Temporarily moves conflicting local systems
# - Runs the build with FetchContent
# - Restores local systems after build
```

### Option 2: Manual Temporary Move

```bash
cd ~/Sources

# Backup local systems
for sys in common_system thread_system logger_system monitoring_system container_system database_system network_system; do
    [ -d "$sys" ] && mv "$sys" "${sys}.backup"
done

# Build
cd messaging_system
./build.sh dev-fetchcontent --clean

# Restore systems
cd ~/Sources
for sys in common_system thread_system logger_system monitoring_system container_system database_system network_system; do
    [ -d "${sys}.backup" ] && mv "${sys}.backup" "$sys"
done
```

### Option 3: Install External Systems (Production)

For production deployments, install all external systems:

```bash
# Install each system
for sys in common_system thread_system logger_system monitoring_system container_system database_system network_system; do
    cd ~/Sources/$sys
    cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
    cmake --build build
    sudo cmake --install build
done

# Then build messaging_system
cd ~/Sources/messaging_system
./build.sh default
```

---

## Available Build Presets

The system provides 10 CMake presets:

| Preset | Description | Use Case |
|--------|-------------|----------|
| `default` | Production with find_package | Installed external systems |
| `dev-fetchcontent` | Development with FetchContent | Development without installs |
| `debug` | Debug build | Debugging |
| `release` | Optimized release | Production |
| `asan` | AddressSanitizer | Memory error detection |
| `tsan` | ThreadSanitizer | Race condition detection |
| `ubsan` | UndefinedBehaviorSanitizer | Undefined behavior detection |
| `ci` | CI/CD with pedantic warnings | Continuous integration |
| `lockfree` | Lock-free data structures | High performance |
| `minimal` | Minimal feature set | Embedded/constrained |

Usage:
```bash
./build.sh [preset] [options]

# Examples:
./build.sh dev-fetchcontent --tests
./build.sh release --examples
./build.sh asan --clean
```

---

## Build Options

```bash
./build.sh [preset] [options]

Options:
  --clean           Remove build directory before building
  --tests           Build and run tests
  --examples        Build examples
  --benchmarks      Build benchmarks
  --verbose         Show detailed build output
  --cores N         Use N cores (default: auto-detect)

Feature Options:
  --lockfree        Enable lock-free implementations
  --no-monitoring   Disable monitoring system
  --no-logging      Disable logging system
  --enable-tls      Enable TLS/SSL support
```

---

## Known Issues

### Issue: Target Name Conflicts

**Symptom**:
```
CMake Error: add_library cannot create target "interfaces" because another
target with the same name already exists.
```

**Cause**: Local external system installations conflict with FetchContent downloads.

**Solutions**:
1. Use `./scripts/build_with_fetchcontent.sh` (automated)
2. Manually move local systems temporarily (see Option 2 above)
3. Install systems and use `default` preset (see Option 3 above)

### Issue: monitoring_system Cannot Find common_system

**Symptom**:
```
CMake Error: common_system is required but was not found.
```

**Cause**: FetchContent order-of-operations issue with monitoring_system's strict dependency checking.

**Solution**: Use the automated build script or ensure all systems are properly installed.

### Issue: GTest Not Found

**Symptom**:
```
Could NOT find GTest (missing: GTEST_LIBRARY GTEST_INCLUDE_DIR GTEST_MAIN_LIBRARY)
```

**Status**: External system integration tests are disabled by default. This warning can be safely ignored.

**Optional Fix**: Install GTest if you want to build external system tests:
```bash
# macOS
brew install googletest

# Ubuntu/Debian
sudo apt-get install libgtest-dev
```

---

## Testing

### Run All Tests

```bash
# Build with tests
./build.sh dev-fetchcontent --tests --clean

# Run tests
ctest --test-dir build-dev --output-on-failure

# Or use preset
ctest --preset default
```

### Run Specific Test

```bash
# After building with --tests
./build-dev/bin/test_topic_router
./build-dev/bin/test_message_bus
./build-dev/bin/test_end_to_end
```

### Test Coverage

- 39 test cases across 6 test files
- Unit tests for all core components
- Integration tests for end-to-end scenarios
- Performance benchmarks (~2900 msg/s)

See `docs/phase4/TEST_SUMMARY.md` for detailed test documentation.

---

## Examples

### Run Basic Messaging Example

```bash
# Build with examples
./build.sh dev-fetchcontent --examples

# Run example
./build-dev/bin/basic_messaging
```

Example demonstrates:
- Simple pub/sub
- Wildcard subscriptions (`*`, `#`)
- Trace context propagation
- Configuration loading

---

## Troubleshooting

For comprehensive troubleshooting, see **`docs/BUILD_TROUBLESHOOTING.md`**.

Quick fixes:

| Problem | Solution |
|---------|----------|
| Target conflicts | Use `./scripts/build_with_fetchcontent.sh` |
| GTest not found | Ignore (tests disabled) or install GTest |
| Build hangs | Reduce cores: `./build.sh --cores 4` |
| Clean build needed | Add `--clean` flag |

---

## Platform-Specific Notes

### macOS
- Requires Xcode Command Line Tools
- Homebrew recommended for dependencies
- Native pthread support (no extra libs)
- Use `grep -E` (BSD grep doesn't support `-P`)

### Linux (Ubuntu/Debian)
- Install build-essential: `sudo apt-get install build-essential cmake`
- GCC 11+ or Clang 14+ for C++20
- Install dependencies: `./dependency.sh`

### Linux (Fedora/RHEL)
- Install development tools: `sudo dnf groupinstall "Development Tools"`
- Enable PowerTools repo for dependencies

---

## Dependencies

### Required
- CMake >= 3.16
- C++20 compiler (GCC 11+, Clang 14+, or MSVC 2019+)
- Git

### Optional
- yaml-cpp (for ConfigLoader)
- ninja (faster builds)
- GTest (for external system tests)

Install dependencies:
```bash
./dependency.sh
```

---

## Project Structure

```
messaging_system/
├── src/
│   ├── core/              # Core messaging components
│   │   ├── messaging_container.cpp
│   │   ├── topic_router.cpp
│   │   └── message_bus.cpp
│   ├── integration/       # Infrastructure integration
│   │   ├── trace_context.cpp
│   │   └── config_loader.cpp
│   └── main.cpp           # Main executable
├── include/
│   └── messaging_system/  # Public headers
├── test/
│   ├── unit/              # Unit tests
│   └── integration/       # Integration tests
├── examples/
│   └── basic_messaging.cpp
├── docs/                  # Documentation
├── scripts/               # Build automation
│   └── build_with_fetchcontent.sh
├── build.sh               # Build script
├── dependency.sh          # Dependency installer
└── CMakeLists.txt         # CMake configuration
```

---

## Next Steps

1. **Build the project**: `./scripts/build_with_fetchcontent.sh --clean`
2. **Run tests**: `ctest --test-dir build-dev`
3. **Try examples**: `./build-dev/bin/basic_messaging`
4. **Read documentation**: `docs/PROJECT_COMPLETION_SUMMARY.md`

---

## Getting Help

- **Build issues**: `docs/BUILD_TROUBLESHOOTING.md`
- **Project overview**: `docs/PROJECT_COMPLETION_SUMMARY.md`
- **Test documentation**: `docs/phase4/TEST_SUMMARY.md`
- **GitHub Issues**: [Create an issue](https://github.com/kcenon/messaging_system/issues)

---

## Version

**Messaging System v2.0** - Complete rebuild with modern C++20 architecture

Branch: `feature/system-rebuild`
Status: ✅ Ready for review and merge

---

Last Updated: 2025-10-21
