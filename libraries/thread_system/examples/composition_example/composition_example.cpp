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

#include <iostream>
#include <chrono>
#include <thread>

#include <kcenon/thread/interfaces/service_container.h>
#include <kcenon/thread/interfaces/thread_context.h>
#include <kcenon/thread/interfaces/logger_interface.h>
#include <kcenon/thread/interfaces/monitoring_interface.h>
#include <kcenon/thread/core/thread_pool.h>
#include <kcenon/thread/core/callback_job.h>
// #include "../../src/impl/typed_pool/typed_thread_pool.h"
// #include "../../src/impl/typed_pool/callback_typed_job.h"

using namespace kcenon::thread;
using namespace kcenon::thread;
// using namespace kcenon::thread;

/**
 * @brief Simple console logger implementation
 */
class console_logger : public logger_interface {
public:
    void log(log_level level, const std::string& message) override {
        std::cout << "[" << level_to_string(level) << "] " << message << std::endl;
    }
    
    void log(log_level level, const std::string& message,
             const std::string& file, int line, const std::string& function) override {
        std::cout << "[" << level_to_string(level) << "] " 
                  << file << ":" << line << " (" << function << ") - " 
                  << message << std::endl;
    }
    
    bool is_enabled(log_level level) const override {
        return true; // Enable all levels for demo
    }
    
    void flush() override {
        std::cout.flush();
    }
    
private:
    std::string level_to_string(log_level level) const {
        switch (level) {
            case log_level::critical: return "CRITICAL";
            case log_level::error: return "ERROR";
            case log_level::warning: return "WARNING";
            case log_level::info: return "INFO";
            case log_level::debug: return "DEBUG";
            case log_level::trace: return "TRACE";
        }
        return "UNKNOWN";
    }
};

/**
 * @brief Simple monitoring implementation
 */
class console_monitoring : public ::monitoring_interface::monitoring_interface {
public:
    void update_system_metrics(const ::monitoring_interface::system_metrics& metrics) override {
        std::cout << "[MONITORING] System - CPU: " << metrics.cpu_usage_percent << "%, "
                  << "Memory: " << metrics.memory_usage_bytes << " bytes, "
                  << "Threads: " << metrics.active_threads << std::endl;
    }
    
    void update_thread_pool_metrics(const ::monitoring_interface::thread_pool_metrics& metrics) override {
        std::cout << "[MONITORING] Pool - Completed: " << metrics.jobs_completed << ", "
                  << "Pending: " << metrics.jobs_pending << ", "
                  << "Workers: " << metrics.worker_threads << " (" 
                  << metrics.idle_threads << " idle)" << std::endl;
    }
    
    void update_worker_metrics(std::size_t worker_id, const ::monitoring_interface::worker_metrics& metrics) override {
        std::cout << "[MONITORING] Worker " << worker_id << " - "
                  << "Processed: " << metrics.jobs_processed << ", "
                  << "Time: " << metrics.total_processing_time_ns << " ns" << std::endl;
    }
    
    ::monitoring_interface::metrics_snapshot get_current_snapshot() const override {
        return current_snapshot_;
    }
    
    std::vector<::monitoring_interface::metrics_snapshot> get_recent_snapshots(std::size_t count) const override {
        return {}; // Not implemented for demo
    }
    
    bool is_active() const override { return true; }
    
private:
    ::monitoring_interface::metrics_snapshot current_snapshot_;
};

/**
 * @brief Demonstrate composition-based design
 */
void demonstrate_composition() {
    std::cout << "\n=== Composition-Based Thread System Demo ===\n" << std::endl;
    
    // 1. Setup service container with implementations
    auto& container = service_container::global();
    
    // Register logger service
    container.register_singleton<logger_interface>(
        std::make_shared<console_logger>());
    
    // Register monitoring service
    container.register_singleton<::monitoring_interface::monitoring_interface>(
        std::make_shared<console_monitoring>());
    
    // 2. Create thread pool with context from global container
    thread_context context; // Will resolve services from container
    auto pool = std::make_shared<thread_pool>("CompositionPool", context);
    
    // 3. Add workers - they inherit context from pool
    std::vector<std::unique_ptr<thread_worker>> workers;
    for (int i = 0; i < 4; ++i) {
        workers.push_back(std::make_unique<thread_worker>());
    }
    {
        auto r = pool->enqueue_batch(std::move(workers));
        if (r.has_error()) {
            std::cerr << "enqueue_batch failed: " << r.get_error().to_string() << std::endl;
            return;
        }
    }
    
    // 4. Start pool - will log through context
    {
        auto r = pool->start();
        if (r.has_error()) {
            std::cerr << "start failed: " << r.get_error().to_string() << std::endl;
            return;
        }
    }
    
    // 5. Submit jobs that will be logged
    for (int i = 0; i < 10; ++i) {
        auto r = pool->enqueue(std::make_unique<callback_job>(
            [i, &context]() -> result_void {
                context.log(log_level::info, 
                    "Processing job " + std::to_string(i));
                
                // Simulate work
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                
                return result_void();
            }
        ));
        if (r.has_error()) {
            std::cerr << "enqueue failed: " << r.get_error().to_string() << std::endl;
        }
    }
    
    // 6. Wait for completion
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 7. Stop pool
    {
        auto r = pool->stop();
        if (r.has_error()) {
            std::cerr << "stop failed: " << r.get_error().to_string() << std::endl;
        }
    }
    
    std::cout << "\n=== Basic Thread Pool Demo Complete ===\n" << std::endl;
}

/*
 * @brief Demonstrate typed thread pool with composition
 * Temporarily disabled - requires typed_pool implementation
 */
/*
void demonstrate_typed_pool_composition() {
    std::cout << "\n=== Typed Thread Pool with Composition Demo ===\n" << std::endl;

    // Use builder pattern for context
    auto context = thread_context_builder()
        .from_global_container()
        .build();

    // Create typed thread pool with priority support
    auto pool = std::make_shared<typed_thread_pool_t<job_types>>("TypedPool", context);

    // Add specialized workers
    for (auto priority : {job_types::RealTime, job_types::Batch, job_types::Background}) {
        auto worker = std::make_unique<typed_thread_worker_t<job_types>>();
        // For typed workers, set the type they handle
        // Note: The typed worker template includes the type in the template parameter
        auto r = pool->enqueue(std::move(worker));
        if (r.has_error()) {
            std::cerr << "enqueue worker failed: " << r.get_error().to_string() << std::endl;
        }
    }

    {
        auto r = pool->start();
        if (r.has_error()) {
            std::cerr << "typed pool start failed: " << r.get_error().to_string() << std::endl;
            return;
        }
    }

    // Submit jobs with different priorities
    for (int i = 0; i < 5; ++i) {
        // Real-time job
        auto r1 = pool->enqueue(std::make_unique<callback_typed_job_t<job_types>>(
            [i, &context]() -> result_void {
                context.log(log_level::info,
                    "RealTime job " + std::to_string(i) + " executing");
                return result_void();
            },
            job_types::RealTime
        ));
        if (r1.has_error()) {
            std::cerr << "enqueue realtime job failed: " << r1.get_error().to_string() << std::endl;
        }

        // Background job
        auto r2 = pool->enqueue(std::make_unique<callback_typed_job_t<job_types>>(
            [i, &context]() -> result_void {
                context.log(log_level::debug,
                    "Background job " + std::to_string(i) + " executing");
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                return result_void();
            },
            job_types::Background
        ));
        if (r2.has_error()) {
            std::cerr << "enqueue background job failed: " << r2.get_error().to_string() << std::endl;
        }
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    {
        auto r = pool->stop();
        if (r.has_error()) {
            std::cerr << "typed pool stop failed: " << r.get_error().to_string() << std::endl;
        }
    }

    std::cout << "\n=== Typed Thread Pool Demo Complete ===\n" << std::endl;
}
*/

/**
 * @brief Demonstrate using thread pool without any services
 */
void demonstrate_minimal_usage() {
    std::cout << "\n=== Minimal Thread Pool (No Services) Demo ===\n" << std::endl;
    
    // Clear any existing services
    service_container::global().clear();
    
    // Create pool without context - no logging or monitoring
    auto pool = std::make_shared<thread_pool>("MinimalPool");
    
    // Add workers
    std::vector<std::unique_ptr<thread_worker>> workers;
    for (int i = 0; i < 2; ++i) {
        workers.push_back(std::make_unique<thread_worker>());
    }
    {
        auto r = pool->enqueue_batch(std::move(workers));
        if (r.has_error()) {
            std::cerr << "enqueue_batch failed: " << r.get_error().to_string() << std::endl;
            return;
        }
    }
    
    {
        auto r = pool->start();
        if (r.has_error()) {
            std::cerr << "start failed: " << r.get_error().to_string() << std::endl;
            return;
        }
    }
    
    // Submit jobs - no logging will occur
    std::atomic<int> counter{0};
    for (int i = 0; i < 5; ++i) {
        auto r = pool->enqueue(std::make_unique<callback_job>(
            [&counter]() -> result_void {
                counter.fetch_add(1);
                return result_void();
            }
        ));
        if (r.has_error()) {
            std::cerr << "enqueue failed: " << r.get_error().to_string() << std::endl;
        }
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    {
        auto r = pool->stop();
        if (r.has_error()) {
            std::cerr << "stop failed: " << r.get_error().to_string() << std::endl;
        }
    }
    
    std::cout << "Completed " << counter.load() << " jobs without any logging/monitoring" << std::endl;
    std::cout << "\n=== Minimal Demo Complete ===\n" << std::endl;
}

int main() {
    try {
        // Show different usage patterns
        demonstrate_minimal_usage();
        demonstrate_composition();
        // demonstrate_typed_pool_composition(); // Temporarily disabled - requires typed_pool implementation
        
        // Clean up
        service_container::global().clear();
        
        std::cout << "\nAll demos completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
