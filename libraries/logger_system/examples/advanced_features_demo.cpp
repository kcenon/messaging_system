/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <kcenon/logger/core/logger.h>
#include <kcenon/logger/writers/console_writer.h>
#include <kcenon/logger/writers/file_writer.h>
#include <kcenon/logger/writers/rotating_file_writer.h>
#include <kcenon/logger/filters/log_filter.h>
#include <kcenon/logger/routing/log_router.h>
#include <iostream>
#include <thread>
#include <chrono>

using namespace kcenon::logger;
using namespace thread_module;
namespace logger_module = kcenon::logger;

int main() {
    std::cout << "=== Logger Advanced Features Demo ===" << std::endl;
    
    // Create logger instance
    auto logger = std::make_unique<kcenon::logger::logger>(true, 1024);
    
    // Add named writers
    logger->add_writer("console", std::make_unique<console_writer>());
    logger->add_writer("error_file", std::make_unique<file_writer>("logs/errors.log"));
    logger->add_writer("debug_file", std::make_unique<file_writer>("logs/debug.log"));
    logger->add_writer("rotating", std::make_unique<rotating_file_writer>(
        "logs/app.log", 
        1024 * 1024,  // 1MB max size
        5             // Keep 5 backup files
    ));
    
    // Start the logger
    logger->start();
    
    std::cout << "\n1. Testing Basic Filtering:" << std::endl;
    
    // Set global filter to only log warnings and above
    logger->set_filter(std::make_unique<level_filter>(log_level::warning));
    
    logger->log(log_level::trace, "This trace message should be filtered out");
    logger->log(log_level::debug, "This debug message should be filtered out");
    logger->log(log_level::info, "This info message should be filtered out");
    logger->log(log_level::warning, "This warning should be logged");
    logger->log(log_level::error, "This error should be logged");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "\n2. Testing Regex Filtering:" << std::endl;
    
    // Filter out messages containing "sensitive"
    logger->set_filter(std::make_unique<regex_filter>("sensitive", false));
    
    logger->log(log_level::error, "This contains sensitive data");
    logger->log(log_level::error, "This is a normal error message");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "\n3. Testing Routing:" << std::endl;
    
    // Clear global filter
    logger->set_filter(nullptr);
    
    // Configure routing
    auto& router = logger->get_router();
    
    // Route errors to error file only
    router_builder builder1(router);
    builder1.when_level(log_level::error)
        .route_to("error_file", true);  // Stop propagation
    
    // Route debug messages to debug file
    router_builder builder2(router);
    builder2.when_level(log_level::trace)
        .route_to(std::vector<std::string>{"debug_file", "console"});
    
    // Route messages containing "performance" to rotating file
    router_builder builder3(router);
    builder3.when_matches("performance")
        .route_to("rotating");
    
    // Set exclusive routes (no default fallback)
    router.set_exclusive_routes(true);
    
    // Test routing
    logger->log(log_level::error, "Error: Database connection failed");
    logger->log(log_level::debug, "Debug: Processing user request");
    logger->log(log_level::info, "Info: Server performance metrics updated");
    logger->log(log_level::warning, "Warning: This should not be logged (no route)");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "\n4. Testing Composite Filtering:" << std::endl;
    
    // Create composite filter: (level >= warning) AND (not contains "ignore")
    auto composite = std::make_unique<composite_filter>(composite_filter::logic_type::AND);
    composite->add_filter(std::make_unique<level_filter>(log_level::warning));
    composite->add_filter(std::make_unique<regex_filter>("ignore", false));
    
    logger->set_filter(std::move(composite));
    
    logger->log(log_level::info, "Info: Should be filtered by level");
    logger->log(log_level::warning, "Warning: Should be logged");
    logger->log(log_level::error, "Error: Please ignore this");  // Filtered by regex
    logger->log(log_level::error, "Error: Real error message");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "\n5. Testing Rotating File Writer:" << std::endl;
    
    // Clear routes and filter for rotating test
    router.clear_routes();
    router.set_exclusive_routes(false);
    logger->set_filter(nullptr);
    
    // Generate enough logs to trigger rotation
    for (int i = 0; i < 1000; ++i) {
        std::string msg = "Log entry " + std::to_string(i) + 
                         " - This is a longer message to fill up the file size quickly. "
                         "Adding more text to reach the rotation threshold faster.";
        logger->log(log_level::info, msg);
    }
    
    std::cout << "\n6. Testing Custom Function Filter:" << std::endl;
    
    // Filter that only logs messages from specific threads
    auto thread_id = std::this_thread::get_id();
    auto thread_filter = std::make_unique<function_filter>(
        [thread_id](log_level level, const std::string& msg, 
                   const std::string& file, int line, const std::string& func) {
            (void)level;
            (void)msg;
            (void)file;
            (void)line;
            (void)func;
            return std::this_thread::get_id() == thread_id;
        }
    );
    
    logger->set_filter(std::move(thread_filter));
    
    // Log from main thread
    logger->log(log_level::info, "Message from main thread");
    
    // Log from another thread
    std::thread other_thread([&logger]() {
        logger->log(log_level::info, "Message from other thread (should be filtered)");
    });
    other_thread.join();
    
    // Cleanup
    logger->flush();
    logger->stop();
    
    std::cout << "\n=== Demo Complete ===" << std::endl;
    std::cout << "Check the logs/ directory for output files:" << std::endl;
    std::cout << "- errors.log: Contains only error messages" << std::endl;
    std::cout << "- debug.log: Contains debug level messages" << std::endl;
    std::cout << "- app.log*: Rotating log files" << std::endl;
    
    return 0;
}