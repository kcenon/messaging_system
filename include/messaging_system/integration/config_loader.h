#pragma once

#include <yaml-cpp/yaml.h>
#include <kcenon/common/patterns/result.h>
#include <kcenon/database/connection_pool_config.h>
#include <string>
#include <vector>
#include <chrono>
#include <functional>

namespace messaging {

struct NetworkConfig {
    uint16_t port{8080};
    size_t max_connections{10000};
    std::chrono::milliseconds timeout{5000};
    int retry_attempts{3};
};

struct ThreadPoolConfig {
    size_t io_workers{2};
    size_t work_workers{16};
    size_t queue_size{100000};
    bool use_lockfree{false};
};

struct DatabaseConfig {
    std::string type{"postgresql"};
    std::string connection_string;
    database::connection_pool_config pool_config;
};

struct LoggingConfig {
    std::string level{"info"};
    bool async{true};
    std::vector<std::string> writers;
};

struct MonitoringConfig {
    bool enabled{true};
    std::chrono::milliseconds interval{1000};
};

struct MessagingSystemConfig {
    std::string version;
    NetworkConfig network;
    ThreadPoolConfig thread_pools;
    DatabaseConfig database;
    LoggingConfig logging;
    MonitoringConfig monitoring;

    static common::Result<MessagingSystemConfig> load_from_file(const std::string& path);
    common::Result<void> validate() const;
};

// File watcher for hot-reload
class ConfigWatcher {
public:
    using Callback = std::function<void(const MessagingSystemConfig&)>;

    common::Result<void> watch(const std::string& path, Callback callback);
    void stop();

private:
    bool running_{false};
};

} // namespace messaging
