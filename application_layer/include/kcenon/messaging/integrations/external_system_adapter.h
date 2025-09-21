#pragma once

#include "../core/message_types.h"
#include "../services/service_interface.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

namespace kcenon::messaging::integrations {

    // External system status
    enum class external_system_status {
        disconnected,
        connecting,
        connected,
        error,
        maintenance
    };

    // External system connection info
    struct external_connection_info {
        std::string system_name;
        std::string endpoint;
        std::string version;
        external_system_status status = external_system_status::disconnected;
        std::chrono::system_clock::time_point last_connected;
        std::chrono::system_clock::time_point last_heartbeat;
        size_t reconnect_attempts = 0;
    };

    // External system adapter interface
    class external_system_adapter {
    public:
        virtual ~external_system_adapter() = default;

        // Connection management
        virtual bool connect() = 0;
        virtual void disconnect() = 0;
        virtual bool is_connected() const = 0;
        virtual bool ping() = 0;

        // Message forwarding
        virtual bool forward_message(const core::message& msg) = 0;
        virtual void set_message_handler(std::function<void(const core::message&)> handler) = 0;

        // System information
        virtual std::string get_system_name() const = 0;
        virtual std::string get_system_version() const = 0;
        virtual external_connection_info get_connection_info() const = 0;

        // Health monitoring
        virtual bool is_healthy() const = 0;
        virtual std::unordered_map<std::string, std::string> get_health_metrics() const = 0;
    };

    // Database system adapter
    class database_adapter : public external_system_adapter {
    private:
        external_connection_info connection_info_;
        std::function<void(const core::message&)> message_handler_;
        bool connected_ = false;

    public:
        explicit database_adapter(const std::string& connection_string)
            : connection_info_{.system_name = "database", .endpoint = connection_string} {}

        bool connect() override {
            // Simulate database connection
            connected_ = true;
            connection_info_.status = external_system_status::connected;
            connection_info_.last_connected = std::chrono::system_clock::now();
            return true;
        }

        void disconnect() override {
            connected_ = false;
            connection_info_.status = external_system_status::disconnected;
        }

        bool is_connected() const override {
            return connected_;
        }

        bool ping() override {
            if (!connected_) return false;
            connection_info_.last_heartbeat = std::chrono::system_clock::now();
            return true;
        }

        bool forward_message(const core::message& msg) override {
            if (!connected_) return false;

            // Handle database-specific message forwarding
            if (msg.payload.topic.starts_with("db.")) {
                // Forward to database system
                return true;
            }
            return false;
        }

        void set_message_handler(std::function<void(const core::message&)> handler) override {
            message_handler_ = std::move(handler);
        }

        std::string get_system_name() const override {
            return "DatabaseSystem";
        }

        std::string get_system_version() const override {
            return "1.0.0";
        }

        external_connection_info get_connection_info() const override {
            return connection_info_;
        }

        bool is_healthy() const override {
            return connected_ && connection_info_.status == external_system_status::connected;
        }

        std::unordered_map<std::string, std::string> get_health_metrics() const override {
            return {
                {"connection_status", connected_ ? "connected" : "disconnected"},
                {"last_ping", std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now() - connection_info_.last_heartbeat).count())},
                {"reconnect_attempts", std::to_string(connection_info_.reconnect_attempts)}
            };
        }
    };

    // Thread system adapter
    class thread_system_adapter : public external_system_adapter {
    private:
        external_connection_info connection_info_;
        std::function<void(const core::message&)> message_handler_;
        bool connected_ = false;

    public:
        thread_system_adapter()
            : connection_info_{.system_name = "thread_system", .endpoint = "internal"} {}

        bool connect() override {
#ifdef HAS_THREAD_SYSTEM_CORE
            connected_ = true;
            connection_info_.status = external_system_status::connected;
            connection_info_.last_connected = std::chrono::system_clock::now();
#endif
            return connected_;
        }

        void disconnect() override {
            connected_ = false;
            connection_info_.status = external_system_status::disconnected;
        }

        bool is_connected() const override {
            return connected_;
        }

        bool ping() override {
            if (!connected_) return false;
            connection_info_.last_heartbeat = std::chrono::system_clock::now();
            return true;
        }

        bool forward_message(const core::message& msg) override {
            if (!connected_) return false;

            // Handle thread system message forwarding
            if (msg.payload.topic.starts_with("thread.")) {
                // Forward to thread system
                return true;
            }
            return false;
        }

        void set_message_handler(std::function<void(const core::message&)> handler) override {
            message_handler_ = std::move(handler);
        }

        std::string get_system_name() const override {
            return "ThreadSystem";
        }

        std::string get_system_version() const override {
            return "1.0.0";
        }

        external_connection_info get_connection_info() const override {
            return connection_info_;
        }

        bool is_healthy() const override {
            return connected_ && connection_info_.status == external_system_status::connected;
        }

        std::unordered_map<std::string, std::string> get_health_metrics() const override {
            return {
                {"connection_status", connected_ ? "connected" : "disconnected"},
                {"system_available", "true"},
                {"thread_pool_status", "healthy"}
            };
        }
    };

    // External system manager
    class external_system_manager {
    private:
        std::unordered_map<std::string, std::unique_ptr<external_system_adapter>> adapters_;
        mutable std::mutex adapters_mutex_;

    public:
        external_system_manager() = default;
        ~external_system_manager() = default;

        // Adapter management
        void register_adapter(const std::string& name, std::unique_ptr<external_system_adapter> adapter) {
            std::lock_guard<std::mutex> lock(adapters_mutex_);
            adapters_[name] = std::move(adapter);
        }

        external_system_adapter* get_adapter(const std::string& name) const {
            std::lock_guard<std::mutex> lock(adapters_mutex_);
            auto it = adapters_.find(name);
            return it != adapters_.end() ? it->second.get() : nullptr;
        }

        std::vector<std::string> get_adapter_names() const {
            std::lock_guard<std::mutex> lock(adapters_mutex_);
            std::vector<std::string> names;
            for (const auto& [name, adapter] : adapters_) {
                names.push_back(name);
            }
            return names;
        }

        // Connection management
        bool connect_all() {
            std::lock_guard<std::mutex> lock(adapters_mutex_);
            bool all_connected = true;
            for (const auto& [name, adapter] : adapters_) {
                if (!adapter->connect()) {
                    all_connected = false;
                }
            }
            return all_connected;
        }

        void disconnect_all() {
            std::lock_guard<std::mutex> lock(adapters_mutex_);
            for (const auto& [name, adapter] : adapters_) {
                adapter->disconnect();
            }
        }

        // Health monitoring
        std::unordered_map<std::string, bool> get_health_status() const {
            std::lock_guard<std::mutex> lock(adapters_mutex_);
            std::unordered_map<std::string, bool> status;
            for (const auto& [name, adapter] : adapters_) {
                status[name] = adapter->is_healthy();
            }
            return status;
        }

        std::unordered_map<std::string, external_connection_info> get_connection_info() const {
            std::lock_guard<std::mutex> lock(adapters_mutex_);
            std::unordered_map<std::string, external_connection_info> info;
            for (const auto& [name, adapter] : adapters_) {
                info[name] = adapter->get_connection_info();
            }
            return info;
        }

        // Message forwarding
        bool forward_message(const std::string& adapter_name, const core::message& msg) {
            auto adapter = get_adapter(adapter_name);
            return adapter ? adapter->forward_message(msg) : false;
        }

        // Broadcast message to all connected adapters
        size_t broadcast_message(const core::message& msg) {
            std::lock_guard<std::mutex> lock(adapters_mutex_);
            size_t sent_count = 0;
            for (const auto& [name, adapter] : adapters_) {
                if (adapter->is_connected() && adapter->forward_message(msg)) {
                    sent_count++;
                }
            }
            return sent_count;
        }
    };

    // Factory functions for creating adapters
    inline std::unique_ptr<external_system_adapter> create_database_adapter(const std::string& connection_string) {
        return std::make_unique<database_adapter>(connection_string);
    }

    inline std::unique_ptr<external_system_adapter> create_thread_system_adapter() {
        return std::make_unique<thread_system_adapter>();
    }

} // namespace kcenon::messaging::integrations