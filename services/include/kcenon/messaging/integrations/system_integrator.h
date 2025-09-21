#pragma once

#include "service_container.h"
#include "../core/config.h"

namespace kcenon::messaging::integrations {

    // Main entry point for the messaging system
    class system_integrator {
    public:
        explicit system_integrator(const config::messaging_config& config = {});
        ~system_integrator();

        // System lifecycle
        bool initialize();
        void shutdown();
        bool is_running() const;

        // Access to core components
        messaging_system_orchestrator& get_orchestrator() { return orchestrator_; }
        const messaging_system_orchestrator& get_orchestrator() const { return orchestrator_; }

        core::message_bus* get_message_bus() const {
            return orchestrator_.get_message_bus();
        }

        service_container& get_container() {
            return orchestrator_.get_container();
        }

        // Convenience methods for common operations
        template<typename T>
        std::shared_ptr<T> get_service(const std::string& name) {
            return orchestrator_.get_container().resolve<T>(name);
        }

        template<typename T>
        std::shared_ptr<T> get_service() {
            return orchestrator_.get_container().resolve<T>();
        }

        bool publish(const std::string& topic, const core::message_payload& payload,
                    const std::string& sender = "");

        void subscribe(const std::string& topic, core::message_handler handler);

        // Configuration management
        const config::messaging_config& get_config() const {
            return orchestrator_.get_config();
        }

        // System health and monitoring
        struct system_health {
            bool message_bus_healthy = false;
            bool all_services_healthy = false;
            size_t active_services = 0;
            size_t total_messages_processed = 0;
            std::chrono::system_clock::time_point last_check;
        };

        system_health check_system_health() const;

        // Create default system instance
        static std::unique_ptr<system_integrator> create_default();
        static std::unique_ptr<system_integrator> create_for_environment(const std::string& environment);

    private:
        messaging_system_orchestrator orchestrator_;
        bool initialized_ = false;
    };

} // namespace kcenon::messaging::integrations