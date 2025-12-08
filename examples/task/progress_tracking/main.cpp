// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file main.cpp
 * @brief Progress tracking example for long-running tasks
 *
 * This example demonstrates:
 * - Updating task progress with update_progress()
 * - Polling for progress from the client side
 * - Displaying progress bars in the console
 */

#include <kcenon/messaging/task/task_system.h>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

using namespace kcenon::messaging::task;
using namespace kcenon::common;

// Helper function to display a progress bar
void display_progress_bar(double progress, const std::string& message = "") {
    const int bar_width = 40;
    int filled = static_cast<int>(progress * bar_width);

    std::cout << "\r[";
    for (int i = 0; i < bar_width; ++i) {
        if (i < filled)
            std::cout << "=";
        else if (i == filled)
            std::cout << ">";
        else
            std::cout << " ";
    }
    std::cout << "] " << std::fixed << std::setprecision(1) << (progress * 100)
              << "%";
    if (!message.empty()) {
        std::cout << " - " << message;
    }
    std::cout << std::flush;
}

int main() {
    std::cout << "=== Progress Tracking Example ===" << std::endl;

    task_system_config config;
    config.worker.concurrency = 2;
    task_system system(config);

    // Handler for a long-running file processing task
    system.register_handler(
        "process.file", [](const task& t, task_context& ctx) {
            auto payload = t.payload();
            auto filename = payload.get_string("filename").value_or("data.csv");
            auto total_steps =
                payload.get_value<int>("steps").value_or(10);

            ctx.log_info("Starting to process: " + filename);

            // Simulate processing with progress updates
            for (int step = 0; step <= total_steps; ++step) {
                // Check for cancellation
                if (ctx.is_cancelled()) {
                    ctx.log_warning("Task cancelled at step " +
                                    std::to_string(step));
                    return Result<container_module::value_container>(
                        error_info{-1, "Task was cancelled"});
                }

                // Update progress
                double progress = static_cast<double>(step) / total_steps;
                std::string message = "Processing step " + std::to_string(step) +
                                      "/" + std::to_string(total_steps);
                ctx.update_progress(progress, message);

                // Simulate work
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }

            ctx.log_info("Processing complete for: " + filename);

            container_module::value_container result;
            result.set_value("filename", filename);
            result.set_value("steps_completed", total_steps);
            result.set_value("status", std::string("completed"));
            return ok(result);
        });

    // Handler for multi-phase task with checkpoints
    system.register_handler(
        "multi.phase", [](const task& t, task_context& ctx) {
            // Check if we have a checkpoint from a previous attempt
            auto checkpoint = ctx.load_checkpoint();
            int start_phase = 1;

            auto saved_phase = checkpoint.get_value<int>("phase");
            if (saved_phase && ctx.attempt_number() > 1) {
                start_phase = saved_phase.value();
                ctx.log_info("Resuming from phase " +
                             std::to_string(start_phase));
            }

            const int total_phases = 5;

            for (int phase = start_phase; phase <= total_phases; ++phase) {
                double progress =
                    static_cast<double>(phase - 1) / total_phases;
                ctx.update_progress(progress,
                                    "Phase " + std::to_string(phase) + "/" +
                                        std::to_string(total_phases));

                // Simulate phase work
                std::this_thread::sleep_for(std::chrono::milliseconds(300));

                // Save checkpoint after each phase
                container_module::value_container state;
                state.set_value("phase", phase + 1);
                ctx.save_checkpoint(state);

                ctx.log_info("Completed phase " + std::to_string(phase));
            }

            ctx.update_progress(1.0, "All phases complete");

            container_module::value_container result;
            result.set_value("phases_completed", total_phases);
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

    // Submit a file processing task
    std::cout << "=== Processing File ===" << std::endl;
    container_module::value_container payload;
    payload.set_value("filename", std::string("large_dataset.csv"));
    payload.set_value("steps", 15);

    auto file_result = system.submit("process.file", payload);
    std::string task_id = file_result.task_id();
    std::cout << "Submitted task: " << task_id << std::endl;

    // Poll for progress
    std::cout << "Tracking progress:\n" << std::endl;

    while (true) {
        auto status = file_result.state();

        // Get progress from the task
        double progress = file_result.progress();
        auto message = file_result.progress_message();

        display_progress_bar(progress, message);

        if (status == task_state::succeeded || status == task_state::failed ||
            status == task_state::cancelled) {
            std::cout << std::endl;  // New line after progress bar
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Get the final result
    auto file_outcome = file_result.get(std::chrono::seconds(1));
    if (file_outcome.is_ok()) {
        auto value = file_outcome.unwrap();
        std::cout << "\nFile processed successfully!" << std::endl;
        std::cout << "  Steps completed: "
                  << value.get_value<int>("steps_completed").value_or(0)
                  << std::endl;
    } else {
        std::cerr << "\nFile processing failed: "
                  << file_outcome.error().message << std::endl;
    }

    // Submit multi-phase task
    std::cout << "\n=== Multi-Phase Task ===" << std::endl;
    auto phase_result = system.submit("multi.phase", {});
    std::cout << "Submitted multi-phase task\n" << std::endl;

    // Track multi-phase progress
    while (true) {
        auto status = phase_result.state();
        double progress = phase_result.progress();
        auto message = phase_result.progress_message();

        display_progress_bar(progress, message);

        if (status == task_state::succeeded || status == task_state::failed ||
            status == task_state::cancelled) {
            std::cout << std::endl;
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    auto phase_outcome = phase_result.get(std::chrono::seconds(1));
    if (phase_outcome.is_ok()) {
        auto value = phase_outcome.unwrap();
        std::cout << "\nMulti-phase task completed!" << std::endl;
        std::cout << "  Phases completed: "
                  << value.get_value<int>("phases_completed").value_or(0)
                  << std::endl;
    }

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
