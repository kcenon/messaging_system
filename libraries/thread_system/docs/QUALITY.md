# Quality & QA Guide

## Coverage

Build with coverage and generate reports:

```bash
cmake -S . -B build -DENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build --output-on-failure
cmake --build build --target coverage
```

Outputs are generated under `build/coverage/`.

Notes
- Requires `lcov` and `genhtml`.
- On macOS, install via Homebrew: `brew install lcov`.

## Sanitizers

Enable sanitizers via CMake options:

```bash
cmake -S . -B build -DENABLE_ASAN=ON -DENABLE_UBSAN=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build --output-on-failure
```

For data races:

```bash
cmake -S . -B build-tsan -DENABLE_TSAN=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-tsan -j
ctest --test-dir build-tsan --output-on-failure
```

## Benchmarks

Build benchmarks:

```bash
cmake -S . -B build -DBUILD_BENCHMARKS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --target benchmarks
cmake --build build --target run_all_benchmarks
```

Compare results using JSON output (Google Benchmark):

```bash
./bin/thread_pool_benchmark --benchmark_format=json --benchmark_out=baseline.json
./bin/thread_pool_benchmark --benchmark_format=json --benchmark_out=current.json
python3 scripts/benchmark_compare.py baseline.json current.json
```

## Static Analysis

```bash
cmake -S . -B build -DENABLE_CLANG_TIDY=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```
