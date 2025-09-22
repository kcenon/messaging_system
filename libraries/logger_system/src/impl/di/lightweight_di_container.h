#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

/**
 * @file lightweight_di_container.h
 * @brief Lightweight dependency injection container implementation
 * 
 * This file provides a simple, lightweight DI container that requires
 * no external dependencies and provides basic factory and singleton
 * registration capabilities.
 */

#include "di_container_interface.h"
#include <unordered_map>
#include <mutex>

namespace kcenon::logger {

/**
 * @brief Lightweight implementation of DI container
 * @tparam T The type of objects this container manages
 * 
 * This container provides thread-safe registration and resolution of
 * components using factories and singletons, with no external dependencies.
 */
template<typename T>
class lightweight_di_container : public di_container_interface<T> {
private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::function<std::shared_ptr<T>()>> factories_;
    std::unordered_map<std::string, std::shared_ptr<T>> singletons_;
    
public:
    /**
     * @brief Default constructor
     */
    lightweight_di_container() = default;
    
    /**
     * @brief Destructor
     */
    ~lightweight_di_container() override = default;
    
    /**
     * @brief Resolve a component by name
     * 
     * First checks singletons, then tries factories if not found.
     * 
     * @param name The registered name of the component
     * @return Result containing the resolved component or error
     */
    result<std::shared_ptr<T>> resolve(const std::string& name) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Check singletons first
        if (auto it = singletons_.find(name); it != singletons_.end()) {
            return it->second;
        }
        
        // Try factory
        if (auto it = factories_.find(name); it != factories_.end()) {
            try {
                if (auto instance = it->second()) {
                    return instance;
                }
                return error_code::creation_failed;
            } catch (const std::exception& e) {
                return error_code::creation_failed;
            }
        }
        
        return error_code::component_not_found;
    }
    
    /**
     * @brief Register a factory function for creating components
     * 
     * @param name The name to register the factory under
     * @param factory Function that creates instances of T
     * @return Result indicating success or error
     */
    result_void register_factory(
        const std::string& name,
        std::function<std::shared_ptr<T>()> factory) override {
        
        if (name.empty()) {
            return make_logger_error(error_code::invalid_argument, 
                            "Factory name cannot be empty");
        }
        
        if (!factory) {
            return make_logger_error(error_code::invalid_argument,
                            "Factory function cannot be null");
        }
        
        std::lock_guard<std::mutex> lock(mutex_);
        factories_[name] = factory;
        return {};
    }
    
    /**
     * @brief Register a singleton instance
     * 
     * @param name The name to register the instance under
     * @param instance The singleton instance to register
     * @return Result indicating success or error
     */
    result_void register_singleton(
        const std::string& name,
        std::shared_ptr<T> instance) override {
        
        if (name.empty()) {
            return make_logger_error(error_code::invalid_argument, 
                            "Factory name cannot be empty");
        }
        
        if (!instance) {
            return error_code::invalid_argument;
        }
        
        std::lock_guard<std::mutex> lock(mutex_);
        singletons_[name] = instance;
        return {};
    }
    
    /**
     * @brief Check if a component is registered
     * 
     * @param name The name to check
     * @return true if registered, false otherwise
     */
    bool is_registered(const std::string& name) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return singletons_.count(name) > 0 || factories_.count(name) > 0;
    }
    
    /**
     * @brief Clear all registrations
     * 
     * @return Result indicating success or error
     */
    result_void clear() override {
        std::lock_guard<std::mutex> lock(mutex_);
        factories_.clear();
        singletons_.clear();
        return {};
    }
    
    /**
     * @brief Get the number of registered components
     * 
     * @return Total number of registered factories and singletons
     */
    size_t size() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return factories_.size() + singletons_.size();
    }
    
    /**
     * @brief Register a component type with default constructor
     * 
     * Convenience method for registering types that have default constructors.
     * 
     * @tparam ComponentType The concrete type to register
     * @param name The name to register under
     * @return Result indicating success or error
     */
    template<typename ComponentType>
    result_void register_type(const std::string& name) {
        static_assert(std::is_base_of_v<T, ComponentType>,
                      "ComponentType must derive from T");
        
        return register_factory(name, []() {
            return std::make_shared<ComponentType>();
        });
    }
    
    /**
     * @brief Register a component type with constructor arguments
     * 
     * @tparam ComponentType The concrete type to register
     * @tparam Args Constructor argument types
     * @param name The name to register under
     * @param args Constructor arguments
     * @return Result indicating success or error
     */
    template<typename ComponentType, typename... Args>
    result_void register_type_with_args(const std::string& name, Args... args) {
        static_assert(std::is_base_of_v<T, ComponentType>,
                      "ComponentType must derive from T");
        
        return register_factory(name, [args...]() {
            return std::make_shared<ComponentType>(args...);
        });
    }
};

} // namespace kcenon::logger