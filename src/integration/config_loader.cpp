#include "messaging_system/integration/config_loader.h"
#include "messaging_system/error_codes.h"
#include <kcenon/common/patterns/result.h>
#include <fstream>
#include <thread>

namespace messaging {

common::Result<MessagingSystemConfig> MessagingSystemConfig::load_from_file(const std::string& path) {
    try {
        YAML::Node config = YAML::LoadFile(path);

        if (!config["messaging_system"]) {
            return common::error<MessagingSystemConfig>(
                common::error_info{
                    error::INVALID_MESSAGE,
                    "Missing 'messaging_system' root node in config",
                    "MessagingSystemConfig::load_from_file",
                    path
                }
            );
        }

        auto root = config["messaging_system"];
        MessagingSystemConfig result;

        // Version
        if (root["version"]) {
            result.version = root["version"].as<std::string>();
        }

        // Network configuration
        if (root["network"]) {
            auto net = root["network"];
            if (net["port"]) result.network.port = net["port"].as<uint16_t>();
            if (net["max_connections"]) result.network.max_connections = net["max_connections"].as<size_t>();
            if (net["timeout_ms"]) {
                result.network.timeout = std::chrono::milliseconds(net["timeout_ms"].as<int>());
            }
            if (net["retry_attempts"]) result.network.retry_attempts = net["retry_attempts"].as<int>();
        }

        // Thread pools configuration
        if (root["thread_pools"]) {
            auto pools = root["thread_pools"];
            if (pools["io"]) {
                auto io = pools["io"];
                if (io["workers"]) result.thread_pools.io_workers = io["workers"].as<size_t>();
                if (io["queue_size"]) result.thread_pools.queue_size = io["queue_size"].as<size_t>();
            }
            if (pools["work"]) {
                auto work = pools["work"];
                if (work["workers"]) result.thread_pools.work_workers = work["workers"].as<size_t>();
                if (work["queue_size"]) result.thread_pools.queue_size = work["queue_size"].as<size_t>();
                if (work["lockfree"]) result.thread_pools.use_lockfree = work["lockfree"].as<bool>();
            }
        }

        // Database configuration
        if (root["database"]) {
            auto db = root["database"];
            if (db["type"]) result.database.type = db["type"].as<std::string>();
            if (db["connection_string"]) {
                result.database.connection_string = db["connection_string"].as<std::string>();
            }
            if (db["pool"]) {
                auto pool = db["pool"];
                if (pool["min_connections"]) {
                    result.database.pool_config.min_connections = pool["min_connections"].as<size_t>();
                }
                if (pool["max_connections"]) {
                    result.database.pool_config.max_connections = pool["max_connections"].as<size_t>();
                }
                if (pool["idle_timeout_s"]) {
                    result.database.pool_config.idle_timeout =
                        std::chrono::seconds(pool["idle_timeout_s"].as<int>());
                }
            }
        }

        // Logging configuration
        if (root["logging"]) {
            auto log = root["logging"];
            if (log["level"]) result.logging.level = log["level"].as<std::string>();
            if (log["async"]) result.logging.async = log["async"].as<bool>();
            if (log["writers"]) {
                for (const auto& writer : log["writers"]) {
                    result.logging.writers.push_back(writer.as<std::string>());
                }
            }
        }

        // Monitoring configuration
        if (root["monitoring"]) {
            auto mon = root["monitoring"];
            if (mon["enabled"]) result.monitoring.enabled = mon["enabled"].as<bool>();
            if (mon["interval_ms"]) {
                result.monitoring.interval = std::chrono::milliseconds(mon["interval_ms"].as<int>());
            }
        }

        return common::Result<MessagingSystemConfig>::ok(std::move(result));

    } catch (const YAML::Exception& e) {
        return common::error<MessagingSystemConfig>(
            common::error_info{
                error::SERIALIZATION_ERROR,
                std::string("YAML parse error: ") + e.what(),
                "MessagingSystemConfig::load_from_file",
                path
            }
        );
    } catch (const std::exception& e) {
        return common::error<MessagingSystemConfig>(
            common::error_info{
                error::SERIALIZATION_ERROR,
                std::string("Failed to load config: ") + e.what(),
                "MessagingSystemConfig::load_from_file",
                path
            }
        );
    }
}

VoidResult MessagingSystemConfig::validate() const {
    // Validate port range
    if (network.port == 0) {
        return common::error<std::monostate>(
            common::error_info{
                error::INVALID_MESSAGE,
                "Invalid network port: 0",
                "MessagingSystemConfig::validate",
                ""
            }
        );
    }

    // Validate thread pool sizes
    if (thread_pools.io_workers == 0 || thread_pools.work_workers == 0) {
        return common::error<std::monostate>(
            common::error_info{
                error::INVALID_MESSAGE,
                "Thread pool workers must be > 0",
                "MessagingSystemConfig::validate",
                ""
            }
        );
    }

    // Validate database config if type is set
    if (!database.type.empty() && database.connection_string.empty()) {
        return common::error<std::monostate>(
            common::error_info{
                error::INVALID_MESSAGE,
                "Database connection string required when type is set",
                "MessagingSystemConfig::validate",
                ""
            }
        );
    }

    return common::ok(std::monostate{});
}

// ConfigWatcher implementation
VoidResult ConfigWatcher::watch(const std::string& path, Callback callback) {
    if (running_) {
        return common::error<std::monostate>(
            common::error_info{
                error::INVALID_MESSAGE,
                "ConfigWatcher already running",
                "ConfigWatcher::watch",
                ""
            }
        );
    }

    running_ = true;

    // Simple polling implementation
    // In production, use inotify (Linux) or FSEvents (macOS) for efficiency
    std::thread([this, path, callback]() {
        auto last_write_time = std::filesystem::last_write_time(path);

        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            try {
                auto current_write_time = std::filesystem::last_write_time(path);
                if (current_write_time != last_write_time) {
                    last_write_time = current_write_time;

                    // File changed, reload config
                    auto config_result = MessagingSystemConfig::load_from_file(path);
                    if (config_result.is_ok()) {
                        callback(config_result.value());
                    }
                }
            } catch (...) {
                // File might be temporarily unavailable during write
                continue;
            }
        }
    }).detach();

    return common::ok(std::monostate{});
}

void ConfigWatcher::stop() {
    running_ = false;
}

} // namespace messaging
