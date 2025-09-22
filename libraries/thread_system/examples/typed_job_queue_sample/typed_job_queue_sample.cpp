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

#include "typed_thread_pool/scheduling/typed_job_queue.h"
#include "typed_thread_pool/jobs/typed_job.h"
#include "typed_thread_pool/jobs/callback_typed_job.h"
#include "logger/core/logger.h"

#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <random>
#include <map>

using namespace kcenon::thread;
using namespace kcenon::thread;
using namespace log_module;
using namespace std::chrono_literals;

// Example 1: Basic typed job queue usage with lock-free MPMC
void basic_typed_queue_example()
{
    write_information("[Example 1] Basic Typed Job Queue (Lock-free MPMC)");
    
    typed_job_queue_t<job_types> queue;
    std::atomic<int> high_jobs{0};
    std::atomic<int> normal_jobs{0};
    std::atomic<int> low_jobs{0};
    
    // Producer thread - creates jobs of different types
    std::thread producer([&queue, &high_jobs, &normal_jobs, &low_jobs]() {
        for (int i = 0; i < 30; ++i) {
            job_types type;
            std::atomic<int>* counter;
            
            if (i % 3 == 0) {
                type = job_types::RealTime;
                counter = &high_jobs;
            } else if (i % 3 == 1) {
                type = job_types::Batch;
                counter = &normal_jobs;
            } else {
                type = job_types::Background;
                counter = &low_jobs;
            }
            
            // Create callback_typed_job directly with lambda and type
            auto typed_job_ptr = std::make_unique<callback_typed_job>(
                [counter, i, type]() -> result_void {
                    counter->fetch_add(1);
                    std::string type_str = type == job_types::RealTime ? "RealTime" : 
                                         (type == job_types::Batch ? "Batch" : "Background");
                    write_information("{} priority job {} completed", type_str, i);
                    return result_void(); // Success
                },
                type
            );
            
            auto result = queue.enqueue(std::move(typed_job_ptr));
            if (result.has_error()) {
                write_error(
                    "Failed to enqueue job {}: {}", i, result.get_error().message());
            }
            
            std::this_thread::sleep_for(10ms);
        }
        write_information("Producer finished");
    });
    
    // Consumer thread - processes jobs respecting type order
    std::thread consumer([&queue]() {
        int total_consumed = 0;
        
        while (total_consumed < 30) {
            // Try to get high priority first
            auto job = queue.dequeue({job_types::RealTime, job_types::Batch, job_types::Background});
            
            if (job.has_value()) {
                auto result = job.value()->do_work();
                if (!result.has_error()) {
                    // Job executed successfully
                    total_consumed++;
                } else {
                    write_error("Job failed: {}", result.get_error().message());
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
        "Jobs processed - RealTime: {}, Batch: {}, Background: {}", 
            high_jobs.load(), normal_jobs.load(), low_jobs.load());
}

// Example 2: Multiple producers and consumers with type-based processing
void mpmc_typed_queue_example()
{
    write_information("\n[Example 2] MPMC Typed Queue Processing");
    
    typed_job_queue_t<job_types> queue;
    const int num_producers = 4;
    const int num_consumers = 3;
    const int jobs_per_producer = 25;
    
    std::atomic<int> total_produced{0};
    std::atomic<int> total_consumed{0};
    std::map<job_types, std::atomic<int>> type_counts;
    type_counts[job_types::RealTime].store(0);
    type_counts[job_types::Batch].store(0);
    type_counts[job_types::Background].store(0);
    
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;
    
    // Start multiple producers
    for (int p = 0; p < num_producers; ++p) {
        producers.emplace_back([&queue, &total_produced, &type_counts, p, jobs_per_producer]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> type_dist(0, 2);
            std::uniform_int_distribution<> delay_dist(1, 10);
            
            for (int i = 0; i < jobs_per_producer; ++i) {
                job_types type = static_cast<job_types>(type_dist(gen));
                
                auto job = std::make_unique<callback_typed_job>(
                    [p, i, type]() -> result_void {
                        // Simulate work based on priority
                        std::this_thread::sleep_for(std::chrono::microseconds(
                            type == job_types::RealTime ? 10 : 
                            (type == job_types::Batch ? 50 : 100)));
                        write_information("Producer {} job {} (type: {})", p, i, static_cast<int>(type));
                        return result_void();
                    },
                    type
                );
                
                // Retry on failure with lock-free queue
                while (true) {
                    auto result = queue.enqueue(std::move(job));
                    if (!result.has_error()) {
                        total_produced.fetch_add(1);
                        type_counts[type].fetch_add(1);
                        break;
                    }
                    std::this_thread::yield();
                    // Re-create job since it was moved
                    job = std::make_unique<callback_typed_job>(
                        [p, i, type]() -> result_void {
                            // Simulate work based on priority
                            std::this_thread::sleep_for(std::chrono::microseconds(
                                type == job_types::RealTime ? 10 : 
                                (type == job_types::Batch ? 50 : 100)));
                            write_information("Producer {} job {} (type: {})", p, i, static_cast<int>(type));
                            return result_void();
                        },
                        type
                    );
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(delay_dist(gen)));
            }
            
            write_information(
                "Producer {} finished", p);
        });
    }
    
    // Start multiple consumers with different type preferences
    for (int c = 0; c < num_consumers; ++c) {
        consumers.emplace_back([&queue, &total_consumed, c, num_producers, jobs_per_producer]() {
            const int total_jobs = num_producers * jobs_per_producer;
            std::vector<job_types> preference;
            
            // Different consumers have different type preferences
            if (c == 0) {
                preference = {job_types::RealTime, job_types::Batch, job_types::Background};
            } else if (c == 1) {
                preference = {job_types::Batch, job_types::RealTime, job_types::Background};
            } else {
                preference = {job_types::Background, job_types::Batch, job_types::RealTime};
            }
            
            while (total_consumed.load() < total_jobs) {
                auto job = queue.dequeue(preference);
                
                if (job.has_value()) {
                    auto result = job.value()->do_work();
                    if (!result) {
                        // Job executed successfully
                        total_consumed.fetch_add(1);
                    } else {
                        write_error("Consumer {} job failed: {}", 
                            c, result.get_error().message());
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
        "Total jobs - Produced: {}, Consumed: {}", 
            total_produced.load(), total_consumed.load());
    write_information(
        "By type - High: {}, Normal: {}, Low: {}", 
            type_counts[job_types::RealTime].load(),
            type_counts[job_types::Batch].load(),
            type_counts[job_types::Background].load());
}

// Example 3: Performance comparison between type-based and non-typed processing
void performance_comparison_example()
{
    write_information("\n[Example 3] Performance Comparison");
    
    const int num_jobs = 50000;
    const int num_workers = 4;
    
    // Test typed queue with lock-free MPMC
    {
        typed_job_queue_t<job_types> queue;
        std::atomic<int> completed{0};
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Enqueue all jobs
        for (int i = 0; i < num_jobs; ++i) {
            job_types type = static_cast<job_types>(i % 3);
            auto job = std::make_unique<callback_typed_job>(
                [&completed]() -> result_void {
                    completed.fetch_add(1);
                    return result_void();
                },
                type
            );
            
            auto enqueue_result = queue.enqueue(std::move(job));
            while (enqueue_result.has_error()) {
                std::this_thread::yield();
                // Re-create job since it was moved
                job = std::make_unique<callback_typed_job>(
                    [&completed]() -> result_void {
                        completed.fetch_add(1);
                        return result_void();
                    },
                    type
                );
                enqueue_result = queue.enqueue(std::move(job));
            }
        }
        
        // Process with multiple workers
        std::vector<std::thread> workers;
        for (int w = 0; w < num_workers; ++w) {
            workers.emplace_back([&queue, &completed, num_jobs]() {
                while (completed.load() < num_jobs) {
                    auto job = queue.dequeue({job_types::RealTime, job_types::Batch, job_types::Background});
                    if (job.has_value()) {
                        auto work_result = job.value()->do_work();
                        (void)work_result; // Ignore result for sample
                    } else {
                        std::this_thread::yield();
                    }
                }
            });
        }
        
        for (auto& t : workers) t.join();
        
        auto duration = std::chrono::high_resolution_clock::now() - start;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        
        write_information(
            "Typed queue (lock-free): {} jobs in {} ms = {} ops/sec",
                num_jobs, ms, num_jobs * 1000.0 / ms);
    }
}

// Example 4: Real-world scenario - Task scheduling system
void task_scheduling_example()
{
    write_information("\n[Example 4] Task Scheduling System");
    
    typed_job_queue_t<job_types> task_queue;
    std::atomic<bool> system_running{true};
    
    // Task statistics
    struct TaskStats {
        std::atomic<int> created{0};
        std::atomic<int> completed{0};
        std::atomic<int> failed{0};
        std::atomic<int64_t> total_latency_us{0};
    };
    
    std::map<job_types, TaskStats> stats;
    
    // Task generator thread
    std::thread generator([&task_queue, &system_running, &stats]() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> type_dist(0, 2);
        std::uniform_int_distribution<> delay_dist(10, 100);
        
        while (system_running) {
            job_types type = static_cast<job_types>(type_dist(gen));
            auto creation_time = std::chrono::high_resolution_clock::now();
            
            auto task = std::make_unique<callback_typed_job>(
                [type, creation_time, &stats]() -> result_void {
                        auto start_time = std::chrono::high_resolution_clock::now();
                        auto latency = std::chrono::duration_cast<std::chrono::microseconds>(
                            start_time - creation_time).count();
                        
                        stats[type].total_latency_us.fetch_add(latency);
                        
                        // Simulate task execution
                        std::this_thread::sleep_for(std::chrono::microseconds(
                            type == job_types::RealTime ? 50 : 
                            (type == job_types::Batch ? 200 : 500)));
                        
                        stats[type].completed.fetch_add(1);
                        
                        write_information("Task completed - Type: {}, Latency: {} μs",
                            static_cast<int>(type), latency);
                        return result_void(); // Success
                    },
                    type
            );
            
            auto enqueue_result = task_queue.enqueue(std::move(task));
            if (!enqueue_result.has_error()) {
                stats[type].created.fetch_add(1);
            } else {
                stats[type].failed.fetch_add(1);
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_dist(gen)));
        }
    });
    
    // Worker threads with type specialization
    std::vector<std::thread> workers;
    
    // High priority specialist
    workers.emplace_back([&task_queue, &system_running]() {
        while (system_running) {
            auto task = task_queue.dequeue({job_types::RealTime});
            if (task.has_value()) {
                auto result = task.value()->do_work();
                if (!result) {
                    // Task executed successfully
                } else {
                    write_error("High priority task failed: {}", 
                        result.get_error().message());
                }
            } else {
                std::this_thread::sleep_for(1ms);
            }
        }
    });
    
    // General workers
    for (int i = 0; i < 2; ++i) {
        workers.emplace_back([&task_queue, &system_running, i]() {
            while (system_running) {
                auto task = task_queue.dequeue({job_types::Batch, job_types::Background, job_types::RealTime});
                if (task.has_value()) {
                    auto result = task.value()->do_work();
                    if (!result) {
                        // Task executed successfully
                    } else {
                        write_error("General worker {} task failed: {}", 
                            i, result.get_error().message());
                    }
                } else {
                    std::this_thread::sleep_for(2ms);
                }
            }
        });
    }
    
    // Run for 5 seconds
    std::this_thread::sleep_for(5s);
    system_running = false;
    
    generator.join();
    for (auto& t : workers) t.join();
    
    // Print statistics
    write_information("Task Scheduling Statistics:");
    for (const auto& [type, stat] : stats) {
        std::string type_name = type == job_types::RealTime ? "RealTime" : 
                               (type == job_types::Batch ? "Batch" : "Background");
        
        int completed = stat.completed.load();
        double avg_latency = completed > 0 ? 
            static_cast<double>(stat.total_latency_us.load()) / completed : 0.0;
        
        write_information(
            "  {} - Created: {}, Completed: {}, Failed: {}, Avg Latency: {:.1f} μs",
                type_name, stat.created.load(), completed, 
                stat.failed.load(), avg_latency);
    }
}

// Example 5: Stress test with high contention
void stress_test_example()
{
    write_information("\n[Example 5] Stress Test - High Contention");
    
    typed_job_queue_t<job_types> queue;
    const int num_threads = 16;
    const int ops_per_thread = 10000;
    std::atomic<int> total_ops{0};
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    
    // Half producers, half consumers
    for (int t = 0; t < num_threads; ++t) {
        if (t < num_threads / 2) {
            // Producer
            threads.emplace_back([&queue, &total_ops, t, ops_per_thread]() {
                for (int i = 0; i < ops_per_thread; ++i) {
                    job_types type = static_cast<job_types>((t + i) % 3);
                    auto job = std::make_unique<callback_typed_job>(
                        [&total_ops]() -> result_void {
                            total_ops.fetch_add(1);
                            return result_void();
                        },
                        type
                    );
                    
                    auto enqueue_result = queue.enqueue(std::move(job));
                    while (enqueue_result.has_error()) {
                        std::this_thread::yield();
                        // Re-create job since it was moved
                        job = std::make_unique<callback_typed_job>(
                            [&total_ops]() -> result_void {
                                total_ops.fetch_add(1);
                                return result_void();
                            },
                            type
                        );
                        enqueue_result = queue.enqueue(std::move(job));
                    }
                }
            });
        } else {
            // Consumer
            threads.emplace_back([&queue, ops_per_thread]() {
                int consumed = 0;
                while (consumed < ops_per_thread) {
                    auto job = queue.dequeue({job_types::RealTime, job_types::Batch, job_types::Background});
                    if (job.has_value()) {
                        auto work_result = job.value()->do_work();
                        (void)work_result; // Ignore result for sample
                        consumed++;
                    } else {
                        std::this_thread::yield();
                    }
                }
            });
        }
    }
    
    for (auto& t : threads) t.join();
    
    auto duration = std::chrono::high_resolution_clock::now() - start;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    
    write_information(
        "Stress test completed: {} operations in {} ms = {} ops/sec",
            total_ops.load(), ms, total_ops.load() * 1000.0 / ms);
}

int main()
{
    log_module::start();
    log_module::console_target(log_types::Debug);
    
    write_information(
        "Typed Job Queue Sample (Lock-free MPMC)\n"
        "=======================================");
    
    try {
        basic_typed_queue_example();
        // mpmc_typed_queue_example();
        // performance_comparison_example();
        // task_scheduling_example();
        // stress_test_example();
    } catch (const std::exception& e) {
        write_error(
            "Exception: {}", e.what());
    }
    
    write_information("\nAll examples completed!");
    
    log_module::stop();
    return 0;
}