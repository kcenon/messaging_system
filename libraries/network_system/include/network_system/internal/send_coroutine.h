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

#include <future>
#include <asio.hpp>
#include "network_system/internal/pipeline.h"
#include "network_system/internal/tcp_socket.h"

namespace network_system::internal
{
	/*!
	 * \brief Launches a separate thread (via std::async) to apply
	 * compression/encryption to \p input_data using \p pl, returning a future
	 * for the processed data.
	 *
	 * \param input_data    The raw buffer to be processed.
	 * \param pl            The pipeline containing compress/encrypt functions.
	 * \param use_compress  If true, calls \c pl.compress().
	 * \param use_encrypt   If true, calls \c pl.encrypt().
	 * \return A std::future that eventually yields the transformed data.
	 */
	auto prepare_data_async(const std::vector<uint8_t>& input_data,
							const pipeline& pl,
							bool use_compress,
							bool use_encrypt)
		-> std::future<std::vector<uint8_t>>;

#ifdef USE_STD_COROUTINE

#include <asio/experimental/as_tuple.hpp>
#include <asio/awaitable.hpp>
#include <asio/use_awaitable.hpp>

	/*!
	 * \brief Coroutine-based function that prepares data (via \c
	 * prepare_data_async) and then \c co_awaits an async write to the \p sock.
	 *
	 * \param sock         The \c tcp_socket where data will be sent.
	 * \param data         The raw data (moved) for transmission.
	 * \param pl           The pipeline for compression/encryption.
	 * \param use_compress If true, compress data first.
	 * \param use_encrypt  If true, encrypt data after (or before) compression.
	 * \return An \c asio::awaitable<std::error_code> that completes after
	 * async_write.
	 *
	 * ### Example
	 * \code
	 * co_await async_send_with_pipeline_co(my_socket, std::move(data),
	 * pipeline, true, false);
	 * \endcode
	 */
	auto async_send_with_pipeline_co(std::shared_ptr<tcp_socket> sock,
									 std::vector<uint8_t> data,
									 const pipeline& pl,
									 bool use_compress,
									 bool use_encrypt)
		-> asio::awaitable<std::error_code>;

#else // fallback

	/*!
	 * \brief Non-coroutine version that prepares data and then sends it
	 * asynchronously, returning a std::future<std::error_code>.
	 *
	 * \param sock         The \c tcp_socket to write on.
	 * \param data         The raw data (moved) for sending.
	 * \param pl           The pipeline for optional compression/encryption.
	 * \param use_compress If true, compress data before sending.
	 * \param use_encrypt  If true, encrypt data after compression.
	 * \return A std::future that eventually yields the \c std::error_code from
	 * async_write.
	 *
	 * ### Example
	 * \code
	 * auto fut = async_send_with_pipeline_no_co(socket, std::move(data), pl,
	 * true, false); std::error_code ec = fut.get(); // blocking if(ec) { //
	 * handle error }
	 * \endcode
	 */
	auto async_send_with_pipeline_no_co(std::shared_ptr<tcp_socket> sock,
										std::vector<uint8_t> data,
										const pipeline& pl,
										bool use_compress,
										bool use_encrypt)
		-> std::future<std::error_code>;

#endif

} // namespace network_system::internal