#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

/**
 * @file monitoring_factory.h
 * @brief Factory for creating appropriate monitoring implementations
 * 
 * This factory creates the appropriate monitoring implementation based on
 * available dependencies and configuration.
 */

#include "monitoring_interface.h"
#include "basic_monitor.h"
#include "thread_system_monitor_adapter.h"
#include <memory>

namespace kcenon::logger {

/**
 * @brief Factory for creating monitoring implementations
 */
class monitoring_factory {
public:
    /**
     * @brief Monitoring backend types
     */
    enum class monitor_type {
        basic,          ///< Basic built-in monitoring
        thread_system,  ///< thread_system monitoring (if available)
        automatic      ///< Automatically choose best available
    };
    
    /**
     * @brief Create a monitoring implementation
     * @param type The type of monitor to create
     * @return Unique pointer to the created monitor
     */
    static std::unique_ptr<monitoring_interface> create_monitor(
        monitor_type type = monitor_type::automatic) {
        
        switch (type) {
            case monitor_type::basic:
                return std::make_unique<basic_monitor>();
                
#ifdef USE_THREAD_SYSTEM
            case monitor_type::thread_system:
                return std::make_unique<thread_system_monitor_adapter>();
#endif
                
            case monitor_type::automatic:
                return create_best_available();
                
            default:
                return std::make_unique<basic_monitor>();
        }
    }
    
    /**
     * @brief Create the best available monitor
     * 
     * Attempts to create a thread_system monitor if available,
     * otherwise falls back to basic implementation.
     * 
     * @return Unique pointer to the created monitor
     */
    static std::unique_ptr<monitoring_interface> create_best_available() {
#ifdef USE_THREAD_SYSTEM
        try {
            return std::make_unique<thread_system_monitor_adapter>();
        } catch (...) {
            // Fallback on failure
        }
#endif
        return std::make_unique<basic_monitor>();
    }
    
    /**
     * @brief Get the name of the monitor type
     * @param type The monitor type
     * @return String representation of the type
     */
    static const char* get_monitor_type_name(monitor_type type) {
        switch (type) {
            case monitor_type::basic:
                return "basic";
            case monitor_type::thread_system:
                return "thread_system";
            case monitor_type::automatic:
                return "automatic";
            default:
                return "unknown";
        }
    }
    
    /**
     * @brief Get the currently available monitor type
     * @return The best available monitor type
     */
    static monitor_type get_available_type() {
#ifdef USE_THREAD_SYSTEM
        return monitor_type::thread_system;
#else
        return monitor_type::basic;
#endif
    }
    
    /**
     * @brief Check if thread_system monitoring is available
     * @return true if thread_system monitoring can be used
     */
    static bool is_thread_system_available() {
#ifdef USE_THREAD_SYSTEM
        return true;
#else
        return false;
#endif
    }
};

} // namespace kcenon::logger