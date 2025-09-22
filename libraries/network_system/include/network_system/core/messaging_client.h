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
#include <string>
#include <string_view>
#include <atomic>
#include <thread>
#include <future>
#include <optional>
#include <type_traits>

#include <asio.hpp>

#include "network_system/internal/tcp_socket.h"
#include "network_system/internal/pipeline.h"

// Use nested namespace definition in C++17
namespace network_system::core
{

	/*!
	 * \class messaging_client
	 * \brief A basic TCP client that connects to a remote host, sends/receives
	 * data using asynchronous operations, and can apply a pipeline for
	 * transformations.
	 *
	 * ### Key Features
	 * - Uses \c asio::io_context in a dedicated thread to handle I/O events.
	 * - Connects via \c async_connect, then wraps the socket in a \c tcp_socket
	 * for asynchronous reads and writes.
	 * - Optionally compresses/encrypts data before sending, and can similarly
	 *   decompress/decrypt incoming data if extended.
	 * - Provides \c start_client(), \c stop_client(), and \c wait_for_stop() to
	 * control lifecycle.
	 */
	class messaging_client
		: public std::enable_shared_from_this<messaging_client>
	{
	public:
		/*!
		 * \brief Constructs a client with a given \p client_id used for logging
		 * or identification.
		 * \param client_id A string identifier for this client instance.
		 */
		messaging_client(std::string_view client_id);

		/*!
		 * \brief Destructor; automatically calls \c stop_client() if the client
		 * is still running.
		 */
		~messaging_client();

		/*!
		 * \brief Starts the client by resolving \p host and \p port, connecting
		 * asynchronously, and spawning a thread to run \c io_context_.
		 * \param host The remote hostname or IP address.
		 * \param port The remote port number to connect.
		 *
		 * ### Steps:
		 * 1. Create \c io_context_.
		 * 2. Launch \c client_thread_ running \c io_context_->run().
		 * 3. Resolve & connect, on success calling \c on_connect().
		 * 4. \c on_connect() sets up the \c tcp_socket and starts reading.
		 */
		auto start_client(std::string_view host, unsigned short port) -> void;

		/*!
		 * \brief Stops the client: closes the socket, stops the \c io_context_,
		 *        and joins the worker thread.
		 */
		auto stop_client() -> void;

		/*!
		 * \brief Blocks until \c stop_client() is invoked, i.e., a
		 * synchronization mechanism.
		 */
		auto wait_for_stop() -> void;

		/*!
		 * \brief Sends data over the connection, optionally
		 * compressing/encrypting via the \c pipeline.
		 * \param data The buffer to send.
		 *
		 * If not connected or not running, this call does nothing.
		 * Otherwise, \c prepare_data_async() + \c tcp_socket::async_send is
		 * used to deliver the bytes.
		 */
		auto send_packet(std::vector<uint8_t> data) -> void;

	private:
		/*!
		 * \brief Internally attempts to resolve and connect to the remote \p
		 * host:\p port.
		 */
		auto do_connect(std::string_view host, unsigned short port) -> void;

		/*!
		 * \brief Callback invoked upon completion of an async connect.
		 * \param ec The \c std::error_code indicating success/failure.
		 */
		auto on_connect(std::error_code ec) -> void;

		/*!
		 * \brief Callback for receiving data from the \c tcp_socket.
		 * \param data A chunk of bytes that has arrived.
		 *
		 * By default, logs the size of received data. To fully handle incoming
		 * messages, one could parse, decompress, decrypt, etc.
		 */
		auto on_receive(const std::vector<uint8_t>& data) -> void;

		/*!
		 * \brief Callback for handling socket errors from \c tcp_socket.
		 * \param ec The \c std::error_code describing the error.
		 *
		 * By default, logs the error message and \c stop_client().
		 */
		auto on_error(std::error_code ec) -> void;

	private:
		std::string client_id_; /*!< Identifier or name for this client. */

		std::atomic<bool> is_running_{
			false
		}; /*!< True if client is active. */
		std::atomic<bool> is_connected_{
			false
		}; /*!< True if connected to remote. */

		std::unique_ptr<asio::io_context>
			io_context_;	/*!< I/O context for async operations. */
		std::unique_ptr<std::thread>
			client_thread_; /*!< Thread running \c io_context->run(). */

		std::optional<std::promise<void>>
			stop_promise_;	/*!< Signals \c wait_for_stop() when stopping. */
		std::future<void> stop_future_; /*!< Used by \c wait_for_stop(). */

		std::shared_ptr<internal::tcp_socket>
			socket_;   /*!< The \c tcp_socket wrapper once connected. */

		internal::pipeline
			pipeline_; /*!< Pipeline for optional compression/encryption. */
		bool compress_mode_{
			false
		}; /*!< If true, compress data before sending. */
		bool encrypt_mode_{
			false
		}; /*!< If true, encrypt data before sending. */
	};

} // namespace network_system::core