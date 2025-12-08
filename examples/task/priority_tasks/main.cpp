// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file main.cpp
 * @brief Priority-based task processing example
 *
 * This example demonstrates:
 * - Creating tasks with different priorities
 * - Observing priority-based processing order
 * - Using multiple task queues
 */

#include <kcenon/messaging/task/task_system.h>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <memory>

using namespace kcenon::messaging::task;
using namespace kcenon::common;
using kcenon::messaging::message_priority;

int main() {
    std::cout << "=== Priority Tasks Example ===" << std::endl;

    // Configure system with a single worker to demonstrate ordering
    task_system_config config;
    config.worker.concurrency = 1;  // Single worker to show priority ordering
    task_system system(config);

    // Register a handler that takes some time to process
    system.register_handler(
        "process", [](const task& t, task_context& ctx) {
            auto payload = t.payload();
            auto name = payload.get_string("name").value_or("unknown");
            auto priority = payload.get_value<int>("priority").value_or(0);

            ctx.log_info("Processing: " + name + " (priority: " +
                         std::to_string(priority) + ")");

            // Simulate work
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            container_module::value_container result;
            result.set_value("processed", name);
            result.set_value("priority", priority);
            return ok(result);
        });

    auto start_result = system.start();
    if (start_result.is_err()) {
        std::cerr << "Failed to start: " << start_result.error().message
                  << std::endl;
        return 1;
    }

    std::cout << "\nSubmitting tasks with different priorities..." << std::endl;
    std::cout << "(Higher priority value executes first)\n" << std::endl;

    std::vector<async_result> results;

    const auto low_priority = message_priority::low;
    const auto medium_priority = message_priority::normal;
    const auto high_priority = message_priority::high;
    auto make_payload = [](std::string name, int priority) {
        auto payload = std::make_shared<container_module::value_container>();
        payload->set_value("name", std::move(name));
        payload->set_value("priority", priority);
        return payload;
    };

    // Submit tasks in reverse priority order to demonstrate priority scheduling
    // Note: The first task may start immediately before the rest are queued

    // Submit low priority tasks first
    for (int i = 0; i < 3; ++i) {
        auto task_result =
            task_builder("process")
                .priority(low_priority)  // Low priority
                .build();

        if (task_result.is_ok()) {
            auto t = task_result.unwrap();
            auto payload = make_payload("Low-" + std::to_string(i),
                                        static_cast<int>(low_priority));
            t.set_task_payload(std::move(payload));

            results.push_back(system.submit(std::move(t)));
            std::cout << "  Submitted: Low-" << i << " (priority "
                      << static_cast<int>(low_priority) << ")"
                      << std::endl;
        }
    }

    // Submit medium priority tasks
    for (int i = 0; i < 2; ++i) {
        auto task_result =
            task_builder("process")
                .priority(medium_priority)  // Medium priority
                .build();

        if (task_result.is_ok()) {
            auto t = task_result.unwrap();
            auto payload = make_payload("Medium-" + std::to_string(i),
                                        static_cast<int>(medium_priority));
            t.set_task_payload(std::move(payload));

            results.push_back(system.submit(std::move(t)));
            std::cout << "  Submitted: Medium-" << i << " (priority "
                      << static_cast<int>(medium_priority) << ")"
                      << std::endl;
        }
    }

    // Submit high priority task
    {
        auto task_result =
            task_builder("process")
                .priority(high_priority)  // High priority
                .build();

        if (task_result.is_ok()) {
            auto t = task_result.unwrap();
            auto payload = make_payload("High-0",
                                        static_cast<int>(high_priority));
            t.set_task_payload(std::move(payload));

            results.push_back(system.submit(std::move(t)));
            std::cout << "  Submitted: High-0 (priority "
                      << static_cast<int>(high_priority) << ")" << std::endl;
        }
    }

    std::cout << "\nProcessing order (observe priority handling):" << std::endl;

    // Collect results in order of completion
    for (size_t i = 0; i < results.size(); ++i) {
        auto outcome = results[i].get(std::chrono::seconds(30));
        if (outcome.is_ok()) {
            auto value = outcome.unwrap();
            auto name = value.get_string("processed").value_or("?");
            auto priority = value.get_value<int>("priority").value_or(0);
            std::cout << "  Completed: " << name << " (priority " << priority
                      << ")" << std::endl;
        }
    }

    // Display statistics
    auto stats = system.get_statistics();
    std::cout << "\n=== Statistics ===" << std::endl;
    std::cout << "Total processed: " << stats.total_tasks_succeeded << std::endl;

    system.shutdown_graceful(std::chrono::seconds(5));
    std::cout << "\nDone!" << std::endl;
    return 0;
}
