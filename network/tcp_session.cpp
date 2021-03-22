﻿#include "tcp_session.h"

#include "values/bool_value.h"
#include "values/string_value.h"

#include "logging.h"
#include "converting.h"
#include "encrypting.h"
#include "compressing.h"
#include "thread_pool.h"
#include "thread_worker.h"
#include "job_pool.h"
#include "job.h"

#include "data_lengths.h"
#include "file_handling.h"

#include "fmt/format.h"

namespace network
{
	using namespace logging;
	using namespace threads;
	using namespace container;
	using namespace converting;
	using namespace encrypting;
	using namespace compressing;
	using namespace file_handling;

	tcp_session::tcp_session(const std::wstring& source_id, const std::wstring& connection_key, asio::ip::tcp::socket& socket)
		: data_handling(246, 135), _confirm(false), _compress_mode(false), _encrypt_mode(false), _bridge_line(false),
		_key(L""), _iv(L""), _socket(std::make_shared<asio::ip::tcp::socket>(std::move(socket))), _thread_pool(nullptr), _source_id(source_id),
		_source_sub_id(L""), _target_id(L""), _target_sub_id(L""), _connection_key(connection_key), _received_file(nullptr), _connection(nullptr)
	{
		_socket->set_option(asio::ip::tcp::no_delay(true));
		_socket->set_option(asio::socket_base::keep_alive(true));
		_socket->set_option(asio::socket_base::receive_buffer_size(buffer_size));

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

	void tcp_session::set_connection_notification(const std::function<void(std::shared_ptr<tcp_session>, const bool&)>& notification)
	{
		_connection = notification;
	}

	void tcp_session::set_file_notification(const std::function<void(const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&)>& notification)
	{
		_received_file = notification;
	}

	const session_types tcp_session::get_session_type(void)
	{
		return _session_type;
	}

	const std::wstring tcp_session::target_id(void)
	{
		return _target_id;
	}

	const std::wstring tcp_session::target_sub_id(void)
	{
		return _target_sub_id;
	}

	void tcp_session::start(const bool& encrypt_mode, const bool& compress_mode, const unsigned short& high_priority, const unsigned short& normal_priority, const unsigned short& low_priority)
	{
		stop();

		_encrypt_mode = encrypt_mode;
		_compress_mode = compress_mode;
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

		logger::handle().write(logging::logging_level::information, fmt::format(L"attempt to send: {}", message->serialize()));

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
		case data_modes::file_mode: decrypt_file(data); break;
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

	bool tcp_session::load_file(const std::vector<char>& data)
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

		std::vector<char> result;
		append_data(result, converter::to_array(message->get_value(L"indication_id")->to_string()));
		append_data(result, converter::to_array(message->source_id()));
		append_data(result, converter::to_array(message->source_sub_id()));
		append_data(result, converter::to_array(message->target_id()));
		append_data(result, converter::to_array(message->target_sub_id()));
		append_data(result, converter::to_array(message->get_value(L"source")->to_string()));
		append_data(result, converter::to_array(message->get_value(L"target")->to_string()));
		append_data(result, file_handler::load(message->get_value(L"source")->to_string()));

		if (_compress_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, result, std::bind(&tcp_session::compress_file, this, std::placeholders::_1)));

			return true;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, result, std::bind(&tcp_session::encrypt_file, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, result, std::bind(&tcp_session::send_file, this, std::placeholders::_1)));

		return true;
	}

	bool tcp_session::compress_file(const std::vector<char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::high, compressor::compression(data), std::bind(&tcp_session::encrypt_file, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, compressor::compression(data), std::bind(&tcp_session::send_file, this, std::placeholders::_1)));

		return true;
	}

	bool tcp_session::encrypt_file(const std::vector<char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, encryptor::encryption(data, _key, _iv), std::bind(&tcp_session::send_file, this, std::placeholders::_1)));

		return true;
	}

	bool tcp_session::send_file(const std::vector<char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		return send_on_tcp(_socket, data_modes::file_mode, data);
	}

	bool tcp_session::decompress_file(const std::vector<char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_compress_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, compressor::decompression(data), std::bind(&tcp_session::receive_file, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::high, data, std::bind(&tcp_session::receive_file, this, std::placeholders::_1)));

		return true;
	}

	bool tcp_session::decrypt_file(const std::vector<char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::high, encryptor::decryption(data, _key, _iv), std::bind(&tcp_session::decompress_file, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::high, data, std::bind(&tcp_session::decompress_file, this, std::placeholders::_1)));

		return true;
	}

	bool tcp_session::receive_file(const std::vector<char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		size_t index = 0;
		std::wstring indication_id = converter::to_wstring(devide_data(data, index));
		std::wstring source_id = converter::to_wstring(devide_data(data, index));
		std::wstring source_sub_id = converter::to_wstring(devide_data(data, index));
		std::wstring target_id = converter::to_wstring(devide_data(data, index));
		std::wstring target_sub_id = converter::to_wstring(devide_data(data, index));
		std::wstring source_path = converter::to_wstring(devide_data(data, index));
		std::wstring target_path = converter::to_wstring(devide_data(data, index));
		if (file_handler::save(target_path, devide_data(data, index)))
		{
			if (_received_file)
			{
				_received_file(source_id, source_sub_id, indication_id, target_path);
			}
		}

		return true;
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
		_session_type = (session_types)message->get_value(L"session_type")->to_short();

		if (!same_key_check(message->get_value(L"connection_key")))
		{
			if (_connection)
			{
				_connection(get_ptr(), false);
			}

			return false;
		}

		if (!same_id_check())
		{
			if (_connection)
			{
				_connection(get_ptr(), false);
			}

			return false;
		}

		generate_key();

		std::shared_ptr<container::value_container> container = std::make_shared<container::value_container>(_source_id, _source_sub_id, _target_id, _target_sub_id, L"confirm_connection");

		container << std::make_shared<container::bool_value>(L"confirm", true);
		container << std::make_shared<container::string_value>(L"key", _key);
		container << std::make_shared<container::string_value>(L"iv", _iv);
		container << std::make_shared<container::bool_value>(L"encrypt_mode", _encrypt_mode);

		if (_compress_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::high, compressor::compression(container->serialize_array()), std::bind(&tcp_session::send_packet, this, std::placeholders::_1)));

			if (_connection)
			{
				_connection(get_ptr(), true);
			}

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, container->serialize_array(), std::bind(&tcp_session::send_packet, this, std::placeholders::_1)));

		if (_connection)
		{
			_connection(get_ptr(), true);
		}

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

	void tcp_session::generate_key(void)
	{
		if (!_encrypt_mode)
		{
			_key = L"";
			_iv = L"";

			return;
		}
		
		auto encrypt_key = encryptor::create_key();
		_key = encrypt_key.first;
		_iv = encrypt_key.second;
	}

	bool tcp_session::same_key_check(std::shared_ptr<container::value> key)
	{
		if (key != nullptr && _connection_key == key->to_string())
		{
			return true;
		}

		logger::handle().write(logging::logging_level::information, L"ignored this line = \"unknown connection key\"");

		std::shared_ptr<container::value_container> container = std::make_shared<container::value_container>(_source_id, _source_sub_id, _target_id, _target_sub_id, L"confirm_connection");

		container << std::make_shared<container::bool_value>(L"confirm", false);
		container << std::make_shared<container::string_value>(L"reason", L"ignored this line = \"unknown connection key\"");

		send(container);

		return false;
	}

	bool tcp_session::same_id_check(void)
	{
		if (_target_id != _source_id)
		{
			return true;
		}

		logger::handle().write(logging::logging_level::information, L"ignored this line = \"cannot use same id with server\"");

		std::shared_ptr<container::value_container> container = std::make_shared<container::value_container>(_source_id, _source_sub_id, _target_id, _target_sub_id, L"confirm_connection");

		container << std::make_shared<container::bool_value>(L"confirm", false);
		container << std::make_shared<container::string_value>(L"reason", L"ignored this line = \"cannot use same id with server\"");

		send(container);

		return false;
	}

	void tcp_session::append_data(std::vector<char>& result, const std::vector<char>& source)
	{
		size_t temp;
		const int size = sizeof(size_t);
		char temp_size[size];

		temp = source.size();
		memcpy(temp_size, &temp, size);
		result.insert(result.end(), temp_size, temp_size + size);
		result.insert(result.end(), source.begin(), source.end());
	}

	std::vector<char> tcp_session::devide_data(const std::vector<char>& source, size_t& index)
	{
		if (source.empty())
		{
			return std::vector<char>();
		}

		size_t temp;
		const int size = sizeof(size_t);

		if (source.size() < index + size)
		{
			return std::vector<char>();
		}

		memcpy(&temp, source.data() + index, size);
		index += size;

		if (source.size() < index + temp)
		{
			return std::vector<char>();
		}

		std::vector<char> result;
		result.insert(result.end(), source.begin() + index, source.begin() + index + temp);

		return result;
	}
}
