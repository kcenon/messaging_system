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

#include "messaging_server.h"

#include "converting.h"
#include "job.h"
#include "logging.h"
#include "messaging_session.h"
#include "thread_worker.h"

#include "values/bool_value.h"
#include "values/string_value.h"

#include "fmt/format.h"
#include "fmt/xchar.h"

#include <algorithm>
#include <future>

namespace network
{
	using namespace logging;
	using namespace threads;
	using namespace converting;

	messaging_server::messaging_server(const std::string& source_id,
									   const unsigned char& start_code_value,
									   const unsigned char& end_code_value)
		: io_context_(nullptr)
		, _acceptor(nullptr)
		, source_id_(source_id)
		, connection_key_("connection_key")
		, encrypt_mode_(false)
		, received_file_(nullptr)
		, received_data_(nullptr)
		, connection_(nullptr)
		, received_message_(nullptr)
		, compress_mode_(false)
		, _high_priority(8)
		, _normal_priority(8)
		, _low_priority(8)
		, _session_limit_count(0)
		, _possible_session_types({ session_types::message_line })
		, _start_code_value(start_code_value)
		, _end_code_value(end_code_value)
		, compress_block_size_(1024)
		, _use_message_response(true)
		, _drop_connection_time(5)
	{
	}

	messaging_server::~messaging_server(void) { stop(); }

	std::shared_ptr<messaging_server> messaging_server::get_ptr(void)
	{
		return shared_from_this();
	}

	void messaging_server::set_encrypt_mode(const bool& encrypt_mode)
	{
		encrypt_mode_ = encrypt_mode;
	}

	void messaging_server::set_compress_mode(const bool& compress_mode)
	{
		compress_mode_ = compress_mode;
	}

	void messaging_server::set_compress_block_size(
		const unsigned short& compress_block_size)
	{
		compress_block_size_ = compress_block_size;
	}

	void messaging_server::set_use_message_response(
		const bool& use_message_response)
	{
		_use_message_response = use_message_response;
	}

	void messaging_server::set_drop_connection_time(
		const unsigned short& drop_connection_time)
	{
		_drop_connection_time = drop_connection_time;
	}

	void messaging_server::set_connection_key(const std::string& connection_key)
	{
		connection_key_ = connection_key;
	}

	void messaging_server::set_acceptable_target_ids(
		const std::vector<std::string>& acceptable_target_ids)
	{
		_acceptable_target_ids = acceptable_target_ids;
	}

	void messaging_server::set_ignore_target_ids(
		const std::vector<std::string>& ignore_target_ids)
	{
		_ignore_target_ids = ignore_target_ids;
	}

	void messaging_server::set_ignore_snipping_targets(
		const std::vector<std::string>& ignore_snipping_targets)
	{
		_ignore_snipping_targets = ignore_snipping_targets;
	}

	void messaging_server::set_possible_session_types(
		const std::vector<session_types>& possible_session_types)
	{
		_possible_session_types = possible_session_types;
	}

	void messaging_server::set_session_limit_count(
		const size_t& session_limit_count)
	{
		_session_limit_count = session_limit_count;
	}

	void messaging_server::set_connection_notification(
		const std::function<void(
			const std::string&, const std::string&, const bool&)>& notification)
	{
		connection_ = notification;
	}

	void messaging_server::set_message_notification(
		const std::function<void(std::shared_ptr<container::value_container>)>&
			notification)
	{
		received_message_ = notification;
	}

	void messaging_server::set_file_notification(
		const std::function<void(const std::string&,
								 const std::string&,
								 const std::string&,
								 const std::string&)>& notification)
	{
		received_file_ = notification;
	}

	void messaging_server::set_binary_notification(
		const std::function<void(const std::string&,
								 const std::string&,
								 const std::string&,
								 const std::string&,
								 const std::vector<uint8_t>&)>& notification)
	{
		received_data_ = notification;
	}

	void messaging_server::set_specific_compress_sequence(
		const std::function<std::vector<uint8_t>(const std::vector<uint8_t>&,
												 const bool&)>&
			specific_compress_sequence)
	{
		specific_compress_sequence_ = specific_compress_sequence;
	}

	void messaging_server::set_specific_encrypt_sequence(
		const std::function<std::vector<uint8_t>(const std::vector<uint8_t>&,
												 const bool&)>&
			specific_encrypt_sequence)
	{
		specific_encrypt_sequence_ = specific_encrypt_sequence;
	}

	void messaging_server::start(const unsigned short& port,
								 const unsigned short& high_priority,
								 const unsigned short& normal_priority,
								 const unsigned short& low_priority)
	{
		stop();

		_high_priority = high_priority;
		_normal_priority = normal_priority;
		_low_priority = low_priority;

		io_context_ = std::make_shared<asio::io_context>();
		_acceptor = std::make_shared<asio::ip::tcp::acceptor>(
			*io_context_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));

		wait_connection();

		thread_ = std::make_shared<std::thread>(
			[&]()
			{
				while (io_context_)
				{
					try
					{
						logger::handle().write(
							logging_level::information,
							fmt::format("start messaging_server({})",
										source_id_));
						io_context_->run();
						io_context_.reset();
						logger::handle().write(
							logging_level::information,
							fmt::format("stop messaging_server({})",
										source_id_));
					}
					catch (const std::overflow_error&)
					{
						if (io_context_ != nullptr)
						{
							io_context_.reset();
							logger::handle().write(
								logging_level::information,
								fmt::format("stop messaging_server({})",
											source_id_));
						}
					}
					catch (const std::runtime_error&)
					{
						if (io_context_ != nullptr)
						{
							io_context_.reset();
							logger::handle().write(
								logging_level::information,
								fmt::format("stop messaging_server({})",
											source_id_));
						}
					}
					catch (const std::exception&)
					{
						if (io_context_ != nullptr)
						{
							io_context_.reset();
							logger::handle().write(
								logging_level::information,
								fmt::format("stop messaging_server({})",
											source_id_));
						}
					}
					catch (...)
					{
						if (io_context_ != nullptr)
						{
							io_context_.reset();
							logger::handle().write(
								logging_level::information,
								fmt::format("stop messaging_server({})",
											source_id_));
						}
					}
				}
			});
	}

	void messaging_server::wait_stop(const unsigned int& seconds)
	{
		if (!_promise_status.has_value())
		{
			_promise_status = { std::promise<bool>() };
		}

		_future_status = _promise_status.value().get_future();

		if (seconds == 0)
		{
			_future_status.wait();
			_promise_status.reset();
			return;
		}

		_future_status.wait_for(std::chrono::seconds(seconds));
		_promise_status.reset();
	}

	void messaging_server::stop(void)
	{
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

		if (_acceptor != nullptr)
		{
			if (_acceptor->is_open())
			{
				_acceptor->close();
				_acceptor.reset();
			}
		}

		for (auto& session : _sessions)
		{
			if (session == nullptr)
			{
				continue;
			}

			session->stop();
		}
		_sessions.clear();

		if (_promise_status.has_value())
		{
			_promise_status.value().set_value(true);
		}
	}

	void messaging_server::disconnect(const std::string& target_id,
									  const std::string& target_sub_id)
	{
		std::vector<std::shared_ptr<messaging_session>>::iterator target
			= _sessions.begin();
		while (target != _sessions.end())
		{
			if (*target == nullptr)
			{
				target++;
				continue;
			}

			if ((*target)->target_id() != target_id
				|| (*target)->target_sub_id() != target_sub_id)
			{
				target++;
				continue;
			}

			target = _sessions.erase(target);
		}
	}

	void messaging_server::echo(void)
	{
		auto sessions = current_sessions();
		for (auto& session : sessions)
		{
			if (session == nullptr)
			{
				continue;
			}

			session->echo();
		}
	}

	bool messaging_server::send(const container::value_container& message,
								std::optional<session_types> type)
	{
		return send(std::make_shared<container::value_container>(message),
					type);
	}

	bool messaging_server::send(
		std::shared_ptr<container::value_container> message,
		std::optional<session_types> type)
	{
		if (message == nullptr)
		{
			return false;
		}

		bool result = false;
		auto sessions = current_sessions();
		for (auto& session : sessions)
		{
			if (session == nullptr)
			{
				continue;
			}

			if (session->get_confirm_status()
				!= connection_conditions::confirmed)
			{
				continue;
			}

			if (type.has_value() && session->get_session_type() != type.value())
			{
				continue;
			}

			result |= session->send(message);
		}

		return result;
	}

	void messaging_server::send_files(const container::value_container& message)
	{
		send_files(std::make_shared<container::value_container>(message));
	}

	void messaging_server::send_files(
		std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}

		auto sessions = current_sessions();
		for (auto& session : sessions)
		{
			if (session == nullptr)
			{
				continue;
			}

			if (session->get_confirm_status()
				!= connection_conditions::confirmed)
			{
				continue;
			}

			session->send_files(message);
		}
	}

	void messaging_server::send_binary(const std::string& target_id,
									   const std::string& target_sub_id,
									   const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		auto sessions = current_sessions();
		for (auto& session : sessions)
		{
			if (session == nullptr)
			{
				continue;
			}

			if (session->get_confirm_status()
				!= connection_conditions::confirmed)
			{
				continue;
			}

			session->send_binary(target_id, target_sub_id, data);
		}
	}

	void messaging_server::send_binary(const std::string& source_id,
									   const std::string& source_sub_id,
									   const std::string& target_id,
									   const std::string& target_sub_id,
									   const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		auto sessions = current_sessions();
		for (auto& session : sessions)
		{
			if (session == nullptr)
			{
				continue;
			}

			if (session->get_confirm_status()
				!= connection_conditions::confirmed)
			{
				continue;
			}

			session->send_binary(source_id, source_sub_id, target_id,
								 target_sub_id, data);
		}
	}

	void messaging_server::wait_connection(void)
	{
		_acceptor->async_accept(
			[this](std::error_code ec, asio::ip::tcp::socket socket)
			{
				if (ec)
				{
					return;
				}

				logger::handle().write(
					logging_level::information,
					fmt::format(
						"accepted new client: {}:{}",
						converter::to_string(
							socket.remote_endpoint().address().to_string()),
						socket.remote_endpoint().port()));

				std::shared_ptr<messaging_session> session
					= std::make_shared<messaging_session>(
						source_id_, connection_key_, socket, _start_code_value,
						_end_code_value);
				if (session == nullptr)
				{
					wait_connection();

					return;
				}

				if (_session_limit_count > 0)
				{
					session->set_kill_code(_sessions.size()
										   >= _session_limit_count);
				}

				session->set_acceptable_target_ids(_acceptable_target_ids);
				session->set_ignore_target_ids(_ignore_target_ids);
				session->set_ignore_snipping_targets(_ignore_snipping_targets);
				session->set_connection_notification(
					std::bind(&messaging_server::connect_condition, this,
							  std::placeholders::_1, std::placeholders::_2));
				session->set_message_notification(
					std::bind(&messaging_server::received_message, this,
							  std::placeholders::_1));
				session->set_file_notification(received_file_);
				session->set_binary_notification(
					std::bind(&messaging_server::received_binary, this,
							  std::placeholders::_1, std::placeholders::_2,
							  std::placeholders::_3, std::placeholders::_4,
							  std::placeholders::_5));
				session->set_specific_compress_sequence(
					specific_compress_sequence_);
				session->set_specific_encrypt_sequence(
					specific_encrypt_sequence_);

				session->start(encrypt_mode_, compress_mode_,
							   compress_block_size_, _possible_session_types,
							   _high_priority, _normal_priority, _low_priority,
							   _drop_connection_time);

				_sessions.push_back(session);

				wait_connection();
			});
	}

	void messaging_server::connect_condition(
		std::shared_ptr<messaging_session> target, const bool& condition)
	{
		if (target == nullptr)
		{
			return;
		}

		logger::handle().write(
			logging_level::information,
			fmt::format("{} a client({}[{}]) {} server",
						(condition ? "connected" : "disconnected"),
						target->target_id(), target->target_sub_id(),
						(condition ? "to" : "from")));

		if (!condition)
		{
			auto iter = find(_sessions.begin(), _sessions.end(), target);
			if (iter != _sessions.end())
			{
				_sessions.erase(iter);
			}
		}

		if (connection_ != nullptr)
		{
			auto result
				= async(std::launch::async, connection_, target->target_id(),
						target->target_sub_id(), condition);
		}
	}

	std::vector<std::shared_ptr<messaging_session>> messaging_server::
		current_sessions(void)
	{
		std::vector<std::shared_ptr<messaging_session>> result;

		result.assign(_sessions.begin(), _sessions.end());

		return result;
	}

	void messaging_server::received_message(
		std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}

		auto target_id = message->target_id();
		if (target_id == source_id_)
		{
			if (received_message_ != nullptr)
			{
				auto result
					= async(std::launch::async, received_message_, message);
			}

			return;
		}

		logger::handle().write(
			logging_level::sequence,
			fmt::format("attempt to transfer message to {}", target_id));

		bool sent = send(message);
		if (!sent)
		{
			logger::handle().write(
				logging_level::sequence,
				fmt::format("there is no target id on server: {}", target_id));
		}

		if (_use_message_response)
		{
			logger::handle().write(
				logging_level::sequence,
				fmt::format("attempt to response message to {}",
							message->source_id()));

			std::shared_ptr<container::value_container> container
				= message->copy(false);
			container->swap_header();
			container->set_message_type(MESSAGE_SENDING_RESPONSE);
			container << std::make_shared<container::string_value>(
				"indication_id",
				message->get_value("indication_id")->to_string());
			container << std::make_shared<container::string_value>(
				"requestor_id",
				message->get_value("requestor_id")->to_string());
			container << std::make_shared<container::string_value>(
				"requestor_sub_id",
				message->get_value("requestor_sub_id")->to_string());
			container << std::make_shared<container::string_value>(
				"message_type", message->message_type());
			container << std::make_shared<container::string_value>(
				"message", fmt::format("attempt to send message to {}",
									   message->target_id()));
			container << std::make_shared<container::bool_value>("response",
																 sent);

			send(container);
		}
	}

	void messaging_server::received_binary(const std::string& source_id,
										   const std::string& source_sub_id,
										   const std::string& target_id,
										   const std::string& target_sub_id,
										   const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (received_data_)
		{
			received_data_(source_id, source_sub_id, target_id, target_sub_id,
						   data);
		}
	}
} // namespace network
