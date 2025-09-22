/**
 * @file log_router.h
 * @brief Log routing functionality
 */

#pragma once

#include <kcenon/logger/interfaces/log_filter_interface.h>
#include <kcenon/logger/interfaces/log_entry.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <regex>

namespace kcenon::logger::routing {

/**
 * @brief Route configuration for log messages
 */
struct route_config {
    std::vector<std::string> writer_names;
    std::unique_ptr<log_filter_interface> filter;
    bool stop_propagation = false;

    route_config() = default;
    route_config(route_config&&) = default;
    route_config& operator=(route_config&&) = default;
};

/**
 * @brief Log router for directing messages to specific writers
 */
class log_router {
private:
    std::vector<route_config> routes_;
    bool exclusive_routes_ = false;

public:
    /**
     * @brief Add a routing rule
     */
    void add_route(route_config config) {
        routes_.push_back(std::move(config));
    }

    /**
     * @brief Set exclusive routing mode
     */
    void set_exclusive_routes(bool exclusive) {
        exclusive_routes_ = exclusive;
    }

    /**
     * @brief Clear all routes
     */
    void clear_routes() {
        routes_.clear();
    }

    /**
     * @brief Get writer names for a log entry
     */
    std::vector<std::string> get_writers_for_log(
        logger_system::log_level level,
        const std::string& message,
        const std::string& file,
        int line,
        const std::string& function) const {

        std::vector<std::string> writers;

        for (const auto& route : routes_) {
            if (route.filter && route.filter->should_log(level, message, file, line, function)) {
                writers.insert(writers.end(), route.writer_names.begin(), route.writer_names.end());
                if (route.stop_propagation) {
                    break;
                }
            }
        }

        return writers;
    }

    /**
     * @brief Check if exclusive routing is enabled
     */
    bool is_exclusive_routing() const {
        return exclusive_routes_;
    }
};

/**
 * @brief Builder for creating routing rules
 */
class router_builder {
private:
    log_router& router_;
    route_config config_;

public:
    explicit router_builder(log_router& router) : router_(router) {}

    router_builder& when_level(logger_system::log_level level) {
        config_.filter = std::make_unique<class level_condition>(level);
        return *this;
    }

    router_builder& when_matches(const std::string& pattern) {
        config_.filter = std::make_unique<class regex_condition>(pattern);
        return *this;
    }

    router_builder& route_to(const std::string& writer_name, bool stop_propagation = false) {
        config_.writer_names.push_back(writer_name);
        config_.stop_propagation = stop_propagation;
        router_.add_route(std::move(config_));
        return *this;
    }

    router_builder& route_to(const std::vector<std::string>& writer_names, bool stop_propagation = false) {
        config_.writer_names = writer_names;
        config_.stop_propagation = stop_propagation;
        router_.add_route(std::move(config_));
        return *this;
    }

private:
    class level_condition : public log_filter_interface {
    private:
        logger_system::log_level target_level_;
    public:
        explicit level_condition(logger_system::log_level level) : target_level_(level) {}
        bool should_log(logger_system::log_level level, const std::string&, const std::string&, int, const std::string&) const override {
            return level == target_level_;
        }
    };

    class regex_condition : public log_filter_interface {
    private:
        std::regex pattern_;
    public:
        explicit regex_condition(const std::string& pattern) : pattern_(pattern) {}
        bool should_log(logger_system::log_level, const std::string& message, const std::string&, int, const std::string&) const override {
            return std::regex_search(message, pattern_);
        }
    };
};

} // namespace kcenon::logger::routing