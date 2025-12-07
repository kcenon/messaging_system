// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file main.cpp
 * @brief Monitoring dashboard example with console-based visualization
 *
 * This example demonstrates:
 * - Using task_monitor for system status
 * - Displaying queue and worker statistics
 * - Tracking task events and failures
 */

#include <kcenon/messaging/task/task_system.h>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

using namespace kcenon::messaging::task;
using namespace kcenon::common;

// Clear screen (works on most terminals)
void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    std::cout << "\033[2J\033[H";
#endif
}

// Get current time string
std::string current_time() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
    return ss.str();
}

// Display a horizontal bar
void print_bar(int width = 60) {
    std::cout << std::string(width, '-') << std::endl;
}

// Display dashboard
void display_dashboard(task_system& system) {
    clear_screen();

    std::cout << "╔══════════════════════════════════════════════════════════╗"
              << std::endl;
    std::cout << "║           Task System Monitoring Dashboard               ║"
              << std::endl;
    std::cout << "║                   " << current_time()
              << "                             ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════════╝"
              << std::endl;
    std::cout << std::endl;

    // Worker statistics
    auto stats = system.get_statistics();
    std::cout << "┌─ Worker Pool Status ──────────────────────────────────────┐"
              << std::endl;
    std::cout << "│ Total Workers:  " << std::setw(5) << system.total_workers()
              << "                                       │" << std::endl;
    std::cout << "│ Active Workers: " << std::setw(5) << system.active_workers()
              << "                                       │" << std::endl;
    std::cout << "│ Idle Workers:   " << std::setw(5)
              << (system.total_workers() - system.active_workers())
              << "                                       │" << std::endl;
    std::cout << "└────────────────────────────────────────────────────────────┘"
              << std::endl;
    std::cout << std::endl;

    // Queue statistics
    std::cout << "┌─ Queue Status ────────────────────────────────────────────┐"
              << std::endl;
    std::cout << "│ Pending Tasks:  " << std::setw(5) << system.pending_count()
              << "                                       │" << std::endl;
    std::cout << "└────────────────────────────────────────────────────────────┘"
              << std::endl;
    std::cout << std::endl;

    // Task statistics
    std::cout << "┌─ Task Statistics ─────────────────────────────────────────┐"
              << std::endl;
    std::cout << "│ Total Submitted:  " << std::setw(8) << stats.total_tasks
              << "                                │" << std::endl;
    std::cout << "│ Succeeded:        " << std::setw(8) << stats.succeeded_tasks
              << "                                │" << std::endl;
    std::cout << "│ Failed:           " << std::setw(8) << stats.failed_tasks
              << "                                │" << std::endl;
    std::cout << "│ Retried:          " << std::setw(8) << stats.retried_tasks
              << "                                │" << std::endl;

    // Calculate success rate
    double success_rate = 0.0;
    if (stats.total_tasks > 0) {
        success_rate = static_cast<double>(stats.succeeded_tasks) /
                       stats.total_tasks * 100.0;
    }
    std::cout << "│ Success Rate:     " << std::fixed << std::setprecision(1)
              << std::setw(7) << success_rate
              << "%                               │" << std::endl;
    std::cout << "└────────────────────────────────────────────────────────────┘"
              << std::endl;
    std::cout << std::endl;

    // Monitor events (if available)
    if (system.monitor()) {
        auto monitor = system.monitor();
        auto events = monitor->recent_events(5);

        std::cout
            << "┌─ Recent Events ────────────────────────────────────────────┐"
            << std::endl;
        if (events.empty()) {
            std::cout
                << "│ No recent events                                           │"
                << std::endl;
        } else {
            for (const auto& event : events) {
                std::stringstream ss;
                ss << "│ " << std::left << std::setw(58)
                   << event.description.substr(0, 56) << "│";
                std::cout << ss.str() << std::endl;
            }
        }
        std::cout
            << "└────────────────────────────────────────────────────────────┘"
            << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Press Ctrl+C to exit" << std::endl;
}

int main() {
    std::cout << "=== Monitoring Dashboard Example ===" << std::endl;
    std::cout << "Starting task system..." << std::endl;

    // Configure with monitoring enabled
    task_system_config config;
    config.worker.concurrency = 4;
    config.enable_monitoring = true;
    config.enable_scheduler = true;
    task_system system(config);

    // Register various handlers
    system.register_handler("quick.task", [](const task& t, task_context& ctx) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        container_module::value_container result;
        result.set_value("status", std::string("done"));
        return ok(result);
    });

    system.register_handler(
        "medium.task", [](const task& t, task_context& ctx) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            container_module::value_container result;
            result.set_value("status", std::string("done"));
            return ok(result);
        });

    system.register_handler("slow.task", [](const task& t, task_context& ctx) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        container_module::value_container result;
        result.set_value("status", std::string("done"));
        return ok(result);
    });

    // Handler that sometimes fails
    static std::atomic<int> fail_counter{0};
    system.register_handler(
        "flaky.task", [](const task& t, task_context& ctx) {
            int count = ++fail_counter;
            if (count % 3 == 0) {
                return Result<container_module::value_container>(
                    error_info{-1, "Random failure"});
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            container_module::value_container result;
            result.set_value("status", std::string("done"));
            return ok(result);
        });

    // Start the system
    auto start_result = system.start();
    if (start_result.is_err()) {
        std::cerr << "Failed to start: " << start_result.error().message
                  << std::endl;
        return 1;
    }

    // Background thread to generate tasks
    std::atomic<bool> running{true};
    std::thread task_generator([&system, &running]() {
        std::vector<std::string> task_types = {"quick.task", "medium.task",
                                               "slow.task", "flaky.task"};
        int index = 0;

        while (running) {
            std::string task_type = task_types[index % task_types.size()];

            auto task_result = task_builder(task_type).build();
            if (task_result.is_ok()) {
                system.submit(task_result.unwrap());
            }

            index++;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    });

    // Main loop - display dashboard
    std::cout << "\nStarting dashboard in 2 seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Run for 15 seconds
    auto start_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::seconds(15);

    while (std::chrono::steady_clock::now() - start_time < duration) {
        display_dashboard(system);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Stop task generation
    running = false;
    task_generator.join();

    // Final display
    clear_screen();
    std::cout << "=== Final Statistics ===" << std::endl;

    auto stats = system.get_statistics();
    std::cout << "Total tasks processed: " << stats.total_tasks << std::endl;
    std::cout << "Succeeded: " << stats.succeeded_tasks << std::endl;
    std::cout << "Failed: " << stats.failed_tasks << std::endl;
    std::cout << "Retried: " << stats.retried_tasks << std::endl;

    if (stats.total_tasks > 0) {
        double success_rate = static_cast<double>(stats.succeeded_tasks) /
                              stats.total_tasks * 100.0;
        std::cout << "Success rate: " << std::fixed << std::setprecision(1)
                  << success_rate << "%" << std::endl;
    }

    std::cout << "\nShutting down..." << std::endl;
    system.shutdown_graceful(std::chrono::seconds(5));
    std::cout << "Done!" << std::endl;

    return 0;
}
