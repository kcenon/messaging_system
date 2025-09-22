# GitHub Workflows  Improvements Based on Thread System

This document describes the CI/CD improvements applied to logger_system workflows, adapted from thread_system's best practices.

Updated: 2025-09-08 (Asia/Seoul)

## Summary of Improvements

All workflows have been enhanced with:
1. **vcpkg binary caching** for faster dependency builds
2. **Stronger cache keys** that properly invalidate when needed
3. **Fallback mechanisms** for resilient builds
4. **Test artifact uploads** for debugging failures
5. **Proper triplet configuration** per platform

## What Changed

### 1. vcpkg Binary Cache Enabled

Added environment variables to all jobs:
```yaml
env:
  VCPKG_BINARY_SOURCES: "clear;x-gha,readwrite"
  VCPKG_FEATURE_FLAGS: "manifests,registries,versions,binarycaching"
  VCPKG_DEFAULT_TRIPLET: x64-linux  # or x64-windows
```

**Effect**: vcpkg reuses prebuilt artifacts via GitHub Actions cache, dramatically reducing build times.

### 2. Improved Cache Key Structure

Cache keys now include:
- Platform and compiler flavor (gcc/clang/vs/msys2)
- Triplet (x64-linux, x64-windows, x64-mingw-dynamic)
- Manifest hash (`hashFiles('vcpkg.json', 'vcpkg-configuration.json')`)
- vcpkg commit hash (determined dynamically)

Example:
```yaml
key: ${{ runner.os }}-gcc-vcpkg-installed-${{ env.VCPKG_DEFAULT_TRIPLET }}-${{ hashFiles('vcpkg.json', 'vcpkg-configuration.json') }}-${{ steps.vcpkg-commit.outputs.commit }}
```

**Result**: Caches invalidate only when necessary, avoiding unnecessary rebuilds.

### 3. Fallback Build Strategy

Each workflow now includes:
1. **Primary build** with vcpkg dependencies
2. **Fallback build** using system libraries if vcpkg fails
3. **Minimal test** to verify the fallback build works

```yaml
- name: Build application (vcpkg first attempt)
  id: build_vcpkg
  continue-on-error: true
  run: |
    # Build with vcpkg toolchain

- name: Build application (fallback to system libraries)
  if: steps.build_vcpkg.outcome != 'success'
  run: |
    # Build without vcpkg toolchain
```

**Benefit**: Builds remain resilient even when vcpkg has issues.

### 4. Test Result Artifacts

All workflows now upload test results:
```yaml
- name: Upload test results
  if: always()
  uses: actions/upload-artifact@v3
  with:
    name: test-results-${{ matrix.os }}-${{ matrix.compiler }}
    path: build/Testing/
    retention-days: 7
```

**Benefit**: Test failures can be debugged by downloading artifacts.

### 5. Proper Permissions

Added explicit permissions for GitHub token:
```yaml
permissions:
  contents: write
```

**Benefit**: Enables proper caching and artifact uploads.

### 6. Upgraded to upload-artifact@v4

All workflows now use the latest version of upload-artifact:
```yaml
- uses: actions/upload-artifact@v4
```

**Benefit**: Avoids deprecation warnings and uses the latest performance improvements.

## Affected Workflows

| Workflow | File | Triplet | Improvements |
|----------|------|---------|--------------|
| Ubuntu GCC | `build-ubuntu-gcc.yaml` | x64-linux |  All improvements |
| Ubuntu Clang | `build-ubuntu-clang.yaml` | x64-linux |  All improvements |
| Windows VS | `build-windows-vs.yaml` | x64-windows |  All improvements |
| Windows MSYS2 | `build-windows-msys2.yaml` | x64-mingw-dynamic | ✅ All improvements |

## Performance Impact

Expected improvements:
- **First run**: ~30-40% faster (binary cache)
- **Subsequent runs**: ~60-70% faster (installed cache hit)
- **Dependency unchanged**: ~80-90% faster (skip vcpkg install)

## Cache Invalidation Triggers

Caches are invalidated when:
- `vcpkg.json` changes (dependency updates)
- `vcpkg-configuration.json` changes (if present)
- Triplet changes
- vcpkg repository updates (after `git pull`)
- Compiler/OS changes

## Monitoring in GitHub Actions

Look for these indicators in the logs:

### Cache Hits
```
Cache restored from key: ubuntu-latest-gcc-vcpkg-installed-x64-linux-...
```

### Binary Cache Usage
```
Using binary cache from GitHub Actions cache
```

### Skipped Installation
```
Install dependencies with vcpkg (uses binary cache) - Skipped (cache hit)
```

### Fallback Activation
```
vcpkg build failed. Falling back to system libraries...
```

## Best Practices Applied

1. **Deterministic Caching**: Cache keys include all relevant factors
2. **Graceful Degradation**: Fallback to system libraries when needed
3. **Observability**: Test artifacts and clear log messages
4. **Performance**: Multiple cache layers (vcpkg source, installed, binary)
5. **Maintainability**: Consistent structure across all workflows

## Future Improvements

- [x] Add Windows MSYS2 workflow improvements (completed)
- [x] Upgrade to upload-artifact@v4 (completed)
- [ ] Consider matrix builds for multiple configurations
- [ ] Add benchmark result tracking
- [ ] Implement coverage reporting

## Troubleshooting

### Cache Not Working
1. Check if cache key components changed
2. Verify `VCPKG_BINARY_SOURCES` is set correctly
3. Ensure GitHub Actions cache storage isn't full

### Build Failures
1. Check if fallback to system libraries succeeded
2. Download test artifacts for detailed logs
3. Verify vcpkg.json is compatible with target platform

### Slow Builds Despite Cache
1. Check if vcpkg commit changed (triggers rebuild)
2. Verify binary cache is being used (check logs)
3. Consider if CMake build cache was invalidated

## References

- [Thread System Workflow Improvements](https://github.com/kcenon/thread_system/.github/workflows/README_WORKFLOW_IMPROVEMENTS.md)
- [vcpkg Binary Caching](https://learn.microsoft.com/en-us/vcpkg/users/binarycaching)
- [GitHub Actions Cache](https://docs.github.com/en/actions/using-workflows/caching-dependencies-to-speed-up-workflows)

---

These improvements ensure logger_system CI/CD is fast, reliable, and maintainable across all platforms.