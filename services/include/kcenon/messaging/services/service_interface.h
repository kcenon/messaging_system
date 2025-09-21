#pragma once

#include "../core/message_types.h"
#include <string>
#include <memory>

// Forward declarations
namespace kcenon::messaging::core {
    class message_bus;
}

namespace kcenon::messaging::services {

    // Service lifecycle states
    enum class service_state {
        uninitialized,
        initializing,
        running,
        stopping,
        stopped,
        error
    };

    // Base interface for all messaging services
    class service_interface {
    public:
        virtual ~service_interface() = default;

        // Lifecycle management
        virtual bool initialize() = 0;
        virtual void shutdown() = 0;
        virtual service_state get_state() const = 0;

        // Service identification
        virtual std::string get_service_name() const = 0;
        virtual std::string get_service_version() const = 0;

        // Message handling
        virtual void handle_message(const core::message& msg) = 0;
        virtual bool can_handle_topic(const std::string& topic) const = 0;

        // Health check
        virtual bool is_healthy() const = 0;
    };

    // Service adapter base class for message bus integration
    class service_adapter {
    public:
        explicit service_adapter(std::shared_ptr<service_interface> service)
            : service_(std::move(service)) {}

        virtual ~service_adapter() = default;

        // Register with message bus
        virtual void register_with_bus(core::message_bus* bus) = 0;

        // Service delegation
        bool initialize() { return service_ ? service_->initialize() : false; }
        void shutdown() { if (service_) service_->shutdown(); }
        service_state get_state() const { return service_ ? service_->get_state() : service_state::error; }
        std::string get_service_name() const { return service_ ? service_->get_service_name() : "unknown"; }
        bool is_healthy() const { return service_ ? service_->is_healthy() : false; }

    protected:
        std::shared_ptr<service_interface> service_;
    };

} // namespace kcenon::messaging::services