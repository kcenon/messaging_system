/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
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

#include <memory>
#include <vector>
#include <optional>
#include <future>
#include <thread>
#include <atomic>
#include <string>
#include <functional>
#include <chrono>
#include <shared_mutex>
#include <unordered_map>
#include <mutex>

#include "network_system/internal/asio_compat.h"

namespace network_module
{
	class messaging_session;

	/*!
	 * \class messaging_server
	 * \brief A server class that manages incoming TCP connections, creating
	 *        \c messaging_session instances for each accepted socket.
	 *
	 * ### Key Responsibilities
	 * - Maintains an \c asio::io_context and \c tcp::acceptor to listen on a
	 * specified port.
	 * - For each incoming connection, instantiates a \c messaging_session to
	 * handle the communication logic (compression, encryption, message parsing,
	 * etc.).
	 * - Allows external control via \c start_server(), \c stop_server(), and \c
	 * wait_for_stop().
	 *
	 * ### Thread Model
	 * - A single background thread calls \c io_context.run() to process I/O
	 * events.
	 * - Each accepted connection runs asynchronously; thus multiple sessions
	 * can be active concurrently without blocking each other.
	 *
	 * ### Usage Example
	 * \code
	 * // 1. Create a messaging_server:
	 * auto server = std::make_shared<network::messaging_server>("ServerID");
	 *
	 * // 2. Start listening on a port:
	 * server->start_server(5555);
	 *
	 * // 3. Wait for stop or run other tasks:
	 * //    ...
	 *
	 * // 4. Eventually stop the server:
	 * server->stop_server();
	 *
	 * // 5. (optional) If a separate thread is calling wait_for_stop():
	 * server->wait_for_stop();
	 * \endcode
	 */
	class messaging_server
		: public std::enable_shared_from_this<messaging_server>
	{
	public:
		/*!
		 * \brief Constructs a \c messaging_server with an optional string \p
		 * server_id.
		 * \param server_id A descriptive identifier for this server instance
		 * (e.g., "main_server").
		 */
		messaging_server(const std::string& server_id);

		/*!
		 * \brief Destructor. If the server is still running, \c stop_server()
		 * is invoked.
		 */
		~messaging_server();

		/*!
		 * \brief Begins listening on the specified TCP \p port, creates a
		 * background thread to run I/O operations, and starts accepting
		 * connections.
		 *
		 * \param port The TCP port to bind and listen on (e.g., 5555).
		 *
		 * #### Behavior
		 * - If the server is already running (\c is_running_ is \c true), this
		 * call does nothing.
		 * - Otherwise, an \c io_context and \c acceptor are created, \c
		 * do_accept() is invoked, and a new thread is spawned to run \c
		 * io_context->run().
		 */
		auto start_server(unsigned short port) -> void;

		/*!
		 * \brief Stops the server, closing the acceptor and all active
		 * sessions, then stops the \c io_context and joins the internal thread.
		 *
		 * #### Steps:
		 * 1. Set \c is_running_ to \c false.
		 * 2. Close the \c acceptor if open.
		 * 3. Iterate through all active sessions, calling \c stop_session().
		 * 4. \c io_context->stop() to halt asynchronous operations.
		 * 5. Join the background thread if it's joinable.
		 * 6. Fulfill the \c stop_promise_ so that \c wait_for_stop() can
		 * return.
		 *
		 * \note If the server is not running, this function does nothing.
		 */
		auto stop_server() -> void;

		/*!
		 * \brief Blocks until \c stop_server() is called, allowing the caller
		 *        to wait for a graceful shutdown in another context.
		 */
		auto wait_for_stop() -> void;

		// Enhanced server features

		/*!
		 * \brief Get current server statistics
		 * \return Server performance and connection statistics
		 */
		struct server_stats {
			std::atomic<size_t> total_connections{0};
			std::atomic<size_t> active_connections{0};
			std::atomic<size_t> total_bytes_sent{0};
			std::atomic<size_t> total_bytes_received{0};
			std::atomic<size_t> failed_connections{0};
			std::chrono::steady_clock::time_point start_time;

			double uptime_seconds() const {
				auto now = std::chrono::steady_clock::now();
				return std::chrono::duration<double>(now - start_time).count();
			}

			double connection_success_rate() const {
				auto total = total_connections.load();
				return total > 0 ?
					static_cast<double>(total - failed_connections.load()) / total : 0.0;
			}
		};
		server_stats get_statistics() const;

		/*!
		 * \brief Set maximum concurrent connections
		 * \param max_connections Maximum number of concurrent connections (0 = unlimited)
		 */
		void set_max_connections(size_t max_connections);

		/*!
		 * \brief Enable/disable connection rate limiting
		 * \param enabled Whether to enable rate limiting
		 * \param connections_per_second Maximum new connections per second
		 */
		void set_rate_limiting(bool enabled, size_t connections_per_second = 100);

		/*!
		 * \brief Configure keep-alive settings
		 * \param enabled Whether to enable keep-alive
		 * \param timeout_seconds Timeout for idle connections
		 */
		void set_keep_alive(bool enabled, unsigned int timeout_seconds = 300);

		/*!
		 * \brief Enable SSL/TLS encryption
		 * \param cert_file Path to certificate file
		 * \param key_file Path to private key file
		 * \return true if SSL was configured successfully
		 */
		bool configure_ssl(const std::string& cert_file, const std::string& key_file);

		/*!
		 * \brief Set connection callback for new connections
		 * \param callback Function to call when new connection is established
		 */
		using connection_callback = std::function<void(std::shared_ptr<messaging_session>)>;
		void set_connection_callback(connection_callback callback);

		/*!
		 * \brief Set disconnection callback
		 * \param callback Function to call when connection is closed
		 */
		using disconnection_callback = std::function<void(const std::string& session_id)>;
		void set_disconnection_callback(disconnection_callback callback);

		/*!
		 * \brief Broadcast message to all connected sessions
		 * \param message Message to broadcast
		 * \return Number of sessions that received the message
		 */
		size_t broadcast_message(const std::string& message);

		/*!
		 * \brief Send message to specific session
		 * \param session_id Target session ID
		 * \param message Message to send
		 * \return true if message was sent successfully
		 */
		bool send_to_session(const std::string& session_id, const std::string& message);

		/*!
		 * \brief Get list of active session IDs
		 * \return Vector of active session identifiers
		 */
		std::vector<std::string> get_active_sessions() const;

	private:
		/*!
		 * \brief Initiates an asynchronous accept operation (\c async_accept).
		 *
		 * On success, \c on_accept() is invoked with a newly accepted \c
		 * tcp::socket.
		 */
		auto do_accept() -> void;

		/*!
		 * \brief Handler called when an asynchronous accept finishes.
		 *
		 * \param ec     The \c std::error_code indicating success or error.
		 * \param socket The newly accepted \c tcp::socket.
		 *
		 * If \c ec is successful and \c is_running_ is still \c true,
		 * a \c messaging_session is created and stored, then \c do_accept() is
		 * invoked again to accept the next connection.
		 */
		auto on_accept(std::error_code ec, asio::ip::tcp::socket socket)
			-> void;

	private:
		std::string
			server_id_;		/*!< Name or identifier for this server instance. */

		std::unique_ptr<asio::io_context>
			io_context_;	/*!< The I/O context for async ops. */
		std::unique_ptr<asio::ip::tcp::acceptor>
			acceptor_;		/*!< Acceptor to listen for new connections. */
		std::unique_ptr<std::thread>
			server_thread_; /*!< Thread that runs \c io_context_->run(). */

		std::optional<std::promise<void>>
			stop_promise_;	/*!< Used to signal \c wait_for_stop(). */
		std::future<void>
			stop_future_;	/*!< Future that \c wait_for_stop() waits on. */

		std::atomic<bool> is_running_{
			false
		}; /*!< Indicates whether the server is active. */

		/*!
		 * \brief Holds all active sessions. When \c stop_server() is invoked,
		 *        each session's \c stop_session() is called and they are
		 * cleared.
		 */
		std::vector<std::shared_ptr<messaging_session>> sessions_;

		// Enhanced server state
		mutable server_stats stats_;
		mutable std::shared_mutex sessions_mutex_;

		// Configuration
		std::atomic<size_t> max_connections_{0}; // 0 = unlimited
		std::atomic<bool> rate_limiting_enabled_{false};
		std::atomic<size_t> max_connections_per_second_{100};
		std::atomic<bool> keep_alive_enabled_{true};
		std::atomic<unsigned int> keep_alive_timeout_seconds_{300};

		// SSL/TLS configuration
		std::atomic<bool> ssl_enabled_{false};
		std::string ssl_cert_file_;
		std::string ssl_key_file_;

		// Callbacks
		connection_callback connection_callback_;
		disconnection_callback disconnection_callback_;
		std::mutex callback_mutex_;

		// Rate limiting
		std::chrono::steady_clock::time_point last_connection_time_;
		std::atomic<size_t> connections_this_second_{0};

		// Session management
		std::unordered_map<std::string, std::weak_ptr<messaging_session>> session_map_;
		std::mutex session_map_mutex_;
	};

} // namespace network_module