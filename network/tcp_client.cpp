#include "tcp_client.h"

#include "value.h"
#include "values/bool_value.h"
#include "values/bytes_value.h"
#include "values/short_value.h"
#include "values/ushort_value.h"
#include "values/string_value.h"
#include "values/container_value.h"

#include "logging.h"
#include "converting.h"
#include "encrypting.h"
#include "compressing.h"
#include "thread_worker.h"
#include "job_pool.h"
#include "job.h"

#include "data_lengths.h"

#include <functional>

#include "fmt/format.h"

namespace network
{
	using namespace logging;
	using namespace threads;
	using namespace container;
	using namespace converting;
	using namespace encrypting;
	using namespace compressing;

	tcp_client::tcp_client(const std::wstring& source_id, const std::wstring& connection_key)
		: data_handling(246, 135), _confirm(false), _auto_echo(false), _compress_mode(false), _encrypt_mode(false), _bridge_line(false),
		_io_context(nullptr), _socket(nullptr), _key(L""), _iv(L""), _thread_pool(nullptr), _auto_echo_interval_seconds(1),
		_connection_key(connection_key), _source_id(source_id), _source_sub_id(L""), _target_id(L""), _target_sub_id(L"")
	{
		_message_handlers.insert({ L"confirm_connection", std::bind(&tcp_client::confirm_message, this, std::placeholders::_1) });
		_message_handlers.insert({ L"echo", std::bind(&tcp_client::echo_message, this, std::placeholders::_1) });
	}

	tcp_client::~tcp_client(void)
	{
		stop();
	}

	std::shared_ptr<tcp_client> tcp_client::get_ptr(void)
	{
		return shared_from_this();
	}

	void tcp_client::set_auto_echo(const bool& auto_echo, const unsigned short& echo_interval)
	{
		_auto_echo = auto_echo;
		_auto_echo_interval_seconds = echo_interval;
	}

	void tcp_client::set_bridge_line(const bool& bridge_line)
	{
		_bridge_line = bridge_line;
	}

	void tcp_client::set_compress_mode(const bool& compress_mode)
	{
		_compress_mode = compress_mode;
	}

	void tcp_client::set_session_types(const session_types& session_type)
	{
		_session_type = session_type;
	}

	void tcp_client::start(const std::wstring& ip, const unsigned short& port, const unsigned short& high_priority, const unsigned short& normal_priority, const unsigned short& low_priority)
	{
		stop();

		_thread_pool = std::make_shared<threads::thread_pool>();

		_thread_pool->append(std::make_shared<thread_worker>(priorities::top), true);
		for (unsigned short high = 0; high < high_priority; ++high)
		{
			_thread_pool->append(std::make_shared<thread_worker>(priorities::high), true);
		}
		for (unsigned short normal = 0; normal < normal_priority; ++normal)
		{
			_thread_pool->append(std::make_shared<thread_worker>(priorities::normal, std::vector<priorities> { priorities::high }), true);
		}
		for (unsigned short low = 0; low < low_priority; ++low)
		{
			_thread_pool->append(std::make_shared<thread_worker>(priorities::low, std::vector<priorities> { priorities::high, priorities::normal }), true);
		}

		logger::handle().write(logging::logging_level::sequence, L"attempts to create io_context");
		_io_context = std::make_shared<asio::io_context>();

		logger::handle().write(logging::logging_level::sequence, L"attempts to create socket");
		_socket = std::make_shared<asio::ip::tcp::socket>(*_io_context);
		_socket->open(asio::ip::tcp::v4());
		_socket->bind(asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 0));
		_socket->connect(asio::ip::tcp::endpoint(asio::ip::address::from_string(converter::to_string(ip)), port));

		_socket->set_option(asio::ip::tcp::no_delay(true));
		_socket->set_option(asio::socket_base::keep_alive(true));
		_socket->set_option(asio::socket_base::receive_buffer_size(buffer_size));

		_source_sub_id = fmt::format(L"{}:{}",
			converter::to_wstring(_socket->local_endpoint().address().to_string()), _socket->local_endpoint().port());
		_target_sub_id = fmt::format(L"{}:{}",
			converter::to_wstring(_socket->remote_endpoint().address().to_string()), _socket->remote_endpoint().port());

		_thread = std::thread([](std::shared_ptr<asio::io_context> context)
			{
				try
				{
					logger::handle().write(logging::logging_level::information, L"start tcp_client");
					context->run();
					logger::handle().write(logging::logging_level::information, L"stop tcp_client");
				}
				catch (const std::overflow_error&) { logger::handle().write(logging::logging_level::exception, L"break tcp_client with overflow error"); }
				catch (const std::runtime_error&) { logger::handle().write(logging::logging_level::exception, L"break tcp_client with runtime error"); }
				catch (const std::exception&) { logger::handle().write(logging::logging_level::exception, L"break tcp_client with exception"); }
				catch (...) { logger::handle().write(logging::logging_level::exception, L"break tcp_client with error"); }
			}, _io_context);

		read_start_code(_socket);
		send_connection();
	}

	void tcp_client::stop(void)
	{
		if (_socket != nullptr && _socket->is_open())
		{
			_socket->close();
		}
		_socket.reset();

		if (_io_context != nullptr)
		{
			_io_context->stop();
		}
		_io_context.reset();

		if (_thread.joinable())
		{
			_thread.join();
		}

		if (_thread_pool != nullptr)
		{
			_thread_pool->stop();
			_thread_pool.reset();
		}
	}

	void tcp_client::send(const container::value_container& message)
	{
		send(std::make_shared<container::value_container>(message));
	}

	void tcp_client::send(std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}

		logger::handle().write(logging::logging_level::information, fmt::format(L"attempt to send: {}", message->serialize()));

		if (_compress_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::high, message->serialize_array(), std::bind(&tcp_client::compress_packet, this, std::placeholders::_1)));

			return;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, message->serialize_array(), std::bind(&tcp_client::encrypt_packet, this, std::placeholders::_1)));

			return;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, message->serialize_array(), std::bind(&tcp_client::send_packet, this, std::placeholders::_1)));
	}

	void tcp_client::send_connection(void)
	{
		std::shared_ptr<container::value_container> container = std::make_shared<container::value_container>(_source_id, _source_sub_id, _target_id, _target_sub_id, L"request_connection");

		container << std::make_shared<container::string_value>(L"connection_key", _connection_key);
		container << std::make_shared<container::bool_value>(L"auto_echo", _auto_echo);
		container << std::make_shared<container::ushort_value>(L"auto_echo_interval_seconds", _auto_echo_interval_seconds);
		container << std::make_shared<container::short_value>(L"session_type", (short)_session_type);
		container << std::make_shared<container::bool_value>(L"bridge_mode", _bridge_line);

		std::shared_ptr<container::container_value> snipping_targets = std::make_shared<container::container_value>(L"snipping_targets");
		for (auto& snipping_target : _snipping_targets)
		{
			snipping_targets->add(std::make_shared<container::string_value>(L"snipping_target", snipping_target));
		}
		container << snipping_targets;

		send(container);
	}

	void tcp_client::receive_on_tcp(const data_modes& data_mode, const std::vector<char>& data)
	{
		switch (data_mode)
		{
		case data_modes::packet_mode: decrypt_packet(data); break;
		}
	}

	bool tcp_client::compress_packet(const std::vector<char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, compressor::compression(data), std::bind(&tcp_client::encrypt_packet, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, compressor::compression(data), std::bind(&tcp_client::send_packet, this, std::placeholders::_1)));

		return true;
	}

	bool tcp_client::encrypt_packet(const std::vector<char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, encryptor::encryption(data, _key, _iv), std::bind(&tcp_client::send_packet, this, std::placeholders::_1)));

		return true;
	}

	bool tcp_client::send_packet(const std::vector<char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		return send_on_tcp(_socket, data_modes::packet_mode, data);
	}

	bool tcp_client::decompress_packet(const std::vector<char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_compress_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, compressor::decompression(data), std::bind(&tcp_client::receive_packet, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::high, data, std::bind(&tcp_client::receive_packet, this, std::placeholders::_1)));

		return true;
	}

	bool tcp_client::decrypt_packet(const std::vector<char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::high, encryptor::decryption(data, _key, _iv), std::bind(&tcp_client::decompress_packet, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::high, data, std::bind(&tcp_client::decompress_packet, this, std::placeholders::_1)));

		return true;
	}

	bool tcp_client::receive_packet(const std::vector<char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		std::shared_ptr<container::value_container> message = std::make_shared<container::value_container>(data);
		if (message == nullptr)
		{
			return false;
		}

		auto target = _message_handlers.find(message->message_type());
		if (target == _message_handlers.end())
		{
			return normal_message(message);
		}

		return target->second(message);
	}

	bool tcp_client::normal_message(std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return false;
		}

		if (!_confirm)
		{
			return false;
		}

		logger::handle().write(logging::logging_level::information, fmt::format(L"normal_message: {}", message->serialize()));

		return true;
	}

	bool tcp_client::confirm_message(std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return false;
		}

		logger::handle().write(logging::logging_level::information, fmt::format(L"confirm_message: {}", message->serialize()));

		if (!message->get_value(L"confirm")->to_boolean())
		{
			return false;
		}

		_confirm = true;
		_key = message->get_value(L"key")->to_string();
		_iv = message->get_value(L"iv")->to_string();
		_encrypt_mode = message->get_value(L"encrypt_mode")->to_boolean();

		return true;
	}

	bool tcp_client::echo_message(std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return false;
		}

		if (!_confirm)
		{
			return false;
		}

		std::vector<std::shared_ptr<value>> response = (*message)[L"response"];
		if (!response.empty())
		{
			return true;
		}

		message->swap_header();

		message << std::make_shared<bool_value>(L"response", true);

		_thread_pool->push(std::make_shared<job>(priorities::top, message->serialize_array(), std::bind(&tcp_client::send_packet, this, std::placeholders::_1)));

		return true;
	}
}