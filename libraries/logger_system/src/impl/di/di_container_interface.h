#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

/**
 * @file di_container_interface.h
 * @brief Abstract interface for dependency injection container
 * 
 * This file provides an abstract interface for DI containers that can be
 * implemented with different backends (lightweight, thread_system, etc.)
 * without creating external dependencies.
 */

#include <memory>
#include <functional>
#include <string>
#include <kcenon/logger/core/error_codes.h>

namespace kcenon::logger {

/**
 * @brief Abstract interface for dependency injection container
 * @tparam T The type of objects this container manages
 * 
 * This interface defines the contract for DI containers, allowing
 * different implementations without coupling to specific DI frameworks.
 */
template<typename T>
class di_container_interface {
public:
    /**
     * @brief Virtual destructor
     */
    virtual ~di_container_interface() = default;
    
    /**
     * @brief Resolve a component by name
     * @param name The registered name of the component
     * @return Result containing the resolved component or error
     */
    virtual result<std::shared_ptr<T>> resolve(const std::string& name) = 0;
    
    /**
     * @brief Register a factory function for creating components
     * @param name The name to register the factory under
     * @param factory Function that creates instances of T
     * @return Result indicating success or error
     */
    virtual result_void register_factory(
        const std::string& name,
        std::function<std::shared_ptr<T>()> factory) = 0;
    
    /**
     * @brief Register a singleton instance
     * @param name The name to register the instance under
     * @param instance The singleton instance to register
     * @return Result indicating success or error
     */
    virtual result_void register_singleton(
        const std::string& name,
        std::shared_ptr<T> instance) = 0;
    
    /**
     * @brief Check if a component is registered
     * @param name The name to check
     * @return true if registered, false otherwise
     */
    virtual bool is_registered(const std::string& name) const = 0;
    
    /**
     * @brief Clear all registrations
     * @return Result indicating success or error
     */
    virtual result_void clear() = 0;
    
    /**
     * @brief Get the number of registered components
     * @return Number of registered components
     */
    virtual size_t size() const = 0;
};

} // namespace kcenon::logger