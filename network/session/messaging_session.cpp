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

#include "network/session/messaging_session.h"
#include "network/internal/send_coroutine.h" // for async_send_with_pipeline_co / no_co
#include <iostream>
#include <string_view>
#include <type_traits>

// Use nested namespace definition (C++17)
namespace network_module
{

	// Use string_view in constructor for efficiency (C++17)
	messaging_session::messaging_session(asio::ip::tcp::socket socket,
										 std::string_view server_id)
		: server_id_(server_id)
	{
		// Create the tcp_socket wrapper
		socket_ = std::make_shared<tcp_socket>(std::move(socket));

		// Initialize the pipeline (stub)
		pipeline_ = make_default_pipeline();

		// Default modes - could use inline initialization in header with C++17
		compress_mode_ = false;
		encrypt_mode_ = false;
	}

	messaging_session::~messaging_session() { stop_session(); }

	auto messaging_session::start_session() -> void
	{
		if (is_stopped_.load())
		{
			return;
		}

		// Set up callbacks
		auto self = shared_from_this();
		socket_->set_receive_callback(
			[this, self](const std::vector<uint8_t>& data)
			{ on_receive(data); });
		socket_->set_error_callback([this, self](std::error_code ec)
									{ on_error(ec); });

		// Begin reading
		socket_->start_read();

		std::cout << "[messaging_session] Started session on server: "
				  << server_id_ << "\n";
	}

	auto messaging_session::stop_session() -> void
	{
		if (is_stopped_.exchange(true))
		{
			return;
		}
		// Close socket
		if (socket_)
		{
			socket_->socket().close();
		}
		std::cout << "[messaging_session] Stopped.\n";
	}

	auto messaging_session::send_packet(std::vector<uint8_t> data) -> void
	{
		if (is_stopped_.load())
		{
			return;
		}
// Using if constexpr for compile-time branching (C++17)
if constexpr (std::is_same_v<decltype(socket_->socket().get_executor()), asio::io_context::executor_type>)
{
#ifdef USE_STD_COROUTINE
		// Coroutine-based approach
		asio::co_spawn(socket_->socket().get_executor(),
					   async_send_with_pipeline_co(socket_, std::move(data),
												   pipeline_, compress_mode_,
												   encrypt_mode_),
					   [](std::error_code ec)
					   {
						   if (ec)
						   {
							   std::cerr << "[messaging_session] Send error: "
										 << ec.message() << "\n";
						   }
					   });
#else
		// Fallback approach
		auto fut = async_send_with_pipeline_no_co(
			socket_, std::move(data), pipeline_, compress_mode_, encrypt_mode_);
		
		// Use structured binding with try/catch for better error handling (C++17)
		try {
			auto result_ec = fut.get();
			if (result_ec)
			{
				std::cerr << "[messaging_session] Send error: "
						<< result_ec.message() << "\n";
			}
		} catch (const std::exception& e) {
			std::cerr << "[messaging_session] Exception while waiting for send: "
					<< e.what() << "\n";
		}
#endif
}
	}

	auto messaging_session::on_receive(const std::vector<uint8_t>& data) -> void
	{
		if (is_stopped_.load())
		{
			return;
		}

		std::cout << "[messaging_session] Received " << data.size()
				  << " bytes.\n";

		// Potentially decompress + decrypt
		// e.g. auto uncompressed = pipeline_.decompress(data);
		//      auto decrypted    = pipeline_.decrypt(uncompressed);
		// Then parse or handle the final data
	}

	auto messaging_session::on_error(std::error_code ec) -> void
	{
		std::cerr << "[messaging_session] Socket error: " << ec.message()
				  << "\n";
		stop_session();
	}

} // namespace network_module
