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

// ============================================================================
// Executor Helper Functions
// ============================================================================

/**
 * @brief Register an executor with the global DI container
 * @param executor The executor to register
 *
 * This is a convenience function for registering an IExecutor.
 * The executor can then be resolved and passed to components like
 * task_client, async_result, and websocket_transport.
 *
 * Example:
 * @code
 * auto pool = std::make_shared<kcenon::thread::thread_pool>();
 * pool->start();
 * register_executor(pool);
 *
 * // Later, in component creation:
 * auto executor = resolve_executor();
 * task_client client(queue, backend, executor);
 * @endcode
 */
inline void register_executor(std::shared_ptr<common::interfaces::IExecutor> executor) {
    get_global_container().register_service<common::interfaces::IExecutor>(std::move(executor));
}

/**
 * @brief Resolve the registered executor from the global DI container
 * @return The registered executor, or nullptr if not registered
 */
inline std::shared_ptr<common::interfaces::IExecutor> resolve_executor() {
    return get_global_container().resolve<common::interfaces::IExecutor>();
}

/**
 * @brief Check if an executor is registered
 * @return true if an executor is registered
 */
inline bool has_executor() {
    return get_global_container().has_service<common::interfaces::IExecutor>();
}

} // namespace kcenon::messaging::di
