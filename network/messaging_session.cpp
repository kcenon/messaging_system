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

#include "messaging_session.h"
#include "send_coroutine.h" // for async_send_with_pipeline_co / no_co
#include <iostream>

namespace network
{

	messaging_session::messaging_session(asio::ip::tcp::socket socket,
										 const std::string& server_id)
		: server_id_(server_id)
	{
		// Create the tcp_socket wrapper
		socket_ = std::make_shared<tcp_socket>(std::move(socket));

		// Initialize the pipeline (stub)
		pipeline_ = make_default_pipeline();

		// Default modes
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
		// We can either block or not
		std::error_code result_ec = fut.get();
		if (result_ec)
		{
			std::cerr << "[messaging_session] Send error: "
					  << result_ec.message() << "\n";
		}
#endif
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

} // namespace network
