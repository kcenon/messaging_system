/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include <logger/logger.h>
#include <logger/writers/console_writer.h>
#include <thread>
#include <vector>
#include <iostream>
#include <chrono>

using namespace logger_module;

void basic_logging_example() {
    std::cout << "\n=== Basic Logging Example ===\n" << std::endl;
    
    // Create logger instance
    auto logger_instance = std::make_shared<logger>();
    
    // Add console writer
    logger_instance->add_writer(std::make_unique<console_writer>());
    
    // Start logger (for async mode)
    logger_instance->start();
    
    // Log messages at different levels
    logger_instance->log(thread_module::log_level::trace, "This is a trace message");
    logger_instance->log(thread_module::log_level::debug, "Debug information here");
    logger_instance->log(thread_module::log_level::info, "Application started successfully");
    logger_instance->log(thread_module::log_level::warning, "This is a warning");
    logger_instance->log(thread_module::log_level::error, "An error occurred!");
    logger_instance->log(thread_module::log_level::critical, "Critical system failure!");
    
    // Log with source location
    logger_instance->log(thread_module::log_level::info, "Message with location", 
                __FILE__, __LINE__, __func__);
    
    // Stop and flush
    logger_instance->stop();
}

void multithreaded_logging_example() {
    std::cout << "\n=== Multithreaded Logging Example ===\n" << std::endl;
    
    // Create async logger
    auto logger_instance = std::make_shared<logger>(true, 16384); // Larger buffer for MT
    logger_instance->add_writer(std::make_unique<console_writer>());
    logger_instance->start();
    
    // Launch multiple threads
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([logger_instance, i]() {
            for (int j = 0; j < 10; ++j) {
                logger_instance->log(thread_module::log_level::info,
                           "Thread " + std::to_string(i) + " - Message " + std::to_string(j));
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }
    
    // Wait for all threads
    for (auto& t : threads) {
        t.join();
    }
    
    logger_instance->stop();
}

void log_level_filtering_example() {
    std::cout << "\n=== Log Level Filtering Example ===\n" << std::endl;
    
    auto logger_instance = std::make_shared<logger>();
    logger_instance->add_writer(std::make_unique<console_writer>());
    logger_instance->start();
    
    // Set minimum level to INFO
    logger_instance->set_min_level(thread_module::log_level::info);
    std::cout << "Minimum level set to INFO\n" << std::endl;
    
    // These won't be logged
    logger_instance->log(thread_module::log_level::trace, "This trace won't show");
    logger_instance->log(thread_module::log_level::debug, "This debug won't show");
    
    // These will be logged
    logger_instance->log(thread_module::log_level::info, "This info will show");
    logger_instance->log(thread_module::log_level::warning, "This warning will show");
    
    logger_instance->stop();
}

void sync_vs_async_example() {
    std::cout << "\n=== Sync vs Async Logging Example ===\n" << std::endl;
    
    // Synchronous logger
    std::cout << "Synchronous logging:" << std::endl;
    auto sync_logger = std::make_shared<logger>(false); // sync mode
    sync_logger->add_writer(std::make_unique<console_writer>());
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; ++i) {
        sync_logger->log(thread_module::log_level::info, "Sync log " + std::to_string(i));
    }
    auto sync_time = std::chrono::high_resolution_clock::now() - start;
    
    // Asynchronous logger
    std::cout << "\nAsynchronous logging:" << std::endl;
    auto async_logger = std::make_shared<logger>(true); // async mode
    async_logger->add_writer(std::make_unique<console_writer>());
    async_logger->start();
    
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; ++i) {
        async_logger->log(thread_module::log_level::info, "Async log " + std::to_string(i));
    }
    auto async_time = std::chrono::high_resolution_clock::now() - start;
    
    async_logger->stop();
    
    std::cout << "\nSync time: " 
              << std::chrono::duration_cast<std::chrono::microseconds>(sync_time).count() 
              << " µs" << std::endl;
    std::cout << "Async time: " 
              << std::chrono::duration_cast<std::chrono::microseconds>(async_time).count() 
              << " µs" << std::endl;
}

int main() {
    try {
        basic_logging_example();
        multithreaded_logging_example();
        log_level_filtering_example();
        sync_vs_async_example();
        
        std::cout << "\n=== All examples completed successfully! ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}