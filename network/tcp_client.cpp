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
#include "file_handling.h"

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
	using namespace file_handling;

	tcp_client::tcp_client(const std::wstring& source_id)
		: data_handling(246, 135), _confirm(false), _auto_echo(false), _compress_mode(false), _encrypt_mode(false), _bridge_line(false),
		_io_context(nullptr), _socket(nullptr), _key(L""), _iv(L""), _thread_pool(nullptr), _auto_echo_interval_seconds(1), _connection(nullptr),
		_connection_key(L"connection_key"), _source_id(source_id), _source_sub_id(L""), _target_id(L""), _target_sub_id(L""), _received_file(nullptr),
		_received_message(nullptr), _received_data(nullptr)
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

	void tcp_client::set_connection_key(const std::wstring& connection_key)
	{
		_connection_key = connection_key;
	}

	void tcp_client::set_connection_notification(const std::function<void(const std::wstring&, const std::wstring&, const bool&)>& notification)
	{
		_connection = notification;
	}

	void tcp_client::set_message_notification(const std::function<void(std::shared_ptr<container::value_container>)>& notification)
	{
		_received_message = notification;
	}

	void tcp_client::set_file_notification(const std::function<void(const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&)>& notification)
	{
		_received_file = notification;
	}

	void tcp_client::set_binary_notification(const std::function<void(const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&, const std::vector<unsigned char>&)>& notification)
	{
		_received_data = notification;
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

#ifdef ASIO_STANDALONE
		_io_context = std::make_shared<asio::io_context>();
#else
		_io_context = std::make_shared<boost::asio::io_context>();
#endif

		logger::handle().write(logging::logging_level::sequence, L"attempts to create socket");

		try
		{
#ifdef ASIO_STANDALONE
			_socket = std::make_shared<asio::ip::tcp::socket>(*_io_context);
			_socket->open(asio::ip::tcp::v4());
			_socket->bind(asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 0));
			_socket->connect(asio::ip::tcp::endpoint(asio::ip::address::from_string(converter::to_string(ip)), port));

			_socket->set_option(asio::ip::tcp::no_delay(true));
			_socket->set_option(asio::socket_base::keep_alive(true));
			_socket->set_option(asio::socket_base::receive_buffer_size(buffer_size));
#else
			_socket = std::make_shared<boost::asio::ip::tcp::socket>(*_io_context);
			_socket->open(boost::asio::ip::tcp::v4());
			_socket->bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 0));
			_socket->connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(converter::to_string(ip)), port));

			_socket->set_option(boost::asio::ip::tcp::no_delay(true));
			_socket->set_option(boost::asio::socket_base::keep_alive(true));
			_socket->set_option(boost::asio::socket_base::receive_buffer_size(buffer_size));
#endif
		}
		catch (const std::overflow_error&) {
			if (_connection != nullptr)
			{
				_connection(_target_id, _target_sub_id, false);
			}
			return; 
		}
		catch (const std::runtime_error&) {
			if (_connection != nullptr)
			{
				_connection(_target_id, _target_sub_id, false);
			}
			return;
		}
		catch (const std::exception&) {
			if (_connection != nullptr)
			{
				_connection(_target_id, _target_sub_id, false);
			}
			return;
		}
		catch (...) {
			if (_connection != nullptr)
			{
				_connection(_target_id, _target_sub_id, false);
			}
			return;
		}

		_source_sub_id = fmt::format(L"{}:{}",
			converter::to_wstring(_socket->local_endpoint().address().to_string()), _socket->local_endpoint().port());
		_target_sub_id = fmt::format(L"{}:{}",
			converter::to_wstring(_socket->remote_endpoint().address().to_string()), _socket->remote_endpoint().port());

#ifdef ASIO_STANDALONE
		_thread = std::thread([](std::shared_ptr<asio::io_context> context)
#else
		_thread = boost::thread([](std::shared_ptr<boost::asio::io_context> context)
#endif
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
		if (_socket != nullptr)
		{
			if (_socket->is_open())
			{
				_socket->close();
			}
			_socket.reset();
		}

		if (_io_context != nullptr)
		{
			_io_context->stop();
			_io_context.reset();
		}

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

	void tcp_client::echo(void)
	{
		std::shared_ptr<container::value_container> container = std::make_shared<container::value_container>(_source_id, _source_sub_id, _target_id, _target_sub_id, L"echo",
			std::vector<std::shared_ptr<container::value>> {});

		send(container);
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

		if (_socket == nullptr)
		{
			return;
		}

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

	void tcp_client::send(const std::wstring target_id, const std::wstring& target_sub_id, const std::vector<unsigned char>& data)
	{
		std::vector<unsigned char> result;
		append_data_on_file_packet(result, converter::to_array(_source_id));
		append_data_on_file_packet(result, converter::to_array(_source_sub_id));
		append_data_on_file_packet(result, converter::to_array(target_id));
		append_data_on_file_packet(result, converter::to_array(target_sub_id));
		append_data_on_file_packet(result, data);

		if (_compress_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, result, std::bind(&tcp_client::compress_binary, this, std::placeholders::_1)));

			return;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, result, std::bind(&tcp_client::encrypt_binary, this, std::placeholders::_1)));

			return;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, result, std::bind(&tcp_client::send_binary, this, std::placeholders::_1)));
	}

	void tcp_client::send_connection(void)
	{
		std::shared_ptr<container::container_value> snipping_targets = std::make_shared<container::container_value>(L"snipping_targets");
		for (auto& snipping_target : _snipping_targets)
		{
			snipping_targets->add(std::make_shared<container::string_value>(L"snipping_target", snipping_target));
		}

		std::shared_ptr<container::value_container> container = std::make_shared<container::value_container>(_source_id, _source_sub_id, _target_id, _target_sub_id, L"request_connection",
			std::vector<std::shared_ptr<container::value>> {
				std::make_shared<container::string_value>(L"connection_key", _connection_key),
				std::make_shared<container::bool_value>(L"auto_echo", _auto_echo),
				std::make_shared<container::ushort_value>(L"auto_echo_interval_seconds", _auto_echo_interval_seconds),
				std::make_shared<container::short_value>(L"session_type", (short)_session_type),
				std::make_shared<container::bool_value>(L"bridge_mode", _bridge_line),
				snipping_targets
		});

		send(container);
	}

	void tcp_client::receive_on_tcp(const data_modes& data_mode, const std::vector<unsigned char>& data)
	{
		switch (data_mode)
		{
		case data_modes::file_mode: decrypt_file(data); break;
		case data_modes::packet_mode: decrypt_packet(data); break;
		}
	}

	void tcp_client::disconnected(void)
	{
		stop();

		if (_connection != nullptr)
		{
			_connection(_target_id, _target_sub_id, false);
		}
	}

	bool tcp_client::compress_packet(const std::vector<unsigned char>& data)
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

	bool tcp_client::encrypt_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, encryptor::encryption(data, _key, _iv), std::bind(&tcp_client::send_packet, this, std::placeholders::_1)));

		return true;
	}

	bool tcp_client::send_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		return send_on_tcp(_socket, data_modes::packet_mode, data);
	}

	bool tcp_client::decompress_packet(const std::vector<unsigned char>& data)
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

	bool tcp_client::decrypt_packet(const std::vector<unsigned char>& data)
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

	bool tcp_client::receive_packet(const std::vector<unsigned char>& data)
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

	bool tcp_client::load_file(const std::vector<unsigned char>& data)
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

		std::vector<unsigned char> result;
		append_data_on_file_packet(result, converter::to_array(message->get_value(L"indication_id")->to_string()));
		append_data_on_file_packet(result, converter::to_array(message->source_id()));
		append_data_on_file_packet(result, converter::to_array(message->source_sub_id()));
		append_data_on_file_packet(result, converter::to_array(message->target_id()));
		append_data_on_file_packet(result, converter::to_array(message->target_sub_id()));
		append_data_on_file_packet(result, converter::to_array(message->get_value(L"source")->to_string()));
		append_data_on_file_packet(result, converter::to_array(message->get_value(L"target")->to_string()));
		append_data_on_file_packet(result, file_handler::load(message->get_value(L"source")->to_string()));

		if (_compress_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, result, std::bind(&tcp_client::compress_file, this, std::placeholders::_1)));

			return true;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, result, std::bind(&tcp_client::encrypt_file, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, result, std::bind(&tcp_client::send_file, this, std::placeholders::_1)));

		return true;
	}

	bool tcp_client::compress_file(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, compressor::compression(data), std::bind(&tcp_client::encrypt_file, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, compressor::compression(data), std::bind(&tcp_client::send_file, this, std::placeholders::_1)));

		return true;
	}

	bool tcp_client::encrypt_file(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, encryptor::encryption(data, _key, _iv), std::bind(&tcp_client::send_file, this, std::placeholders::_1)));

		return true;
	}

	bool tcp_client::send_file(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		return send_on_tcp(_socket, data_modes::file_mode, data);
	}

	bool tcp_client::decompress_file(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_compress_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, compressor::decompression(data), std::bind(&tcp_client::receive_file, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::high, data, std::bind(&tcp_client::receive_file, this, std::placeholders::_1)));

		return true;
	}

	bool tcp_client::decrypt_file(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::high, encryptor::decryption(data, _key, _iv), std::bind(&tcp_client::decompress_file, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::high, data, std::bind(&tcp_client::decompress_file, this, std::placeholders::_1)));

		return true;
	}

	bool tcp_client::receive_file(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		size_t index = 0;
		std::wstring indication_id = converter::to_wstring(devide_data_on_file_packet(data, index));
		std::wstring source_id = converter::to_wstring(devide_data_on_file_packet(data, index));
		std::wstring source_sub_id = converter::to_wstring(devide_data_on_file_packet(data, index));
		std::wstring target_id = converter::to_wstring(devide_data_on_file_packet(data, index));
		std::wstring target_sub_id = converter::to_wstring(devide_data_on_file_packet(data, index));
		std::wstring source_path = converter::to_wstring(devide_data_on_file_packet(data, index));
		std::wstring target_path = converter::to_wstring(devide_data_on_file_packet(data, index));
		if (file_handler::save(target_path, devide_data_on_file_packet(data, index)))
		{
			if(_received_file)
			{
				_received_file(source_id, source_sub_id, indication_id, target_path);
			}
		}

		return true;
	}

	bool tcp_client::compress_binary(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, compressor::compression(data), std::bind(&tcp_client::encrypt_binary, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, compressor::compression(data), std::bind(&tcp_client::send_binary, this, std::placeholders::_1)));

		return true;
	}

	bool tcp_client::encrypt_binary(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, encryptor::encryption(data, _key, _iv), std::bind(&tcp_client::send_binary, this, std::placeholders::_1)));

		return true;
	}

	bool tcp_client::send_binary(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		return send_on_tcp(_socket, data_modes::binary_mode, data);
	}

	bool tcp_client::decompress_binary(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_compress_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, compressor::decompression(data), std::bind(&tcp_client::receive_binary, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::high, data, std::bind(&tcp_client::receive_binary, this, std::placeholders::_1)));

		return true;
	}

	bool tcp_client::decrypt_binary(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::high, encryptor::decryption(data, _key, _iv), std::bind(&tcp_client::decompress_binary, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::high, data, std::bind(&tcp_client::decompress_binary, this, std::placeholders::_1)));

		return true;
	}

	bool tcp_client::receive_binary(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		size_t index = 0;
		std::wstring source_id = converter::to_wstring(devide_data_on_file_packet(data, index));
		std::wstring source_sub_id = converter::to_wstring(devide_data_on_file_packet(data, index));
		std::wstring target_id = converter::to_wstring(devide_data_on_file_packet(data, index));
		std::wstring target_sub_id = converter::to_wstring(devide_data_on_file_packet(data, index));
		std::vector<unsigned char> target_data = devide_data_on_file_packet(data, index);
		if (_received_data)
		{
			_received_data(source_id, source_sub_id, target_id, target_sub_id, target_data);
		}

		return true;
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

		if (_received_message)
		{
			_received_message(message);
		}

		return true;
	}

	bool tcp_client::confirm_message(std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return false;
		}

		if (!message->get_value(L"confirm")->to_boolean())
		{
			if (_connection)
			{
				_connection(message->source_id(), message->source_sub_id(), false);
			}

			return false;
		}

		_confirm = true;
		_key = message->get_value(L"key")->to_string();
		_iv = message->get_value(L"iv")->to_string();
		_encrypt_mode = message->get_value(L"encrypt_mode")->to_boolean();

		if (_connection)
		{
			_connection(message->source_id(), message->source_sub_id(), true);
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

		std::vector<std::shared_ptr<value>> response = (*message)[L"response"];
		if (!response.empty())
		{
			logger::handle().write(logging::logging_level::information, fmt::format(L"received echo: {}", message->serialize()));

			return true;
		}

		message->swap_header();

		message << std::make_shared<bool_value>(L"response", true);

		_thread_pool->push(std::make_shared<job>(priorities::top, message->serialize_array(), std::bind(&tcp_client::send_packet, this, std::placeholders::_1)));

		return true;
	}
}