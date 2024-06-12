/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2021, 🍀☀🌕🌥 🌊
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

#include "messaging_client.h"

#include "value.h"
#include "values/bool_value.h"
#include "values/bytes_value.h"
#include "values/container_value.h"
#include "values/short_value.h"
#include "values/string_value.h"
#include "values/ushort_value.h"

#include "compressing.h"
#include "converting.h"
#include "encrypting.h"
#include "job.h"
#include "job_pool.h"
#include "logging.h"
#include "thread_worker.h"

#include "binary_combiner.h"
#include "data_lengths.h"
#include "file_handler.h"

#include <functional>
#include <future>

#include "fmt/format.h"
#include "fmt/xchar.h"

namespace network
{
	using namespace logging;
	using namespace threads;
	using namespace container;
	using namespace converting;
	using namespace encrypting;
	using namespace compressing;
	using namespace file_handler;
	using namespace binary_parser;

	messaging_client::messaging_client(const std::string& source_id,
									   const unsigned char& start_code_value,
									   const unsigned char& end_code_value)
		: data_handling(start_code_value, end_code_value)
		, auto_echo_(false)
		, bridge_line_(false)
		, io_context_(nullptr)
		, auto_echo_interval_seconds_(1)
		, connection_(nullptr)
		, connection_key_("connection_key")
		, source_id_(source_id)
		, source_sub_id_("")
		, target_id_("unknown")
		, target_sub_id_("0.0.0.0:0")
		, socket_(nullptr)
		, session_type_(session_types::message_line)
		, thread_(nullptr)
	{
		message_handlers_.insert({ "confirm_connection",
								   std::bind(&messaging_client::confirm_message,
											 this, std::placeholders::_1) });
		message_handlers_.insert(
			{ "request_files", std::bind(&messaging_client::request_files, this,
										 std::placeholders::_1) });
		message_handlers_.insert(
			{ "echo", std::bind(&messaging_client::echo_message, this,
								std::placeholders::_1) });
	}

	messaging_client::~messaging_client(void) { stop(); }

	std::shared_ptr<messaging_client> messaging_client::get_ptr(void)
	{
		return shared_from_this();
	}

	std::string messaging_client::source_id(void) const { return source_id_; }

	std::string messaging_client::source_sub_id(void) const
	{
		return source_sub_id_;
	}

	void messaging_client::set_auto_echo(const bool& auto_echo,
										 const unsigned short& echo_interval)
	{
		if (session_type_ == session_types::binary_line)
		{
			logger::handle().write(logging_level::error,
								   "cannot set auto echo mode on binary line");
			return;
		}

		auto_echo_ = auto_echo;
		auto_echo_interval_seconds_ = echo_interval;
	}

	void messaging_client::set_bridge_line(const bool& bridge_line)
	{
		bridge_line_ = bridge_line;
	}

	void messaging_client::set_encrypt_mode(const bool& encrypt_mode)
	{
		encrypt_mode_ = encrypt_mode;
	}

	void messaging_client::set_compress_mode(const bool& compress_mode)
	{
		compress_mode_ = compress_mode;
	}

	void messaging_client::set_compress_block_size(
		const unsigned short& compress_block_size)
	{
		compress_block_size_ = compress_block_size;
	}

	void messaging_client::set_session_types(const session_types& session_type)
	{
		session_type_ = session_type;
	}

	void messaging_client::set_connection_key(const std::string& connection_key)
	{
		connection_key_ = connection_key;
	}

	void messaging_client::set_snipping_targets(
		const std::vector<std::string>& snipping_targets)
	{
		snipping_targets_ = snipping_targets;
	}

	void messaging_client::set_connection_notification(
		const std::function<void(
			const std::string&, const std::string&, const bool&)>& notification)
	{
		connection_ = notification;
	}

	void messaging_client::set_message_notification(
		const std::function<void(std::shared_ptr<container::value_container>)>&
			notification)
	{
		received_message_ = notification;
	}

	void messaging_client::set_file_notification(
		const std::function<void(const std::string&,
								 const std::string&,
								 const std::string&,
								 const std::string&)>& notification)
	{
		received_file_ = notification;
	}

	void messaging_client::set_binary_notification(
		const std::function<void(const std::string&,
								 const std::string&,
								 const std::string&,
								 const std::string&,
								 const std::vector<uint8_t>&)>& notification)
	{
		received_data_ = notification;
	}

	void messaging_client::set_specific_compress_sequence(
		const std::function<std::vector<uint8_t>(const std::vector<uint8_t>&,
												 const bool&)>&
			specific_compress_sequence)
	{
		specific_compress_sequence_ = specific_compress_sequence;
	}

	void messaging_client::set_specific_encrypt_sequence(
		const std::function<std::vector<uint8_t>(const std::vector<uint8_t>&,
												 const bool&)>&
			specific_encrypt_sequence)
	{
		specific_encrypt_sequence_ = specific_encrypt_sequence;
	}

	connection_conditions messaging_client::get_confirm_status(void) const
	{
		return confirm_;
	}

	void messaging_client::start(const std::string& ip,
								 const unsigned short& port,
								 const unsigned short& high_priority,
								 const unsigned short& normal_priority,
								 const unsigned short& low_priority)
	{
		stop();

		create_thread_pool(high_priority, normal_priority, low_priority);

		logger::handle().write(logging_level::sequence,
							   "attempts to create io_context");

		io_context_ = std::make_shared<asio::io_context>();

		logger::handle().write(logging_level::sequence,
							   "attempts to create socket");

		if (!create_socket(ip, port))
		{
			return;
		}

		source_sub_id_
			= fmt::format("{}:{}",
						  converter::to_string(
							  socket_->local_endpoint().address().to_string()),
						  socket_->local_endpoint().port());
		target_sub_id_
			= fmt::format("{}:{}",
						  converter::to_string(
							  socket_->remote_endpoint().address().to_string()),
						  socket_->remote_endpoint().port());

		read_start_code(socket_);

		send_connection();

		thread_ = std::make_shared<std::thread>(
			std::bind(&messaging_client::run, this));
	}

	void messaging_client::stop(void)
	{
		if (_thread_pool != nullptr)
		{
			_thread_pool->stop();
			_thread_pool.reset();
		}

		if (io_context_ != nullptr)
		{
			if (thread_ != nullptr)
			{
				if (thread_->joinable() && !io_context_->stopped())
				{
					io_context_->stop();
					thread_->join();
				}

				thread_.reset();
			}

			io_context_.reset();
		}

		if (socket_ != nullptr)
		{
			if (socket_->is_open())
			{
				asio::error_code ec;
				socket_->shutdown(asio::ip::tcp::socket::shutdown_both, ec);
				socket_->close();
			}
			socket_.reset();
		}
	}

	bool messaging_client::echo(void)
	{
		std::shared_ptr<container::value_container> container
			= std::make_shared<container::value_container>(
				source_id_, source_sub_id_, target_id_, target_sub_id_, "echo",
				std::vector<std::shared_ptr<container::value>>{});

		return send(container);
	}

	bool messaging_client::send(const container::value_container& message)
	{
		return send(std::make_shared<container::value_container>(message));
	}

	bool messaging_client::send(
		std::shared_ptr<container::value_container> message)
	{
		if (socket_ == nullptr)
		{
			return false;
		}

		if (message == nullptr)
		{
			return false;
		}

		if (session_type_ == session_types::binary_line)
		{
			return false;
		}

		if (get_confirm_status() != connection_conditions::confirmed)
		{
			logger::handle().write(logging_level::error,
								   "cannot send data on not confirmed line");

			return false;
		}

		if (message->source_id().empty())
		{
			message->set_source(source_id_, source_sub_id_);
		}

		auto serialize = message->serialize();
		auto serialize_array = converter::to_array(serialize);

		logger::handle().write(logging_level::packet,
							   fmt::format("send: {}", serialize));

		send_packet_job(serialize_array);

		return true;
	}

	bool messaging_client::send_files(const container::value_container& message)
	{
		return send_files(
			std::make_shared<container::value_container>(message));
	}

	bool messaging_client::send_files(
		std::shared_ptr<container::value_container> message)
	{
		if (socket_ == nullptr)
		{
			return false;
		}

		if (message == nullptr)
		{
			return false;
		}

		if (session_type_ != session_types::file_line)
		{
			return false;
		}

		if (get_confirm_status() != connection_conditions::confirmed)
		{
			logger::handle().write(logging_level::error,
								   "cannot send data on not confirmed line");

			return false;
		}

		if (message->source_id().empty())
		{
			message->set_source(source_id_, source_sub_id_);
		}

		std::shared_ptr<container::value_container> container
			= message->copy(false);
		container->swap_header();
		container->set_message_type("request_files");

		std::vector<std::shared_ptr<container::value>> files
			= message->value_array("file");
		for (auto& file : files)
		{
			container << std::make_shared<container::string_value>(
				"indication_id",
				message->get_value("indication_id")->to_string());
			container << std::make_shared<container::string_value>(
				"source", (*file)["source"]->to_string());
			container << std::make_shared<container::string_value>(
				"target", (*file)["target"]->to_string());

			send_file_job(container->serialize_array());
			container->clear_value();
		}

		return true;
	}

	bool messaging_client::send_binary(const std::string& target_id,
									   const std::string& target_sub_id,
									   const std::vector<uint8_t>& data)
	{
		if (socket_ == nullptr)
		{
			return false;
		}

		if (session_type_ != session_types::binary_line)
		{
			return false;
		}

		if (get_confirm_status() != connection_conditions::confirmed)
		{
			logger::handle().write(logging_level::error,
								   "cannot send data on not confirmed line");

			return false;
		}

		std::vector<uint8_t> result;
		combiner::append(result, converter::to_array(source_id_));
		combiner::append(result, converter::to_array(source_sub_id_));
		combiner::append(result, converter::to_array(target_id));
		combiner::append(result, converter::to_array(target_sub_id));
		combiner::append(result, data);

		logger::handle().write(
			logging_level::packet,
			fmt::format("send binary: source[{}:{}] -> target[{}:{}], {} bytes",
						source_id_, source_sub_id_, target_id, target_sub_id,
						data.size()));

		send_binary_job(result);

		return true;
	}

	void messaging_client::send_connection(void)
	{
		std::shared_ptr<container::container_value> snipping_targets
			= std::make_shared<container::container_value>("snipping_targets");
		for (auto& snipping_target : snipping_targets_)
		{
			snipping_targets->add(std::make_shared<container::string_value>(
				"snipping_target", snipping_target));
		}

		std::shared_ptr<container::value_container> container
			= std::make_shared<container::value_container>(
				source_id_, source_sub_id_, target_id_, target_sub_id_,
				"request_connection",
				std::vector<std::shared_ptr<container::value>>{
					std::make_shared<container::string_value>("connection_key",
															  connection_key_),
					std::make_shared<container::bool_value>("auto_echo",
															auto_echo_),
					std::make_shared<container::ushort_value>(
						"auto_echo_interval_seconds",
						auto_echo_interval_seconds_),
					std::make_shared<container::short_value>(
						"session_type", (short)session_type_),
					std::make_shared<container::bool_value>("bridge_mode",
															bridge_line_),
					snipping_targets });

		auto serialize = container->serialize();
		auto serialize_array = converter::to_array(serialize);

#ifdef _DEBUG
		logger::handle().write(logging_level::packet,
							   fmt::format("send: {}", serialize));
#endif

		send_packet_job(serialize_array);
	}

	void messaging_client::disconnected(void)
	{
		connection_notification(false);
	}

	void messaging_client::send_packet(const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		send_on_tcp(socket_, data_modes::packet_mode, data);
	}

	void messaging_client::send_file_packet(const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		send_on_tcp(socket_, data_modes::file_mode, data);
	}

	void messaging_client::send_binary_packet(const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		send_on_tcp(socket_, data_modes::binary_mode, data);
	}

	void messaging_client::normal_message(
		std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}

		if (confirm_ != connection_conditions::confirmed)
		{
			logger::handle().write(logging_level::error,
								   "cannot send data on not confirmed line");

			return;
		}

		if (received_message_ != nullptr)
		{
			auto result = async(std::launch::async, received_message_, message);
		}
	}

	void messaging_client::confirm_message(
		std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}

		logger::handle().write(logging_level::sequence,
							   "received confirm_message");

		target_id_ = message->source_id();
		target_sub_id_ = message->source_sub_id();
		source_sub_id_ = message->target_sub_id();

		if (!message->get_value("confirm")->to_boolean())
		{
			connection_notification(false);

			return;
		}

		confirm_ = connection_conditions::confirmed;
		encrypt_mode_ = message->get_value("encrypt_mode")->to_boolean();

		if (encrypt_mode_)
		{
			key_ = message->get_value("key")->to_string();
			iv_ = message->get_value("iv")->to_string();
		}

		std::vector<std::shared_ptr<value>> snipping_targets
			= message->get_value("snipping_targets")->children();
		for (auto& snipping_target : snipping_targets)
		{
			if (snipping_target == nullptr)
			{
				continue;
			}

			if (snipping_target->name() != "snipping_target")
			{
				continue;
			}

			logger::handle().write(logging_level::information,
								   fmt::format("accepted snipping target: {}",
											   snipping_target->to_string()));
		}

		connection_notification(true);
	}

	void messaging_client::request_files(
		std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}

		if (confirm_ != connection_conditions::confirmed)
		{
			logger::handle().write(logging_level::error,
								   "cannot send data on not confirmed line");

			return;
		}

		std::shared_ptr<container::value_container> container
			= message->copy(false);
		container->swap_header();
		container->set_message_type("request_file");

		std::vector<std::shared_ptr<container::value>> files
			= message->value_array("file");
		for (auto& file : files)
		{
			container << std::make_shared<container::string_value>(
				"indication_id",
				message->get_value("indication_id")->to_string());
			container << std::make_shared<container::string_value>(
				"source", (*file)["source"]->to_string());
			container << std::make_shared<container::string_value>(
				"target", (*file)["target"]->to_string());

			send_file_job(container->serialize_array());
			container->clear_value();
		}
	}

	void messaging_client::echo_message(
		std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}

		if (_thread_pool == nullptr)
		{
			return;
		}

		if (confirm_ != connection_conditions::confirmed)
		{
			logger::handle().write(logging_level::error,
								   "cannot send data on not confirmed line");

			return;
		}

		std::vector<std::shared_ptr<value>> response = (*message)["response"];
		if (!response.empty())
		{
			logger::handle().write(
				logging_level::sequence,
				fmt::format("received echo: {}", response[0]->to_boolean()));

			return;
		}

		message->swap_header();

		message << std::make_shared<bool_value>("response", true);

		send_packet_job(converter::to_array(message->serialize()));
	}

	void messaging_client::connection_notification(const bool& condition)
	{
		logger::handle().write(
			logging_level::information,
			fmt::format("{} a client {} {}[{}]",
						(condition ? "connected" : "disconnected"),
						(condition ? "to" : "from"), target_id_,
						target_sub_id_));

		if (!condition)
		{
			confirm_ = connection_conditions::expired;
		}

		if (connection_ != nullptr)
		{
			auto result = async(std::launch::async, connection_, target_id_,
								target_sub_id_, condition);
		}
	}

	bool messaging_client::create_socket(const std::string& ip,
										 const unsigned short& port)
	{
		try
		{
			socket_ = std::make_shared<asio::ip::tcp::socket>(*io_context_);
			socket_->open(asio::ip::tcp::v4());
			socket_->bind(asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 0));
			socket_->connect(asio::ip::tcp::endpoint(
				asio::ip::address::from_string(converter::to_string(ip)),
				port));

			socket_->set_option(asio::ip::tcp::no_delay(true));
			socket_->set_option(asio::socket_base::keep_alive(true));
			socket_->set_option(
				asio::socket_base::receive_buffer_size(buffer_size));

			return true;
		}
		catch (const std::overflow_error&)
		{
			connection_notification(false);

			return false;
		}
		catch (const std::runtime_error&)
		{
			connection_notification(false);

			return false;
		}
		catch (const std::exception&)
		{
			connection_notification(false);

			return false;
		}
		catch (...)
		{
			connection_notification(false);

			return false;
		}
	}

	void messaging_client::run(void)
	{
		try
		{
			logger::handle().write(
				logging_level::information,
				fmt::format("start messaging_client({})", source_id_));
			io_context_->run();
		}
		catch (const std::overflow_error&)
		{
		}
		catch (const std::runtime_error&)
		{
		}
		catch (const std::exception&)
		{
		}
		catch (...)
		{
		}

		logger::handle().write(
			logging_level::information,
			fmt::format("stop messaging_client({})", source_id_));
		connection_notification(false);
	}

	void messaging_client::create_thread_pool(
		const unsigned short& high_priority,
		const unsigned short& normal_priority,
		const unsigned short& low_priority)
	{
		_thread_pool
			= std::make_shared<threads::thread_pool>("messaging_client");

		_thread_pool->append(std::make_shared<thread_worker>(priorities::top),
							 true);
		for (unsigned short high = 0; high < high_priority; ++high)
		{
			_thread_pool->append(std::make_shared<thread_worker>(
									 priorities::high,
									 std::vector<priorities>{
										 priorities::normal, priorities::low }),
								 true);
		}
		for (unsigned short normal = 0; normal < normal_priority; ++normal)
		{
			_thread_pool->append(std::make_shared<thread_worker>(
									 priorities::normal,
									 std::vector<priorities>{
										 priorities::high, priorities::low }),
								 true);
		}
		for (unsigned short low = 0; low < low_priority; ++low)
		{
			_thread_pool->append(
				std::make_shared<thread_worker>(
					priorities::low,
					std::vector<priorities>{ priorities::high,
											 priorities::normal }),
				true);
		}
	}
} // namespace network
