// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file main.cpp
 * @brief Chord pattern example (parallel execution with aggregation)
 *
 * This example demonstrates:
 * - Executing multiple tasks in parallel
 * - Aggregating results when all tasks complete
 * - Fan-out/Fan-in pattern for data collection
 */

#include <kcenon/messaging/task/task_system.h>
#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <thread>

using namespace kcenon::messaging::task;
using namespace kcenon::common;

int main() {
    std::cout << "=== Chord Aggregation Example ===" << std::endl;
    std::cout << "Collecting data from multiple sources in parallel\n"
              << std::endl;

    task_system_config config;
    config.worker.concurrency = 4;  // Multiple workers for parallel execution
    task_system system(config);

    // Random number generator for simulating varying response times
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> delay_dist(100, 500);

    // Handler for fetching data from different sources
    system.register_handler(
        "fetch.data", [&gen, &delay_dist](const task& t, task_context& ctx) {
            auto payload = t.payload();
            auto source = payload.get_string("source").value_or("unknown");

            ctx.log_info("Fetching from source: " + source);
            ctx.update_progress(0.0, "Connecting to " + source);

            // Simulate varying network latency
            auto delay = std::chrono::milliseconds(delay_dist(gen));
            std::this_thread::sleep_for(delay);

            ctx.update_progress(0.5, "Downloading data");

            // Simulate data with different values from each source
            int value = 0;
            if (source == "database")
                value = 100;
            else if (source == "cache")
                value = 50;
            else if (source == "api")
                value = 75;
            else if (source == "file")
                value = 25;

            ctx.update_progress(1.0, "Fetch complete");
            ctx.log_info(source + " returned value: " + std::to_string(value));

            container_module::value_container result;
            result.set("source", source);
            result.set("value", value);
            result.set("latency_ms", static_cast<int>(delay.count()));
            return ok(result);
        });

    // Aggregation handler - called when all parallel tasks complete
    system.register_handler(
        "aggregate", [](const task& t, task_context& ctx) {
            ctx.log_info("Aggregating results from all sources");
            ctx.update_progress(0.0, "Starting aggregation");

            auto payload = t.payload();

            // In a real implementation, the chord pattern would pass
            // all results to this callback. For this example, we simulate.
            int total = 100 + 50 + 75 + 25;  // Sum of all source values
            int avg = total / 4;

            ctx.update_progress(0.5, "Calculating statistics");

            ctx.update_progress(1.0, "Aggregation complete");
            ctx.log_info("Aggregated total: " + std::to_string(total) +
                         ", average: " + std::to_string(avg));

            container_module::value_container result;
            result.set("total", total);
            result.set("average", avg);
            result.set("source_count", 4);
            result.set("aggregation_type", std::string("sum_and_avg"));
            return ok(result);
        });

    // Start the system
    auto start_result = system.start();
    if (start_result.is_err()) {
        std::cerr << "Failed to start: " << start_result.error().message
                  << std::endl;
        return 1;
    }
    std::cout << "Task system started with " << system.total_workers()
              << " workers\n"
              << std::endl;

    // Build the chord: Parallel tasks with aggregation callback
    std::cout << "Setting up chord pattern:" << std::endl;
    std::cout << "  Parallel tasks: fetch from [database, cache, api, file]"
              << std::endl;
    std::cout << "  Callback: aggregate results\n" << std::endl;

    std::vector<task> parallel_tasks;
    std::vector<std::string> sources = {"database", "cache", "api", "file"};

    for (const auto& source : sources) {
        auto task_result = task_builder("fetch.data").build();
        if (task_result.is_ok()) {
            auto t = task_result.unwrap();
            auto payload = std::make_shared<container_module::value_container>();
            payload->set_value("source", source);
            t.set_payload(std::move(payload));
            parallel_tasks.push_back(std::move(t));
            std::cout << "  Added fetch task for: " << source << std::endl;
        }
    }

    // Create the aggregation callback task
    auto callback_result = task_builder("aggregate").build();
    if (callback_result.is_err()) {
        std::cerr << "Failed to create callback task" << std::endl;
        return 1;
    }
    auto callback_task = callback_result.unwrap();

    // Execute chord pattern
    std::cout << "\nExecuting chord (parallel tasks + callback)..." << std::endl;
    auto chord_result =
        system.client().chord(std::move(parallel_tasks), std::move(callback_task));

    std::cout << "Waiting for all parallel tasks and aggregation...\n"
              << std::endl;

    // Wait for the aggregated result
    auto outcome = chord_result.get(std::chrono::seconds(30));

    if (outcome.is_ok()) {
        auto result = outcome.unwrap();
        std::cout << "=== Chord Completed Successfully ===" << std::endl;

        auto total = result.get_value<int>("total").value_or(0);
        auto average = result.get_value<int>("average").value_or(0);
        auto source_count = result.get_value<int>("source_count").value_or(0);
        auto agg_type =
            result.get_string("aggregation_type").value_or("unknown");

        std::cout << "Sources processed: " << source_count << std::endl;
        std::cout << "Aggregation type: " << agg_type << std::endl;
        std::cout << "Total value: " << total << std::endl;
        std::cout << "Average value: " << average << std::endl;
    } else {
        std::cerr << "Chord failed: " << outcome.error().message << std::endl;
    }

    // Also demonstrate individual parallel tasks without callback
    std::cout << "\n--- Bonus: Simple parallel execution ---" << std::endl;

    std::vector<async_result> individual_results;
    for (const auto& source : sources) {
        auto task_result = task_builder("fetch.data").build();
        if (task_result.is_ok()) {
            auto t = task_result.unwrap();
            auto payload = std::make_shared<container_module::value_container>();
            payload->set_value("source", source);
            t.set_payload(std::move(payload));
            individual_results.push_back(system.submit(std::move(t)));
        }
    }

    std::cout << "Submitted " << individual_results.size()
              << " parallel tasks" << std::endl;

    // Collect all results
    int parallel_total = 0;
    for (size_t i = 0; i < individual_results.size(); ++i) {
        auto result = individual_results[i].get(std::chrono::seconds(10));
        if (result.is_ok()) {
            auto value = result.unwrap();
            auto source = value.get_string("source").value_or("?");
            auto val = value.get_value<int>("value").value_or(0);
            auto latency = value.get_value<int>("latency_ms").value_or(0);
            parallel_total += val;
            std::cout << "  " << source << ": value=" << val
                      << ", latency=" << latency << "ms" << std::endl;
        }
    }
    std::cout << "  Total: " << parallel_total << std::endl;

    // Display statistics
    auto stats = system.get_statistics();
    std::cout << "\n=== Statistics ===" << std::endl;
    std::cout << "Total tasks processed: " << stats.total_tasks_processed << std::endl;
    std::cout << "Succeeded: " << stats.total_tasks_succeeded << std::endl;

    std::cout << "\nShutting down..." << std::endl;
    system.shutdown_graceful(std::chrono::seconds(5));
    std::cout << "Done!" << std::endl;

    return 0;
}
