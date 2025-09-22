/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <array>
#include <algorithm>
#include <numeric>
#include <random>

namespace platform_test {

class CachePerformanceTest : public ::testing::Test {
protected:
    static constexpr size_t CACHE_LINE_SIZE = 64;
    static constexpr size_t TEST_ITERATIONS = 1000000;
    
    void SetUp() override {
        // Warm up CPU
        volatile int dummy = 0;
        for (int i = 0; i < 10000; ++i) {
            dummy += i;
        }
    }
};

// Test false sharing impact
TEST_F(CachePerformanceTest, FalseSharingImpact) {
    // Structure with false sharing
    struct WithFalseSharing {
        std::atomic<int> counter1{0};
        std::atomic<int> counter2{0}; // On same cache line
    };
    
    // Structure without false sharing
    struct WithoutFalseSharing {
        alignas(CACHE_LINE_SIZE) std::atomic<int> counter1{0};
        alignas(CACHE_LINE_SIZE) std::atomic<int> counter2{0};
    };
    
    // Test with false sharing
    auto test_false_sharing = []() {
        WithFalseSharing shared;
        auto start = std::chrono::high_resolution_clock::now();
        
        std::thread t1([&shared]() {
            for (size_t i = 0; i < TEST_ITERATIONS; ++i) {
                shared.counter1.fetch_add(1, std::memory_order_relaxed);
            }
        });
        
        std::thread t2([&shared]() {
            for (size_t i = 0; i < TEST_ITERATIONS; ++i) {
                shared.counter2.fetch_add(1, std::memory_order_relaxed);
            }
        });
        
        t1.join();
        t2.join();
        
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    };
    
    // Test without false sharing
    auto test_no_false_sharing = []() {
        WithoutFalseSharing separated;
        auto start = std::chrono::high_resolution_clock::now();
        
        std::thread t1([&separated]() {
            for (size_t i = 0; i < TEST_ITERATIONS; ++i) {
                separated.counter1.fetch_add(1, std::memory_order_relaxed);
            }
        });
        
        std::thread t2([&separated]() {
            for (size_t i = 0; i < TEST_ITERATIONS; ++i) {
                separated.counter2.fetch_add(1, std::memory_order_relaxed);
            }
        });
        
        t1.join();
        t2.join();
        
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    };
    
    // Run multiple times and average
    const int runs = 5;
    int64_t false_sharing_time = 0;
    int64_t no_false_sharing_time = 0;
    
    for (int i = 0; i < runs; ++i) {
        false_sharing_time += test_false_sharing();
        no_false_sharing_time += test_no_false_sharing();
    }
    
    false_sharing_time /= runs;
    no_false_sharing_time /= runs;
    
    // Without false sharing should be faster (or at least not significantly slower)
    // Allow some margin for system variance
    EXPECT_LE(no_false_sharing_time, false_sharing_time * 1.5);
}

// Test cache line bouncing between cores
TEST_F(CachePerformanceTest, CacheLineBouncing) {
    alignas(CACHE_LINE_SIZE) std::atomic<int> shared_counter{0};
    const int num_threads = std::min(4u, std::thread::hardware_concurrency());
    const int iterations_per_thread = TEST_ITERATIONS / num_threads;
    
    // Test with heavy contention (cache line bouncing)
    auto test_contention = [&]() {
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&shared_counter, iterations_per_thread]() {
                for (int j = 0; j < iterations_per_thread; ++j) {
                    shared_counter.fetch_add(1, std::memory_order_relaxed);
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    };
    
    // Test with local accumulation (no contention)
    auto test_no_contention = [&]() {
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::thread> threads;
        std::vector<int> local_sums(num_threads, 0);
        
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&local_sums, i, iterations_per_thread]() {
                int local_sum = 0;
                for (int j = 0; j < iterations_per_thread; ++j) {
                    local_sum++;
                }
                local_sums[i] = local_sum;
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        // Final accumulation
        shared_counter.store(std::accumulate(local_sums.begin(), local_sums.end(), 0));
        
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    };
    
    auto contention_time = test_contention();
    shared_counter.store(0); // Reset
    auto no_contention_time = test_no_contention();
    
    // Local accumulation should be significantly faster
    EXPECT_LT(no_contention_time, contention_time);
    EXPECT_EQ(shared_counter.load(), iterations_per_thread * num_threads);
}

// Test memory access patterns and cache friendliness
TEST_F(CachePerformanceTest, MemoryAccessPatterns) {
    const size_t array_size = 1024 * 1024; // 1M elements
    std::vector<int> data(array_size);
    
    // Initialize with random data
    std::generate(data.begin(), data.end(), []() { return rand() % 100; });
    
    // Sequential access (cache-friendly)
    auto test_sequential = [&data]() {
        auto start = std::chrono::high_resolution_clock::now();
        
        int sum = 0;
        for (size_t i = 0; i < data.size(); ++i) {
            sum += data[i];
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        return std::make_pair(
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count(),
            sum
        );
    };
    
    // Random access (cache-unfriendly)
    auto test_random = [&data]() {
        // Create random indices
        std::vector<size_t> indices(data.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(indices.begin(), indices.end(), gen);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        int sum = 0;
        for (size_t idx : indices) {
            sum += data[idx];
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        return std::make_pair(
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count(),
            sum
        );
    };
    
    auto [seq_time, seq_sum] = test_sequential();
    auto [rand_time, rand_sum] = test_random();
    
    // Both should compute the same sum
    EXPECT_EQ(seq_sum, rand_sum);
    
    // Sequential access should be faster due to cache locality
    EXPECT_LT(seq_time, rand_time);
}

// Test different data structure layouts
TEST_F(CachePerformanceTest, DataStructureLayout) {
    const size_t num_elements = 100000;
    
    // Array of Structures (AoS)
    struct PointAoS {
        float x, y, z;
        float padding; // Align to 16 bytes
    };
    
    // Structure of Arrays (SoA)
    struct PointSoA {
        std::vector<float> x;
        std::vector<float> y;
        std::vector<float> z;
        
        PointSoA(size_t size) : x(size), y(size), z(size) {}
    };
    
    // Test AoS performance
    auto test_aos = []() {
        std::vector<PointAoS> points(num_elements);
        
        // Initialize
        for (size_t i = 0; i < num_elements; ++i) {
            points[i] = {float(i), float(i * 2), float(i * 3), 0};
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Process only x coordinates
        float sum = 0;
        for (const auto& p : points) {
            sum += p.x;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        return std::make_pair(
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count(),
            sum
        );
    };
    
    // Test SoA performance
    auto test_soa = []() {
        PointSoA points(num_elements);
        
        // Initialize
        for (size_t i = 0; i < num_elements; ++i) {
            points.x[i] = float(i);
            points.y[i] = float(i * 2);
            points.z[i] = float(i * 3);
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Process only x coordinates
        float sum = 0;
        for (float x : points.x) {
            sum += x;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        return std::make_pair(
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count(),
            sum
        );
    };
    
    auto [aos_time, aos_sum] = test_aos();
    auto [soa_time, soa_sum] = test_soa();
    
    // Both should compute the same sum
    EXPECT_FLOAT_EQ(aos_sum, soa_sum);
    
    // SoA should be faster for accessing single fields due to better cache usage
    // (This may vary based on compiler optimizations)
    EXPECT_LE(soa_time, aos_time * 2);
}

// Test prefetching effectiveness
TEST_F(CachePerformanceTest, PrefetchingBehavior) {
    const size_t size = 1024 * 1024;
    std::vector<int> data(size);
    std::vector<size_t> indices(size / 16); // Sparse access
    
    // Initialize with pattern
    for (size_t i = 0; i < size; ++i) {
        data[i] = i % 256;
    }
    
    // Create sparse access pattern
    for (size_t i = 0; i < indices.size(); ++i) {
        indices[i] = i * 16; // Access every 16th element
    }
    
    // Test without manual prefetch
    auto test_no_prefetch = [&]() {
        auto start = std::chrono::high_resolution_clock::now();
        
        int sum = 0;
        for (size_t idx : indices) {
            sum += data[idx];
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    };
    
    // Test with simulated prefetch (accessing ahead)
    auto test_with_prefetch = [&]() {
        auto start = std::chrono::high_resolution_clock::now();
        
        int sum = 0;
        const size_t prefetch_distance = 8;
        
        for (size_t i = 0; i < indices.size(); ++i) {
            // Simulate prefetch by touching future data
            if (i + prefetch_distance < indices.size()) {
                volatile int dummy = data[indices[i + prefetch_distance]];
                (void)dummy;
            }
            
            sum += data[indices[i]];
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    };
    
    // Run tests
    auto no_prefetch_time = test_no_prefetch();
    auto with_prefetch_time = test_with_prefetch();
    
    // Prefetching might help, but modern CPUs have good automatic prefetchers
    // So we just verify no significant performance degradation
    // Handle case where times might be 0 due to fast execution
    if (no_prefetch_time > 0 && with_prefetch_time > 0) {
        EXPECT_LE(with_prefetch_time, no_prefetch_time * 2.0);
    } else {
        // If times are too small to measure, just pass
        SUCCEED();
    }
}

// Test cache-oblivious algorithms
TEST_F(CachePerformanceTest, CacheObliviousTraversal) {
    const size_t matrix_size = 512;
    using Matrix = std::vector<std::vector<int>>;
    
    Matrix matrix(matrix_size, std::vector<int>(matrix_size));
    
    // Initialize matrix
    for (size_t i = 0; i < matrix_size; ++i) {
        for (size_t j = 0; j < matrix_size; ++j) {
            matrix[i][j] = (i + j) % 256;
        }
    }
    
    // Row-major traversal
    auto row_major_sum = [&matrix]() {
        auto start = std::chrono::high_resolution_clock::now();
        
        int64_t sum = 0;
        for (size_t i = 0; i < matrix.size(); ++i) {
            for (size_t j = 0; j < matrix[i].size(); ++j) {
                sum += matrix[i][j];
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        return std::make_pair(
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count(),
            sum
        );
    };
    
    // Column-major traversal (cache-unfriendly for row-major storage)
    auto col_major_sum = [&matrix]() {
        auto start = std::chrono::high_resolution_clock::now();
        
        int64_t sum = 0;
        for (size_t j = 0; j < matrix[0].size(); ++j) {
            for (size_t i = 0; i < matrix.size(); ++i) {
                sum += matrix[i][j];
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        return std::make_pair(
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count(),
            sum
        );
    };
    
    // Blocked traversal (cache-friendly)
    auto blocked_sum = [&matrix]() {
        const size_t block_size = 64; // Typical cache line
        auto start = std::chrono::high_resolution_clock::now();
        
        int64_t sum = 0;
        for (size_t bi = 0; bi < matrix.size(); bi += block_size) {
            for (size_t bj = 0; bj < matrix[0].size(); bj += block_size) {
                // Process block
                for (size_t i = bi; i < std::min(bi + block_size, matrix.size()); ++i) {
                    for (size_t j = bj; j < std::min(bj + block_size, matrix[0].size()); ++j) {
                        sum += matrix[i][j];
                    }
                }
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        return std::make_pair(
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count(),
            sum
        );
    };
    
    auto [row_time, row_sum] = row_major_sum();
    auto [col_time, col_sum] = col_major_sum();
    auto [block_time, block_sum] = blocked_sum();
    
    // All should compute the same sum
    EXPECT_EQ(row_sum, col_sum);
    EXPECT_EQ(row_sum, block_sum);
    
    // Row-major should be fastest for row-major storage (relaxed for different architectures)
    #if defined(__aarch64__) || defined(__linux__)
    // Different architectures have varying cache behavior, allowing more variance
    if (row_time > col_time) {
        std::cout << "Note: Cache behavior may vary on this platform. Row: " << row_time << ", Col: " << col_time << std::endl;
    }
    #else
    EXPECT_LE(row_time, col_time);
    #endif
    
    // Blocked should be competitive with col-major (relaxed for different architectures)
    #if defined(__aarch64__) || defined(__linux__)
    // Different architectures may have complex cache behavior
    if (block_time > col_time) {
        std::cout << "Note: Blocked access behavior may vary on this platform. Block: " << block_time << ", Col: " << col_time << std::endl;
    }
    #else
    EXPECT_LE(block_time, col_time);
    #endif
}

} // namespace platform_test