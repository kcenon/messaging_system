#pragma once

#include <string>
#include <string_view>
#include <future>
#include <functional>
#include <memory>
#include <any>
#include <chrono>

namespace kcenon::shared {

/**
 * @brief Log level enumeration
 */
enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Critical
};

/**
 * @brief Common logging interface
 */
class ILogger {
public:
    virtual ~ILogger() = default;

    /**
     * @brief Log a message with specified level
     * @param level Log level
     * @param message Message to log
     */
    virtual void log(LogLevel level, std::string_view message) = 0;

    /**
     * @brief Log a formatted message
     * @param level Log level
     * @param format Format string
     * @param args Arguments for formatting
     */
    template<typename... Args>
    void log_formatted(LogLevel level, std::string_view format, Args&&... args) {
        // Default implementation - derived classes can override
        log(level, format); // Simplified for now
    }
};

/**
 * @brief Metrics snapshot structure
 */
struct MetricsSnapshot {
    std::chrono::steady_clock::time_point timestamp;
    std::size_t active_threads{0};
    std::size_t pending_tasks{0};
    double cpu_usage{0.0};
    std::size_t memory_usage_mb{0};
    std::size_t logs_per_second{0};
    double average_task_duration_ms{0.0};
};

/**
 * @brief Common monitoring interface
 */
class IMonitorable {
public:
    virtual ~IMonitorable() = default;

    /**
     * @brief Get current metrics snapshot
     * @return Current metrics
     */
    virtual MetricsSnapshot get_metrics() const = 0;

    /**
     * @brief Enable or disable metrics collection
     * @param enabled True to enable, false to disable
     */
    virtual void set_metrics_enabled(bool enabled) = 0;
};

/**
 * @brief Common task executor interface
 */
class IExecutor {
public:
    virtual ~IExecutor() = default;

    /**
     * @brief Execute a task asynchronously
     * @param task Task to execute
     * @return Future for task completion
     */
    virtual std::future<void> execute(std::function<void()> task) = 0;

    /**
     * @brief Execute a task with result
     * @tparam T Result type
     * @param task Task to execute
     * @return Future with result
     */
    template<typename T>
    std::future<T> execute_with_result(std::function<T()> task) {
        // Default implementation using std::async
        return std::async(std::launch::async, std::move(task));
    }

    /**
     * @brief Get executor capacity
     * @return Maximum number of concurrent tasks
     */
    virtual std::size_t capacity() const = 0;

    /**
     * @brief Get current load
     * @return Number of active tasks
     */
    virtual std::size_t active_tasks() const = 0;
};

/**
 * @brief Service lifecycle interface
 */
class IService {
public:
    virtual ~IService() = default;

    /**
     * @brief Initialize the service
     * @return True if successful
     */
    virtual bool initialize() = 0;

    /**
     * @brief Shutdown the service
     */
    virtual void shutdown() = 0;

    /**
     * @brief Check if service is running
     * @return True if running
     */
    virtual bool is_running() const = 0;

    /**
     * @brief Get service name
     * @return Service name
     */
    virtual std::string name() const = 0;
};

/**
 * @brief Configuration interface
 */
class IConfigurable {
public:
    virtual ~IConfigurable() = default;

    /**
     * @brief Apply configuration
     * @param config Configuration object (any type)
     */
    virtual void configure(const std::any& config) = 0;

    /**
     * @brief Get current configuration
     * @return Current configuration
     */
    virtual std::any get_configuration() const = 0;

    /**
     * @brief Validate configuration
     * @param config Configuration to validate
     * @return True if valid
     */
    virtual bool validate_configuration(const std::any& config) const = 0;
};

} // namespace kcenon::shared