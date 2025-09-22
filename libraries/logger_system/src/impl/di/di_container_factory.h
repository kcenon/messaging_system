#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

/**
 * @file di_container_factory.h
 * @brief Factory for creating appropriate DI containers
 * 
 * This factory creates the appropriate DI container based on available
 * dependencies and configuration, providing automatic fallback to
 * lightweight implementation when needed.
 */

#include "di_container_interface.h"
#include "lightweight_di_container.h"
#include "thread_system_di_adapter.h"

namespace kcenon::logger {

/**
 * @brief Factory for creating DI containers
 * 
 * This factory provides methods to create the appropriate DI container
 * based on configuration and available dependencies.
 */
class di_container_factory {
public:
    /**
     * @brief Container types available
     */
    enum class container_type {
        lightweight,    ///< Built-in lightweight container
        thread_system,  ///< thread_system container (if available)
        automatic      ///< Automatically choose best available
    };
    
    /**
     * @brief Create a DI container of specified type
     * @tparam T The type of objects the container will manage
     * @param type The type of container to create
     * @return Unique pointer to the created container
     */
    template<typename T>
    static std::unique_ptr<di_container_interface<T>> create_container(
        container_type type = container_type::automatic) {
        
        switch (type) {
            case container_type::lightweight:
                return std::make_unique<lightweight_di_container<T>>();
                
#ifdef USE_THREAD_SYSTEM
            case container_type::thread_system:
                try {
                    return std::make_unique<thread_system_di_adapter<T>>();
                } catch (...) {
                    // Fallback to lightweight if thread_system fails
                    return std::make_unique<lightweight_di_container<T>>();
                }
#endif
                
            case container_type::automatic:
                return create_best_available<T>();
                
            default:
                return std::make_unique<lightweight_di_container<T>>();
        }
    }
    
    /**
     * @brief Create the best available container
     * 
     * Attempts to create a thread_system container if available,
     * otherwise falls back to lightweight implementation.
     * 
     * @tparam T The type of objects the container will manage
     * @return Unique pointer to the created container
     */
    template<typename T>
    static std::unique_ptr<di_container_interface<T>> create_best_available() {
#ifdef USE_THREAD_SYSTEM
        if (is_thread_system_available()) {
            try {
                return std::make_unique<thread_system_di_adapter<T>>();
            } catch (...) {
                // Fallback on failure
            }
        }
#endif
        return std::make_unique<lightweight_di_container<T>>();
    }
    
    /**
     * @brief Check if thread_system is available
     * @return true if thread_system can be used
     */
    static bool is_thread_system_available() {
#ifdef USE_THREAD_SYSTEM
        // Try to create a test container to verify availability
        try {
            kcenon::thread::service_container test_container;
            return true;
        } catch (...) {
            return false;
        }
#else
        return false;
#endif
    }
    
    /**
     * @brief Get the name of the container type
     * @param type The container type
     * @return String representation of the type
     */
    static const char* get_container_type_name(container_type type) {
        switch (type) {
            case container_type::lightweight:
                return "lightweight";
            case container_type::thread_system:
                return "thread_system";
            case container_type::automatic:
                return "automatic";
            default:
                return "unknown";
        }
    }
    
    /**
     * @brief Get the currently available container type
     * @return The best available container type
     */
    static container_type get_available_type() {
#ifdef USE_THREAD_SYSTEM
        if (is_thread_system_available()) {
            return container_type::thread_system;
        }
#endif
        return container_type::lightweight;
    }
};

} // namespace kcenon::logger