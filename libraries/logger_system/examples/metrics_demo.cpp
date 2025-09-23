/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <kcenon/logger/core/logger.h>
#include <kcenon/logger/core/metrics/logger_metrics.h>
#include <kcenon/logger/writers/console_writer.h>
#include <thread>
#include <iostream>
#include <random>

using namespace kcenon::logger;
using namespace kcenon::logger::metrics;
namespace thread_module = kcenon::thread;

void generate_logs(logger* log, int thread_id, int count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> level_dist(0, 5);
    std::uniform_int_distribution<> message_size_dist(10, 200);
    
    for (int i = 0; i < count; ++i) {
        auto level = static_cast<kcenon::thread::log_level>(level_dist(gen));
        
        // Generate message of random size
        std::string message = "Thread " + std::to_string(thread_id) + " - Message " + std::to_string(i);
        message.append(message_size_dist(gen), 'x');
        
        log->log(level, message);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void print_metrics(const logger_performance_stats& metrics) {
    std::cout << "\n=== Logger Performance Metrics ===" << std::endl;
    std::cout << "Messages logged: " << metrics.messages_logged.load() << std::endl;
    std::cout << "Messages dropped: " << metrics.messages_dropped.load() << std::endl;
    std::cout << "Writer errors: " << metrics.writer_errors.load() << std::endl;
    std::cout << "Queue size: " << metrics.queue_size.load() << std::endl;
    std::cout << "Max queue size: " << metrics.max_queue_size.load() << std::endl;
    std::cout << "Throughput: " << metrics.get_messages_per_second() << " msg/s" << std::endl;
    std::cout << "Queue utilization: " << metrics.get_queue_utilization_percent() << "%" << std::endl;
}

void test_logging_levels() {
    std::cout << "\n=== Testing Different Log Levels ===" << std::endl;

    auto test_logger = std::make_shared<logger>(false); // Sync mode for testing
    test_logger->add_writer(std::make_unique<console_writer>());
    test_logger->start();

    // Test different log levels
    test_logger->log(kcenon::thread::log_level::debug, "Debug message");
    test_logger->log(kcenon::thread::log_level::info, "Info message");
    test_logger->log(kcenon::thread::log_level::warning, "Warning message");
    test_logger->log(kcenon::thread::log_level::error, "Error message");
    test_logger->log(kcenon::thread::log_level::critical, "Critical message");

    test_logger->stop();
}

int main() {
    // Create logger with metrics enabled
    auto logger = std::make_unique<kcenon::logger::logger>(true, 1024); // Small buffer to test dropping
    
    // Add console writer
    logger->add_writer(std::make_unique<console_writer>());

    // Start logger
    logger->start();
    
    std::cout << "Starting logger metrics demo..." << std::endl;
    std::cout << "Generating logs from multiple threads..." << std::endl;
    
    // Launch multiple threads to generate logs
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(generate_logs, logger.get(), i, 100);
    }
    
    // Wait for threads to complete
    for (auto& t : threads) {
        t.join();
    }
    
    // Wait a bit for processing
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Get and display metrics
    print_metrics(g_logger_stats);
    
    // Test logging levels
    test_logging_levels();
    
    // Stop logger
    logger->stop();
    
    std::cout << "\nDemo completed!" << std::endl;
    
    return 0;
}