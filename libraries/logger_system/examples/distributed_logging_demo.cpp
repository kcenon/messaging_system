/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <kcenon/logger/core/logger.h>
#include <kcenon/logger/writers/console_writer.h>
#include <kcenon/logger/writers/network_writer.h>
#include <kcenon/logger/server/log_server.h>
#include <kcenon/logger/analysis/log_analyzer.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <atomic>

using namespace kcenon::logger;
using namespace thread_module;
namespace logger_module = kcenon::logger;

// Simulate a client application
void simulate_client(int client_id, const std::string& server_host, uint16_t server_port) {
    std::cout << "Client " << client_id << " starting..." << std::endl;
    
    // Create logger with network writer
    auto logger = std::make_unique<kcenon::logger::logger>(true, 1024);
    logger->add_writer("console", std::make_unique<console_writer>());
    logger->add_writer("network", std::make_unique<network_writer>(
        server_host, server_port, network_writer::protocol_type::tcp
    ));
    
    logger->start();
    
    // Simulate various log patterns
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> level_dist(0, 5);
    std::uniform_int_distribution<> sleep_dist(10, 100);
    
    std::vector<std::string> messages = {
        "User login successful",
        "Database query executed",
        "API request processed",
        "Cache miss occurred",
        "Background job completed",
        "Error: Connection timeout",
        "Warning: High memory usage",
        "Critical: Disk space low"
    };
    
    for (int i = 0; i < 100; ++i) {
        auto level = static_cast<log_level>(level_dist(gen));
        auto& msg = messages[i % messages.size()];
        
        logger->log(level, "Client " + std::to_string(client_id) + ": " + msg);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_dist(gen)));
    }
    
    logger->flush();
    logger->stop();
    
    std::cout << "Client " << client_id << " finished" << std::endl;
}

int main() {
    std::cout << "=== Distributed Logging Demo ===" << std::endl;
    
    const uint16_t server_port = 9999;
    const std::string server_host = "localhost";
    
    // Create log analyzer
    auto analyzer = std::make_unique<log_analyzer>(
        std::chrono::seconds(10),  // 10-second windows
        6                          // Keep 1 minute of history
    );
    
    // Add patterns to track
    analyzer->add_pattern("errors", "error|fail|exception");
    analyzer->add_pattern("warnings", "warning|warn");
    analyzer->add_pattern("database", "database|query|sql");
    analyzer->add_pattern("api", "api|request|endpoint");
    
    // Add alert rules
    analyzer->add_alert_rule({
        "high_error_rate",
        [](const log_analyzer::time_window_stats& stats) {
            auto error_count = stats.level_counts.count(log_level::error) ? 
                              stats.level_counts.at(log_level::error) : 0;
            return error_count > 10;
        },
        [](const std::string& rule_name, const log_analyzer::time_window_stats& stats) {
            std::cout << "\n🚨 ALERT: " << rule_name 
                     << " - Error count: " << stats.level_counts.at(log_level::error)
                     << " in current window" << std::endl;
        }
    });
    
    // Create aggregator
    auto aggregator = std::make_unique<log_aggregator>();
    
    // Create and start log server
    auto server = std::make_unique<log_server>(server_port, true);
    
    // Add handler to process received logs
    server->add_handler([&analyzer, &aggregator](const log_server::network_log_entry& entry) {
        // Extract log level from parsed fields
        log_level level = log_level::info;
        if (entry.parsed_fields.count("level")) {
            const auto& level_str = entry.parsed_fields.at("level");
            if (level_str == "TRACE") level = log_level::trace;
            else if (level_str == "DEBUG") level = log_level::debug;
            else if (level_str == "INFO") level = log_level::info;
            else if (level_str == "WARNING") level = log_level::warning;
            else if (level_str == "ERROR") level = log_level::error;
            else if (level_str == "CRITICAL") level = log_level::critical;
        }
        
        // Get message
        std::string message = entry.parsed_fields.count("message") ? 
                             entry.parsed_fields.at("message") : entry.raw_data;
        
        // Analyze
        analyzer->analyze(level, message, "", 0, "", entry.received_time);
        
        // Aggregate by source
        aggregator->add_log(entry.source_address, level, message, entry.raw_data.size());
    });
    
    if (!server->start()) {
        std::cerr << "Failed to start log server" << std::endl;
        return 1;
    }
    
    std::cout << "\n1. Testing Network Logging:" << std::endl;
    std::cout << "Starting log server on port " << server_port << std::endl;
    
    // Give server time to start
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Start multiple client threads
    std::vector<std::thread> clients;
    for (int i = 1; i <= 3; ++i) {
        clients.emplace_back(simulate_client, i, server_host, server_port);
    }
    
    // Monitor server statistics
    std::thread monitor_thread([&server, &analyzer]() {
        for (int i = 0; i < 30; ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            
            auto server_stats = server->get_stats();
            std::cout << "\n--- Server Stats ---" << std::endl;
            std::cout << "Total logs received: " << server_stats.total_logs_received << std::endl;
            std::cout << "Active connections: " << server_stats.active_connections << std::endl;
            
            // Show current analysis window
            auto current_stats = analyzer->get_current_stats();
            std::cout << "Current window messages/sec: " 
                     << current_stats.messages_per_second << std::endl;
        }
    });
    
    // Wait for clients to finish
    for (auto& client : clients) {
        client.join();
    }
    
    std::cout << "\n2. Generating Analysis Report:" << std::endl;
    
    // Wait a bit for final logs
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Generate report
    std::string report = analyzer->generate_report(std::chrono::seconds(60));
    std::cout << "\n" << report << std::endl;
    
    // Show aggregated statistics
    std::cout << "\n3. Source Statistics:" << std::endl;
    auto all_sources = aggregator->get_all_stats();
    for (const auto& [source_id, stats] : all_sources) {
        std::cout << "\nSource: " << source_id << std::endl;
        std::cout << "  Total messages: " << stats.total_messages << std::endl;
        std::cout << "  Average rate: " << stats.average_message_rate << " msg/sec" << std::endl;
        std::cout << "  Level distribution:" << std::endl;
        for (const auto& [level, count] : stats.level_counts) {
            std::cout << "    " << static_cast<int>(level) << ": " << count << std::endl;
        }
    }
    
    // Stop monitoring
    monitor_thread.join();
    
    // Stop server
    server->stop();
    
    std::cout << "\n=== Demo Complete ===" << std::endl;
    
    return 0;
}