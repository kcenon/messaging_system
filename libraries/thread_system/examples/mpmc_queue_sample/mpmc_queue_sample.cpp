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

#include "thread_base/lockfree/queues/lockfree_job_queue.h"
#include "thread_base/jobs/callback_job.h"
#include "logger/core/logger.h"

#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <random>

using namespace kcenon::thread;
using namespace log_module;
using namespace std::chrono_literals;

// Example 1: Basic single producer, single consumer
void basic_spsc_example()
{
    write_information("[Example 1] Basic SPSC Pattern");
    
    lockfree_job_queue queue;
    std::atomic<int> counter{0};
    
    // Producer thread
    std::thread producer([&queue, &counter]() {
        for (int i = 0; i < 10; ++i) {
            auto job = std::make_unique<callback_job>([&counter, i]() -> result_void {
                counter.fetch_add(1);
                write_information("Processed job {}", i);
                return result_void(); // Success
            });
            
            auto result = queue.enqueue(std::move(job));
            if (result.has_error()) {
                write_error(
                    "Failed to enqueue job {}: {}", i, result.get_error().message());
            }
            
            std::this_thread::sleep_for(10ms);
        }
        write_information("Producer finished");
    });
    
    // Consumer thread
    std::thread consumer([&queue, &counter]() {
        int consumed = 0;
        while (consumed < 10) {
            auto result = queue.dequeue();
            if (result.has_value()) {
                auto& job = result.value();
                auto work_result = job->do_work();
                if (!work_result) {
                    // Job executed successfully
                    consumed++;
                } else {
                    write_error("Job failed: {}", work_result.get_error().message());
                }
            } else {
                std::this_thread::sleep_for(5ms);
            }
        }
        write_information("Consumer finished");
    });
    
    producer.join();
    consumer.join();
    
    write_information(
        "Total jobs processed: {}", counter.load());
}

// Example 2: Multiple producers, multiple consumers
void mpmc_example()
{
    write_information("\n[Example 2] MPMC Pattern");
    
    lockfree_job_queue queue;
    std::atomic<int> produced{0};
    std::atomic<int> consumed{0};
    const int num_producers = 3;
    const int num_consumers = 2;
    const int jobs_per_producer = 20;
    
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;
    
    // Start producers
    for (int p = 0; p < num_producers; ++p) {
        producers.emplace_back([&queue, &produced, p, jobs_per_producer]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> delay_dist(1, 10);
            
            for (int i = 0; i < jobs_per_producer; ++i) {
                auto job = std::make_unique<callback_job>(
                    [p, i]() -> result_void {
                        write_information("Job from producer {} #{}", p, i);
                        return result_void();
                    });
                
                // Retry on failure (high contention scenario)
                while (true) {
                    auto result = queue.enqueue(std::move(job));
                    if (!result.has_error()) {
                        produced.fetch_add(1);
                        break;
                    }
                    std::this_thread::yield();
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(delay_dist(gen)));
            }
            
            write_information(
                "Producer {} finished", p);
        });
    }
    
    // Start consumers
    for (int c = 0; c < num_consumers; ++c) {
        consumers.emplace_back([&queue, &consumed, c, num_producers, jobs_per_producer]() {
            const int total_jobs = num_producers * jobs_per_producer;
            
            while (consumed.load() < total_jobs) {
                auto result = queue.dequeue();
                if (result.has_value()) {
                    auto& job = result.value();
                    auto work_result = job->do_work();
                    if (!work_result) {
                        // Job executed successfully
                        consumed.fetch_add(1);
                    } else {
                        write_error("Consumer {} job failed: {}", c, work_result.get_error().message());
                    }
                } else {
                    std::this_thread::sleep_for(1ms);
                }
            }
            
            write_information(
                "Consumer {} finished", c);
        });
    }
    
    // Wait for all threads
    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();
    
    write_information(
        "Total produced: {}, consumed: {}", 
            produced.load(), consumed.load());
}

// Example 3: Batch operations
void batch_operations_example()
{
    write_information("\n[Example 3] Batch Operations");
    
    lockfree_job_queue queue;
    std::atomic<int> processed{0};
    
    // Batch enqueue
    std::vector<std::unique_ptr<job>> batch;
    for (int i = 0; i < 50; ++i) {
        batch.push_back(std::make_unique<callback_job>(
            [&processed, i]() -> result_void {
                processed.fetch_add(1);
                write_information("Batch job {}", i);
                return result_void();
            }));
    }
    
    write_information(
        "Enqueueing {} jobs in batch", batch.size());
    
    auto enqueue_result = queue.enqueue_batch(std::move(batch));
    if (enqueue_result.has_error()) {
        write_error(
            "Batch enqueue failed: {}", enqueue_result.get_error().message());
        return;
    }
    
    // Batch dequeue
    auto dequeued = queue.dequeue_batch();
    write_information(
        "Dequeued {} jobs in batch", dequeued.size());
    
    // Process all dequeued jobs
    for (auto& job : dequeued) {
        auto result = job->do_work();
        if (!result) {
            // Job executed successfully
        } else {
            write_error("Batch job failed: {}", result.get_error().message());
        }
    }
    
    write_information(
        "Total processed: {}", processed.load());
}

// Example 4: Performance measurement
void performance_example()
{
    write_information("\n[Example 4] Performance Measurement");
    
    lockfree_job_queue queue;
    const int num_operations = 100000;
    
    // Measure enqueue performance
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_operations; ++i) {
        auto job = std::make_unique<callback_job>([]() -> result_void {
            return result_void();
        });
        
        while (true) {
            auto r = queue.enqueue(std::move(job));
            if (!r.has_error()) break;
            std::this_thread::yield();
            job = std::make_unique<callback_job>([]() -> result_void { return result_void(); });
        }
    }
    
    auto enqueue_time = std::chrono::high_resolution_clock::now() - start;
    
    // Measure dequeue performance
    start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_operations; ++i) {
        while (true) {
            auto result = queue.dequeue();
            if (result.has_value()) {
                break;
            }
            std::this_thread::yield();
        }
    }
    
    auto dequeue_time = std::chrono::high_resolution_clock::now() - start;
    
    // Get statistics
    auto stats = queue.get_statistics();
    
    write_information(
        "Enqueue performance: {} ops in {} ms = {} ops/sec",
            num_operations,
            std::chrono::duration_cast<std::chrono::milliseconds>(enqueue_time).count(),
            num_operations * 1000.0 / std::chrono::duration_cast<std::chrono::milliseconds>(enqueue_time).count());
    
    write_information(
        "Dequeue performance: {} ops in {} ms = {} ops/sec",
            num_operations,
            std::chrono::duration_cast<std::chrono::milliseconds>(dequeue_time).count(),
            num_operations * 1000.0 / std::chrono::duration_cast<std::chrono::milliseconds>(dequeue_time).count());
    
    write_information(
        "Queue statistics:\n"
            "  Enqueued: {}\n"
            "  Dequeued: {}\n"
            "  Retries: {}\n"
            "  Average enqueue latency: {} ns\n"
            "  Average dequeue latency: {} ns",
            stats.enqueue_count,
            stats.dequeue_count,
            stats.retry_count,
            stats.get_average_enqueue_latency_ns(),
            stats.get_average_dequeue_latency_ns());
}

int main()
{
    log_module::start();
    log_module::console_target(log_types::Debug);
    
    write_information(
        "Lock-Free MPMC Queue Sample\n"
        "===========================");
    
    try {
        basic_spsc_example();
        mpmc_example();
        batch_operations_example();
        performance_example();
    } catch (const std::exception& e) {
        write_error(
            "Exception: {}", e.what());
    }
    
    write_information("\nAll examples completed!");
    
    log_module::stop();
    return 0;
}
