# Build Troubleshooting Guide

## Common Build Issues and Solutions

### Issue 1: Target Name Conflicts

**Symptoms**:
```
CMake Error: add_library cannot create target "interfaces" because another target with
the same name already exists.
```

**Cause**: Local installations of external systems (e.g., `/Users/username/Sources/thread_system`) conflict with FetchContent downloads.

**Solution 1 - Use FetchContent Only**:
Remove or rename local system directories temporarily:
```bash
cd ~/Sources
mv thread_system thread_system.backup
mv logger_system logger_system.backup
mv monitoring_system monitoring_system.backup
# Repeat for other systems if needed
```

**Solution 2 - Build Without External Systems**:
```bash
cmake --preset dev-fetchcontent -DMESSAGING_USE_EXTERNAL_SYSTEMS=OFF
cmake --build --preset dev
```

**Solution 3 - Use Installed Packages** (when available):
```bash
# Install external systems first
cd ~/Sources/common_system && cmake --install build
cd ~/Sources/thread_system && cmake --install build
# ... repeat for all systems

# Then build messaging_system with find_package
./build.sh default
```

---

### Issue 2: GTest Not Found in External Systems

**Symptoms**:
```
Could NOT find GTest (missing: GTEST_LIBRARY GTEST_INCLUDE_DIR GTEST_MAIN_LIBRARY)
```

**Cause**: External systems' integration tests require GTest, which is not installed.

**Solution**: Install GTest or disable external system tests (already done in CMakeLists.txt):
```bash
# macOS
brew install googletest

# Ubuntu/Debian
sudo apt-get install libgtest-dev

# Or build without tests
cmake --preset dev-fetchcontent -DBUILD_TESTING=OFF
```

---

### Issue 3: grep -P Not Supported (macOS)

**Symptoms**:
```
grep: invalid option -- P
```

**Cause**: macOS's BSD grep doesn't support Perl regex (-P flag).

**Solution**: Already fixed in build.sh and dependency.sh (uses `grep -E` instead).

If you see this error, ensure you're using the latest version:
```bash
git pull origin feature/system-rebuild
```

---

### Issue 4: yaml-cpp Not Found

**Symptoms**:
```
yaml-cpp not found (ConfigLoader disabled)
```

**Cause**: Optional dependency yaml-cpp is not installed.

**Solution**:
```bash
# macOS
brew install yaml-cpp

# Ubuntu/Debian
sudo apt-get install libyaml-cpp-dev

# Fedora/RHEL
sudo dnf install yaml-cpp-devel
```

**Note**: ConfigLoader tests will be skipped if yaml-cpp is not available. This is not an error.

---

## Recommended Build Workflow

### For Development (FetchContent Mode)

1. **Ensure No Local Conflicts**:
```bash
# Temporarily rename local external systems
cd ~/Sources
for sys in common_system thread_system logger_system monitoring_system container_system database_system network_system; do
    [ -d "$sys" ] && mv "$sys" "${sys}.backup"
done
```

2. **Build messaging_system**:
```bash
cd ~/Sources/messaging_system
./build.sh dev-fetchcontent --clean
```

3. **Restore Local Systems** (after build):
```bash
cd ~/Sources
for sys in common_system thread_system logger_system monitoring_system container_system database_system network_system; do
    [ -d "${sys}.backup" ] && mv "${sys}.backup" "$sys"
done
```

### For Production (find_package Mode)

1. **Install All External Systems**:
```bash
for sys in common_system thread_system logger_system monitoring_system container_system database_system network_system; do
    cd ~/Sources/$sys
    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build
    sudo cmake --install build
done
```

2. **Build messaging_system**:
```bash
cd ~/Sources/messaging_system
./build.sh default
```

---

## Alternative: Minimal Build (No External Systems)

If external system integration is causing issues, you can build a minimal version:

```bash
cd ~/Sources/messaging_system
cmake -B build-minimal \
    -DCMAKE_BUILD_TYPE=Debug \
    -DMESSAGING_USE_EXTERNAL_SYSTEMS=OFF \
    -DMESSAGING_BUILD_TESTS=OFF \
    -DMESSAGING_BUILD_EXAMPLES=OFF

cmake --build build-minimal
```

**Note**: This builds only the core messaging_system library without external integrations. Limited functionality.

---

## Verifying Successful Build

After a successful build, you should see:

```
============================================
       Build Completed Successfully
============================================

Configuration:
  Preset: dev-fetchcontent
  Cores: 8

Available binaries in build-dev/bin:
  messaging_system
  basic_messaging
  test_messaging_container
  test_topic_router
  test_message_bus
  test_trace_context
  test_config_loader
  test_end_to_end

Quick commands:
  Run main:     ./build-dev/bin/messaging_system
  Run tests:    ctest --test-dir build-dev
  Run example:  ./build-dev/bin/basic_messaging
```

---

## Getting Help

If you encounter issues not covered here:

1. Check CMake configuration output for specific errors
2. Verify all dependencies are installed: `./dependency.sh`
3. Try a clean build: `./build.sh --clean`
4. Check [GitHub Issues](https://github.com/kcenon/messaging_system/issues)

---

## Known Limitations

1. **FetchContent + Local Systems**: Cannot coexist due to CMake target name conflicts
2. **External System Tests**: Disabled by default to avoid GTest dependency issues
3. **macOS grep**: Requires BSD-compatible regex patterns (grep -E instead of grep -P)
4. **Integration Tests**: Require all 7 external systems to be properly configured

---

## Platform-Specific Notes

### macOS
- Use Homebrew for dependencies: `brew install cmake yaml-cpp`
- Xcode Command Line Tools required: `xcode-select --install`
- Native pthread support (no additional libraries needed)

### Linux (Ubuntu/Debian)
- Install build essentials: `sudo apt-get install build-essential cmake git`
- GCC 11+ or Clang 14+ recommended for C++20 support

### Linux (Fedora/RHEL)
- Install development tools: `sudo dnf groupinstall "Development Tools"`
- Enable PowerTools repo for some dependencies

---

## Quick Fix Summary

| Issue | Quick Fix |
|-------|-----------|
| Target conflicts | Rename local system directories |
| GTest not found | Install GTest or ignore (tests disabled) |
| grep -P error | Update build.sh/dependency.sh scripts |
| yaml-cpp missing | Install yaml-cpp (optional) |
| Build hangs | Reduce --cores N in build.sh |
| Clean build needed | Use --clean flag |

---

Last Updated: 2025-10-21
