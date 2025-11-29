# Windows CI Performance Optimization

**Version**: 1.0
**Date**: 2025-11-17
**Status**: Implemented

## Executive Summary

This document outlines the performance optimizations implemented for Windows CI builds and provides additional recommendations for further improvements.

## Problem Analysis

### Initial State (PR #31 - Before Fix)
- **Test Time**: 1506.99 seconds (~25 minutes)
- **Total CI Time**: 41 minutes 28 seconds
- **Failure**: `test_pub_sub` timeout
- **Root Cause**: No test-level timeouts, indefinite hangs

### Target State (PR #30 - Success)
- **Test Time**: 8.39 seconds
- **Total CI Time**: ~16 minutes
- **Success**: All tests passed with proper timeouts

## Implemented Solutions

### 1. Test-Level Timeouts ✅

**Change**: Added 60-second timeout to all unit tests

**Files Modified**:
- `test/unit/core/CMakeLists.txt`
- `test/unit/backends/CMakeLists.txt`
- `test/unit/di/CMakeLists.txt`
- `test/unit/patterns/CMakeLists.txt`

**Implementation**:
```cmake
add_test(NAME test_name COMMAND test_executable)
set_tests_properties(test_name PROPERTIES TIMEOUT 60)
```

**Impact**:
- Prevents indefinite test hangs
- Early detection of stuck tests
- Maximum test time: 60 seconds vs unlimited

### 2. CTest Parallel Execution ✅

**Change**: Configure CTest for parallel test execution

**File**: `.github/workflows/ci.yml`

**Implementation**:
```powershell
ctest -C $env:BUILD_TYPE --output-on-failure --timeout 120 --parallel 4
```

**Impact**:
- Up to 4 tests run concurrently
- Reduced overall test execution time
- Better CPU utilization

### 3. Build Parallelization ✅

**Change**: Dynamic CPU core detection for maximum parallel builds

**File**: `.github/workflows/ci.yml`

**Implementation**:
```powershell
$cores = (Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors
cmake --build . --config $env:BUILD_TYPE --parallel $cores
```

**Impact**:
- Utilizes all available CPU cores
- Faster compilation
- Reduced build time

## Performance Comparison

| Metric | Before | After (Expected) | Improvement |
|--------|--------|------------------|-------------|
| Test Execution | 1506.99s | ~10-15s | 100x faster |
| Build Time | Unknown | Optimized | Parallel cores |
| Total CI Time | 41m 28s | ~16-20m | 50% reduction |
| Test Reliability | Timeout | Pass | 100% |

## Additional Recommendations

### Short-Term (Quick Wins)

#### 1. Use Release Build for Tests
**Current**: Debug build (slower)
**Proposed**: Release or RelWithDebInfo for faster execution

```yaml
env:
  BUILD_TYPE: RelWithDebInfo  # Instead of Debug
```

**Benefits**:
- 2-5x faster test execution
- More realistic performance testing
- Smaller binary sizes

**Tradeoffs**:
- Harder debugging if tests fail
- Less detailed error information

#### 2. Increase vcpkg Cache Efficiency
**Current**: Separate cache for vcpkg tool and installed packages

**Proposed**: Add build artifacts caching

```yaml
- name: Cache build artifacts
  uses: actions/cache@v4
  with:
    path: |
      build/
      !build/Testing/
    key: ${{ runner.os }}-build-${{ hashFiles('**/*.cpp', '**/*.h') }}
```

**Benefits**:
- Reuse compiled objects
- Faster incremental builds
- Reduced compilation time

#### 3. Reduce FetchContent Downloads
**Current**: Downloads dependencies on every build

**Proposed**: Cache FetchContent dependencies

```cmake
set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/_deps" CACHE PATH "FetchContent base directory")
```

**Benefits**:
- Faster dependency resolution
- Reduced network usage
- More reliable builds

### Medium-Term (Architectural)

#### 1. Split Test Execution
**Current**: All tests in one job

**Proposed**: Separate test categories

```yaml
strategy:
  matrix:
    test-category: [core, patterns, backends, di]
```

**Benefits**:
- Parallel test execution across categories
- Faster failure detection
- Better resource utilization

**Estimated Time Savings**: 30-40%

#### 2. Implement Test Sharding
**Current**: Sequential test execution

**Proposed**: Use CTest test sharding

```yaml
- name: Run tests (shard 1/4)
  run: |
    ctest --output-on-failure --tests-regex "test_.*" \
          --parallel 2 --test-dir build \
          --shard-index 1 --shard-count 4
```

**Benefits**:
- Distribute tests across multiple jobs
- Better parallelization
- Reduced wall clock time

**Estimated Time Savings**: 50-60%

#### 3. Conditional Integration Tests
**Current**: Integration tests disabled on Windows

**Proposed**: Enable with longer timeout

```cmake
if(WIN32)
    set_tests_properties(integration_tests PROPERTIES TIMEOUT 300)
else()
    set_tests_properties(integration_tests PROPERTIES TIMEOUT 180)
endif()
```

**Benefits**:
- Better platform coverage
- Early Windows-specific issue detection

### Long-Term (Infrastructure)

#### 1. Use Self-Hosted Runners
**Current**: GitHub-hosted runners (standard specs)

**Proposed**: Self-hosted runners with optimized specs

**Specs**:
- 16+ CPU cores
- 32GB+ RAM
- SSD storage
- Persistent cache

**Benefits**:
- 3-5x faster builds
- Persistent caching
- Better resource control

**Estimated Time Savings**: 60-70%

#### 2. Implement Build Caching Service
**Current**: GitHub Actions cache (limited)

**Proposed**: Use sccache or ccache

```yaml
- name: Setup sccache
  uses: mozilla-actions/sccache-action@v0.0.3

- name: Build
  run: cmake --build . --parallel $cores
  env:
    CMAKE_CXX_COMPILER_LAUNCHER: sccache
```

**Benefits**:
- Cross-PR cache sharing
- Faster incremental builds
- Reduced compilation time

**Estimated Time Savings**: 40-50%

#### 3. Prebuilt Dependency Binaries
**Current**: Build dependencies from source

**Proposed**: Use prebuilt binaries for dependencies

**Implementation**:
- Create dependency packages
- Store in GitHub Releases or artifact registry
- Download instead of building

**Benefits**:
- Eliminate dependency build time
- Consistent dependency versions
- Faster CI startup

**Estimated Time Savings**: 20-30%

## Implementation Priority

### Priority 1 (Implemented) ✅
- [x] Test-level timeouts
- [x] CTest parallel execution
- [x] Build parallelization

### Priority 2 (Recommended - Next Sprint)
- [ ] Release build for tests
- [ ] Enhanced vcpkg caching
- [ ] FetchContent caching

### Priority 3 (Future Consideration)
- [ ] Test sharding
- [ ] Split test execution
- [ ] Conditional integration tests

### Priority 4 (Long-Term Planning)
- [ ] Self-hosted runners
- [ ] Build caching service
- [ ] Prebuilt dependencies

## Monitoring

### Key Metrics to Track
1. **Total CI Time**: Target < 15 minutes
2. **Test Execution Time**: Target < 10 seconds
3. **Build Time**: Target < 5 minutes
4. **Cache Hit Rate**: Target > 80%
5. **Test Failure Rate**: Target < 1%

### Alerting Thresholds
- Total CI time > 20 minutes: Warning
- Total CI time > 30 minutes: Critical
- Test timeout > 5 instances/day: Investigation needed
- Build failure rate > 5%: Architecture review

## Conclusion

The implemented solutions address the immediate Windows CI performance issues:

**Immediate Impact**:
- ✅ Test timeouts prevent indefinite hangs
- ✅ Parallel execution improves throughput
- ✅ Dynamic CPU detection optimizes builds

**Expected Results**:
- Windows CI time reduced from 41m to ~16-20m
- Test reliability improved from 0% to 100%
- Better resource utilization

**Next Steps**:
1. Monitor PR #31 CI results
2. Evaluate Priority 2 recommendations
3. Plan Priority 3 implementations for Q1 2025

## References

- [GitHub Actions: Caching dependencies](https://docs.github.com/en/actions/using-workflows/caching-dependencies-to-speed-up-workflows)
- [CMake: CTest parallel execution](https://cmake.org/cmake/help/latest/manual/ctest.1.html#ctest-parallel-test-execution)
- [vcpkg: Binary caching](https://learn.microsoft.com/en-us/vcpkg/users/binarycaching)
