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

#include "messaging_session.h"

#include "values/bool_value.h"
#include "values/container_value.h"
#include "values/string_value.h"

#include "compressing.h"
#include "converting.h"
#include "encrypting.h"
#include "job.h"
#include "job_pool.h"
#include "logging.h"
#include "thread_pool.h"
#include "thread_worker.h"

#include "binary_combiner.h"
#include "data_lengths.h"
#include "file_handler.h"

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

	messaging_session::messaging_session(const std::string& source_id,
										 const std::string& connection_key,
										 asio::ip::tcp::socket& socket,
										 const unsigned char& start_code_value,
										 const unsigned char& end_code_value)
		: data_handling(start_code_value, end_code_value)
		, bridge_line_(false)
		, source_id_(source_id)
		, source_sub_id_("")
		, target_id_("")
		, target_sub_id_("")
		, connection_key_(connection_key)
		, connection_(nullptr)
		, _kill_code(false)
		, socket_(std::make_shared<asio::ip::tcp::socket>(std::move(socket)))
		, auto_echo_interval_seconds_(1)
		, auto_echo_(false)
	{
		socket_->set_option(asio::ip::tcp::no_delay(true));
		socket_->set_option(asio::socket_base::keep_alive(true));
		socket_->set_option(
			asio::socket_base::receive_buffer_size(buffer_size));

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

		message_handlers_.insert(
			{ "request_connection",
			  std::bind(&messaging_session::connection_message, this,
						std::placeholders::_1) });
		message_handlers_.insert(
			{ "request_files", std::bind(&messaging_session::request_files,
										 this, std::placeholders::_1) });
		message_handlers_.insert(
			{ "echo", std::bind(&messaging_session::echo_message, this,
								std::placeholders::_1) });
	}

	messaging_session::~messaging_session(void)
	{
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

		stop();
	}

	std::shared_ptr<messaging_session> messaging_session::get_ptr(void)
	{
		return shared_from_this();
	}

	void messaging_session::set_kill_code(const bool& kill_code)
	{
		_kill_code = kill_code;
	}

	void messaging_session::set_acceptable_target_ids(
		const std::vector<std::string>& acceptable_target_ids)
	{
		_acceptable_target_ids = acceptable_target_ids;
	}

	void messaging_session::set_ignore_target_ids(
		const std::vector<std::string>& ignore_target_ids)
	{
		_ignore_target_ids = ignore_target_ids;
	}

	void messaging_session::set_ignore_snipping_targets(
		const std::vector<std::string>& ignore_snipping_targets)
	{
		_ignore_snipping_targets = ignore_snipping_targets;
	}

	void messaging_session::set_connection_notification(
		const std::function<void(std::shared_ptr<messaging_session>,
								 const bool&)>& notification)
	{
		connection_ = notification;
	}

	void messaging_session::set_message_notification(
		const std::function<void(std::shared_ptr<container::value_container>)>&
			notification)
	{
		received_message_ = notification;
	}

	void messaging_session::set_file_notification(
		const std::function<void(const std::string&,
								 const std::string&,
								 const std::string&,
								 const std::string&)>& notification)
	{
		received_file_ = notification;
	}

	void messaging_session::set_binary_notification(
		const std::function<void(const std::string&,
								 const std::string&,
								 const std::string&,
								 const std::string&,
								 const std::vector<uint8_t>&)>& notification)
	{
		received_data_ = notification;
	}

	void messaging_session::set_specific_compress_sequence(
		const std::function<std::vector<uint8_t>(const std::vector<uint8_t>&,
												 const bool&)>&
			specific_compress_sequence)
	{
		specific_compress_sequence_ = specific_compress_sequence;
	}

	void messaging_session::set_specific_encrypt_sequence(
		const std::function<std::vector<uint8_t>(const std::vector<uint8_t>&,
												 const bool&)>&
			specific_encrypt_sequence)
	{
		specific_encrypt_sequence_ = specific_encrypt_sequence;
	}

	const connection_conditions messaging_session::get_confirm_status(void)
	{
		return confirm_;
	}

	const session_types messaging_session::get_session_type(void)
	{
		return session_type_;
	}

	const std::string messaging_session::target_id(void) { return target_id_; }

	const std::string messaging_session::target_sub_id(void)
	{
		return target_sub_id_;
	}

	void messaging_session::start(
		const bool& encrypt_mode,
		const bool& compress_mode,
		const unsigned short& compress_block_size,
		const std::vector<session_types>& possible_session_types,
		const unsigned short& high_priority,
		const unsigned short& normal_priority,
		const unsigned short& low_priority,
		const unsigned short& drop_connection_time)
	{
		stop();

		encrypt_mode_ = encrypt_mode;
		compress_mode_ = compress_mode;
		compress_block_size_ = compress_block_size;
		_drop_connection_time = drop_connection_time;
		_possible_session_types = possible_session_types;

		confirm_ = connection_conditions::waiting;

		_thread_pool
			= std::make_shared<threads::thread_pool>("messaging_session");
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

		read_start_code(socket_);

		logger::handle().write(
			logging_level::information,
			fmt::format("started session: {}:{}",
						converter::to_string(
							socket_->remote_endpoint().address().to_string()),
						socket_->remote_endpoint().port()));

		_thread_pool->push(std::make_shared<job>(
			priorities::low,
			std::bind(&messaging_session::check_confirm_condition, this)));
	}

	void messaging_session::stop(void)
	{
		confirm_ = connection_conditions::expired;

		if (_thread_pool != nullptr)
		{
			_thread_pool->stop();
			_thread_pool.reset();
		}
	}

	void messaging_session::echo(void)
	{
		std::shared_ptr<container::value_container> container
			= std::make_shared<container::value_container>(
				source_id_, source_sub_id_, target_id_, target_sub_id_, "echo",
				std::vector<std::shared_ptr<container::value>>{});

		send(container);
	}

	bool messaging_session::send(
		std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return false;
		}

		if (!bridge_line_ && message->target_id() != target_id_
			&& !contained_snipping_target(message->target_id()))
		{
			return false;
		}

		if (!bridge_line_ && !contained_snipping_target(message->target_id())
			&& !message->target_sub_id().empty()
			&& message->target_sub_id() != target_sub_id_)
		{
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

	void messaging_session::send_files(
		std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}

		if (session_type_ != session_types::file_line)
		{
			return;
		}

		if (target_id_ != message->source_id()
			&& target_sub_id_ != message->source_sub_id())
		{
			return;
		}

		if (message->source_id().empty())
		{
			message->set_source(source_id_, source_sub_id_);
		}

		std::shared_ptr<container::value_container> container
			= message->copy(false);
		container->swap_header();
		container->set_target(
			message->get_value("gateway_source_id")->to_string(),
			message->get_value("gateway_source_sub_id")->to_string());
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
	}

	void messaging_session::send_binary(const std::string& target_id,
										const std::string& target_sub_id,
										const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (session_type_ != session_types::binary_line)
		{
			return;
		}

		if (!bridge_line_ && target_id != target_id_)
		{
			return;
		}

		if (!bridge_line_ && !target_sub_id.empty()
			&& target_sub_id != target_sub_id_)
		{
			return;
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
	}

	void messaging_session::send_binary(const std::string& source_id,
										const std::string& source_sub_id,
										const std::string& target_id,
										const std::string& target_sub_id,
										const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (session_type_ != session_types::binary_line)
		{
			return;
		}

		if (!bridge_line_ && target_id != target_id_)
		{
			return;
		}

		if (!bridge_line_ && !target_sub_id.empty()
			&& target_id != target_sub_id_)
		{
			return;
		}

		std::vector<uint8_t> result;
		combiner::append(result, converter::to_array(source_id));
		combiner::append(result, converter::to_array(source_sub_id));
		combiner::append(result, converter::to_array(target_id));
		combiner::append(result, converter::to_array(target_sub_id));
		combiner::append(result, data);

		logger::handle().write(
			logging_level::packet,
			fmt::format("send binary: source[{}:{}] -> target[{}:{}], {} bytes",
						source_id, source_sub_id, target_id, target_sub_id,
						data.size()));

		send_binary_job(result);
	}

	void messaging_session::disconnected(void)
	{
		stop();

		if (connection_ != nullptr)
		{
			auto result
				= async(std::launch::async, connection_, get_ptr(), false);
		}
	}

	bool messaging_session::contained_snipping_target(
		const std::string& snipping_target)
	{
		auto target = find(snipping_targets_.begin(), snipping_targets_.end(),
						   snipping_target);
		if (target == snipping_targets_.end())
		{
			return false;
		}

		return true;
	}

	void messaging_session::check_confirm_condition(void)
	{
		for (unsigned short index = 0; index < _drop_connection_time; ++index)
		{
			if (confirm_ != connection_conditions::waiting)
			{
				return;
			}

			std::this_thread::sleep_for(std::chrono::seconds(1));
		}

		if (confirm_ != connection_conditions::waiting)
		{
			return;
		}

		confirm_ = connection_conditions::expired;
		logger::handle().write(
			logging_level::error,
			fmt::format("expired line: {}[{}]", source_id_, source_sub_id_));

		disconnected();
	}

	void messaging_session::send_auto_echo(void)
	{
		if (!auto_echo_)
		{
			return;
		}

		for (unsigned short index = 0; index < auto_echo_interval_seconds_;
			 ++index)
		{
			if (confirm_ != connection_conditions::confirmed)
			{
				return;
			}

			std::this_thread::sleep_for(std::chrono::seconds(1));
		}

		echo();

		if (_thread_pool != nullptr)
		{
			_thread_pool->push(std::make_shared<job>(
				priorities::low,
				std::bind(&messaging_session::send_auto_echo, this)));
		}
	}

	void messaging_session::send_packet(const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		send_on_tcp(socket_, data_modes::packet_mode, data);
	}

	void messaging_session::send_file_packet(const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		send_on_tcp(socket_, data_modes::file_mode, data);
	}

	void messaging_session::send_binary_packet(const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		send_on_tcp(socket_, data_modes::binary_mode, data);
	}

	void messaging_session::normal_message(
		std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}

		if (confirm_ != connection_conditions::confirmed)
		{
			return;
		}

		if (received_message_)
		{
			auto result = async(std::launch::async, received_message_, message);
		}
	}

	void messaging_session::connection_message(
		std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			logger::handle().write(
				logging_level::error,
				"cannot parse connection message with empty message");

			return;
		}

		target_id_ = message->source_id();
		target_sub_id_
			= fmt::format("{}:{}",
						  converter::to_string(
							  socket_->remote_endpoint().address().to_string()),
						  socket_->remote_endpoint().port());

		session_type_
			= (session_types)message->get_value("session_type")->to_short();
		bridge_line_ = message->get_value("bridge_mode")->to_boolean();
		if (session_type_ != session_types::binary_line)
		{
			auto_echo_ = message->get_value("auto_echo")->to_boolean();
			auto_echo_interval_seconds_
				= message->get_value("auto_echo_interval_seconds")->to_ushort();

			logger::handle().write(
				logging_level::sequence,
				fmt::format("auto echo mode: {}, interval: {}", auto_echo_,
							auto_echo_interval_seconds_));
		}

		auto iter = find_if(
			_possible_session_types.begin(), _possible_session_types.end(),
			[&](const session_types& type) { return type == session_type_; });
		if (iter == _possible_session_types.end())
		{
			confirm_ = connection_conditions::expired;
			logger::handle().write(
				logging_level::error,
				"expired this line = \"cannot accept unknown session type\"");

			return;
		}

		if (source_id_ == target_id_)
		{
			confirm_ = connection_conditions::expired;
			logger::handle().write(
				logging_level::error,
				"expired this line = \"cannot use same id with server\"");

			return;
		}

		if (!_acceptable_target_ids.empty())
		{
			auto target = find(_acceptable_target_ids.begin(),
							   _acceptable_target_ids.end(), target_id_);
			if (target == _acceptable_target_ids.end())
			{
				confirm_ = connection_conditions::expired;
				logger::handle().write(logging_level::error,
									   "expired this line = \"cannot connect "
									   "with unknown id on server\"");

				return;
			}
		}

		if (!_ignore_target_ids.empty())
		{
			auto target = find(_ignore_target_ids.begin(),
							   _ignore_target_ids.end(), target_id_);
			if (target != _ignore_target_ids.end())
			{
				confirm_ = connection_conditions::expired;
				logger::handle().write(logging_level::error,
									   "expired this line = \"cannot connect "
									   "with ignored id on server\"");

				return;
			}
		}

		if (_kill_code)
		{
			confirm_ = connection_conditions::expired;
			logger::handle().write(logging_level::error,
								   "expired this line = \"set kill code\"");

			return;
		}

		// check connection key
		if (!same_key_check(message->get_value("connection_key")))
		{
			confirm_ = connection_conditions::expired;

			return;
		}

		logger::handle().write(logging_level::sequence,
							   "confirmed connection key");

		// compare both session id an client id
		if (!same_id_check())
		{
			confirm_ = connection_conditions::expired;

			return;
		}

		logger::handle().write(logging_level::sequence, "confirmed client id");

		generate_key();

		// check snipping target list
		std::shared_ptr<value> acceptable_snipping_targets
			= std::make_shared<container::container_value>("snipping_targets");

		snipping_targets_.clear();
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

			auto target = find(_ignore_snipping_targets.begin(),
							   _ignore_snipping_targets.end(),
							   snipping_target->to_string());
			if (target != _ignore_snipping_targets.end())
			{
				continue;
			}

			snipping_targets_.push_back(snipping_target->to_string());

			acceptable_snipping_targets->add(
				std::make_shared<container::string_value>(
					"snipping_target", snipping_target->to_string()));
		}

		std::vector<std::shared_ptr<container::value>> temp;
		temp.push_back(
			std::make_shared<container::bool_value>("confirm", true));
		temp.push_back(std::make_shared<container::bool_value>("encrypt_mode",
															   encrypt_mode_));
		temp.push_back(acceptable_snipping_targets);
		if (encrypt_mode_)
		{
			temp.push_back(
				std::make_shared<container::string_value>("key", key_));
			temp.push_back(
				std::make_shared<container::string_value>("iv", iv_));
		}

		std::shared_ptr<container::value_container> container
			= std::make_shared<container::value_container>(
				source_id_, source_sub_id_, target_id_, target_sub_id_,
				"confirm_connection", temp);

		auto serialize = container->serialize();
		auto serialize_array = converter::to_array(serialize);

#ifdef _DEBUG
		logger::handle().write(logging_level::packet,
							   fmt::format("send: {}", serialize));
#endif

		send_packet_job(serialize_array);

		confirm_ = connection_conditions::confirmed;

		if (connection_ != nullptr)
		{
			auto result
				= async(std::launch::async, connection_, get_ptr(), true);
		}

		if (_thread_pool != nullptr)
		{
			_thread_pool->push(std::make_shared<job>(
				priorities::low,
				std::bind(&messaging_session::send_auto_echo, this)));
		}
	}

	void messaging_session::request_files(
		std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}

		if (confirm_ != connection_conditions::confirmed)
		{
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

	void messaging_session::echo_message(
		std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}

		if (confirm_ != connection_conditions::confirmed)
		{
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

		std::shared_ptr<container::value_container> container
			= message->copy(false);
		container->swap_header();

		container << std::make_shared<bool_value>("response", true);

		send_packet_job(converter::to_array(container->serialize()));
	}

	void messaging_session::generate_key(void)
	{
		if (!encrypt_mode_)
		{
			key_ = "";
			iv_ = "";

			return;
		}

		logger::handle().write(logging_level::sequence,
							   "attempt to generate encrypt key");

		auto [key, iv] = cryptor::create_key();
		key_ = key;
		iv_ = iv;

		logger::handle().write(logging_level::sequence,
							   "generated encrypt key");
	}

	bool messaging_session::same_key_check(
		std::shared_ptr<container::value> key)
	{
		if (key != nullptr && connection_key_ == key->to_string())
		{
			return true;
		}

		logger::handle().write(
			logging_level::information,
			"ignored this line = \"unknown connection key\"");

		std::shared_ptr<container::value_container> container
			= std::make_shared<container::value_container>(
				source_id_, source_sub_id_, target_id_, target_sub_id_,
				"confirm_connection",
				std::vector<std::shared_ptr<container::value>>{
					std::make_shared<container::bool_value>("confirm", false),
					std::make_shared<container::string_value>(
						"reason",
						"ignored this line = \"unknown connection key\"") });

		send(container);

		return false;
	}

	bool messaging_session::same_id_check(void)
	{
		if (target_id_ != source_id_)
		{
			return true;
		}

		logger::handle().write(
			logging_level::information,
			"ignored this line = \"cannot use same id with server\"");

		std::shared_ptr<container::value_container> container
			= std::make_shared<container::value_container>(
				source_id_, source_sub_id_, target_id_, target_sub_id_,
				"confirm_connection",
				std::vector<std::shared_ptr<container::value>>{
					std::make_shared<container::bool_value>("confirm", false),
					std::make_shared<container::string_value>(
						"reason", "ignored this line = \"cannot use same id "
								  "with server\"") });

		send(container);

		return false;
	}
} // namespace network
