#pragma once

#include <memory>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <mutex>
#include <any>

namespace kcenon::messaging::di {

/**
 * @enum service_lifetime
 * @brief Service lifetime management strategies
 */
enum class service_lifetime {
    singleton,   // Single instance, shared across all requests
    transient,   // New instance on each request
    scoped       // Single instance per scope (not implemented yet)
};

/**
 * @class service_descriptor
 * @brief Describes a service registration
 */
class service_descriptor {
public:
    std::type_index service_type;
    service_lifetime lifetime;
    std::any instance;  // For singleton instances
    std::function<std::any()> factory;  // For transient instances

    service_descriptor(
        std::type_index type,
        service_lifetime lt,
        std::function<std::any()> factory_func
    ) : service_type(type), lifetime(lt), factory(std::move(factory_func)) {}

    service_descriptor(
        std::type_index type,
        std::any singleton_instance
    ) : service_type(type),
        lifetime(service_lifetime::singleton),
        instance(std::move(singleton_instance)) {}
};

/**
 * @class service_registry
 * @brief Advanced service registry with lifetime management
 */
class service_registry {
    std::unordered_map<std::type_index, service_descriptor> descriptors_;
    mutable std::mutex mutex_;

public:
    /**
     * @brief Register a singleton service
     */
    template<typename T>
    void register_singleton(std::shared_ptr<T> instance) {
        std::lock_guard lock(mutex_);
        descriptors_.insert_or_assign(
            typeid(T),
            service_descriptor(typeid(T), std::any(instance))
        );
    }

    /**
     * @brief Register a transient service with factory
     */
    template<typename T>
    void register_transient(std::function<std::shared_ptr<T>()> factory) {
        std::lock_guard lock(mutex_);
        descriptors_.insert_or_assign(
            typeid(T),
            service_descriptor(
                typeid(T),
                service_lifetime::transient,
                [factory]() -> std::any { return factory(); }
            )
        );
    }

    /**
     * @brief Resolve a service
     */
    template<typename T>
    std::shared_ptr<T> resolve() const {
        std::lock_guard lock(mutex_);
        auto it = descriptors_.find(typeid(T));
        if (it == descriptors_.end()) {
            return nullptr;
        }

        const auto& descriptor = it->second;

        if (descriptor.lifetime == service_lifetime::singleton) {
            return std::any_cast<std::shared_ptr<T>>(descriptor.instance);
        } else if (descriptor.lifetime == service_lifetime::transient) {
            return std::any_cast<std::shared_ptr<T>>(descriptor.factory());
        }

        return nullptr;
    }

    /**
     * @brief Check if service is registered
     */
    template<typename T>
    bool has_service() const {
        std::lock_guard lock(mutex_);
        return descriptors_.find(typeid(T)) != descriptors_.end();
    }

    /**
     * @brief Clear all services
     */
    void clear() {
        std::lock_guard lock(mutex_);
        descriptors_.clear();
    }

    /**
     * @brief Get number of registered services
     */
    size_t count() const {
        std::lock_guard lock(mutex_);
        return descriptors_.size();
    }
};

/**
 * @brief Get global service registry
 */
service_registry& get_global_registry();

} // namespace kcenon::messaging::di
