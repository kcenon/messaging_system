# GitHub Workflows — vcpkg Caching & Fallback Improvements

This document summarizes the CI improvements to make builds faster and more reliable by:
- Enabling vcpkg binary caching (x‑gha) so dependencies are not rebuilt when nothing changed.
- Keeping the existing graceful fallback to system libraries when vcpkg fails.

Updated: 2025-09-08 (Asia/Seoul)

## What Changed

1) vcpkg binary cache enabled
- Added environment variables to all relevant jobs:
  - `VCPKG_BINARY_SOURCES="clear;x-gha,readwrite"`
  - `VCPKG_FEATURE_FLAGS="manifests,registries,versions,binarycaching"`
- Effect: vcpkg reuses prebuilt artifacts via GitHub Actions cache storage and uploads new builds automatically.

2) Stronger cache keys for vcpkg_installed
- Cache key now includes platform, triplet, manifest hash, optional `vcpkg-configuration.json` hash, and vcpkg commit:
  - `${{ runner.os }}-<flavor>-vcpkg-installed-${{ env.VCPKG_DEFAULT_TRIPLET }}-${{ hashFiles('vcpkg.json','vcpkg-configuration.json') }}-${{ steps.vcpkg-commit.outputs.commit }}`
- A dedicated step determines the vcpkg git commit after cloning/pulling vcpkg.
- Result: cache invalidates only when it should (manifest/triplet/vcpkg version changes), avoiding unnecessary rebuilds.

3) Triplet standardized per platform
- `VCPKG_DEFAULT_TRIPLET` is set explicitly:
  - Ubuntu (GCC/Clang): `x64-linux`
  - Windows (Visual Studio): `x64-windows`
  - Windows (MinGW/MSYS2): `x64-mingw-dynamic`
- `vcpkg install` uses `--triplet $VCPKG_DEFAULT_TRIPLET` for consistency and cache isolation.

4) Install step skips on cache hit
- The `Install dependencies with vcpkg` step only runs when the `vcpkg_installed` cache misses.
- Combined with the x‑gha binary cache, dependency build work is minimized on repeat runs.

## Affected Workflows

- Ubuntu GCC: `.github/workflows/build-ubuntu-gcc.yaml`
- Ubuntu Clang: `.github/workflows/build-ubuntu-clang.yaml`
- Windows Visual Studio: `.github/workflows/build-windows-vs.yaml`
- Windows MinGW: `.github/workflows/build-windows-mingw.yaml`
- Windows MSYS2: `.github/workflows/build-windows-msys2.yaml`

All of the above received the caching changes while preserving the existing fallback logic.

## How It Works

1) vcpkg sources are cloned/updated and bootstrapped as before.
2) The workflow reads the vcpkg commit (`git -C vcpkg rev-parse HEAD`).
3) `actions/cache` restores `vcpkg_installed` using a key that combines:
   - OS + flavor (gcc/clang/vs/msys2/mingw)
   - Triplet (per‑platform)
   - `hashFiles('vcpkg.json','vcpkg-configuration.json')` (the latter is optional; empty hash if missing)
   - vcpkg commit hash
4) If cache hit: skip `vcpkg install`.
5) If cache miss: run `vcpkg install` (which also benefits from x‑gha binary cache), then proceed to build.
6) If the “vcpkg first attempt” build fails, the workflows still fall back to system libraries and run a minimal verification test.

## Verifying in Logs

Look for the following in the Actions logs:
- `Cache restored from key: ... vcpkg-installed-...` → vcpkg installation cache hit
- `Using binary cache` or fast `vcpkg install` times → x‑gha cache effective
- `Install dependencies with vcpkg` step is skipped when `cache-hit == 'true'`
- If vcpkg build fails: a second CMake configure/build without the toolchain file

## When Caches Invalidate

- `vcpkg.json` changes (add/remove/update dependencies)
- `vcpkg-configuration.json` changes (registries/versions), if present
- Triplet changes (`VCPKG_DEFAULT_TRIPLET`)
- vcpkg repository commit changes (after `git pull`)
- Runner OS/flavor changes (e.g., gcc → clang)

## Notes & Caveats

- MSYS2 job treats vcpkg as optional; commit detection and caching run only if vcpkg setup succeeds.
- ARM64 runners may set `VCPKG_FORCE_SYSTEM_BINARIES=arm`; this is orthogonal to caching and remains supported.
- The CMake build directory is still recreated in each run by default. If you want to speed up full rebuilds further, consider:
  - Removing unconditional `rm -rf build` and relying on the existing CMake build cache action.
  - Tightening workflow triggers with `paths:` filters so builds run only when relevant files change.

## Why This Matters

- Faster CI with fewer dependency rebuilds.
- More deterministic cache invalidation tied to actual sources of change.
- Resilient builds thanks to the existing system‑library fallback path.

## Example Snippets

Environment variables in jobs:

```
env:
  BUILD_TYPE: Debug
  VCPKG_BINARY_SOURCES: "clear;x-gha,readwrite"
  VCPKG_FEATURE_FLAGS: "manifests,registries,versions,binarycaching"
  VCPKG_DEFAULT_TRIPLET: x64-linux  # or x64-windows / x64-mingw-dynamic
```

Determine vcpkg commit (Linux/MSYS2 shell):

```
- name: Determine vcpkg commit
  id: vcpkg-commit
  run: echo "commit=$(git -C vcpkg rev-parse HEAD)" >> $GITHUB_OUTPUT
```

Cache key structure:

```
key: ${{ runner.os }}-<flavor>-vcpkg-installed-${{ env.VCPKG_DEFAULT_TRIPLET }}-
     ${{ hashFiles('vcpkg.json','vcpkg-configuration.json') }}-
     ${{ steps.vcpkg-commit.outputs.commit }}
```

Install using the selected triplet:

```
./vcpkg/vcpkg install \
  --x-manifest-root=. \
  --x-install-root=${{ github.workspace }}/vcpkg_installed \
  --triplet $VCPKG_DEFAULT_TRIPLET
```

These changes ensure the Thread System remains buildable, testable, and fast across platforms while avoiding unnecessary rebuilds when nothing has changed.
