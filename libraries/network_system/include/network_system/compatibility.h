#pragma once

/**
 * @file compatibility.h
 * @brief Compatibility layer for messaging_system migration
 *
 * This header provides backward compatibility aliases and wrappers
 * to allow existing messaging_system code to work with network_system
 * without modification.
 *
 * @author kcenon
 * @date 2025-09-20
 */

#include "network_system/core/messaging_server.h"
#include "network_system/core/messaging_client.h"
#include "network_system/session/messaging_session.h"
#include "network_system/integration/messaging_bridge.h"
#include "network_system/integration/thread_integration.h"
#include "network_system/integration/container_integration.h"

// Legacy namespace aliases for backward compatibility
namespace network_module {
    // Core types
    using messaging_server = ::network_system::core::messaging_server;
    using messaging_client = ::network_system::core::messaging_client;

    // Session types
    using messaging_session = ::network_system::session::messaging_session;

    // Integration types
    using messaging_bridge = ::network_system::integration::messaging_bridge;

    // Thread integration
    using thread_pool_interface = ::network_system::integration::thread_pool_interface;
    using basic_thread_pool = ::network_system::integration::basic_thread_pool;
    using thread_integration_manager = ::network_system::integration::thread_integration_manager;

    // Container integration
    using container_interface = ::network_system::integration::container_interface;
    using basic_container = ::network_system::integration::basic_container;
    using container_manager = ::network_system::integration::container_manager;

#ifdef BUILD_WITH_CONTAINER_SYSTEM
    using container_system_adapter = ::network_system::integration::container_system_adapter;
#endif

    /**
     * @brief Legacy factory function for creating servers
     * @param server_id Server identifier
     * @return Shared pointer to messaging server
     */
    inline std::shared_ptr<messaging_server> create_server(const std::string& server_id) {
        return std::make_shared<messaging_server>(server_id);
    }

    /**
     * @brief Legacy factory function for creating clients
     * @param client_id Client identifier
     * @return Shared pointer to messaging client
     */
    inline std::shared_ptr<messaging_client> create_client(const std::string& client_id) {
        return std::make_shared<messaging_client>(client_id);
    }

    /**
     * @brief Legacy factory function for creating bridges
     * @return Shared pointer to messaging bridge
     */
    inline std::shared_ptr<messaging_bridge> create_bridge() {
        return std::make_shared<messaging_bridge>();
    }
}

// Additional compatibility namespace
namespace messaging {
    // Import all types from network_module for double compatibility
    using namespace network_module;
}

// Feature detection macros
#ifdef BUILD_WITH_CONTAINER_SYSTEM
    #define HAS_CONTAINER_INTEGRATION 1
#else
    #define HAS_CONTAINER_INTEGRATION 0
#endif

#ifdef BUILD_WITH_THREAD_SYSTEM
    #define HAS_THREAD_INTEGRATION 1
#else
    #define HAS_THREAD_INTEGRATION 0
#endif

// Deprecated macro for marking old code
#ifdef __GNUC__
    #define NETWORK_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
    #define NETWORK_DEPRECATED __declspec(deprecated)
#else
    #define NETWORK_DEPRECATED
#endif

/**
 * @namespace network_system::compat
 * @brief Compatibility utilities namespace
 */
namespace network_system::compat {

    /**
     * @brief Check if container integration is available
     * @return true if container system is integrated
     */
    inline constexpr bool has_container_support() {
        return HAS_CONTAINER_INTEGRATION;
    }

    /**
     * @brief Check if thread integration is available
     * @return true if thread system is integrated
     */
    inline constexpr bool has_thread_support() {
        return HAS_THREAD_INTEGRATION;
    }

    /**
     * @brief Initialize network system with default settings
     */
    inline void initialize() {
        // Initialize thread pool if not already set
        auto& thread_mgr = integration::thread_integration_manager::instance();
        if (!thread_mgr.get_thread_pool()) {
            thread_mgr.set_thread_pool(
                std::make_shared<integration::basic_thread_pool>()
            );
        }

        // Initialize container manager if not already set
        auto& container_mgr = integration::container_manager::instance();
        if (!container_mgr.get_default_container()) {
            container_mgr.set_default_container(
                std::make_shared<integration::basic_container>()
            );
        }
    }

    /**
     * @brief Shutdown network system cleanly
     */
    inline void shutdown() {
        // Clean shutdown of thread pool
        auto& thread_mgr = integration::thread_integration_manager::instance();
        if (auto pool = thread_mgr.get_thread_pool()) {
            if (auto basic = std::dynamic_pointer_cast<integration::basic_thread_pool>(pool)) {
                basic->stop(true);  // Wait for pending tasks
            }
        }
    }
}