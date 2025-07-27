/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
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

#include "thread_base/lockfree/queues/adaptive_job_queue.h"
#include "thread_base/jobs/callback_job.h"
#include "logger/core/logger.h"

#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <random>

using namespace thread_module;
using namespace log_module;
using namespace std::chrono_literals;

// Example 1: Basic queue strategies comparison
void strategy_comparison_example()
{
    write_information("[Example 1] Queue Strategy Comparison");
    
    const int num_jobs = 10000;
    const int num_producers = 4;
    const int num_consumers = 4;
    
    // Test each strategy
    for (auto strategy : {adaptive_job_queue::queue_strategy::FORCE_LEGACY,
                         adaptive_job_queue::queue_strategy::FORCE_LOCKFREE,
                         adaptive_job_queue::queue_strategy::ADAPTIVE})
    {
        adaptive_job_queue queue(strategy);
        std::atomic<int> produced{0};
        std::atomic<int> consumed{0};
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::thread> producers;
        std::vector<std::thread> consumers;
        
        // Start producers
        for (int p = 0; p < num_producers; ++p) {
            producers.emplace_back([&queue, &produced, p, num_jobs]() {
                for (int i = 0; i < num_jobs / num_producers; ++i) {
                    auto job = std::make_unique<callback_job>(
                        [p, i]() -> result_void {
                            return result_void();
                        });
                    
                    while (!queue.enqueue(std::move(job))) {
                        std::this_thread::yield();
                    }
                    produced.fetch_add(1);
                }
            });
        }
        
        // Start consumers
        for (int c = 0; c < num_consumers; ++c) {
            consumers.emplace_back([&queue, &consumed, num_jobs]() {
                while (consumed.load() < num_jobs) {
                    auto result = queue.dequeue();
                    if (result.has_value()) {
                        auto& job = result.value();
                        auto work_result = job->do_work();
                        (void)work_result; // Ignore result for sample
                        consumed.fetch_add(1);
                    } else {
                        std::this_thread::yield();
                    }
                }
            });
        }
        
        // Wait for completion
        for (auto& t : producers) t.join();
        for (auto& t : consumers) t.join();
        
        auto duration = std::chrono::high_resolution_clock::now() - start;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        
        std::string strategy_name;
        switch (strategy) {
            case adaptive_job_queue::queue_strategy::FORCE_LEGACY:
                strategy_name = "Mutex-based";
                break;
            case adaptive_job_queue::queue_strategy::FORCE_LOCKFREE:
                strategy_name = "Lock-free";
                break;
            case adaptive_job_queue::queue_strategy::ADAPTIVE:
                strategy_name = "Adaptive";
                break;
            case adaptive_job_queue::queue_strategy::AUTO_DETECT:
                strategy_name = "Auto-detect";
                break;
        }
        
        write_information("{} strategy: {} jobs in {} ms = {} ops/sec",
                strategy_name, num_jobs, ms, 
                num_jobs * 1000.0 / ms);
    }
}

// Example 2: Adaptive strategy behavior under varying contention
void adaptive_behavior_example()
{
    write_information("\n[Example 2] Adaptive Strategy Behavior");
    
    adaptive_job_queue queue(adaptive_job_queue::queue_strategy::ADAPTIVE);
    
    // Low contention phase (1 producer, 1 consumer)
    write_information("Phase 1: Low contention (1P-1C)");
    {
        std::atomic<bool> running{true};
        std::atomic<int> jobs_processed{0};
        
        std::thread producer([&queue, &running]() {
            while (running) {
                auto job = std::make_unique<callback_job>(
                    []() -> result_void { return result_void(); });
                auto enqueue_result = queue.enqueue(std::move(job));
                (void)enqueue_result; // Ignore result for sample
                std::this_thread::sleep_for(1ms);
            }
        });
        
        std::thread consumer([&queue, &running, &jobs_processed]() {
            while (running) {
                auto result = queue.dequeue();
                if (result.has_value()) {
                    auto work_result = result.value()->do_work();
                    (void)work_result; // Ignore result for sample
                    jobs_processed.fetch_add(1);
                }
                std::this_thread::sleep_for(1ms);
            }
        });
        
        std::this_thread::sleep_for(2s);
        running = false;
        producer.join();
        consumer.join();
        
        auto current_type = queue.get_current_type();
        write_information(
            "  Current type: {}, Jobs processed: {}",
                current_type,
                jobs_processed.load());
    }
    
    // High contention phase (8 producers, 8 consumers)
    write_information("Phase 2: High contention (8P-8C)");
    {
        std::atomic<bool> running{true};
        std::atomic<int> jobs_processed{0};
        std::vector<std::thread> threads;
        
        // Start producers
        for (int i = 0; i < 8; ++i) {
            threads.emplace_back([&queue, &running]() {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dist(0, 100);
                
                while (running) {
                    auto job = std::make_unique<callback_job>(
                        []() -> result_void { return result_void(); });
                    auto enqueue_result = queue.enqueue(std::move(job));
                    (void)enqueue_result; // Ignore result for sample
                    if (dist(gen) < 10) {  // 10% chance of sleep
                        std::this_thread::sleep_for(std::chrono::microseconds(dist(gen)));
                    }
                }
            });
        }
        
        // Start consumers
        for (int i = 0; i < 8; ++i) {
            threads.emplace_back([&queue, &running, &jobs_processed]() {
                while (running) {
                    auto result = queue.dequeue();
                    if (result.has_value()) {
                        auto work_result = result.value()->do_work();
                        (void)work_result; // Ignore result for sample
                        jobs_processed.fetch_add(1);
                    }
                }
            });
        }
        
        std::this_thread::sleep_for(2s);
        running = false;
        for (auto& t : threads) t.join();
        
        auto current_type = queue.get_current_type();
        write_information(
            "  Current type: {}, Jobs processed: {}",
                current_type,
                jobs_processed.load());
    }
}

// Example 3: Different queue strategies
void different_strategies_example()
{
    write_information("\n[Example 3] Different Queue Strategies");
    
    // Create queue with forced mutex-based strategy
    adaptive_job_queue mutex_queue(adaptive_job_queue::queue_strategy::FORCE_LEGACY);
    write_information(
        "Mutex-based queue type: {}",
            mutex_queue.get_current_type());
    
    // Perform some operations
    std::vector<std::unique_ptr<job>> jobs;
    for (int i = 0; i < 100; ++i) {
        jobs.push_back(std::make_unique<callback_job>(
            [i]() -> result_void {
                write_information("Job {} executed", i);
                return result_void(); // Success
            }));
    }
    
    auto enqueue_result = mutex_queue.enqueue_batch(std::move(jobs));
    if (enqueue_result) {
        write_information("Batch enqueue successful");
    }
    
    // Create queue with forced lock-free strategy
    adaptive_job_queue lockfree_queue(adaptive_job_queue::queue_strategy::FORCE_LOCKFREE);
    write_information(
        "Lock-free queue type: {}",
            lockfree_queue.get_current_type());
    
    // Dequeue jobs from mutex queue
    auto dequeued = mutex_queue.dequeue_batch();
    write_information(
        "Dequeued {} jobs from mutex queue", dequeued.size());
    
    // Process dequeued jobs
    for (auto& job : dequeued) {
        auto result = job->do_work();
        if (!result) {
            // Job executed successfully
        } else {
            write_error("Job failed: {}", result.get_error().message());
        }
    }
}

// Example 4: Performance monitoring
void performance_monitoring_example()
{
    write_information("\n[Example 4] Performance Monitoring");
    
    adaptive_job_queue queue(adaptive_job_queue::queue_strategy::ADAPTIVE);
    
    const int num_operations = 50000;
    std::atomic<bool> running{true};
    std::atomic<int> enqueued{0};
    std::atomic<int> dequeued{0};
    
    // Producer thread
    std::thread producer([&queue, &running, &enqueued, num_operations]() {
        for (int i = 0; i < num_operations; ++i) {
            auto job = std::make_unique<callback_job>(
                []() -> result_void { return result_void(); });
            
            while (!queue.enqueue(std::move(job))) {
                std::this_thread::yield();
            }
            enqueued.fetch_add(1);
        }
    });
    
    // Consumer thread
    std::thread consumer([&queue, &running, &dequeued, num_operations]() {
        while (dequeued.load() < num_operations) {
            auto result = queue.dequeue();
            if (result.has_value()) {
                auto work_result = result.value()->do_work();
                (void)work_result; // Ignore result for sample
                dequeued.fetch_add(1);
            }
        }
    });
    
    // Monitor thread
    std::thread monitor([&queue, &running, &enqueued, &dequeued, num_operations]() {
        auto start = std::chrono::steady_clock::now();
        
        while (dequeued.load() < num_operations) {
            std::this_thread::sleep_for(500ms);
            
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration<double>(now - start).count();
            
            auto current_type = queue.get_current_type();
            
            write_information(
                "Status: {} type, Enqueued: {}, Dequeued: {}, Rate: {:.0f} ops/sec",
                    current_type, enqueued.load(), dequeued.load(),
                    dequeued.load() / elapsed);
        }
    });
    
    producer.join();
    consumer.join();
    running = false;
    monitor.join();
    
    write_information(
        "Completed {} operations", num_operations);
}

// Example 5: Real-world scenario - Web server simulation
void web_server_simulation()
{
    write_information("\n[Example 5] Web Server Simulation");
    
    adaptive_job_queue request_queue(adaptive_job_queue::queue_strategy::ADAPTIVE);
    std::atomic<bool> server_running{true};
    std::atomic<int> requests_handled{0};
    std::atomic<int> requests_failed{0};
    
    // Request types
    enum class request_type { GET, POST, PUT, DELETE };
    
    // Simulate incoming requests
    std::vector<std::thread> clients;
    for (int client_id = 0; client_id < 5; ++client_id) {
        clients.emplace_back([&request_queue, &server_running, &requests_failed, client_id]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> type_dist(0, 3);
            std::uniform_int_distribution<> delay_dist(10, 100);
            
            while (server_running) {
                auto type = static_cast<request_type>(type_dist(gen));
                
                auto request = std::make_unique<callback_job>(
                    [client_id, type]() -> result_void {
                        // Simulate request processing
                        std::this_thread::sleep_for(std::chrono::microseconds(
                            type == request_type::GET ? 10 : 50));
                        
                        std::string type_str;
                        switch (type) {
                            case request_type::GET: type_str = "GET"; break;
                            case request_type::POST: type_str = "POST"; break;
                            case request_type::PUT: type_str = "PUT"; break;
                            case request_type::DELETE: type_str = "DELETE"; break;
                        }
                        
                        write_information("Client {} {} request completed", 
                            client_id, type_str);
                        return result_void(); // Success
                    });
                
                if (!request_queue.enqueue(std::move(request))) {
                    requests_failed.fetch_add(1);
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(delay_dist(gen)));
            }
        });
    }
    
    // Worker threads (server handlers)
    std::vector<std::thread> workers;
    for (int worker_id = 0; worker_id < 3; ++worker_id) {
        workers.emplace_back([&request_queue, &server_running, &requests_handled, worker_id]() {
            while (server_running) {
                auto request = request_queue.dequeue();
                if (request.has_value()) {
                    auto result = request.value()->do_work();
                    if (!result) {
                        // Request processed successfully
                        requests_handled.fetch_add(1);
                    } else {
                        write_error("Worker {} request failed: {}", 
                            worker_id, result.get_error().message());
                    }
                } else {
                    std::this_thread::sleep_for(1ms);
                }
            }
        });
    }
    
    // Run simulation for 5 seconds
    std::this_thread::sleep_for(5s);
    server_running = false;
    
    // Cleanup
    for (auto& t : clients) t.join();
    for (auto& t : workers) t.join();
    
    write_information(
        "Server simulation complete: {} requests handled, {} failed",
            requests_handled.load(), requests_failed.load());
}

int main()
{
    log_module::start();
    log_module::console_target(log_types::Debug);
    
    write_information(
        "Adaptive Job Queue Sample\n"
        "=========================");
    
    try {
        strategy_comparison_example();
        adaptive_behavior_example();
        different_strategies_example();
        performance_monitoring_example();
        web_server_simulation();
    } catch (const std::exception& e) {
        write_error(
            "Exception: {}", e.what());
    }
    
    write_information("\nAll examples completed!");
    
    log_module::stop();
    return 0;
}