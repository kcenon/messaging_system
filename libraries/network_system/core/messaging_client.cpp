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

#include "network_system/core/messaging_client.h"
#include "network_system/internal/send_coroutine.h"
#include "network_system/integration/logger_integration.h"
#include <string_view>
#include <type_traits>
#include <optional>

// Use nested namespace definition (C++17)
namespace network_module
{

	using tcp = asio::ip::tcp;

	// Use string_view for better efficiency (C++17)
	messaging_client::messaging_client(std::string_view client_id)
		: client_id_(client_id)
	{
		// Optionally configure pipeline or modes here:
		pipeline_ = make_default_pipeline();
		compress_mode_ = false; // set true if you want to compress
		encrypt_mode_ = false;	// set true if you want to encrypt
	}

	messaging_client::~messaging_client() { stop_client(); }

	// Use string_view for more efficient string handling (C++17)
	auto messaging_client::start_client(std::string_view host,
										unsigned short port) -> void
	{
		if (is_running_.load())
		{
			return;
		}
		is_running_.store(true);
		is_connected_.store(false);

		io_context_ = std::make_unique<asio::io_context>();
		// For wait_for_stop()
		stop_promise_.emplace();
		stop_future_ = stop_promise_->get_future();

		// Launch the thread to run the io_context
		client_thread_ = std::make_unique<std::thread>(
			[this]()
			{
				try
				{
					io_context_->run();
				}
				catch (...)
				{
					// handle exceptions if needed
				}
			});

		do_connect(host, port);

		NETWORK_LOG_INFO("[messaging_client] started. ID=" + client_id_
				+ " target=" + std::string(host) + ":" + std::to_string(port));
	}

	auto messaging_client::stop_client() -> void
	{
		if (!is_running_.load())
		{
			return;
		}
		is_running_.store(false);

		// Close the socket
		if (socket_)
		{
			socket_->socket().close();
		}
		// Stop io_context
		if (io_context_)
		{
			io_context_->stop();
		}
		// Join thread
		if (client_thread_ && client_thread_->joinable())
		{
			client_thread_->join();
		}
		// Signal stop
		if (stop_promise_.has_value())
		{
			stop_promise_->set_value();
			stop_promise_.reset();
		}

		NETWORK_LOG_INFO("[messaging_client] stopped.");
	}

	auto messaging_client::wait_for_stop() -> void
	{
		if (stop_future_.valid())
		{
			stop_future_.wait();
		}
	}

	auto messaging_client::do_connect(std::string_view host,
									  unsigned short port) -> void
	{
		// Use resolver to get endpoints
		tcp::resolver resolver(*io_context_);
		auto self = shared_from_this();

		resolver.async_resolve(
			std::string(host), std::to_string(port),
			[this, self](std::error_code ec,
						   tcp::resolver::results_type results)
			{
				if (ec)
				{
					NETWORK_LOG_ERROR("[messaging_client] Resolve error: " + ec.message());
					return;
				}
				// Attempt to connect to one of the resolved endpoints
				// Create a raw ASIO socket
				tcp::socket raw_socket(*io_context_);
				asio::async_connect(
					raw_socket, results,
					[this, self, &raw_socket](std::error_code connect_ec,
											 const tcp::endpoint& endpoint)
					{
						if (connect_ec)
						{
							NETWORK_LOG_ERROR("[messaging_client] Connect error: " + connect_ec.message());
							return;
						}
						// On success, wrap it in our tcp_socket
						socket_ = std::make_shared<tcp_socket>(
							std::move(raw_socket));
						on_connect(connect_ec);
					});
			});
	}

	auto messaging_client::on_connect(std::error_code ec) -> void
	{
		if (ec)
		{
			NETWORK_LOG_ERROR("[messaging_client] on_connect error: " + ec.message());
			return;
		}
		NETWORK_LOG_INFO("[messaging_client] Connected successfully.");
		is_connected_.store(true);

		// set callbacks and start read loop
		auto self = shared_from_this();
		socket_->set_receive_callback(
			[this, self](const std::vector<uint8_t>& chunk)
			{ on_receive(chunk); });
		socket_->set_error_callback([this, self](std::error_code err)
									{ on_error(err); });
		socket_->start_read();
	}

	auto messaging_client::send_packet(std::vector<uint8_t> data) -> void
	{
		if (!is_connected_.load() || !is_running_.load() || !socket_)
		{
			return;
		}
// Using if constexpr for compile-time branching (C++17)
if constexpr (std::is_same_v<decltype(socket_->socket().get_executor()), asio::io_context::executor_type>)
{
#ifdef USE_STD_COROUTINE
		// Coroutine approach
		asio::co_spawn(socket_->socket().get_executor(),
					   async_send_with_pipeline_co(socket_, std::move(data),
												   pipeline_, compress_mode_,
												   encrypt_mode_),
					   [](std::error_code ec)
					   {
						   if (ec)
						   {
							   NETWORK_LOG_ERROR("[messaging_client] Send error: " + ec.message());
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
			NETWORK_LOG_ERROR("[messaging_client] Send error: " + result_ec.message());
			}
		} catch (const std::exception& e) {
			NETWORK_LOG_ERROR("[messaging_client] Exception while waiting for send: " + std::string(e.what()));
		}
#endif
}
	}

	auto messaging_client::on_receive(const std::vector<uint8_t>& data) -> void
	{
		if (!is_connected_.load())
		{
			return;
		}
		NETWORK_LOG_DEBUG("[messaging_client] Received " + std::to_string(data.size())
				+ " bytes");

		// Decompress/Decrypt if needed?
		// For demonstration, ignoring. In real usage:
		//   auto uncompressed = pipeline_.decompress(...);
		//   auto decrypted = pipeline_.decrypt(...);
		//   parse or handle...
	}

	auto messaging_client::on_error(std::error_code ec) -> void
	{
		NETWORK_LOG_ERROR("[messaging_client] Socket error: " + ec.message());
		// Perhaps reconnect or just stop
		stop_client();
	}

} // namespace network_module