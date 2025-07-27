/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <logger/logger.h>
#include <logger/writers/console_writer.h>
#include <logger/structured/structured_logger.h>
#include <thread>
#include <iostream>
#include <random>

using namespace logger_module;

void generate_logs(logger* log, int thread_id, int count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> level_dist(0, 5);
    std::uniform_int_distribution<> message_size_dist(10, 200);
    
    for (int i = 0; i < count; ++i) {
        auto level = static_cast<thread_module::log_level>(level_dist(gen));
        
        // Generate message of random size
        std::string message = "Thread " + std::to_string(thread_id) + " - Message " + std::to_string(i);
        message.append(message_size_dist(gen), 'x');
        
        log->log(level, message);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void print_metrics(const performance_metrics& metrics) {
    std::cout << "\n=== Logger Performance Metrics ===" << std::endl;
    std::cout << "Messages enqueued: " << metrics.messages_enqueued.load() << std::endl;
    std::cout << "Messages processed: " << metrics.messages_processed.load() << std::endl;
    std::cout << "Messages dropped: " << metrics.messages_dropped.load() << std::endl;
    std::cout << "Drop rate: " << metrics.get_drop_rate_percent() << "%" << std::endl;
    std::cout << "Throughput: " << metrics.get_messages_per_second() << " msg/s" << std::endl;
    std::cout << "Bandwidth: " << metrics.get_bytes_per_second() / 1024.0 << " KB/s" << std::endl;
    std::cout << "Queue utilization: " << metrics.get_queue_utilization_percent() << "%" << std::endl;
    std::cout << "Avg enqueue time: " << metrics.get_avg_enqueue_time_ns() << " ns" << std::endl;
    
    std::cout << "\n--- Writer Metrics ---" << std::endl;
    for (const auto& [name, writer_metrics] : metrics.writer_stats) {
        std::cout << "Writer '" << name << "':" << std::endl;
        std::cout << "  Messages written: " << writer_metrics.messages_written.load() << std::endl;
        std::cout << "  Bytes written: " << writer_metrics.bytes_written.load() / 1024.0 << " KB" << std::endl;
        std::cout << "  Write failures: " << writer_metrics.write_failures.load() << std::endl;
        std::cout << "  Avg write time: " << writer_metrics.get_avg_write_time_us() << " μs" << std::endl;
    }
}

void test_structured_logging() {
    std::cout << "\n=== Testing Structured Logging ===" << std::endl;
    
    auto base_logger = std::make_shared<logger>(false); // Sync mode for testing
    base_logger->add_writer(std::make_unique<console_writer>());
    base_logger->start();
    
    // Test different output formats
    auto formats = {
        structured_logger::output_format::json,
        structured_logger::output_format::logfmt,
        structured_logger::output_format::plain
    };
    
    for (auto format : formats) {
        std::cout << "\n--- Format: " << 
            (format == structured_logger::output_format::json ? "JSON" :
             format == structured_logger::output_format::logfmt ? "LOGFMT" : "PLAIN") 
            << " ---" << std::endl;
            
        structured_logger slog(base_logger, format);
        slog.set_service_info("metrics_demo", "1.0.0", "development");
        
        slog.info("User logged in")
            .field("user_id", 12345)
            .field("ip_address", "192.168.1.100")
            .field("login_method", "oauth")
            .context("request_id", "abc-123-def")
            .commit();
            
        slog.error("Database connection failed")
            .field("database", "users")
            .field("host", "db.example.com")
            .field("port", 5432)
            .field("retry_count", 3)
            .duration(std::chrono::milliseconds(1500))
            .commit();
    }
}

int main() {
    // Create logger with metrics enabled
    auto logger = std::make_unique<logger_module::logger>(true, 1024); // Small buffer to test dropping
    
    // Add console writer
    logger->add_writer(std::make_unique<console_writer>());
    
    // Enable metrics collection
    logger->enable_metrics_collection(true);
    
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
    auto metrics = logger->get_current_metrics();
    print_metrics(metrics);
    
    // Test structured logging
    test_structured_logging();
    
    // Stop logger
    logger->stop();
    
    std::cout << "\nDemo completed!" << std::endl;
    
    return 0;
}