#include "tcp_session.h"

#include "values/bool_value.h"

#include "logging.h"
#include "converting.h"
#include "encrypting.h"
#include "compressing.h"
#include "thread_pool.h"
#include "thread_worker.h"
#include "job_pool.h"
#include "job.h"

#include "fmt/format.h"

namespace network
{
	using namespace logging;
	using namespace threads;
	using namespace container;
	using namespace converting;
	using namespace encrypting;
	using namespace compressing;

	tcp_session::tcp_session(const std::wstring& source_id, asio::ip::tcp::socket& socket)
		: data_handling(246, 135), _confirm(false), _compress_mode(false), _encrypt_mode(false), _bridge_line(false), _buffer_size(1024),
		_key(L""), _iv(L""), _socket(std::make_shared<asio::ip::tcp::socket>(std::move(socket))), _thread_pool(nullptr), _source_id(source_id),
		_source_sub_id(L""), _target_id(L""), _target_sub_id(L"")
	{
		_socket->set_option(asio::ip::tcp::no_delay(true));
		_socket->set_option(asio::socket_base::keep_alive(true));
		_socket->set_option(asio::socket_base::receive_buffer_size(_buffer_size));

		_source_sub_id = fmt::format(L"{}:{}",
			converter::to_wstring(_socket->local_endpoint().address().to_string()), _socket->local_endpoint().port());
		_target_sub_id = fmt::format(L"{}:{}",
			converter::to_wstring(_socket->remote_endpoint().address().to_string()), _socket->remote_endpoint().port());

		_message_handlers.insert({ L"request_connection", std::bind(&tcp_session::connection_message, this, std::placeholders::_1) });
		_message_handlers.insert({ L"echo", std::bind(&tcp_session::echo_message, this, std::placeholders::_1) });
	}

	tcp_session::~tcp_session(void)
	{
		stop();
	}

	std::shared_ptr<tcp_session> tcp_session::get_ptr(void)
	{
		return shared_from_this();
	}

	void tcp_session::start(const unsigned short& high_priority, const unsigned short& normal_priority, const unsigned short& low_priority)
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

		read_start_code(_socket);

		logger::handle().write(logging::logging_level::information, fmt::format(L"started session: {}:{}", 
			converter::to_wstring(_socket->remote_endpoint().address().to_string()), _socket->remote_endpoint().port()));
	}

	void tcp_session::stop(void)
	{
		if (_thread_pool != nullptr)
		{
			_thread_pool->stop();
			_thread_pool.reset();
		}
	}

	void tcp_session::send(std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}

		if (_bridge_line)
		{
			if (_compress_mode)
			{
				_thread_pool->push(std::make_shared<job>(priorities::high, message->serialize_array(), std::bind(&tcp_session::compress_packet, this, std::placeholders::_1)));

				return;
			}

			if (_encrypt_mode)
			{
				_thread_pool->push(std::make_shared<job>(priorities::normal, message->serialize_array(), std::bind(&tcp_session::encrypt_packet, this, std::placeholders::_1)));

				return;
			}

			_thread_pool->push(std::make_shared<job>(priorities::top, message->serialize_array(), std::bind(&tcp_session::send_packet, this, std::placeholders::_1)));

			return;
		}

		if (message->target_id() != _target_id)
		{
			return;
		}

		if (!message->target_sub_id().empty() && message->target_sub_id() != _target_sub_id)
		{
			return;
		}

		if (_compress_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::high, message->serialize_array(), std::bind(&tcp_session::compress_packet, this, std::placeholders::_1)));

			return;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, message->serialize_array(), std::bind(&tcp_session::encrypt_packet, this, std::placeholders::_1)));

			return;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, message->serialize_array(), std::bind(&tcp_session::send_packet, this, std::placeholders::_1)));
	}

	void tcp_session::receive_on_tcp(const data_modes& data_mode, const std::vector<char>& data)
	{
		switch (data_mode)
		{
		case data_modes::packet_mode: decrypt_packet(data); break;
		}
	}

	bool tcp_session::compress_packet(const std::vector<char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, compressor::compression(data), std::bind(&tcp_session::encrypt_packet, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, compressor::compression(data), std::bind(&tcp_session::send_packet, this, std::placeholders::_1)));

		return true;
	}

	bool tcp_session::encrypt_packet(const std::vector<char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, encryptor::encryption(data, _key, _iv), std::bind(&tcp_session::send_packet, this, std::placeholders::_1)));

		return true;
	}

	bool tcp_session::send_packet(const std::vector<char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		return send_on_tcp(_socket, data_modes::packet_mode, data);
	}

	bool tcp_session::decompress_packet(const std::vector<char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_compress_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, compressor::decompression(data), std::bind(&tcp_session::receive_packet, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::high, data, std::bind(&tcp_session::receive_packet, this, std::placeholders::_1)));

		return true;
	}

	bool tcp_session::decrypt_packet(const std::vector<char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::high, encryptor::decryption(data, _key, _iv), std::bind(&tcp_session::decompress_packet, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::high, data, std::bind(&tcp_session::decompress_packet, this, std::placeholders::_1)));

		return true;
	}

	bool tcp_session::receive_packet(const std::vector<char>& data)
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

	bool tcp_session::normal_message(std::shared_ptr<container::value_container> message)
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

	bool tcp_session::connection_message(std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return false;
		}

		logger::handle().write(logging::logging_level::information, fmt::format(L"connection_message: {}", message->serialize()));

		_target_id = message->source_id();

		message->swap_header();
		message->set_source(_source_id, _source_sub_id);
		message->set_message_type(L"confirm");

		send(message);

		return true;
	}

	bool tcp_session::echo_message(std::shared_ptr<container::value_container> message)
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

		_thread_pool->push(std::make_shared<job>(priorities::top, message->serialize_array(), std::bind(&tcp_session::send_packet, this, std::placeholders::_1)));

		return true;
	}
}
