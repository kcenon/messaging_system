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

/**
 * @file integration_example.cpp
 * @brief Demonstrates integration of thread_system with external logger and monitoring
 * 
 * This example shows how to:
 * 1. Use thread_system with external logger implementation
 * 2. Use thread_system with external monitoring implementation
 * 3. Combine both for a complete system
 * 
 * Note: This example assumes logger_system and monitoring_system are installed
 * or available in the build path.
 */

#include <iostream>
#include <chrono>
#include <thread>
#include <vector>

// Thread system headers
#include "../../sources/interfaces/service_container.h"
#include "../../sources/interfaces/thread_context.h"
#include "../../sources/thread_pool/core/thread_pool.h"
#include "../../sources/thread_base/jobs/callback_job.h"

// External logger headers (would be from installed package)
// #include <logger_system/logger.h>
// #include <logger_system/writers/console_writer.h>

// External monitoring headers (would be from installed package)
// #include <monitoring_system/monitoring.h>

// For this example, we'll create mock implementations
#include "mock_logger.h"
#include "mock_monitoring.h"

using namespace thread_module;
using namespace thread_pool_module;

/**
 * @brief Example 1: Thread pool with external logger only
 */
void thread_pool_with_logger_example() {
    std::cout << "\n=== Thread Pool with External Logger ===\n" << std::endl;
    
    // 1. Create and configure external logger
    auto logger = std::make_shared<mock_logger>();
    logger->start();
    
    // 2. Register logger in service container
    service_container::global().register_singleton<logger_interface>(logger);
    
    // 3. Create thread pool - it will automatically use the logger
    thread_context context; // Resolves from global container
    auto pool = std::make_shared<thread_pool>("LoggedPool", context);
    
    // 4. Add workers
    std::vector<std::unique_ptr<thread_worker>> workers;
    for (int i = 0; i < 4; ++i) {
        workers.push_back(std::make_unique<thread_worker>());
    }
    pool->enqueue_batch(std::move(workers));
    
    // 5. Start pool
    pool->start();
    
    // 6. Submit jobs
    for (int i = 0; i < 10; ++i) {
        pool->enqueue(std::make_unique<callback_job>(
            [i, &context]() -> result_void {
                // Job logs through context
                context.log(log_level::info, 
                    "Executing job " + std::to_string(i));
                
                // Simulate work
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                
                return result_void();
            }
        ));
    }
    
    // 7. Wait and stop
    std::this_thread::sleep_for(std::chrono::seconds(1));
    pool->stop();
    logger->stop();
    
    // 8. Clear service container
    service_container::global().clear();
}

/**
 * @brief Example 2: Thread pool with external monitoring only
 */
void thread_pool_with_monitoring_example() {
    std::cout << "\n=== Thread Pool with External Monitoring ===\n" << std::endl;
    
    // 1. Create and configure external monitoring
    auto monitor = std::make_shared<mock_monitoring>();
    monitor->start();
    
    // 2. Register monitoring in service container
    service_container::global().register_singleton<monitoring_interface::monitoring_interface>(monitor);
    
    // 3. Create thread pool with monitoring
    thread_context context;
    auto pool = std::make_shared<thread_pool>("MonitoredPool", context);
    
    // 4. Add workers and start
    std::vector<std::unique_ptr<thread_worker>> workers;
    for (int i = 0; i < 4; ++i) {
        workers.push_back(std::make_unique<thread_worker>());
    }
    pool->enqueue_batch(std::move(workers));
    pool->start();
    
    // 5. Submit jobs and monitor
    std::cout << "Submitting jobs and monitoring performance..." << std::endl;
    
    for (int batch = 0; batch < 3; ++batch) {
        // Submit batch of jobs
        for (int i = 0; i < 20; ++i) {
            pool->enqueue(std::make_unique<callback_job>(
                [&context]() -> result_void {
                    // Simulate varying workload
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(10 + rand() % 40));
                    return result_void();
                }
            ));
        }
        
        // Wait and check metrics
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        auto snapshot = monitor->get_current_snapshot();
        std::cout << "Batch " << batch + 1 << " metrics:" << std::endl;
        std::cout << "  Jobs completed: " << snapshot.thread_pool.jobs_completed << std::endl;
        std::cout << "  Jobs pending: " << snapshot.thread_pool.jobs_pending << std::endl;
        std::cout << "  Active workers: " << snapshot.thread_pool.worker_threads 
                  << " (" << snapshot.thread_pool.idle_threads << " idle)" << std::endl;
    }
    
    // 6. Stop and get final stats
    pool->stop();
    monitor->stop();
    
    auto stats = monitor->get_stats();
    std::cout << "\nFinal monitoring stats:" << std::endl;
    std::cout << "  Total collections: " << stats.total_collections << std::endl;
    
    service_container::global().clear();
}

/**
 * @brief Example 3: Complete integration with both logger and monitoring
 */
void complete_integration_example() {
    std::cout << "\n=== Complete Integration Example ===\n" << std::endl;
    
    // 1. Setup external services
    auto logger = std::make_shared<mock_logger>();
    auto monitor = std::make_shared<mock_monitoring>();
    
    logger->start();
    monitor->start();
    
    // 2. Register both services
    service_container::global().register_singleton<logger_interface>(logger);
    service_container::global().register_singleton<monitoring_interface::monitoring_interface>(monitor);
    
    // 3. Create fully integrated thread pool
    thread_context context;
    auto pool = std::make_shared<thread_pool>("IntegratedPool", context);
    
    // Log that we're starting
    context.log(log_level::info, "Starting integrated thread pool example");
    
    // 4. Configure pool
    std::vector<std::unique_ptr<thread_worker>> workers;
    for (int i = 0; i < 4; ++i) {
        workers.push_back(std::make_unique<thread_worker>(true)); // Enable timing
    }
    pool->enqueue_batch(std::move(workers));
    pool->start();
    
    // 5. Run workload with full instrumentation
    std::cout << "Running workload with logging and monitoring..." << std::endl;
    
    auto workload_start = std::chrono::steady_clock::now();
    
    for (int i = 0; i < 50; ++i) {
        pool->enqueue(std::make_unique<callback_job>(
            [i, &context]() -> result_void {
                // Log job start
                context.log(log_level::debug, 
                    "Job " + std::to_string(i) + " started");
                
                // Simulate work
                auto work_time = 20 + (i % 30);
                std::this_thread::sleep_for(std::chrono::milliseconds(work_time));
                
                // Simulate occasional warnings
                if (i % 10 == 0) {
                    context.log(log_level::warning, 
                        "Job " + std::to_string(i) + " took longer than expected");
                }
                
                return result_void();
            }
        ));
    }
    
    // 6. Monitor progress
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        
        auto snapshot = monitor->get_current_snapshot();
        context.log(log_level::info, 
            "Progress: " + std::to_string(snapshot.thread_pool.jobs_completed) + 
            " jobs completed, " + std::to_string(snapshot.thread_pool.jobs_pending) + 
            " pending");
    }
    
    // 7. Wait for completion
    pool->stop();
    
    auto workload_end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        workload_end - workload_start);
    
    context.log(log_level::info, 
        "Workload completed in " + std::to_string(duration.count()) + " ms");
    
    // 8. Final metrics
    auto final_snapshot = monitor->get_current_snapshot();
    std::cout << "\nFinal metrics:" << std::endl;
    std::cout << "  Total jobs: " << final_snapshot.thread_pool.jobs_completed << std::endl;
    std::cout << "  Total processing time: " 
              << (final_snapshot.thread_pool.total_execution_time_ns / 1000000.0) 
              << " ms" << std::endl;
    
    // 9. Cleanup
    logger->stop();
    monitor->stop();
    service_container::global().clear();
}

/**
 * @brief Example 4: Dynamic service registration
 */
void dynamic_service_example() {
    std::cout << "\n=== Dynamic Service Registration Example ===\n" << std::endl;
    
    // Create thread pool without any services
    auto pool = std::make_shared<thread_pool>("DynamicPool");
    
    std::vector<std::unique_ptr<thread_worker>> workers;
    for (int i = 0; i < 2; ++i) {
        workers.push_back(std::make_unique<thread_worker>());
    }
    pool->enqueue_batch(std::move(workers));
    pool->start();
    
    // Submit jobs without logging
    std::cout << "Running without services..." << std::endl;
    for (int i = 0; i < 5; ++i) {
        pool->enqueue(std::make_unique<callback_job>(
            []() -> result_void {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                return result_void();
            }
        ));
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    // Now add logger dynamically
    std::cout << "\nAdding logger service dynamically..." << std::endl;
    auto logger = std::make_shared<mock_logger>();
    logger->start();
    service_container::global().register_singleton<logger_interface>(logger);
    
    // Create new context that will pick up the logger
    thread_context new_context;
    
    // Submit more jobs with logging
    for (int i = 5; i < 10; ++i) {
        pool->enqueue(std::make_unique<callback_job>(
            [i, &new_context]() -> result_void {
                new_context.log(log_level::info, 
                    "Job " + std::to_string(i) + " with dynamic logger");
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                return result_void();
            }
        ));
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    
    pool->stop();
    logger->stop();
    service_container::global().clear();
}

int main() {
    try {
        std::cout << "=== Thread System Integration Examples ===" << std::endl;
        std::cout << "Demonstrating integration with external logger and monitoring systems\n";
        
        thread_pool_with_logger_example();
        thread_pool_with_monitoring_example();
        complete_integration_example();
        dynamic_service_example();
        
        std::cout << "\n=== All integration examples completed successfully! ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}