#pragma once

#include <benchmark/benchmark.h>

namespace bench_framework {

inline void set_common_benchmark_config() {
    // Customize common settings if needed
    ::benchmark::Initialize(nullptr, nullptr);
}

template <class Fn>
inline void run(Fn&&) {
    // Placeholder for custom runner extensions.
}

} // namespace bench_framework

