/**
 * @file event_bus_example.cpp
 * @brief Example demonstrating event-driven monitoring system
 */

#include <monitoring/core/event_bus.h>
#include <monitoring/core/event_types.h>
#include <monitoring/adapters/thread_system_adapter.h>
#include <monitoring/adapters/logger_system_adapter.h>
#include <iostream>
#include <thread>
#include <chrono>

using namespace monitoring_system;
using namespace std::chrono_literals;

int main() {
    std::cout << "=== Event Bus Example ===" << std::endl;

    // Create event bus with configuration
    event_bus::config bus_config;
    bus_config.max_queue_size = 1000;
    bus_config.worker_thread_count = 2;
    bus_config.auto_start = true;

    auto bus = std::make_shared<event_bus>(bus_config);

    // Example 1: Subscribe to performance alerts
    std::cout << "\nExample 1: Performance Alert Monitoring" << std::endl;

    auto alert_token = bus->subscribe_event<performance_alert_event>(
        [](const performance_alert_event& event) {
            std::cout << "  Alert: " << event.get_message()
                     << " (Component: " << event.get_component() << ")" << std::endl;
        },
        event_priority::high
    );

    // Publish a performance alert
    performance_alert_event alert(
        performance_alert_event::alert_type::high_cpu_usage,
        performance_alert_event::alert_severity::warning,
        "main_processor",
        "CPU usage exceeds 80%",
        80.0,  // threshold
        85.5   // actual value
    );

    bus->publish_event(alert);
    std::this_thread::sleep_for(100ms);

    // Example 2: System resource monitoring
    std::cout << "\nExample 2: System Resource Monitoring" << std::endl;

    bus->subscribe_event<system_resource_event>(
        [](const system_resource_event& event) {
            auto stats = event.get_stats();
            std::cout << "  Resource Update:" << std::endl;
            std::cout << "    CPU: " << stats.cpu_usage_percent << "%" << std::endl;
            std::cout << "    Memory: " << stats.memory_used_bytes / (1024*1024) << " MB" << std::endl;
        }
    );

    // Simulate resource event
    system_resource_event::resource_stats resources;
    resources.cpu_usage_percent = 45.2;
    resources.memory_used_bytes = 1024 * 1024 * 512; // 512 MB
    resources.memory_total_bytes = 1024 * 1024 * 1024 * 8; // 8 GB

    bus->publish_event(system_resource_event(resources));
    std::this_thread::sleep_for(100ms);

    // Example 3: Component lifecycle tracking
    std::cout << "\nExample 3: Component Lifecycle Tracking" << std::endl;

    bus->subscribe_event<component_lifecycle_event>(
        [](const component_lifecycle_event& event) {
            std::cout << "  Component '" << event.get_component()
                     << "' changed from "
                     << static_cast<int>(event.get_old_state())
                     << " to "
                     << static_cast<int>(event.get_new_state()) << std::endl;
        }
    );

    // Simulate component lifecycle
    bus->publish_event(component_lifecycle_event(
        "database_connector",
        component_lifecycle_event::lifecycle_state::stopped,
        component_lifecycle_event::lifecycle_state::initializing
    ));

    bus->publish_event(component_lifecycle_event(
        "database_connector",
        component_lifecycle_event::lifecycle_state::initializing,
        component_lifecycle_event::lifecycle_state::running
    ));

    std::this_thread::sleep_for(100ms);

    // Example 4: Thread system adapter (when available)
    std::cout << "\nExample 4: Thread System Adapter" << std::endl;

    thread_system_adapter thread_adapter(bus);

    if (thread_adapter.is_thread_system_available()) {
        std::cout << "  Thread system is available" << std::endl;

        // Start collection
        collection_config config;
        config.interval = 1s;
        thread_adapter.start_collection(config);

        std::this_thread::sleep_for(3s);

        thread_adapter.stop_collection();
    } else {
        std::cout << "  Thread system is not available (expected)" << std::endl;
    }

    // Example 5: Logger system adapter
    std::cout << "\nExample 5: Logger System Adapter" << std::endl;

    logger_system_adapter logger_adapter(bus);

    if (logger_adapter.is_logger_system_available()) {
        std::cout << "  Logger system is available" << std::endl;
    } else {
        std::cout << "  Logger system is not available (expected)" << std::endl;
    }

    // Example 6: Event bus statistics
    std::cout << "\nExample 6: Event Bus Statistics" << std::endl;

    auto stats = bus->get_stats();
    std::cout << "  Total published: " << stats.total_published << std::endl;
    std::cout << "  Total processed: " << stats.total_processed << std::endl;
    std::cout << "  Total dropped: " << stats.total_dropped << std::endl;
    std::cout << "  Queue size: " << stats.current_queue_size << std::endl;
    std::cout << "  Subscribers: " << stats.subscriber_count << std::endl;

    // Example 7: Health check events
    std::cout << "\nExample 7: Health Check Events" << std::endl;

    bus->subscribe_event<health_check_event>(
        [](const health_check_event& event) {
            auto overall = event.get_overall_status();
            std::cout << "  Health check for '" << event.get_component() << "': ";
            switch (overall) {
                case health_check_event::health_status::healthy:
                    std::cout << "HEALTHY" << std::endl;
                    break;
                case health_check_event::health_status::degraded:
                    std::cout << "DEGRADED" << std::endl;
                    break;
                case health_check_event::health_status::unhealthy:
                    std::cout << "UNHEALTHY" << std::endl;
                    break;
                default:
                    std::cout << "UNKNOWN" << std::endl;
            }
        }
    );

    // Create health check results
    std::vector<health_check_event::health_check_result> health_results;

    health_check_event::health_check_result db_check;
    db_check.check_name = "database_connection";
    db_check.status = health_check_event::health_status::healthy;
    db_check.message = "Connection OK";
    db_check.response_time = 15ms;
    health_results.push_back(db_check);

    health_check_event::health_check_result api_check;
    api_check.check_name = "api_endpoint";
    api_check.status = health_check_event::health_status::degraded;
    api_check.message = "Slow response";
    api_check.response_time = 500ms;
    health_results.push_back(api_check);

    bus->publish_event(health_check_event("backend_service", health_results));
    std::this_thread::sleep_for(100ms);

    // Cleanup
    std::cout << "\nStopping event bus..." << std::endl;
    bus->stop();

    std::cout << "✅ Event bus example completed successfully!" << std::endl;

    return 0;
}