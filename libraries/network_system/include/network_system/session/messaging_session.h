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
#include <vector>
#include <atomic>
#include <type_traits>

#include <asio.hpp>

#include "network_system/internal/tcp_socket.h"
#include "network_system/internal/pipeline.h"

// Use nested namespace definition in C++17
namespace network_system::session
{

	/*!
	 * \class messaging_session
	 * \brief Manages a single connected client session on the server side,
	 *        providing asynchronous read/write operations and pipeline
	 * transformations.
	 *
	 * ### Responsibilities
	 * - Owns a \c tcp_socket for non-blocking I/O.
	 * - Optionally applies compression/encryption via \c pipeline_ before
	 * sending, and can do the reverse upon receiving data (if needed).
	 * - Provides callbacks (\c on_receive, \c on_error) for data handling and
	 * error detection.
	 *
	 * ### Lifecycle
	 * - Constructed with an accepted \c asio::ip::tcp::socket.
	 * - \c start_session() sets up callbacks and begins \c
	 * socket_->start_read().
	 * - \c stop_session() closes the underlying socket, stopping further I/O.
	 */
	class messaging_session
		: public std::enable_shared_from_this<messaging_session>
	{
	public:
		/*!
		 * \brief Constructs a session with a given \p socket and \p server_id.
		 * \param socket    The \c asio::ip::tcp::socket (already connected).
		 * \param server_id An identifier for this server instance or context.
		 */
		// Using string_view in constructor for efficiency (C++17)
	messaging_session(asio::ip::tcp::socket socket,
						  std::string_view server_id);

		/*!
		 * \brief Destructor; calls \c stop_session() if not already stopped.
		 */
		~messaging_session();

		/*!
		 * \brief Starts the session: sets up read/error callbacks and begins
		 * reading data.
		 */
		auto start_session() -> void;

		/*!
		 * \brief Stops the session by closing the socket and marking the
		 * session as inactive.
		 */
		auto stop_session() -> void;

		/*!
		 * \brief Sends data to the connected client, optionally using
		 * compression/encryption.
		 * \param data The raw bytes to transmit.
		 *
		 * ### Notes
		 * - If \c compress_mode_ or \c encrypt_mode_ is true, the data will be
		 * processed by the pipeline's compress/encrypt functions before
		 * writing.
		 */
		auto send_packet(std::vector<uint8_t> data) -> void;

	private:
		/*!
		 * \brief Callback for when data arrives from the client.
		 * \param data A vector containing a chunk of received bytes.
		 *
		 * Override or extend the logic here to parse messages, handle commands,
		 * etc. If decompression/decryption is needed, apply \c pipeline_
		 * accordingly.
		 */
		auto on_receive(const std::vector<uint8_t>& data) -> void;

		/*!
		 * \brief Callback for handling socket errors from \c tcp_socket.
		 * \param ec The \c std::error_code describing the error.
		 *
		 * By default, logs the error and calls \c stop_session().
		 */
		auto on_error(std::error_code ec) -> void;

	private:
		std::string server_id_; /*!< Identifier for the server side. */

		std::shared_ptr<internal::tcp_socket>
			socket_;			/*!< The wrapped TCP socket for this session. */
		internal::pipeline
			pipeline_; /*!< Pipeline for compress/encrypt transformations. */

		bool compress_mode_{
			false
		}; /*!< If true, compress data before sending. */
		bool encrypt_mode_{
			false
		}; /*!< If true, encrypt data before sending. */

		std::atomic<bool> is_stopped_{
			false
		}; /*!< Indicates whether this session is stopped. */
	};

} // namespace network_system::session
