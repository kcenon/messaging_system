# Dependency Conflict Resolution Guide

## Overview

This guide helps developers identify, understand, and resolve dependency conflicts in the thread_system project. It provides step-by-step solutions for common issues and preventive measures.

## Table of Contents

1. [Common Conflict Types](#common-conflict-types)
2. [Detection Methods](#detection-methods)
3. [Resolution Strategies](#resolution-strategies)
4. [Prevention Guidelines](#prevention-guidelines)
5. [Emergency Procedures](#emergency-procedures)
6. [Troubleshooting Tools](#troubleshooting-tools)

---

## Common Conflict Types

### 1. Version Conflicts

**Symptoms:**
- Build fails with "version not found" errors
- Incompatible API usage warnings
- Runtime crashes due to ABI incompatibility

**Common Scenarios:**
```
❌ fmt 9.1.0 + spdlog 1.12.0 → spdlog requires fmt 10+
❌ gtest 1.11.0 + benchmark 1.8.0 → incompatible C++20 features
❌ vcpkg outdated → packages not found in registry
```

### 2. Platform-Specific Conflicts

**Symptoms:**
- "Package not available for platform" errors
- Different behavior on Windows vs Linux/macOS
- Missing system libraries

**Common Scenarios:**
```
❌ libiconv on Windows → not needed, causes conflicts
❌ pthread on Windows → use std::thread instead
❌ Different compiler versions → ABI incompatibility
```

### 3. Feature Conflicts

**Symptoms:**
- Missing optional features
- Conflicting feature flags
- CMake configuration warnings

**Common Scenarios:**
```
❌ gtest without gmock feature → missing mocking capabilities
❌ spdlog header-only vs compiled → linking conflicts
❌ fmt with different standard versions → API mismatches
```

### 4. Transitive Dependency Conflicts

**Symptoms:**
- Indirect dependency version mismatches
- Diamond dependency problems
- Complex error messages

**Common Scenarios:**
```
thread-system → fmt 10.2.1
thread-system → spdlog 1.12.0 → fmt 10.1.0
Result: fmt version conflict between direct and transitive
```

---

## Detection Methods

### 1. Automated Detection

#### CMake Dependency Checker
```bash
# Enable dependency checking in CMake
cmake -B build -DCHECK_DEPENDENCIES=ON

# The build will fail with detailed conflict information
```

#### Dependency Analyzer Tool
```bash
# Analyze dependency tree
./scripts/dependency_analyzer.py --visualize

# Generate HTML report
./scripts/dependency_analyzer.py --html

# Security scan
./scripts/dependency_analyzer.py --security-scan
```

### 2. Manual Detection

#### Check vcpkg.json Consistency
```bash
# Validate JSON syntax
python3 -m json.tool vcpkg.json

# Check for missing version constraints
grep -E '"name".*:' vcpkg.json | grep -v version
```

#### Build Log Analysis
```bash
# Look for common conflict indicators
cmake --build build 2>&1 | grep -E "(version|conflict|incompatible)"

# Check for missing packages
cmake --build build 2>&1 | grep -E "(not found|could not find)"
```

---

## Resolution Strategies

### 1. Version Constraint Resolution

#### Step 1: Identify Conflicting Versions
```bash
# Use dependency analyzer
./scripts/dependency_analyzer.py --project-root .

# Check compatibility matrix
cat docs/dependency_compatibility_matrix.md
```

#### Step 2: Update vcpkg.json
```json
{
  "dependencies": [
    {
      "name": "fmt",
      "version>=": "10.2.0"
    },
    {
      "name": "spdlog", 
      "version>=": "1.12.0"
    }
  ],
  "overrides": [
    {
      "name": "fmt",
      "version": "10.2.1"
    }
  ]
}
```

#### Step 3: Validate Resolution
```bash
# Clean and rebuild
rm -rf build vcpkg_installed
cmake -B build -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg.cmake
cmake --build build
```

### 2. Platform-Specific Resolution

#### Cross-Platform Dependencies
```json
{
  "dependencies": [
    {
      "name": "libiconv",
      "platform": "!windows"
    },
    {
      "name": "pthread",
      "platform": "linux"
    }
  ]
}
```

#### Conditional CMake Logic
```cmake
if(WIN32)
    # Windows-specific dependencies
    find_package(WindowsSDK REQUIRED)
elseif(UNIX AND NOT APPLE)
    # Linux-specific dependencies
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(ICONV REQUIRED iconv)
endif()
```

### 3. Feature Flag Resolution

#### Enable Required Features
```json
{
  "dependencies": [
    {
      "name": "gtest",
      "version>=": "1.14.0",
      "features": ["gmock"]
    }
  ]
}
```

#### CMake Feature Detection
```cmake
# Check if feature is available
if(TARGET GTest::gmock)
    message(STATUS "✅ GMock available")
    set(HAVE_GMOCK ON)
else()
    message(WARNING "⚠️ GMock not available - some tests will be disabled")
    set(HAVE_GMOCK OFF)
endif()
```

### 4. Transitive Dependency Resolution

#### Use Version Overrides
```json
{
  "overrides": [
    {
      "name": "fmt",
      "version": "10.2.1"
    }
  ]
}
```

#### Explicit Dependency Declaration
```json
{
  "dependencies": [
    {
      "name": "fmt",
      "version>=": "10.2.0",
      "comment": "Explicit version to prevent transitive conflicts"
    }
  ]
}
```

---

## Prevention Guidelines

### 1. Proactive Dependency Management

#### Regular Updates
```bash
# Monthly dependency check
./scripts/upgrade_dependencies.sh --dry-run

# Security updates (run immediately when notified)
./scripts/upgrade_dependencies.sh --security-only
```

#### Version Pinning Strategy
- Pin major versions in `overrides` for stability
- Use `version>=` for security patches
- Test thoroughly before updating major versions

#### Documentation Maintenance
- Update compatibility matrix with each change
- Document known issues and workarounds
- Maintain changelog of dependency updates

### 2. Development Best Practices

#### Pre-commit Checks
```bash
# Add to .pre-commit-hooks.yaml
- repo: local
  hooks:
  - id: dependency-check
    name: Check dependencies
    entry: cmake -B build_check -DCHECK_DEPENDENCIES=ON
    language: system
    pass_filenames: false
```

#### Branch Protection
- Require dependency validation in CI/CD
- Block merges with unresolved conflicts
- Automated security scanning

#### Team Guidelines
- Always test with clean vcpkg_installed directory
- Document any manual vcpkg modifications
- Use feature branches for dependency updates

---

## Emergency Procedures

### 1. Critical Build Failure

#### Immediate Actions (< 5 minutes)
```bash
# 1. Rollback to last known good state
git checkout HEAD~1 -- vcpkg.json

# 2. Clear vcpkg cache
rm -rf vcpkg_installed build

# 3. Quick rebuild test
cmake -B build && cmake --build build --target thread_base
```

#### Full Recovery (< 30 minutes)
```bash
# 1. Use dependency rollback script
./scripts/upgrade_dependencies.sh --rollback $(date -d yesterday +%Y%m%d_120000)

# 2. Validate core functionality
cmake --build build --target thread_pool interfaces
./build/bin/thread_base_unit --gtest_brief
```

### 2. Deployment Day Issues

#### Pre-deployment Checklist
- [ ] All dependency checks pass
- [ ] Security scan clean
- [ ] Compatibility matrix updated
- [ ] Backup created

#### Emergency Rollback Plan
```bash
# 1. Immediate rollback command
./scripts/upgrade_dependencies.sh --rollback YYYYMMDD_HHMMSS

# 2. Notify team
echo "Dependency rollback completed due to: [REASON]" | \
mail -s "Emergency Rollback - thread_system" team@company.com

# 3. Create incident report
./scripts/dependency_analyzer.py --html --output-dir incident_$(date +%s)
```

---

## Troubleshooting Tools

### 1. Built-in Tools

#### CMake Dependency Checker
```bash
# Enable verbose dependency checking
cmake -B build -DCHECK_DEPENDENCIES=ON -DCMAKE_VERBOSE_MAKEFILE=ON
```

#### Dependency Analyzer
```bash
# Full analysis with all options
./scripts/dependency_analyzer.py \
  --visualize \
  --html \
  --security-scan \
  --output-dir debug_$(date +%s)
```

### 2. External Tools

#### vcpkg Tools
```bash
# List installed packages
vcpkg list

# Show package details
vcpkg show fmt

# Dependency graph
vcpkg depend-info fmt --recurse
```

#### System Package Managers
```bash
# macOS - check system conflicts
brew doctor
brew deps fmt

# Ubuntu - check system packages
apt list --installed | grep -E "(fmt|spdlog|gtest)"
dpkg -l | grep -E "(fmt|spdlog|gtest)"
```

### 3. Debug Commands

#### CMake Debug Output
```bash
# Enable debug output
cmake -B build --debug-output

# Show all variables
cmake -B build -LAH | grep -E "(FMT|SPDLOG|GTEST)"

# Trace dependency resolution
cmake -B build --trace
```

#### Compilation Debug
```bash
# Verbose compilation
make VERBOSE=1 -C build

# Preprocessor output (check included headers)
g++ -E src/main.cpp | grep -E "(fmt|spdlog)"
```

---

## Quick Reference

### Common Resolution Commands

```bash
# 1. Clean slate rebuild
rm -rf build vcpkg_installed && cmake -B build

# 2. Dependency upgrade (safe)
./scripts/upgrade_dependencies.sh --security-only

# 3. Force latest versions (risky)
./scripts/upgrade_dependencies.sh --latest --force

# 4. Analyze current state
./scripts/dependency_analyzer.py --html

# 5. Emergency rollback
./scripts/upgrade_dependencies.sh --rollback 20251201_143022
```

### Emergency Contacts

- **Build System Issues**: DevOps Team
- **Security Vulnerabilities**: Security Team  
- **Dependency Questions**: Senior Developer
- **Critical Production Issues**: Incident Response Team

---

## Appendix

### A. Known Issues Database

| Issue | Symptoms | Solution | Status |
|-------|----------|----------|--------|
| fmt 9.x + spdlog 1.12+ | Link errors | Upgrade fmt to 10+ | Resolved |
| gtest missing gmock | Test failures | Add "gmock" feature | Resolved |
| Windows libiconv | Build failures | Use platform exclusion | Resolved |

### B. Compatibility Matrix Quick Reference

| fmt | spdlog | gtest | benchmark | Status |
|-----|--------|-------|-----------|---------|
| 10.2.1 | 1.12.0 | 1.14.0 | 1.8.0 | ✅ Recommended |
| 10.1.1 | 1.12.0 | 1.14.0 | 1.8.0 | ✅ Supported |
| 9.1.0 | 1.12.0 | - | - | ❌ Incompatible |

### C. Update Checklist Template

```markdown
## Dependency Update Checklist

- [ ] Backup created: `backup_YYYYMMDD_HHMMSS`
- [ ] Compatibility matrix checked
- [ ] Security scan passed  
- [ ] Local build successful
- [ ] Unit tests passing
- [ ] Integration tests passing
- [ ] Documentation updated
- [ ] Team notification sent
- [ ] Rollback plan confirmed
```

---

**Last Updated**: 2025-09-13  
**Version**: 1.0  
**Maintainer**: DevOps Team