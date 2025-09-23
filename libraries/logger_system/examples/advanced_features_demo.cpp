/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <kcenon/logger/core/logger.h>
#include <kcenon/logger/writers/console_writer.h>
#include <kcenon/logger/writers/file_writer.h>
#include <kcenon/logger/writers/rotating_file_writer.h>
#include <iostream>
#include <thread>
#include <chrono>

using namespace kcenon::logger;

int main() {
    std::cout << "=== Logger Advanced Features Demo ===" << std::endl;

    // Create logger instance
    auto logger = std::make_unique<kcenon::logger::logger>(true, 1024);

    // Add multiple writers
    std::cout << "\n1. Adding Multiple Writers:" << std::endl;
    logger->add_writer(std::make_unique<console_writer>());
    logger->add_writer(std::make_unique<file_writer>("logs/app.log"));
    logger->add_writer(std::make_unique<rotating_file_writer>(
        "logs/rotating.log",
        1024 * 1024,  // 1MB max size
        5             // Keep 5 backup files
    ));

    // Start the logger
    logger->start();

    std::cout << "\n2. Testing Multiple Log Levels:" << std::endl;

    // Log messages at different levels
    logger->log(kcenon::thread::log_level::trace, "Trace: Detailed debugging information");
    logger->log(kcenon::thread::log_level::debug, "Debug: Debugging information");
    logger->log(kcenon::thread::log_level::info, "Info: Informational message");
    logger->log(kcenon::thread::log_level::warning, "Warning: Something needs attention");
    logger->log(kcenon::thread::log_level::error, "Error: An error occurred");
    logger->log(kcenon::thread::log_level::critical, "Critical: System critical error");

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "\n3. Testing Rotating File Writer:" << std::endl;

    // Generate enough logs to potentially trigger rotation
    for (int i = 0; i < 100; ++i) {
        std::string msg = "Log entry " + std::to_string(i) +
                         " - This is a longer message to fill up the file size. "
                         "Adding more text to demonstrate rotating file functionality.";
        logger->log(kcenon::thread::log_level::info, msg);
    }

    std::cout << "\n4. Testing Multi-threaded Logging:" << std::endl;

    // Create multiple threads that log concurrently
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&logger, i]() {
            for (int j = 0; j < 10; ++j) {
                std::string msg = "Thread " + std::to_string(i) +
                                 " - Message " + std::to_string(j);
                logger->log(kcenon::thread::log_level::info, msg);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }

    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "\n5. Testing Flush Operation:" << std::endl;

    // Log some messages
    logger->log(kcenon::thread::log_level::info, "Message before flush");

    // Flush to ensure all messages are written
    logger->flush();
    std::cout << "Flushed all pending messages to writers" << std::endl;

    std::cout << "\n6. Testing Final Messages:" << std::endl;

    // Log final messages
    logger->log(kcenon::thread::log_level::info, "Final info message");
    logger->log(kcenon::thread::log_level::warning, "Final warning message");
    logger->log(kcenon::thread::log_level::error, "Final error message");

    // Cleanup
    logger->flush();
    logger->stop();

    std::cout << "\n=== Demo Complete ===" << std::endl;
    std::cout << "Check the logs/ directory for output files:" << std::endl;
    std::cout << "- app.log: Contains all logged messages" << std::endl;
    std::cout << "- rotating.log*: Rotating log files" << std::endl;

    return 0;
}