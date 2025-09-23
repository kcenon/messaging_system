/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <kcenon/logger/core/logger.h>
#include <kcenon/logger/writers/console_writer.h>
#include <kcenon/logger/writers/file_writer.h>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <random>
#include <atomic>

using namespace kcenon::logger;

// Simulate a distributed system with multiple components
class DistributedComponent {
public:
    DistributedComponent(const std::string& name, int id)
        : name_(name), id_(id), message_count_(0) {
        logger_ = std::make_unique<logger>(true, 2048);
        std::string filename = "logs/component_" + name_ + "_" + std::to_string(id_) + ".log";
        logger_->add_writer(std::make_unique<file_writer>(filename));
        logger_->add_writer(std::make_unique<console_writer>());
        logger_->start();
    }

    ~DistributedComponent() {
        if (logger_) {
            logger_->flush();
            logger_->stop();
        }
    }

    void simulate_work() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> level_dist(0, 5);
        std::uniform_int_distribution<> sleep_dist(10, 100);

        for (int i = 0; i < 20; ++i) {
            auto level = static_cast<kcenon::thread::log_level>(level_dist(gen));
            std::string message = "[" + name_ + "-" + std::to_string(id_) + "] ";

            switch(level) {
                case kcenon::thread::log_level::critical:
                    message += "Critical system failure detected";
                    break;
                case kcenon::thread::log_level::error:
                    message += "Error processing request #" + std::to_string(message_count_++);
                    break;
                case kcenon::thread::log_level::warning:
                    message += "Warning: Resource usage high";
                    break;
                case kcenon::thread::log_level::info:
                    message += "Processing request successfully";
                    break;
                case kcenon::thread::log_level::debug:
                    message += "Debug: Internal state updated";
                    break;
                default:
                    message += "Trace: Detailed execution info";
                    break;
            }

            logger_->log(level, message);
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_dist(gen)));
        }
    }

    int get_message_count() const { return message_count_; }

private:
    std::string name_;
    int id_;
    std::atomic<int> message_count_;
    std::unique_ptr<logger> logger_;
};

int main() {
    std::cout << "=== Distributed Logging Demo ===" << std::endl;
    std::cout << "Simulating a distributed system with multiple components..." << std::endl;

    // Create multiple components simulating different services
    std::vector<std::unique_ptr<DistributedComponent>> components;
    components.push_back(std::make_unique<DistributedComponent>("WebServer", 1));
    components.push_back(std::make_unique<DistributedComponent>("Database", 1));
    components.push_back(std::make_unique<DistributedComponent>("Cache", 1));
    components.push_back(std::make_unique<DistributedComponent>("MessageQueue", 1));

    // Run components in parallel threads
    std::vector<std::thread> threads;
    for (auto& component : components) {
        threads.emplace_back([&component]() {
            component->simulate_work();
        });
    }

    // Create a central logger for aggregated messages
    auto central_logger = std::make_unique<logger>(true, 4096);
    central_logger->add_writer(std::make_unique<file_writer>("logs/central.log"));
    central_logger->add_writer(std::make_unique<console_writer>());
    central_logger->start();

    // Simulate central monitoring
    std::cout << "\n=== Central Monitoring System ===" << std::endl;
    for (int i = 0; i < 10; ++i) {
        central_logger->log(kcenon::thread::log_level::info,
            "Central: System health check #" + std::to_string(i));
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // Wait for all component threads to complete
    for (auto& t : threads) {
        t.join();
    }

    // Log summary
    std::cout << "\n=== Summary ===" << std::endl;
    int total_messages = 0;
    for (const auto& component : components) {
        total_messages += component->get_message_count();
    }

    central_logger->log(kcenon::thread::log_level::info,
        "Total messages processed: " + std::to_string(total_messages));

    // Cleanup
    central_logger->flush();
    central_logger->stop();

    std::cout << "\nDistributed logging demo complete." << std::endl;
    std::cout << "Check logs/ directory for individual component logs and central.log" << std::endl;

    return 0;
}