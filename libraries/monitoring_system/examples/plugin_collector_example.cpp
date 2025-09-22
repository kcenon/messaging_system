#include <chrono>
#include <iostream>
#include <thread>

#include <kcenon/monitoring/collectors/logger_system_collector.h>
#include <kcenon/monitoring/collectors/plugin_metric_collector.h>
#include <kcenon/monitoring/collectors/system_resource_collector.h>
#include <kcenon/monitoring/collectors/thread_system_collector.h>

using namespace monitoring_system;

int main() {
    std::cout << "=== Plugin-based Metric Collector Example ===" << std::endl;

    // Create plugin collector with configuration
    plugin_collector_config config;
    config.collection_interval = std::chrono::milliseconds(1000);
    config.enable_caching = true;
    config.enable_streaming = false;
    config.worker_threads = 2;

    auto collector = std::make_unique<plugin_metric_collector>(config);

    // Create and register system resource collector
    auto sys_collector = std::make_unique<system_resource_collector>();
    if (sys_collector->initialize({})) {
        std::cout << "System resource collector initialized" << std::endl;
        collector->register_plugin(std::move(sys_collector));
    }

    // Create and register thread system collector
    auto thread_collector = std::make_unique<thread_system_collector>();
    if (thread_collector->initialize({})) {
        std::cout << "Thread system collector initialized" << std::endl;
        collector->register_plugin(std::move(thread_collector));
    }

    // Create and register logger system collector
    auto logger_collector = std::make_unique<logger_system_collector>();
    if (logger_collector->initialize({})) {
        std::cout << "Logger system collector initialized" << std::endl;
        collector->register_plugin(std::move(logger_collector));
    }

    // List registered plugins
    std::cout << "\nRegistered plugins:" << std::endl;
    for (const auto& plugin_name : collector->get_registered_plugins()) {
        std::cout << "  - " << plugin_name << std::endl;
    }

    // Start collection
    if (collector->start()) {
        std::cout << "\nCollection started successfully" << std::endl;
    } else {
        std::cerr << "Failed to start collection" << std::endl;
        return 1;
    }

    // Run for a few seconds and collect metrics
    std::cout << "\nCollecting metrics for 5 seconds..." << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Force immediate collection
        auto metrics = collector->force_collect();
        std::cout << "Collected " << metrics.size() << " metrics" << std::endl;

        // Display some metrics
        for (const auto& metric : metrics) {
            if (i == 0) {  // Only show first iteration to avoid clutter
                std::cout << "  " << metric.name << ": " << metric.value << " " << metric.unit << std::endl;
            }
        }
    }

    // Get cached metrics
    auto cached = collector->get_cached_metrics();
    std::cout << "\nTotal cached metrics: " << cached.size() << std::endl;

    // Stop collection
    collector->stop();
    std::cout << "Collection stopped" << std::endl;

    return 0;
}