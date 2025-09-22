#pragma once

#include "../service_interface.h"
#include "../../core/message_types.h"
#include <memory>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <iostream>

#ifdef HAS_LOGGER_SYSTEM
#include <logger/logger_system.h>
#endif

namespace kcenon::messaging::services::logging {

    // Log level enumeration
    enum class log_level {
        debug = 0,
        info = 1,
        warning = 2,
        error = 3,
        critical = 4
    };

    // Convert log level to string
    inline std::string log_level_to_string(log_level level) {
        switch (level) {
            case log_level::debug: return "DEBUG";
            case log_level::info: return "INFO";
            case log_level::warning: return "WARNING";
            case log_level::error: return "ERROR";
            case log_level::critical: return "CRITICAL";
            default: return "UNKNOWN";
        }
    }

    // Logging service interface
    class logging_service : public service_interface {
    public:
        virtual ~logging_service() = default;

        // Service lifecycle
        bool initialize() override { return true; }
        void shutdown() override {}
        bool is_running() const override { return true; }

        // Logging specific methods
        virtual void log(log_level level, const std::string& message, const std::string& component = "") = 0;
        virtual void log_message_event(const std::string& event_type, const core::message& msg) = 0;
        virtual void log_system_event(const std::string& event_type, const std::string& details) = 0;
        virtual void set_log_level(log_level min_level) = 0;
        virtual log_level get_log_level() const = 0;
    };

    // Internal logging service implementation
    class internal_logging_service : public logging_service {
    private:
        log_level min_level_;
        mutable std::mutex log_mutex_;

        std::string format_timestamp() const {
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) % 1000;

            std::stringstream ss;
            ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
            ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
            return ss.str();
        }

        bool should_log(log_level level) const {
            return static_cast<int>(level) >= static_cast<int>(min_level_);
        }

    public:
        explicit internal_logging_service(log_level min_level = log_level::info)
            : min_level_(min_level) {}

        void log(log_level level, const std::string& message, const std::string& component = "") override {
            if (!should_log(level)) return;

            std::lock_guard<std::mutex> lock(log_mutex_);
            std::cout << "[" << format_timestamp() << "] "
                      << "[" << log_level_to_string(level) << "] ";

            if (!component.empty()) {
                std::cout << "[" << component << "] ";
            }

            std::cout << message << std::endl;
        }

        void log_message_event(const std::string& event_type, const core::message& msg) override {
            std::stringstream ss;
            ss << "Message " << event_type
               << ": topic=" << msg.payload.topic
               << ", sender=" << msg.metadata.sender
               << ", priority=" << static_cast<int>(msg.metadata.priority)
               << ", data_size=" << msg.payload.data.size();

            log(log_level::info, ss.str(), "MessageBus");
        }

        void log_system_event(const std::string& event_type, const std::string& details) override {
            std::stringstream ss;
            ss << "System " << event_type << ": " << details;
            log(log_level::info, ss.str(), "System");
        }

        void set_log_level(log_level min_level) override {
            min_level_ = min_level;
        }

        log_level get_log_level() const override {
            return min_level_;
        }
    };

#ifdef HAS_LOGGER_SYSTEM
    // External logging service implementation
    class external_logging_service : public logging_service {
    private:
        std::unique_ptr<logger::logger_system> external_logger_;
        log_level min_level_;

        logger::log_level convert_log_level(log_level level) const {
            switch (level) {
                case log_level::debug: return logger::log_level::debug;
                case log_level::info: return logger::log_level::info;
                case log_level::warning: return logger::log_level::warning;
                case log_level::error: return logger::log_level::error;
                case log_level::critical: return logger::log_level::critical;
                default: return logger::log_level::info;
            }
        }

    public:
        explicit external_logging_service(log_level min_level = log_level::info)
            : min_level_(min_level) {
            external_logger_ = std::make_unique<logger::logger_system>();
        }

        bool initialize() override {
            if (external_logger_) {
                return external_logger_->initialize();
            }
            return false;
        }

        void shutdown() override {
            if (external_logger_) {
                external_logger_->shutdown();
            }
        }

        bool is_running() const override {
            return external_logger_ && external_logger_->is_running();
        }

        void log(log_level level, const std::string& message, const std::string& component = "") override {
            if (external_logger_) {
                auto logger_level = convert_log_level(level);
                if (component.empty()) {
                    external_logger_->log(logger_level, message);
                } else {
                    external_logger_->log(logger_level, message, {{"component", component}});
                }
            }
        }

        void log_message_event(const std::string& event_type, const core::message& msg) override {
            if (external_logger_) {
                auto logger_level = convert_log_level(log_level::info);
                std::unordered_map<std::string, std::string> context = {
                    {"event_type", event_type},
                    {"topic", msg.payload.topic},
                    {"sender", msg.metadata.sender},
                    {"priority", std::to_string(static_cast<int>(msg.metadata.priority))},
                    {"message_id", msg.metadata.id}
                };

                external_logger_->log(logger_level, "Message event occurred", context);
            }
        }

        void log_system_event(const std::string& event_type, const std::string& details) override {
            if (external_logger_) {
                auto logger_level = convert_log_level(log_level::info);
                std::unordered_map<std::string, std::string> context = {
                    {"event_type", event_type},
                    {"details", details}
                };

                external_logger_->log(logger_level, "System event occurred", context);
            }
        }

        void set_log_level(log_level min_level) override {
            min_level_ = min_level;
            if (external_logger_) {
                auto logger_level = convert_log_level(min_level);
                external_logger_->set_log_level(logger_level);
            }
        }

        log_level get_log_level() const override {
            return min_level_;
        }
    };
#endif

    // Logging service adapter
    class logging_service_adapter : public service_adapter {
    private:
        std::shared_ptr<logging_service> logging_service_;

    public:
        explicit logging_service_adapter(std::shared_ptr<logging_service> service)
            : service_adapter(service), logging_service_(std::move(service)) {}

        void register_with_bus(core::message_bus* bus) {
            // Subscribe to logging topics
            if (bus && logging_service_) {
                bus->subscribe("logging.*", [this](const core::message& msg) {
                    this->handle_logging_message(msg);
                });
            }
        }

    private:
        void handle_logging_message(const core::message& msg) {
            if (!logging_service_) return;

            // Process logging messages based on their topic
            if (msg.payload.topic == "logging.log_message") {
                auto level_str = msg.payload.get<std::string>("level", "info");
                auto message = msg.payload.get<std::string>("message", "");
                auto component = msg.payload.get<std::string>("component", "");

                log_level level = log_level::info;
                if (level_str == "debug") level = log_level::debug;
                else if (level_str == "warning") level = log_level::warning;
                else if (level_str == "error") level = log_level::error;
                else if (level_str == "critical") level = log_level::critical;

                logging_service_->log(level, message, component);
            }
            else if (msg.payload.topic == "logging.message_event") {
                auto event_type = msg.payload.get<std::string>("event_type", "unknown");
                // Log the message event (the msg itself represents the original message)
                logging_service_->log_message_event(event_type, msg);
            }
            else if (msg.payload.topic == "logging.system_event") {
                auto event_type = msg.payload.get<std::string>("event_type", "unknown");
                auto details = msg.payload.get<std::string>("details", "");
                logging_service_->log_system_event(event_type, details);
            }
            else if (msg.payload.topic == "logging.set_level") {
                auto level_str = msg.payload.get<std::string>("level", "info");
                log_level level = log_level::info;
                if (level_str == "debug") level = log_level::debug;
                else if (level_str == "warning") level = log_level::warning;
                else if (level_str == "error") level = log_level::error;
                else if (level_str == "critical") level = log_level::critical;

                logging_service_->set_log_level(level);
                logging_service_->log_system_event("config_change",
                    "Log level changed to " + level_str);
            }
        }

    public:
        std::shared_ptr<logging_service> get_logging_service() const {
            return logging_service_;
        }
    };

    // Factory function for creating logging service
    inline std::shared_ptr<logging_service> create_logging_service(bool use_external = false,
                                                                  log_level min_level = log_level::info) {
#ifdef HAS_LOGGER_SYSTEM
        if (use_external) {
            return std::make_shared<external_logging_service>(min_level);
        }
#endif
        return std::make_shared<internal_logging_service>(min_level);
    }

} // namespace kcenon::messaging::services::logging