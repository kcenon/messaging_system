#pragma once

#include "message_bus.h"
#include "../services/container/container_service.h"
#include "../services/network/network_service.h"
#include <string>
#include <memory>
#include <unordered_map>

namespace kcenon::messaging::config {

    // Global messaging system configuration
    struct messaging_config {
        // Core message bus configuration
        core::message_bus_config message_bus;

        // Service configurations
        services::container::container_config container;
        services::network::network_config network;

        // External system integration flags
        bool enable_thread_system = false;
        bool enable_logger_system = false;
        bool enable_monitoring_system = false;
        bool enable_database_system = false;
        bool enable_network_system = false;

        // System-wide settings
        std::string system_name = "messaging_system";
        std::string version = "2.0.0";
        std::string environment = "development";  // development, staging, production

        // Logging configuration (when external logger is not available)
        struct {
            bool enable = true;
            std::string level = "info";  // debug, info, warn, error
            std::string format = "json";
            std::string output = "console";  // console, file, syslog
        } logging;

        // Monitoring configuration (when external monitoring is not available)
        struct {
            bool enable = true;
            std::chrono::seconds collection_interval{30};
            bool enable_metrics_export = false;
            std::string metrics_endpoint = "/metrics";
        } monitoring;

        // Performance settings
        struct {
            size_t worker_threads = 4;
            size_t max_memory_usage = 512 * 1024 * 1024;  // 512MB
            std::chrono::seconds health_check_interval{60};
        } performance;
    };

    // Configuration builder for easy setup
    class config_builder {
    public:
        config_builder() = default;

        // Message bus configuration
        config_builder& set_worker_threads(size_t threads) {
            config_.message_bus.worker_threads = threads;
            config_.performance.worker_threads = threads;
            return *this;
        }

        config_builder& set_queue_size(size_t size) {
            config_.message_bus.max_queue_size = size;
            return *this;
        }

        config_builder& enable_priority_queue(bool enable = true) {
            config_.message_bus.enable_priority_queue = enable;
            return *this;
        }

        // Service configuration
        config_builder& set_container_max_size(size_t size) {
            config_.container.max_message_size = size;
            return *this;
        }

        config_builder& enable_compression(bool enable = true) {
            config_.container.enable_compression = enable;
            return *this;
        }

        config_builder& set_network_port(uint16_t port) {
            config_.network.listen_port = port;
            return *this;
        }

        config_builder& enable_ssl(bool enable = true) {
            config_.network.enable_ssl = enable;
            return *this;
        }

        // External system integration
        config_builder& enable_external_logger(bool enable = true) {
            config_.enable_logger_system = enable;
            return *this;
        }

        config_builder& enable_external_monitoring(bool enable = true) {
            config_.enable_monitoring_system = enable;
            return *this;
        }

        config_builder& enable_external_thread_system(bool enable = true) {
            config_.enable_thread_system = enable;
            return *this;
        }

        // Environment and system settings
        config_builder& set_environment(const std::string& env) {
            config_.environment = env;

            // Adjust settings based on environment
            if (env == "production") {
                config_.logging.level = "warn";
                config_.monitoring.enable = true;
                config_.performance.health_check_interval = std::chrono::seconds{30};
            } else if (env == "development") {
                config_.logging.level = "debug";
                config_.monitoring.collection_interval = std::chrono::seconds{10};
            }

            return *this;
        }

        config_builder& set_system_name(const std::string& name) {
            config_.system_name = name;
            return *this;
        }

        // Build final configuration
        messaging_config build() {
            validate_config();
            return config_;
        }

        // Load from environment variables or file
        config_builder& load_from_env();
        config_builder& load_from_file(const std::string& filepath);

    private:
        messaging_config config_;

        void validate_config() {
            // Basic validation
            if (config_.performance.worker_threads == 0) {
                config_.performance.worker_threads = 1;
            }

            if (config_.message_bus.max_queue_size == 0) {
                config_.message_bus.max_queue_size = 1000;
            }

            if (config_.container.max_message_size == 0) {
                config_.container.max_message_size = 1024 * 1024;  // 1MB default
            }
        }
    };

    // Configuration validation utilities
    namespace validation {
        bool validate_config(const messaging_config& config);
        std::vector<std::string> get_validation_errors(const messaging_config& config);
    }

    // Default configurations for different environments
    namespace defaults {
        messaging_config development();
        messaging_config testing();
        messaging_config staging();
        messaging_config production();
    }

} // namespace kcenon::messaging::config