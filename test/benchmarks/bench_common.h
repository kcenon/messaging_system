#pragma once

#include <chrono>
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <numeric>

namespace kcenon::messaging::benchmark {

/**
 * @class BenchmarkTimer
 * @brief Simple timer for benchmarking
 */
class BenchmarkTimer {
    std::chrono::steady_clock::time_point start_;

public:
    BenchmarkTimer() : start_(std::chrono::steady_clock::now()) {}

    void reset() {
        start_ = std::chrono::steady_clock::now();
    }

    template<typename Duration = std::chrono::milliseconds>
    typename Duration::rep elapsed() const {
        auto end = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<Duration>(end - start_).count();
    }

    double elapsed_seconds() const {
        return elapsed<std::chrono::nanoseconds>() / 1'000'000'000.0;
    }
};

/**
 * @class BenchmarkResults
 * @brief Store and analyze benchmark results
 */
class BenchmarkResults {
    std::vector<double> durations_;
    std::string name_;

public:
    explicit BenchmarkResults(std::string name) : name_(std::move(name)) {}

    void add_duration(double duration_ms) {
        durations_.push_back(duration_ms);
    }

    void print() const {
        if (durations_.empty()) {
            std::cout << name_ << ": No results\n";
            return;
        }

        auto sorted = durations_;
        std::sort(sorted.begin(), sorted.end());

        double sum = std::accumulate(sorted.begin(), sorted.end(), 0.0);
        double mean = sum / sorted.size();

        double p50 = sorted[sorted.size() / 2];
        double p95 = sorted[static_cast<size_t>(sorted.size() * 0.95)];
        double p99 = sorted[static_cast<size_t>(sorted.size() * 0.99)];
        double min = sorted.front();
        double max = sorted.back();

        std::cout << "\n=== " << name_ << " ===\n";
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "  Count: " << sorted.size() << "\n";
        std::cout << "  Mean:  " << mean << " ms\n";
        std::cout << "  Min:   " << min << " ms\n";
        std::cout << "  Max:   " << max << " ms\n";
        std::cout << "  P50:   " << p50 << " ms\n";
        std::cout << "  P95:   " << p95 << " ms\n";
        std::cout << "  P99:   " << p99 << " ms\n";
    }

    double mean() const {
        if (durations_.empty()) return 0.0;
        double sum = std::accumulate(durations_.begin(), durations_.end(), 0.0);
        return sum / durations_.size();
    }

    double p99() const {
        if (durations_.empty()) return 0.0;
        auto sorted = durations_;
        std::sort(sorted.begin(), sorted.end());
        return sorted[static_cast<size_t>(sorted.size() * 0.99)];
    }
};

/**
 * @brief Run a benchmark iteration
 */
template<typename Func>
double measure(Func&& func) {
    BenchmarkTimer timer;
    func();
    return timer.elapsed<std::chrono::nanoseconds>() / 1'000'000.0; // Convert to ms
}

/**
 * @brief Run benchmark multiple times and collect results
 */
template<typename Func>
BenchmarkResults run_benchmark(const std::string& name, int iterations, Func&& func) {
    BenchmarkResults results(name);

    std::cout << "Running " << name << " (" << iterations << " iterations)...\n";

    for (int i = 0; i < iterations; ++i) {
        double duration = measure(func);
        results.add_duration(duration);

        if ((i + 1) % (iterations / 10) == 0) {
            std::cout << "  Progress: " << (i + 1) << "/" << iterations << "\n";
        }
    }

    results.print();
    return results;
}

/**
 * @brief Calculate throughput
 */
inline void print_throughput(const std::string& name, int operations, double duration_seconds) {
    double throughput = operations / duration_seconds;

    std::cout << "\n=== " << name << " Throughput ===\n";
    std::cout << std::fixed << std::setprecision(0);
    std::cout << "  Operations: " << operations << "\n";
    std::cout << "  Duration:   " << std::setprecision(3) << duration_seconds << " seconds\n";
    std::cout << "  Throughput: " << std::setprecision(0) << throughput << " ops/sec\n";
}

} // namespace kcenon::messaging::benchmark
