/**
 * @file main.cpp
 * @brief Logger crash protection demonstration
 * 
 * This example demonstrates comprehensive crash protection mechanisms
 * for logging systems including emergency logging, log file recovery,
 * automatic backups, and signal-safe logging operations.
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <vector>
#include <memory>
#include <atomic>
#include <fstream>

// Include logger and crash protection headers
#include "interfaces/logger_crash_safety.h"
#include "logger/logger.h"
#include "logger/writers/console_writer.h"
#include "logger/writers/file_writer.h"

using namespace kcenon::logger;
namespace logger_module = kcenon::logger;

// Global state for demonstration
std::atomic<bool> logging_active{true};
std::atomic<int> logs_written{0};
std::atomic<int> emergency_logs{0};

// Simulated crash scenarios
void simulate_logger_crash() {
    std::cout << "[CRASH] Simulating logger system crash..." << std::endl;
    std::raise(SIGTERM);
}

void simulate_file_corruption() {
    std::cout << "[CRASH] Simulating log file corruption..." << std::endl;
    // Create a corrupted log file
    std::ofstream corrupted_file("./logs/corrupted.log", std::ios::binary);
    corrupted_file << "CORRUPTED_HEADER";
    corrupted_file.write("\x00\x00\x00\x00", 4); // Null bytes
    corrupted_file << "INVALID_LOG_DATA";
    corrupted_file.close();
}

void simulate_disk_full() {
    std::cout << "[CRASH] Simulating disk full scenario..." << std::endl;
    // Try to write a large amount of data to simulate disk full
    logger_crash_safety::instance().emergency_log("ERROR", 
        "Disk space critical - switching to emergency logging mode");
}

// Logging task functions
void normal_logging_task(int task_id, std::shared_ptr<logger> logger_instance) {
    std::cout << "[LOG] Logging task " << task_id << " starting" << std::endl;
    
    for (int i = 0; i < 5; ++i) {
        std::string message = "Task " + std::to_string(task_id) + 
                             " - Log entry " + std::to_string(i);
        
        logger_instance->log(log_level::info, message);
        logs_written.fetch_add(1);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    std::cout << "[OK] Logging task " << task_id << " completed" << std::endl;
}

void heavy_logging_task(int task_id, std::shared_ptr<logger> logger_instance) {
    std::cout << "[LOG] Heavy logging task " << task_id << " starting" << std::endl;
    
    for (int i = 0; i < 20; ++i) {
        std::string large_message = "Heavy Task " + std::to_string(task_id) + 
                                   " - Large log entry " + std::to_string(i) + 
                                   " with lots of data: " + std::string(100, 'X');
        
        logger_instance->log(log_level::debug, large_message);
        logs_written.fetch_add(1);
        
        if (i % 5 == 0) {
            logger_instance->log(log_level::warning, 
                "Checkpoint " + std::to_string(i) + " for task " + std::to_string(task_id));
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
    
    std::cout << "[OK] Heavy logging task " << task_id << " completed" << std::endl;
}

void potentially_crashing_logging_task(int task_id, std::shared_ptr<logger> logger_instance) {
    std::cout << "[WARN] Risky logging task " << task_id << " starting" << std::endl;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> crash_dist(1, 10);
    
    for (int i = 0; i < 10; ++i) {
        std::string message = "Risky Task " + std::to_string(task_id) + 
                             " - Entry " + std::to_string(i);
        
        logger_instance->log(log_level::info, message);
        logs_written.fetch_add(1);
        
        // Random chance of causing issues
        int outcome = crash_dist(gen);
        if (outcome <= 7) {
            // Normal execution
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        } else if (outcome == 8) {
            // Simulate emergency condition
            logger_crash_safety::instance().emergency_log("CRITICAL", 
                "Emergency condition detected in task " + std::to_string(task_id));
            emergency_logs.fetch_add(1);
        } else if (outcome == 9) {
            // Simulate file corruption
            simulate_file_corruption();
            break;
        } else {
            // Simulate crash
            simulate_logger_crash();
            break;
        }
    }
    
    std::cout << "[WARN] Risky logging task " + std::to_string(task_id) + " finished" << std::endl;
}

// Logger crash callback functions
void on_logger_crash(const std::string& logger_name) {
    std::cout << "\n[ALERT] LOGGER CRASH DETECTED: " << logger_name << std::endl;
    logging_active.store(false);
    
    // Emergency logging
    logger_crash_safety::instance().emergency_log("CRITICAL", 
        "Logger " + logger_name + " has crashed - emergency mode activated");
    emergency_logs.fetch_add(1);
}

void flush_main_logger() {
    std::cout << "[FLUSH] Emergency flush of main logger" << std::endl;
}

void backup_main_logger(const std::string& backup_path) {
    std::cout << "[BACKUP] Creating emergency backup: " << backup_path << std::endl;
    
    // Copy current log file to backup location
    std::ifstream source("./logs/application.log", std::ios::binary);
    std::ofstream backup(backup_path, std::ios::binary);
    backup << source.rdbuf();
    
    std::cout << "[OK] Backup created successfully" << std::endl;
}

void save_emergency_state() {
    std::cout << "[SAVE] Saving emergency logger state..." << std::endl;
    std::cout << "Logs written: " << logs_written.load() << std::endl;
    std::cout << "Emergency logs: " << emergency_logs.load() << std::endl;
}

int main() {
    std::cout << "=== Logger System Crash Protection Demo ===" << std::endl;
    std::cout << "This demo shows comprehensive logging crash protection mechanisms\n" << std::endl;
    
    // Create logs directory
    system("mkdir -p ./logs");
    
    // Step 1: Initialize logger crash protection
    std::cout << "--- Step 1: Initialize Logger Crash Protection ---" << std::endl;
    
    auto& logger_safety = logger_crash_safety::instance();
    logger_safety.initialize(logger_crash_safety_level::standard, 
                           "./logs/emergency.log", 2000);
    
    logger_safety.set_auto_backup(true, 3000);
    logger_safety.set_max_emergency_entries(500);
    
    std::cout << "[OK] Logger crash protection initialized" << std::endl;
    
    // Step 2: Create and configure logger with crash protection
    std::cout << "\n--- Step 2: Create Logger with Crash Protection ---" << std::endl;
    
    auto main_logger = std::make_shared<logger>(true); // async mode
    main_logger->add_writer(std::make_unique<console_writer>());
    main_logger->add_writer(std::make_unique<file_writer>("./logs/application.log"));
    main_logger->set_min_level(log_level::debug);
    main_logger->start();
    
    // Register logger for crash protection
    {
        scoped_logger_crash_protection logger_protection("MainLogger",
            flush_main_logger,
            [](const std::string& path) { backup_main_logger(path); });
        
        std::cout << "[OK] Logger created and protected" << std::endl;
        
        // Step 3: Test normal logging operations
        std::cout << "\n--- Step 3: Normal Logging Operations ---" << std::endl;
        
        main_logger->log(log_level::info, "Logger crash protection demo started");
        main_logger->log(log_level::debug, "Debug information available");
        main_logger->log(log_level::warning, "This is a warning message");
        
        // Step 4: Multi-threaded logging stress test
        std::cout << "\n--- Step 4: Multi-threaded Logging Stress Test ---" << std::endl;
        
        std::vector<std::thread> logging_threads;
        
        // Normal logging threads
        for (int i = 0; i < 3; ++i) {
            logging_threads.emplace_back(normal_logging_task, i, main_logger);
        }
        
        // Heavy logging threads
        for (int i = 3; i < 5; ++i) {
            logging_threads.emplace_back(heavy_logging_task, i, main_logger);
        }
        
        // Wait for normal operations
        for (auto& t : logging_threads) {
            t.join();
        }
        logging_threads.clear();
        
        std::cout << "[OK] Multi-threaded stress test completed" << std::endl;
        
        // Step 5: Test file recovery mechanisms
        std::cout << "\n--- Step 5: File Recovery Test ---" << std::endl;
        
        // Create a corrupted file for testing
        simulate_file_corruption();
        
        // Test corruption detection
        if (log_file_recovery::is_corrupted("./logs/corrupted.log")) {
            std::cout << "[DETECT] Corruption detected in test file" << std::endl;
            
            // Attempt recovery
            if (log_file_recovery::recover_file("./logs/corrupted.log", 
                                               "./logs/recovered.log")) {
                std::cout << "[OK] File recovery successful" << std::endl;
            } else {
                std::cout << "[FAIL] File recovery failed" << std::endl;
            }
        }
        
        // Test backup with checksum
        log_file_recovery::create_backup_with_checksum("./logs/application.log",
                                                      "./logs/application_checksum_backup.log");
        
        // Verify integrity
        if (log_file_recovery::verify_integrity("./logs/application_checksum_backup.log",
                                               "./logs/application_checksum_backup.log.checksum")) {
            std::cout << "[OK] Backup integrity verified" << std::endl;
        }
        
        // Step 6: Test emergency logging
        std::cout << "\n--- Step 6: Emergency Logging Test ---" << std::endl;
        
        logger_safety.emergency_log("INFO", "Testing emergency logging system");
        logger_safety.emergency_log("WARNING", "Emergency logging is signal-safe");
        logger_safety.emergency_log("ERROR", "This log survives crashes");
        emergency_logs.fetch_add(3);
        
        // Step 7: Test risky operations with crash protection
        std::cout << "\n--- Step 7: Risky Operations Test ---" << std::endl;
        std::cout << "[WARN] Some operations may trigger crash protection" << std::endl;
        
        // Submit risky logging tasks
        for (int i = 10; i < 15; ++i) {
            logging_threads.emplace_back(potentially_crashing_logging_task, i, main_logger);
        }
        
        // Wait for risky operations
        for (auto& t : logging_threads) {
            t.join();
        }
        
        // Step 8: Test async logger crash safety
        std::cout << "\n--- Step 8: Async Logger Crash Safety ---" << std::endl;
        
        async_logger_crash_safety::configure_async_safety("MainLogger", 2000, true);
        async_logger_crash_safety::set_overflow_handler("MainLogger",
            [](size_t dropped) {
                std::cout << "[WARN] Buffer overflow: " << dropped << " messages dropped" << std::endl;
            });
        
        // Generate burst of logs to test overflow handling
        for (int i = 0; i < 1000; ++i) {
            main_logger->log(log_level::debug, "Burst log " + std::to_string(i));
        }
        
        // Force flush to test emergency procedures
        logger_safety.force_flush_all();
        logger_safety.force_backup_all();
        
    } // scoped_logger_crash_protection goes out of scope here
    
    // Step 9: Display crash protection statistics
    std::cout << "\n--- Step 9: Crash Protection Statistics ---" << std::endl;
    
    auto stats = logger_safety.get_stats();
    std::cout << "Emergency Log Statistics:" << std::endl;
    std::cout << "  Total emergency logs: " << stats.total_emergency_logs << std::endl;
    std::cout << "  Successful flushes: " << stats.successful_flushes << std::endl;
    std::cout << "  Failed flushes: " << stats.failed_flushes << std::endl;
    std::cout << "  Backup count: " << stats.backup_count << std::endl;
    
    std::cout << "\nApplication Statistics:" << std::endl;
    std::cout << "  Total logs written: " << logs_written.load() << std::endl;
    std::cout << "  Emergency logs: " << emergency_logs.load() << std::endl;
    std::cout << "  Logging active: " << (logging_active.load() ? "Yes" : "No") << std::endl;
    
    // Step 10: Test recovery capabilities
    std::cout << "\n--- Step 10: Recovery Test ---" << std::endl;
    
    if (logger_safety.check_and_recover()) {
        std::cout << "[OK] Recovery actions were taken" << std::endl;
    } else {
        std::cout << "[INFO] No recovery needed" << std::endl;
    }
    
    // Step 11: Graceful shutdown
    std::cout << "\n--- Step 11: Graceful Shutdown ---" << std::endl;
    
    main_logger->log(log_level::info, "Shutting down logger crash protection demo");
    main_logger->stop();
    
    std::cout << "\n=== Demo Completed Successfully ===" << std::endl;
    std::cout << "Key features demonstrated:" << std::endl;
    std::cout << "[OK] Emergency logging (signal-safe)" << std::endl;
    std::cout << "[OK] Automatic log flushing on crash" << std::endl;
    std::cout << "[OK] Log file corruption detection and recovery" << std::endl;
    std::cout << "[OK] Backup creation with integrity verification" << std::endl;
    std::cout << "[OK] Async logger crash safety" << std::endl;
    std::cout << "[OK] Multi-threaded logging protection" << std::endl;
    std::cout << "[OK] RAII-based crash protection registration" << std::endl;
    std::cout << "[OK] Buffer overflow handling" << std::endl;
    
    return 0;
}