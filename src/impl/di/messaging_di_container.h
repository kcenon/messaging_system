#pragma once

#include <kcenon/common/interfaces/executor_interface.h>
#include <kcenon/common/interfaces/logger_interface.h>
#include <kcenon/common/interfaces/monitoring_interface.h>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <any>
#include <mutex>

namespace kcenon::messaging::di {

/**
 * @class messaging_di_container
 * @brief Dependency injection container for messaging system
 *
 * Pattern borrowed from logger_system for consistent DI
 */
class messaging_di_container {
    std::unordered_map<std::type_index, std::any> services_;
    mutable std::mutex mutex_;

public:
    /**
     * @brief Register a service
     */
    template<typename T>
    void register_service(std::shared_ptr<T> service) {
        std::lock_guard lock(mutex_);
        services_[typeid(T)] = service;
    }

    /**
     * @brief Resolve a service
     */
    template<typename T>
    std::shared_ptr<T> resolve() const {
        std::lock_guard lock(mutex_);
        auto it = services_.find(typeid(T));
        if (it != services_.end()) {
            return std::any_cast<std::shared_ptr<T>>(it->second);
        }
        return nullptr;
    }

    /**
     * @brief Check if service is registered
     */
    template<typename T>
    bool has_service() const {
        std::lock_guard lock(mutex_);
        return services_.find(typeid(T)) != services_.end();
    }

    /**
     * @brief Clear all services
     */
    void clear() {
        std::lock_guard lock(mutex_);
        services_.clear();
    }
};

/**
 * @brief Get global DI container
 */
messaging_di_container& get_global_container();

} // namespace kcenon::messaging::di
