#include "tcp_client.h"

#include "values/bool_value.h"

#include "logging.h"
#include "converting.h"
#include "encrypting.h"
#include "compressing.h"
#include "thread_worker.h"
#include "job_pool.h"
#include "job.h"

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

	tcp_client::tcp_client(void) 
		: _confirm(false), _compress_mode(false), _encrypt_mode(false), _bridge_line(false), _io_context(nullptr), _socket(nullptr), _buffer_size(1024),
		_key(L""), _iv(L""), _thread_pool(nullptr)
	{
		_message_handlers.insert({ L"confirm", std::bind(&tcp_client::confirm_message, this, std::placeholders::_1) });
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
		_socket->set_option(asio::socket_base::receive_buffer_size(_buffer_size));

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

		logger::handle().write(logging::logging_level::sequence, fmt::format(L"attempts to send message: {}", message->message_type()));

		if (_compress_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::high, message->serialize_array(), std::bind(&tcp_client::compress_packet, this, std::placeholders::_1)));

			return;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, message->serialize_array(), std::bind(&tcp_client::send_packet, this, std::placeholders::_1)));
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

		return true;
	}

	bool tcp_client::decompress_packet(const std::vector<char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, compressor::decompression(data), std::bind(&tcp_client::decrypt_packet, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::high, compressor::decompression(data), std::bind(&tcp_client::receive_packet, this, std::placeholders::_1)));

		return true;
	}

	bool tcp_client::decrypt_packet(const std::vector<char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		_thread_pool->push(std::make_shared<job>(priorities::high, encryptor::decryption(data, _key, _iv), std::bind(&tcp_client::receive_packet, this, std::placeholders::_1)));

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

		return true;
	}

	bool tcp_client::confirm_message(std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return false;
		}

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

		std::shared_ptr<value> response = (*message)[L"response"];
		if (!response->is_null())
		{
			return true;
		}

		message->swap_header();

		message << std::make_shared<bool_value>(L"response", true);

		_thread_pool->push(std::make_shared<job>(priorities::top, message->serialize_array(), std::bind(&tcp_client::send_packet, this, std::placeholders::_1)));

		return true;
	}
}