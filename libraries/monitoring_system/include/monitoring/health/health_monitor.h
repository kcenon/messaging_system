#pragma once

// Health monitoring interface - basic implementation for compatibility
#include <string>
#include <unordered_map>
#include <chrono>
#include <functional>
#include <memory>

namespace monitoring_system {

/**
 * @brief Basic health status enumeration
 */
enum class health_status {
    healthy,
    degraded,
    unhealthy,
    unknown
};

/**
 * @brief Health check result
 */
struct health_check_result {
    health_status status{health_status::unknown};
    std::string message;
    std::chrono::steady_clock::time_point timestamp;
    std::unordered_map<std::string, std::string> details;

    static health_check_result healthy(const std::string& msg = "OK") {
        health_check_result result;
        result.status = health_status::healthy;
        result.message = msg;
        result.timestamp = std::chrono::steady_clock::now();
        return result;
    }

    static health_check_result unhealthy(const std::string& msg = "FAILED") {
        health_check_result result;
        result.status = health_status::unhealthy;
        result.message = msg;
        result.timestamp = std::chrono::steady_clock::now();
        return result;
    }

    static health_check_result degraded(const std::string& msg = "DEGRADED") {
        health_check_result result;
        result.status = health_status::degraded;
        result.message = msg;
        result.timestamp = std::chrono::steady_clock::now();
        return result;
    }
};

/**
 * @brief Health monitor configuration
 */
struct health_monitor_config {
    std::chrono::seconds check_interval{std::chrono::seconds(5)};
    std::chrono::seconds cache_duration{std::chrono::seconds(1)};
    bool enable_auto_recovery{true};
    size_t max_consecutive_failures{3};
    std::chrono::seconds recovery_timeout{std::chrono::seconds(30)};
};

/**
 * @brief Health check types
 */
enum class health_check_type {
    liveness,
    readiness,
    startup
};

/**
 * @brief Functional health check implementation
 */
class functional_health_check {
public:
    functional_health_check(const std::string& name,
                          health_check_type type,
                          std::function<health_check_result()> check_func,
                          std::chrono::milliseconds timeout = std::chrono::milliseconds(1000),
                          bool critical = false)
        : name_(name), type_(type), check_func_(check_func), timeout_(timeout), critical_(critical) {}

    health_check_result execute() {
        if (check_func_) {
            return check_func_();
        }
        return health_check_result{};
    }

    const std::string& get_name() const { return name_; }
    health_check_type get_type() const { return type_; }
    std::chrono::milliseconds get_timeout() const { return timeout_; }
    bool is_critical() const { return critical_; }

private:
    std::string name_;
    health_check_type type_;
    std::function<health_check_result()> check_func_;
    std::chrono::milliseconds timeout_;
    bool critical_;
};

/**
 * @brief Basic health monitor interface - stub implementation
 */
class health_monitor {
public:
    health_monitor() = default;
    explicit health_monitor(const health_monitor_config& config) : config_(config) {}
    virtual ~health_monitor() = default;

    virtual health_check_result check_health() const {
        health_check_result result;
        result.status = health_status::healthy;
        result.message = "Health monitor stub - always healthy";
        result.timestamp = std::chrono::steady_clock::now();
        return result;
    }

    virtual void start() {}
    virtual void stop() {}
    virtual bool is_running() const { return true; }

    // Additional methods for health monitoring example
    virtual void register_check(const std::string& name, std::shared_ptr<functional_health_check> check) {
        checks_[name] = check;
    }

    virtual std::unordered_map<std::string, health_check_result> check_all() {
        std::unordered_map<std::string, health_check_result> results;
        for (const auto& [name, check] : checks_) {
            results[name] = check->execute();
        }
        return results;
    }

    virtual health_status get_overall_status() {
        auto results = check_all();
        bool has_unhealthy = false;
        bool has_degraded = false;

        for (const auto& [name, result] : results) {
            if (result.status == health_status::unhealthy) {
                has_unhealthy = true;
            } else if (result.status == health_status::degraded) {
                has_degraded = true;
            }
        }

        if (has_unhealthy) return health_status::unhealthy;
        if (has_degraded) return health_status::degraded;
        return health_status::healthy;
    }

    virtual void register_recovery_handler(const std::string& check_name, std::function<bool()> handler) {
        recovery_handlers_[check_name] = handler;
    }

    virtual void refresh() {
        // Trigger recovery handlers for failed checks
        auto results = check_all();
        for (const auto& [name, result] : results) {
            if (result.status == health_status::unhealthy) {
                auto it = recovery_handlers_.find(name);
                if (it != recovery_handlers_.end()) {
                    it->second(); // Execute recovery handler
                }
            }
        }
    }

    virtual std::string get_health_report() {
        std::string report = "Health Report:\n";
        auto results = check_all();
        for (const auto& [name, result] : results) {
            report += "  " + name + ": ";
            switch (result.status) {
                case health_status::healthy: report += "HEALTHY"; break;
                case health_status::degraded: report += "DEGRADED"; break;
                case health_status::unhealthy: report += "UNHEALTHY"; break;
                default: report += "UNKNOWN"; break;
            }
            report += " - " + result.message + "\n";
        }
        return report;
    }

private:
    health_monitor_config config_;
    std::unordered_map<std::string, std::shared_ptr<functional_health_check>> checks_;
    std::unordered_map<std::string, std::function<bool()>> recovery_handlers_;
};

} // namespace monitoring_system