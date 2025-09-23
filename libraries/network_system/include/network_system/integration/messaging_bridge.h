/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, kcenon
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#pragma once

/**
 * @file messaging_bridge.h
 * @brief Bridge for messaging_system compatibility
 *
 * This bridge provides backward compatibility with the existing messaging_system
 * while using the new independent network_system implementation.
 *
 * @author kcenon
 * @date 2025-09-19
 */

#include "network_system/core/messaging_client.h"
#include "network_system/core/messaging_server.h"

#ifdef BUILD_WITH_CONTAINER_SYSTEM
#include "container.h"
#endif

#ifdef BUILD_WITH_THREAD_SYSTEM
#include <kcenon/thread/core/thread_pool.h>
#endif

#include "network_system/integration/thread_integration.h"

#include <memory>
#include <string>
#include <functional>
#include <chrono>

namespace network_system::integration {

/**
 * @class messaging_bridge
 * @brief Bridge class for messaging_system compatibility
 *
 * This class provides a compatibility layer that allows existing messaging_system
 * code to work with the new independent network_system without modification.
 */
class messaging_bridge {
public:
    /**
     * @brief Default constructor
     */
    messaging_bridge();

    /**
     * @brief Destructor
     */
    ~messaging_bridge();

    /**
     * @brief Create a messaging server with messaging_system compatible API
     * @param server_id Unique identifier for the server
     * @return Shared pointer to the created server
     */
    std::shared_ptr<core::messaging_server> create_server(
        const std::string& server_id
    );

    /**
     * @brief Create a messaging client with messaging_system compatible API
     * @param client_id Unique identifier for the client
     * @return Shared pointer to the created client
     */
    std::shared_ptr<core::messaging_client> create_client(
        const std::string& client_id
    );

#ifdef BUILD_WITH_CONTAINER_SYSTEM
    /**
     * @brief Set container for message serialization/deserialization
     * @param container Shared pointer to value container
     */
    void set_container(
        std::shared_ptr<container_module::value_container> container
    );

    /**
     * @brief Set container message handler
     * @param handler Function to handle container messages
     */
    void set_container_message_handler(
        std::function<void(const container_module::value_container&)> handler
    );
#endif

#ifdef BUILD_WITH_THREAD_SYSTEM
    /**
     * @brief Set thread pool for asynchronous operations
     * @param pool Shared pointer to thread pool
     */
    void set_thread_pool(
        std::shared_ptr<kcenon::thread::thread_pool> pool
    );
#endif

    /**
     * @brief Set thread pool using the integration interface
     * @param pool Thread pool interface implementation
     */
    void set_thread_pool_interface(
        std::shared_ptr<thread_pool_interface> pool
    );

    /**
     * @brief Get the thread pool interface
     * @return Current thread pool interface
     */
    std::shared_ptr<thread_pool_interface> get_thread_pool_interface() const;

    /**
     * @brief Performance metrics structure
     */
    struct performance_metrics {
        uint64_t messages_sent = 0;
        uint64_t messages_received = 0;
        uint64_t bytes_sent = 0;
        uint64_t bytes_received = 0;
        uint64_t connections_active = 0;
        std::chrono::milliseconds avg_latency{0};
        std::chrono::steady_clock::time_point start_time;
    };

    /**
     * @brief Get current performance metrics
     * @return Current performance metrics
     */
    performance_metrics get_metrics() const;

    /**
     * @brief Reset performance metrics
     */
    void reset_metrics();

    /**
     * @brief Check if bridge is initialized
     * @return true if initialized, false otherwise
     */
    bool is_initialized() const;

private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};

// Namespace alias for backward compatibility
namespace bridge = network_system::integration;

} // namespace network_system::integration

// Additional backward compatibility aliases
namespace network_module {
    using messaging_bridge = network_system::integration::messaging_bridge;
    using messaging_server = network_system::core::messaging_server;
    using messaging_client = network_system::core::messaging_client;
    using messaging_session = network_system::session::messaging_session;
}