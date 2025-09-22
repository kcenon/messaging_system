#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

/**
 * @file thread_system_di_adapter.h
 * @brief Adapter for thread_system's service container
 * 
 * This file provides an adapter that allows thread_system's service
 * container to be used through our DI interface. Only compiled when
 * USE_THREAD_SYSTEM is defined.
 */

#include "di_container_interface.h"

#ifdef USE_THREAD_SYSTEM
#include <interfaces/service_container.h>

namespace kcenon::logger {

/**
 * @brief Adapter to use thread_system's service container
 * @tparam T The type of objects this container manages
 * 
 * This adapter wraps thread_system's service container to provide
 * compatibility with our DI interface, enabling seamless integration
 * when thread_system is available.
 */
template<typename T>
class thread_system_di_adapter : public di_container_interface<T> {
private:
    kcenon::thread::service_container* container_;
    bool owns_container_;
    
public:
    /**
     * @brief Constructor with external container
     * @param container Pointer to existing service container
     */
    explicit thread_system_di_adapter(kcenon::thread::service_container* container)
        : container_(container), owns_container_(false) {
        if (!container_) {
            throw std::invalid_argument("Container cannot be null");
        }
    }
    
    /**
     * @brief Constructor that creates its own container
     */
    thread_system_di_adapter()
        : container_(new kcenon::thread::service_container()),
          owns_container_(true) {}
    
    /**
     * @brief Destructor
     */
    ~thread_system_di_adapter() override {
        if (owns_container_ && container_) {
            delete container_;
        }
    }
    
    /**
     * @brief Resolve a component by name
     * @param name The registered name of the component
     * @return Result containing the resolved component or error
     */
    result<std::shared_ptr<T>> resolve(const std::string& name) override {
        try {
            if (auto instance = container_->resolve<T>(name)) {
                return instance;
            }
            return error_code::component_not_found;
        } catch (const std::exception& e) {
            return error_code::component_not_found;
        }
    }
    
    /**
     * @brief Register a factory function for creating components
     * @param name The name to register the factory under
     * @param factory Function that creates instances of T
     * @return Result indicating success or error
     */
    result_void register_factory(
        const std::string& name,
        std::function<std::shared_ptr<T>()> factory) override {
        
        if (name.empty() || !factory) {
            return error_code::invalid_argument;
        }
        
        try {
            container_->register_factory<T>(name, factory);
            return {};
        } catch (const std::exception& e) {
            return error_code::registration_failed;
        }
    }
    
    /**
     * @brief Register a singleton instance
     * @param name The name to register the instance under
     * @param instance The singleton instance to register
     * @return Result indicating success or error
     */
    result_void register_singleton(
        const std::string& name,
        std::shared_ptr<T> instance) override {
        
        if (name.empty() || !instance) {
            return error_code::invalid_argument;
        }
        
        try {
            container_->register_singleton<T>(name, instance);
            return {};
        } catch (const std::exception& e) {
            return error_code::registration_failed;
        }
    }
    
    /**
     * @brief Check if a component is registered
     * @param name The name to check
     * @return true if registered, false otherwise
     */
    bool is_registered(const std::string& name) const override {
        try {
            return container_->is_registered<T>(name);
        } catch (...) {
            return false;
        }
    }
    
    /**
     * @brief Clear all registrations
     * @return Result indicating success or error
     */
    result_void clear() override {
        try {
            container_->clear<T>();
            return {};
        } catch (const std::exception& e) {
            return error_code::operation_failed;
        }
    }
    
    /**
     * @brief Get the number of registered components
     * @return Number of registered components
     */
    size_t size() const override {
        try {
            return container_->size<T>();
        } catch (...) {
            return 0;
        }
    }
    
    /**
     * @brief Get the underlying thread_system container
     * @return Pointer to the wrapped container
     */
    kcenon::thread::service_container* get_native_container() {
        return container_;
    }
};

} // namespace kcenon::logger

#endif // USE_THREAD_SYSTEM