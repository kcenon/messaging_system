// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file main.cpp
 * @brief Scheduled and periodic task example
 *
 * This example demonstrates:
 * - Scheduling tasks with countdown delays
 * - Creating periodic tasks with intervals
 * - Using cron expressions for scheduling
 */

#include <kcenon/messaging/task/task_system.h>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <thread>

using namespace kcenon::messaging::task;
using namespace kcenon::common;

// Helper function to get current time as string
std::string current_time() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
    return ss.str();
}

int main() {
    std::cout << "=== Scheduled Tasks Example ===" << std::endl;
    std::cout << "Start time: " << current_time() << std::endl;

    // Configure with scheduler enabled
    task_system_config config;
    config.worker.concurrency = 2;
    config.enable_scheduler = true;
    task_system system(config);

    // Counter for periodic task
    std::atomic<int> heartbeat_count{0};

    // Register a handler for delayed tasks
    system.register_handler(
        "delayed.task", [](const task& t, task_context& ctx) {
            auto payload = t.payload();
            auto message = payload.get_string("message").value_or("no message");

            std::cout << "[" << current_time() << "] Delayed task executed: "
                      << message << std::endl;

            container_module::value_container result;
            result.set_value("executed_at", current_time());
            return ok(result);
        });

    // Register a handler for periodic heartbeat
    system.register_handler(
        "heartbeat", [&heartbeat_count](const task& t, task_context& ctx) {
            int count = ++heartbeat_count;
            std::cout << "[" << current_time() << "] Heartbeat #" << count
                      << std::endl;

            container_module::value_container result;
            result.set_value("count", count);
            return ok(result);
        });

    // Register a cleanup handler
    system.register_handler("cleanup", [](const task& t, task_context& ctx) {
        std::cout << "[" << current_time() << "] Running cleanup task"
                  << std::endl;

        container_module::value_container result;
        result.set_value("status", std::string("cleaned"));
        return ok(result);
    });

    // Start the system
    auto start_result = system.start();
    if (start_result.is_err()) {
        std::cerr << "Failed to start: " << start_result.error().message
                  << std::endl;
        return 1;
    }
    std::cout << "Task system started\n" << std::endl;

    // 1. Submit a task with countdown delay
    std::cout << "Scheduling delayed task (3 seconds)..." << std::endl;
    {
        auto task_result = task_builder("delayed.task").build();
        if (task_result.is_ok()) {
            auto t = task_result.unwrap();
            auto payload = std::make_shared<container_module::value_container>();
            payload->set_value("message",
                               std::string("This was delayed by 3 seconds"));
            t.set_payload(std::move(payload));
            system.submit_later(std::move(t), std::chrono::seconds(3));
        }
    }

    // 2. Schedule a periodic heartbeat (every 2 seconds)
    std::cout << "Setting up periodic heartbeat (every 2 seconds)..."
              << std::endl;
    {
        auto task_result = task_builder("heartbeat").build();
        if (task_result.is_ok()) {
            auto schedule_result = system.schedule_periodic(
                "heartbeat-schedule", task_result.unwrap(),
                std::chrono::seconds(2));
            if (schedule_result.is_err()) {
                std::cerr << "Failed to schedule: "
                          << schedule_result.error().message << std::endl;
            }
        }
    }

    // 3. Schedule a cron-based task (every minute at :30 seconds)
    // Note: This uses a simplified cron format
    std::cout << "Setting up cron task (for demonstration)..." << std::endl;
    {
        auto task_result = task_builder("cleanup").build();
        if (task_result.is_ok()) {
            // Run every minute (at second 0)
            auto schedule_result = system.schedule_cron(
                "cleanup-schedule", task_result.unwrap(), "0 * * * * *");
            if (schedule_result.is_err()) {
                std::cout << "  (Cron scheduling may not trigger during demo)"
                          << std::endl;
            }
        }
    }

    // 4. Submit another delayed task
    std::cout << "Scheduling another delayed task (5 seconds)..." << std::endl;
    {
        auto task_result = task_builder("delayed.task").build();
        if (task_result.is_ok()) {
            auto t = task_result.unwrap();
            auto payload = std::make_shared<container_module::value_container>();
            payload->set_value("message",
                               std::string("This was delayed by 5 seconds"));
            t.set_payload(std::move(payload));
            system.submit_later(std::move(t), std::chrono::seconds(5));
        }
    }

    std::cout << "\nWaiting for scheduled tasks to execute..." << std::endl;
    std::cout << "(Running for 8 seconds)\n" << std::endl;

    // Let the system run for a while
    std::this_thread::sleep_for(std::chrono::seconds(8));

    // Check scheduler status
    if (system.scheduler()) {
        auto schedules = system.scheduler()->list_schedules();
        std::cout << "\n=== Active Schedules ===" << std::endl;
        for (const auto& entry : schedules) {
            std::cout << "  - " << entry.name << " (";
            if (entry.is_cron()) {
                std::cout << "cron: " << entry.cron_expression();
            } else {
                std::cout << "interval: " << entry.interval().count() << "s";
            }
            std::cout << ", runs=" << entry.run_count << ")" << std::endl;
        }
    }

    // Display statistics
    auto stats = system.get_statistics();
    std::cout << "\n=== Statistics ===" << std::endl;
    std::cout << "Total tasks processed: " << stats.total_tasks_processed << std::endl;
    std::cout << "Succeeded: " << stats.total_tasks_succeeded << std::endl;
    std::cout << "Heartbeats: " << heartbeat_count.load() << std::endl;

    std::cout << "\nShutting down..." << std::endl;
    system.shutdown_graceful(std::chrono::seconds(5));
    std::cout << "Done! End time: " << current_time() << std::endl;

    return 0;
}
