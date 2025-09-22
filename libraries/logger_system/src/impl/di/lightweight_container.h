#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

/**
 * @file lightweight_container.h
 * @brief Lightweight dependency injection container for logger system
 * 
 * This file provides a simple DI container implementation for managing
 * logger dependencies when the full DI system is not available.
 */

#include <memory>
#include <unordered_map>
#include <typeindex>
#include <functional>
#include <stdexcept>

namespace kcenon::logger {

/**
 * @class lightweight_container
 * @brief Simple dependency injection container
 * 
 * This class provides basic dependency injection functionality
 * for the logger system when operating in lightweight/standalone mode.
 */
class lightweight_container {
public:
    /**
     * @brief Register a singleton instance
     * @tparam T Type to register
     * @param instance Shared pointer to the instance
     */
    template<typename T>
    void register_singleton(std::shared_ptr<T> instance) {
        services_[std::type_index(typeid(T))] = instance;
    }
    
    /**
     * @brief Register a factory function
     * @tparam T Type to register
     * @param factory Factory function that creates instances
     */
    template<typename T>
    void register_factory(std::function<std::shared_ptr<T>()> factory) {
        factories_[std::type_index(typeid(T))] = [factory]() -> std::shared_ptr<void> {
            return factory();
        };
    }
    
    /**
     * @brief Resolve a registered type
     * @tparam T Type to resolve
     * @return Shared pointer to the resolved instance
     * @throws std::runtime_error if type is not registered
     */
    template<typename T>
    std::shared_ptr<T> resolve() {
        auto type_idx = std::type_index(typeid(T));
        
        // Check singletons first
        auto it = services_.find(type_idx);
        if (it != services_.end()) {
            return std::static_pointer_cast<T>(it->second);
        }
        
        // Check factories
        auto factory_it = factories_.find(type_idx);
        if (factory_it != factories_.end()) {
            return std::static_pointer_cast<T>(factory_it->second());
        }
        
        throw std::runtime_error("Type not registered in container");
    }
    
    /**
     * @brief Check if a type is registered
     * @tparam T Type to check
     * @return true if registered, false otherwise
     */
    template<typename T>
    bool is_registered() const {
        auto type_idx = std::type_index(typeid(T));
        return services_.find(type_idx) != services_.end() ||
               factories_.find(type_idx) != factories_.end();
    }
    
    /**
     * @brief Clear all registrations
     */
    void clear() {
        services_.clear();
        factories_.clear();
    }
    
private:
    std::unordered_map<std::type_index, std::shared_ptr<void>> services_;
    std::unordered_map<std::type_index, std::function<std::shared_ptr<void>()>> factories_;
};

/**
 * @brief Get the global lightweight container instance
 * @return Reference to the global container
 */
inline lightweight_container& get_container() {
    static lightweight_container container;
    return container;
}

} // namespace kcenon::logger