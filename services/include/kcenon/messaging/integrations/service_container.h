#pragma once

#include "../core/message_bus.h"
#include "../core/config.h"
#include "../services/service_interface.h"
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <functional>
#include <string>
#include <vector>

// Forward declaration
namespace kcenon::messaging::integrations {
    class external_system_manager;
}

namespace kcenon::messaging::integrations {

    // Service factory function type
    template<typename T>
    using service_factory = std::function<std::shared_ptr<T>()>;

    // Service registration information
    struct service_registration {
        std::string name;
        std::type_index type;
        std::function<std::shared_ptr<void>()> factory;
        bool is_singleton = true;
        std::shared_ptr<void> instance = nullptr;

        // Default constructor
        service_registration() : type(std::type_index(typeid(void))) {}

        template<typename T>
        service_registration(const std::string& service_name, service_factory<T> factory_func, bool singleton = true)
            : name(service_name)
            , type(std::type_index(typeid(T)))
            , factory([factory_func]() -> std::shared_ptr<void> { return factory_func(); })
            , is_singleton(singleton) {}
    };

    // Dependency injection container
    class service_container {
    public:
        service_container() = default;
        ~service_container() = default;

        // Non-copyable
        service_container(const service_container&) = delete;
        service_container& operator=(const service_container&) = delete;

        // Register services
        template<typename T>
        void register_service(const std::string& name, service_factory<T> factory, bool singleton = true) {
            std::lock_guard<std::mutex> lock(mutex_);

            service_registration registration(name, factory, singleton);
            registrations_[name] = std::move(registration);
            type_to_name_[std::type_index(typeid(T))] = name;
        }

        template<typename T>
        void register_singleton(const std::string& name, std::shared_ptr<T> instance) {
            std::lock_guard<std::mutex> lock(mutex_);

            service_factory<T> factory = [instance]() -> std::shared_ptr<T> { return instance; };
            service_registration registration(name, factory, true);
            registration.instance = instance;

            registrations_[name] = std::move(registration);
            type_to_name_[std::type_index(typeid(T))] = name;
        }

        // Resolve services
        template<typename T>
        std::shared_ptr<T> resolve(const std::string& name) {
            std::lock_guard<std::mutex> lock(mutex_);

            auto it = registrations_.find(name);
            if (it == registrations_.end()) {
                return nullptr;
            }

            auto& registration = it->second;

            // Return existing instance for singletons
            if (registration.is_singleton && registration.instance) {
                return std::static_pointer_cast<T>(registration.instance);
            }

            // Create new instance
            auto instance = std::static_pointer_cast<T>(registration.factory());

            // Store for singletons
            if (registration.is_singleton) {
                registration.instance = instance;
            }

            return instance;
        }

        template<typename T>
        std::shared_ptr<T> resolve() {
            auto type_index = std::type_index(typeid(T));
            auto it = type_to_name_.find(type_index);
            if (it != type_to_name_.end()) {
                return resolve<T>(it->second);
            }
            return nullptr;
        }

        // Check if service is registered
        bool is_registered(const std::string& name) const {
            std::lock_guard<std::mutex> lock(mutex_);
            return registrations_.find(name) != registrations_.end();
        }

        template<typename T>
        bool is_registered() const {
            auto type_index = std::type_index(typeid(T));
            std::lock_guard<std::mutex> lock(mutex_);
            return type_to_name_.find(type_index) != type_to_name_.end();
        }

        // Get all registered service names
        std::vector<std::string> get_registered_services() const {
            std::lock_guard<std::mutex> lock(mutex_);
            std::vector<std::string> names;
            for (const auto& [name, registration] : registrations_) {
                names.push_back(name);
            }
            return names;
        }

        // Clear all registrations
        void clear() {
            std::lock_guard<std::mutex> lock(mutex_);
            registrations_.clear();
            type_to_name_.clear();
        }

    private:
        mutable std::mutex mutex_;
        std::unordered_map<std::string, service_registration> registrations_;
        std::unordered_map<std::type_index, std::string> type_to_name_;
    };

    // Messaging system orchestrator
    class messaging_system_orchestrator {
    public:
        explicit messaging_system_orchestrator(const config::messaging_config& config);
        ~messaging_system_orchestrator();

        // Initialize the entire messaging system
        bool initialize();
        void shutdown();

        // Service container access
        service_container& get_container() { return container_; }
        const service_container& get_container() const { return container_; }

        // Message bus access
        core::message_bus* get_message_bus() const { return message_bus_.get(); }

        // Configuration access
        const config::messaging_config& get_config() const { return config_; }

        // External systems access
        class external_system_manager& get_external_systems() { return *external_systems_; }

        // System status
        bool is_running() const { return initialized_ && message_bus_ && message_bus_->is_running(); }

        // Register default services
        void register_core_services();
        void register_messaging_services();

        // Service adapter management
        void register_service_adapter(const std::string& name,
                                     std::shared_ptr<services::service_adapter> adapter);

        std::vector<std::string> get_registered_adapters() const;

    private:
        config::messaging_config config_;
        service_container container_;
        std::unique_ptr<core::message_bus> message_bus_;

        // Service adapters
        std::unordered_map<std::string, std::shared_ptr<services::service_adapter>> adapters_;

        // External system manager (forward declaration only, defined in implementation)
        std::unique_ptr<class external_system_manager> external_systems_;

        bool initialized_ = false;
        mutable std::mutex mutex_;

        void setup_external_integrations();
        void initialize_adapters();
        void shutdown_adapters();
    };

} // namespace kcenon::messaging::integrations