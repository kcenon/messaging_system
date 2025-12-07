// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file main.cpp
 * @brief Simple worker example demonstrating basic Task module usage
 *
 * This example shows:
 * - Creating a task_system with default configuration
 * - Registering a handler with a lambda function
 * - Submitting a task and waiting for the result
 */

#include <kcenon/messaging/task/task_system.h>
#include <iostream>
#include <string>

using namespace kcenon::messaging::task;
using namespace kcenon::common;

int main() {
    std::cout << "=== Simple Worker Example ===" << std::endl;

    // Create task system with default configuration
    task_system_config config;
    config.worker.concurrency = 2;  // Use 2 worker threads
    task_system system(config);

    // Register a simple handler that processes greetings
    system.register_handler("greet", [](const task& t, task_context& ctx) {
        ctx.log_info("Processing greeting task");

        // Get the name from the payload
        auto payload = t.payload();
        auto name_opt = payload.get_string("name");
        std::string name = name_opt.value_or("World");

        // Create the result
        container_module::value_container result;
        result.set_value("greeting", "Hello, " + name + "!");
        result.set_value("processed", true);

        ctx.update_progress(1.0, "Completed");
        return ok(result);
    });

    // Register an addition handler
    system.register_handler("add", [](const task& t, task_context& ctx) {
        auto payload = t.payload();
        auto a_opt = payload.get_value<int>("a");
        auto b_opt = payload.get_value<int>("b");

        if (!a_opt || !b_opt) {
            return Result<container_module::value_container>(
                error_info{-1, "Missing 'a' or 'b' parameter"});
        }

        int sum = a_opt.value() + b_opt.value();

        container_module::value_container result;
        result.set_value("sum", sum);
        ctx.log_info("Calculated sum: " + std::to_string(sum));

        return ok(result);
    });

    // Start the system
    auto start_result = system.start();
    if (start_result.is_err()) {
        std::cerr << "Failed to start system: " << start_result.error().message
                  << std::endl;
        return 1;
    }
    std::cout << "Task system started with " << system.total_workers()
              << " workers" << std::endl;

    // Submit greeting task
    container_module::value_container greet_payload;
    greet_payload.set_value("name", std::string("Task System"));

    std::cout << "\nSubmitting greeting task..." << std::endl;
    auto greet_result = system.submit("greet", greet_payload);

    // Wait for the result with timeout
    auto greet_outcome = greet_result.get(std::chrono::seconds(10));
    if (greet_outcome.is_ok()) {
        auto value = greet_outcome.unwrap();
        auto greeting = value.get_string("greeting");
        std::cout << "Result: " << greeting.value_or("(no greeting)")
                  << std::endl;
    } else {
        std::cerr << "Greeting task failed: " << greet_outcome.error().message
                  << std::endl;
    }

    // Submit addition task
    container_module::value_container add_payload;
    add_payload.set_value("a", 10);
    add_payload.set_value("b", 25);

    std::cout << "\nSubmitting addition task (10 + 25)..." << std::endl;
    auto add_result = system.submit("add", add_payload);

    auto add_outcome = add_result.get(std::chrono::seconds(10));
    if (add_outcome.is_ok()) {
        auto value = add_outcome.unwrap();
        auto sum = value.get_value<int>("sum");
        std::cout << "Result: " << sum.value_or(0) << std::endl;
    } else {
        std::cerr << "Addition task failed: " << add_outcome.error().message
                  << std::endl;
    }

    // Display statistics
    auto stats = system.get_statistics();
    std::cout << "\n=== Statistics ===" << std::endl;
    std::cout << "Total tasks: " << stats.total_tasks << std::endl;
    std::cout << "Succeeded: " << stats.succeeded_tasks << std::endl;
    std::cout << "Failed: " << stats.failed_tasks << std::endl;

    // Stop the system gracefully
    std::cout << "\nShutting down..." << std::endl;
    system.shutdown_graceful(std::chrono::seconds(5));

    std::cout << "Done!" << std::endl;
    return 0;
}
