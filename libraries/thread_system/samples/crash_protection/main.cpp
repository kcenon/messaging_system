/**
 * @file main.cpp
 * @brief Crash protection demonstration for thread system
 * 
 * This example demonstrates comprehensive crash protection mechanisms
 * including signal handling, graceful shutdown, resource cleanup, and
 * automatic recovery capabilities.
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <vector>
#include <memory>
#include <atomic>

// Include crash protection headers
#include "interfaces/crash_handler.h"
#include "thread_pool/thread_pool.h"

using namespace thread_module;

// Global state for demonstration
std::atomic<bool> system_running{true};
std::atomic<int> tasks_completed{0};
std::atomic<int> tasks_failed{0};

// Simulated resource that needs cleanup
class critical_resource {
public:
    critical_resource(const std::string& name) : name_(name), allocated_(true) {
        std::cout << "[ALLOC] Allocated critical resource: " << name_ << std::endl;
    }
    
    ~critical_resource() {
        cleanup();
    }
    
    void cleanup() {
        if (allocated_) {
            std::cout << "[CLEANUP] Cleaning up critical resource: " << name_ << std::endl;
            allocated_ = false;
        }
    }
    
    const std::string& name() const { return name_; }
    bool is_allocated() const { return allocated_; }

private:
    std::string name_;
    bool allocated_;
};

// Global critical resources
std::vector<std::shared_ptr<critical_resource>> global_resources;

// Crash simulation functions
void simulate_segmentation_fault() {
    std::cout << "[CRASH] Simulating segmentation fault..." << std::endl;
    int* null_ptr = nullptr;
    *null_ptr = 42; // This will cause SIGSEGV
}

void simulate_division_by_zero() {
    std::cout << "[CRASH] Simulating division by zero..." << std::endl;
    volatile int zero = 0;
    volatile int result = 100 / zero; // This will cause SIGFPE
    (void)result;
}

void simulate_abort() {
    std::cout << "[CRASH] Simulating abort..." << std::endl;
    std::abort(); // This will cause SIGABRT
}

// Task functions for thread pool
void normal_task(int task_id) {
    std::cout << "[TASK] Task " << task_id << " starting normally" << std::endl;
    
    // Simulate some work
    std::this_thread::sleep_for(std::chrono::milliseconds(100 + (task_id % 200)));
    
    tasks_completed.fetch_add(1);
    std::cout << "[TASK] Task " << task_id << " completed successfully" << std::endl;
}

void potentially_crashing_task(int task_id) {
    std::cout << "[WARN] Task " << task_id << " starting (potentially dangerous)" << std::endl;
    
    // Random chance of crash
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> crash_dist(1, 10);
    
    int outcome = crash_dist(gen);
    
    if (outcome <= 7) {
        // Normal execution
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        tasks_completed.fetch_add(1);
        std::cout << "[TASK] Task " << task_id << " completed safely" << std::endl;
    } else if (outcome == 8) {
        // Segmentation fault
        tasks_failed.fetch_add(1);
        simulate_segmentation_fault();
    } else if (outcome == 9) {
        // Division by zero
        tasks_failed.fetch_add(1);
        simulate_division_by_zero();
    } else {
        // Abort
        tasks_failed.fetch_add(1);
        simulate_abort();
    }
}

// Crash callback functions
void on_system_crash(const crash_context& context) {
    std::cout << "\n[ALERT] CRASH DETECTED!" << std::endl;
    std::cout << "Signal: " << context.signal_name << " (" << context.signal_number << ")" << std::endl;
    std::cout << "Thread: " << context.crashing_thread << std::endl;
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::seconds>(
        context.crash_time.time_since_epoch()).count() << std::endl;
    
    if (!context.stack_trace.empty()) {
        std::cout << "Stack trace available" << std::endl;
    }
    
    // Mark system as no longer running
    system_running.store(false);
}

void cleanup_global_resources() {
    std::cout << "[CLEANUP] Cleaning up global resources..." << std::endl;
    for (auto& resource : global_resources) {
        if (resource && resource->is_allocated()) {
            resource->cleanup();
        }
    }
    global_resources.clear();
}

void emergency_state_save() {
    std::cout << "[SAVE] Saving emergency state..." << std::endl;
    std::cout << "Tasks completed: " << tasks_completed.load() << std::endl;
    std::cout << "Tasks failed: " << tasks_failed.load() << std::endl;
    std::cout << "Resources allocated: " << global_resources.size() << std::endl;
}

int main() {
    std::cout << "=== Thread System Crash Protection Demo ===" << std::endl;
    std::cout << "This demo shows comprehensive crash protection mechanisms\n" << std::endl;
    
    // Step 1: Initialize crash protection
    std::cout << "--- Step 1: Initialize Crash Protection ---" << std::endl;
    
    auto& crash_handler = crash_handler::instance();
    crash_handler.initialize(crash_safety_level::standard, true);
    crash_handler.set_crash_log_directory("./crash_logs");
    
    // Register crash callbacks
    auto crash_callback_id = crash_handler.register_crash_callback(
        "SystemCrashHandler", on_system_crash, 10);
    
    // Register cleanup functions
    crash_handler.register_cleanup("GlobalResources", cleanup_global_resources, 2000);
    crash_handler.register_cleanup("EmergencyState", emergency_state_save, 1000);
    
    std::cout << "[OK] Crash protection initialized" << std::endl;
    
    // Step 2: Allocate critical resources
    std::cout << "\n--- Step 2: Allocate Critical Resources ---" << std::endl;
    
    global_resources.push_back(std::make_shared<critical_resource>("DatabaseConnection"));
    global_resources.push_back(std::make_shared<critical_resource>("NetworkSocket"));
    global_resources.push_back(std::make_shared<critical_resource>("FileHandle"));
    global_resources.push_back(std::make_shared<critical_resource>("SharedMemory"));
    
    // Step 3: Create and configure thread pool with crash protection
    std::cout << "\n--- Step 3: Create Thread Pool ---" << std::endl;
    
    auto thread_pool = std::make_shared<thread_pool>(4);
    
    // Enable crash protection for thread pool
    thread_pool_crash_safety::enable_for_pool("MainPool", *thread_pool);
    thread_pool_crash_safety::set_job_crash_handler(
        [](const std::string& pool_name, const crash_context& context) {
            std::cout << "[CRASH] Job crashed in pool: " << pool_name << std::endl;
            std::cout << "Signal: " << context.signal_name << std::endl;
        });
    
    thread_pool->start();
    std::cout << "[OK] Thread pool started with crash protection" << std::endl;
    
    // Step 4: Submit normal tasks
    std::cout << "\n--- Step 4: Submit Normal Tasks ---" << std::endl;
    
    std::vector<std::future<void>> normal_futures;
    for (int i = 0; i < 10; ++i) {
        normal_futures.push_back(
            thread_pool->enqueue([i] { normal_task(i); })
        );
    }
    
    // Wait for normal tasks
    for (auto& future : normal_futures) {
        future.wait();
    }
    
    std::cout << "[OK] All normal tasks completed" << std::endl;
    
    // Step 5: Submit potentially crashing tasks
    std::cout << "\n--- Step 5: Submit Potentially Crashing Tasks ---" << std::endl;
    std::cout << "[WARN] Some of these tasks may crash - crash protection will handle them" << std::endl;
    
    std::vector<std::future<void>> risky_futures;
    for (int i = 10; i < 25; ++i) {
        risky_futures.push_back(
            thread_pool->enqueue([i] { 
                try {
                    potentially_crashing_task(i);
                } catch (...) {
                    std::cout << "[PROTECT] Exception caught and handled for task " << i << std::endl;
                }
            })
        );
    }
    
    // Wait for risky tasks (some may cause crashes)
    for (auto& future : risky_futures) {
        try {
            future.wait();
        } catch (...) {
            // Some tasks may have caused crashes
        }
    }
    
    // Step 6: Test manual crash scenarios
    std::cout << "\n--- Step 6: Manual Crash Tests ---" << std::endl;
    
    // Test 1: Manual crash trigger (for testing)
    std::cout << "\nTest 1: Manual crash trigger" << std::endl;
    crash_context test_context;
    test_context.signal_number = SIGUSR1;
    test_context.signal_name = "SIGUSR1";
    test_context.fault_address = nullptr;
    test_context.stack_trace = "Manual test stack trace";
    test_context.crash_time = std::chrono::system_clock::now();
    test_context.crashing_thread = std::this_thread::get_id();
    
    crash_handler.trigger_crash_handling(test_context);
    
    // Test 2: Scoped crash protection
    std::cout << "\nTest 2: Scoped crash protection" << std::endl;
    {
        scoped_crash_callback scoped_protection("ScopedTest",
            [](const crash_context& ctx) {
                std::cout << "[PROTECT] Scoped crash handler activated" << std::endl;
            }, 50);
        
        std::cout << "[OK] Scoped protection active" << std::endl;
        // Scoped protection will be automatically removed when leaving this block
    }
    std::cout << "[OK] Scoped protection removed" << std::endl;
    
    // Step 7: Display final statistics
    std::cout << "\n--- Step 7: Final Statistics ---" << std::endl;
    
    auto stats = crash_handler.get_stats();
    std::cout << "Crash Statistics:" << std::endl;
    std::cout << "  Total crashes handled: " << stats.total_crashes_handled << std::endl;
    std::cout << "  Successful cleanups: " << stats.successful_cleanups << std::endl;
    std::cout << "  Failed cleanups: " << stats.failed_cleanups << std::endl;
    
    std::cout << "\nTask Statistics:" << std::endl;
    std::cout << "  Tasks completed: " << tasks_completed.load() << std::endl;
    std::cout << "  Tasks failed: " << tasks_failed.load() << std::endl;
    
    std::cout << "\nResource Status:" << std::endl;
    for (const auto& resource : global_resources) {
        std::cout << "  " << resource->name() << ": " 
                  << (resource->is_allocated() ? "allocated" : "cleaned up") << std::endl;
    }
    
    // Step 8: Graceful shutdown
    std::cout << "\n--- Step 8: Graceful Shutdown ---" << std::endl;
    
    std::cout << "Stopping thread pool..." << std::endl;
    thread_pool->stop();
    
    std::cout << "Cleaning up resources..." << std::endl;
    cleanup_global_resources();
    
    std::cout << "Unregistering crash callbacks..." << std::endl;
    crash_handler.unregister_crash_callback(crash_callback_id);
    
    std::cout << "\n=== Demo Completed Successfully ===" << std::endl;
    std::cout << "Key features demonstrated:" << std::endl;
    std::cout << "[OK] Signal handling and crash detection" << std::endl;
    std::cout << "[OK] Stack trace generation" << std::endl;
    std::cout << "[OK] Resource cleanup on crash" << std::endl;
    std::cout << "[OK] Thread pool crash protection" << std::endl;
    std::cout << "[OK] Scoped crash protection" << std::endl;
    std::cout << "[OK] Graceful shutdown coordination" << std::endl;
    std::cout << "[OK] Crash statistics and monitoring" << std::endl;
    
    return 0;
}