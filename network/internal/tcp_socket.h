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

#include <asio.hpp>
#include <functional>
#include <memory>
#include <vector>
#include <array>
#include <system_error>

#include "network/internal/common_defs.h"

namespace network_module
{
	/*!
	 * \class tcp_socket
	 * \brief A lightweight wrapper around \c asio::ip::tcp::socket,
	 *        enabling asynchronous read and write operations.
	 *
	 * ### Key Features
	 * - Maintains a \c socket_ (from ASIO) for TCP communication.
	 * - Exposes \c set_receive_callback() to handle inbound data
	 *   and \c set_error_callback() for error handling.
	 * - \c start_read() begins an ongoing loop of \c async_read_some().
	 * - \c async_send() performs an \c async_write of a given data buffer.
	 *
	 * ### Thread Safety
	 * - Typically, all asynchronous calls (e.g., \c async_read_some, \c
	 * async_write) should be called from the same thread context unless
	 * carefully managed with strands.
	 * - The provided callbacks will be invoked on an ASIO worker thread;
	 *   ensure that your callback logic is thread-safe if it shares data.
	 */
	class tcp_socket : public std::enable_shared_from_this<tcp_socket>
	{
	public:
		/*!
		 * \brief Constructs a \c tcp_socket by taking ownership of a moved \p
		 * socket.
		 * \param socket An \c asio::ip::tcp::socket that must be open/connected
		 * or at least valid.
		 *
		 * After construction, you can immediately call \c start_read() to begin
		 * receiving data. For sending, call \c async_send().
		 */
		tcp_socket(asio::ip::tcp::socket socket);

		/*!
		 * \brief Default destructor (no special cleanup needed).
		 */
		~tcp_socket() = default;

		/*!
		 * \brief Sets a callback to receive inbound data chunks.
		 * \param callback A function with signature \c void(const
		 * std::vector<uint8_t>&), called whenever a chunk of data is
		 * successfully read.
		 *
		 * If no callback is set, received data is effectively discarded.
		 */
		auto set_receive_callback(
			std::function<void(const std::vector<uint8_t>&)> callback) -> void;

		/*!
		 * \brief Sets a callback to handle socket errors (e.g., read/write
		 * failures).
		 * \param callback A function with signature \c void(std::error_code),
		 *        invoked when any asynchronous operation fails.
		 *
		 * If no callback is set, errors are not explicitly handled here (beyond
		 * stopping reads).
		 */
		auto set_error_callback(std::function<void(std::error_code)> callback)
			-> void;

		/*!
		 * \brief Begins the continuous asynchronous read loop.
		 *
		 * Once called, the class repeatedly calls \c async_read_some().
		 * If an error occurs, \c on_error() is triggered, stopping further
		 * reads.
		 */
		auto start_read() -> void;

		/*!
		 * \brief Initiates an asynchronous write of the given \p data buffer.
		 * \param data The buffer to send over TCP.
		 * \param handler A completion handler with signature \c
		 * void(std::error_code, std::size_t) that is invoked upon success or
		 * failure.
		 *
		 * The handler receives:
		 * - \c ec : the \c std::error_code from the write operation,
		 * - \c bytes_transferred : how many bytes were actually written.
		 *
		 * ### Example
		 * \code
		 * auto sock = std::make_shared<network::tcp_socket>(...);
		 * std::vector<uint8_t> buf = {0x01, 0x02, 0x03};
		 * sock->async_send(buf, [](std::error_code ec, std::size_t len) {
		 *     if(ec) {
		 *         // handle error
		 *     }
		 *     else {
		 *         // handle success
		 *     }
		 * });
		 * \endcode
		 */
		auto async_send(
			const std::vector<uint8_t>& data,
			std::function<void(std::error_code, std::size_t)> handler) -> void;

		/*!
		 * \brief Provides direct access to the underlying \c
		 * asio::ip::tcp::socket in case advanced operations are needed.
		 * \return A reference to the wrapped \c asio::ip::tcp::socket.
		 */
		auto socket() -> asio::ip::tcp::socket& { return socket_; }

	private:
		/*!
		 * \brief Internal function to handle the read logic with \c
		 * async_read_some().
		 *
		 * Upon success, it calls \c receive_callback_ if set, then schedules
		 * another read. On error, it calls \c error_callback_ if available.
		 */
		auto do_read() -> void;

	private:
		asio::ip::tcp::socket socket_; /*!< The underlying ASIO TCP socket. */

		std::array<uint8_t, 4096>
			read_buffer_; /*!< Buffer for receiving data in \c do_read(). */

		std::function<void(const std::vector<uint8_t>&)>
			receive_callback_; /*!< Inbound data callback. */
		std::function<void(std::error_code)>
			error_callback_;   /*!< Error callback. */
	};
} // namespace network_module
