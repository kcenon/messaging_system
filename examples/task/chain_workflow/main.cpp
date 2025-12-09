// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file main.cpp
 * @brief Chain workflow example (sequential task execution)
 *
 * This example demonstrates:
 * - Creating a chain of tasks that execute sequentially
 * - Passing results from one task to the next
 * - ETL (Extract-Transform-Load) pipeline pattern
 */

#include <kcenon/messaging/task/task_system.h>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using namespace kcenon::messaging::task;
using namespace kcenon::common;

int main() {
    std::cout << "=== Chain Workflow Example ===" << std::endl;
    std::cout << "Demonstrating ETL (Extract-Transform-Load) pipeline\n"
              << std::endl;

    task_system_config config;
    config.worker.concurrency = 4;
    task_system system(config);

    // Step 1: Extract - Simulate reading data from source
    system.register_handler("extract", [](const task& t, task_context& ctx) {
        ctx.log_info("Starting extraction...");
        ctx.update_progress(0.0, "Extracting data");

        // Simulate extracting data from a source
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ctx.update_progress(0.5, "Reading records");

        std::vector<std::string> records = {"record1", "record2", "record3",
                                            "record4", "record5"};

        ctx.update_progress(1.0, "Extraction complete");
        ctx.log_info("Extracted " + std::to_string(records.size()) +
                     " records");

        container_module::value_container result;
        result.set_value("record_count", static_cast<int>(records.size()));
        result.set_value("source", std::string("database"));
        result.set_value("step", std::string("extract"));
        return ok(result);
    });

    // Step 2: Transform - Process and transform the data
    system.register_handler("transform", [](const task& t, task_context& ctx) {
        ctx.log_info("Starting transformation...");
        ctx.update_progress(0.0, "Transforming data");

        auto payload = t.payload();
        auto record_count = payload.get_value<int>("record_count").value_or(0);

        // Simulate transformation
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
        ctx.update_progress(0.5, "Processing records");

        // Transform: double the records (as an example)
        int transformed_count = record_count * 2;

        ctx.update_progress(1.0, "Transformation complete");
        ctx.log_info("Transformed to " + std::to_string(transformed_count) +
                     " records");

        container_module::value_container result;
        result.set_value("record_count", transformed_count);
        result.set_value("transformation", std::string("normalized"));
        result.set_value("step", std::string("transform"));
        return ok(result);
    });

    // Step 3: Load - Write data to destination
    system.register_handler("load", [](const task& t, task_context& ctx) {
        ctx.log_info("Starting load...");
        ctx.update_progress(0.0, "Loading data");

        auto payload = t.payload();
        auto record_count = payload.get_value<int>("record_count").value_or(0);
        auto transformation =
            payload.get_string("transformation").value_or("none");

        // Simulate loading to destination
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        ctx.update_progress(0.5, "Writing to destination");

        ctx.update_progress(1.0, "Load complete");
        ctx.log_info("Loaded " + std::to_string(record_count) +
                     " records to destination");

        container_module::value_container result;
        result.set_value("loaded_count", record_count);
        result.set_value("destination", std::string("data_warehouse"));
        result.set_value("transformation_applied", transformation);
        result.set_value("step", std::string("load"));
        result.set_value("success", true);
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

    // Build the chain: Extract -> Transform -> Load
    std::cout << "Building ETL chain: Extract -> Transform -> Load"
              << std::endl;

    std::vector<task> chain_tasks;

    // Create extract task
    auto extract_result = task_builder("extract").build();
    if (extract_result.is_ok()) {
        auto t = extract_result.unwrap();
        auto payload = std::make_shared<container_module::value_container>();
        payload->set_value("source_table", std::string("users"));
        t.set_task_payload(std::move(payload));
        chain_tasks.push_back(std::move(t));
    }

    // Create transform task
    auto transform_result = task_builder("transform").build();
    if (transform_result.is_ok()) {
        chain_tasks.push_back(transform_result.unwrap());
    }

    // Create load task
    auto load_result = task_builder("load").build();
    if (load_result.is_ok()) {
        chain_tasks.push_back(load_result.unwrap());
    }

    // Execute the chain
    std::cout << "\nExecuting chain..." << std::endl;
    auto chain_result = system.client().chain(std::move(chain_tasks));

    std::cout << "Waiting for chain to complete...\n" << std::endl;

    // Wait for the final result
    auto outcome = chain_result.get(std::chrono::seconds(30));

    if (outcome.is_ok()) {
        auto result = outcome.unwrap();
        std::cout << "=== Chain Completed Successfully ===" << std::endl;

        auto loaded_count = result.get_value<int>("loaded_count").value_or(0);
        auto destination = result.get_string("destination").value_or("?");
        auto transformation =
            result.get_string("transformation_applied").value_or("?");

        std::cout << "Records loaded: " << loaded_count << std::endl;
        std::cout << "Destination: " << destination << std::endl;
        std::cout << "Transformation: " << transformation << std::endl;
    } else {
        std::cerr << "Chain failed: " << outcome.error().message << std::endl;
    }

    // Display statistics
    auto stats = system.get_statistics();
    std::cout << "\n=== Statistics ===" << std::endl;
    std::cout << "Total tasks processed: " << stats.total_tasks_processed << std::endl;
    std::cout << "Succeeded: " << stats.total_tasks_succeeded << std::endl;
    std::cout << "Failed: " << stats.total_tasks_failed << std::endl;

    std::cout << "\nShutting down..." << std::endl;
    system.shutdown_graceful(std::chrono::seconds(5));
    std::cout << "Done!" << std::endl;

    return 0;
}
