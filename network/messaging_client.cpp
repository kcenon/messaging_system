#include "messaging_client.h"

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

	messaging_client::messaging_client(const std::wstring& source_id)
		: data_handling(246, 135), _confirm(false), _auto_echo(false), _compress_mode(false), _encrypt_mode(false), _bridge_line(false),
		_io_context(nullptr), _socket(nullptr), _key(L""), _iv(L""), _thread_pool(nullptr), _auto_echo_interval_seconds(1), _connection(nullptr),
		_connection_key(L"connection_key"), _source_id(source_id), _source_sub_id(L""), _target_id(L"unknown"), _target_sub_id(L"0.0.0.0:0"), _received_file(nullptr),
		_received_message(nullptr), _received_data(nullptr), _session_type(session_types::message_line)
	{
		_message_handlers.insert({ L"confirm_connection", std::bind(&messaging_client::confirm_message, this, std::placeholders::_1) });
		_message_handlers.insert({ L"echo", std::bind(&messaging_client::echo_message, this, std::placeholders::_1) });
	}

	messaging_client::~messaging_client(void)
	{
		stop();
	}

	std::shared_ptr<messaging_client> messaging_client::get_ptr(void)
	{
		return shared_from_this();
	}

	std::wstring messaging_client::source_id(void) const
	{
		return _source_id;
	}

	std::wstring messaging_client::source_sub_id(void) const
	{
		return _source_sub_id;
	}

	void messaging_client::set_auto_echo(const bool& auto_echo, const unsigned short& echo_interval)
	{
		_auto_echo = auto_echo;
		_auto_echo_interval_seconds = echo_interval;
	}

	void messaging_client::set_bridge_line(const bool& bridge_line)
	{
		_bridge_line = bridge_line;
	}

	void messaging_client::set_compress_mode(const bool& compress_mode)
	{
		_compress_mode = compress_mode;
	}

	void messaging_client::set_session_types(const session_types& session_type)
	{
		_session_type = session_type;
	}

	void messaging_client::set_connection_key(const std::wstring& connection_key)
	{
		_connection_key = connection_key;
	}

	void messaging_client::set_snipping_targets(const std::vector<std::wstring>& snipping_targets)
	{
		_snipping_targets = snipping_targets;
	}

	void messaging_client::set_connection_notification(const std::function<void(const std::wstring&, const std::wstring&, const bool&)>& notification)
	{
		_connection = notification;
	}

	void messaging_client::set_message_notification(const std::function<void(std::shared_ptr<container::value_container>)>& notification)
	{
		_received_message = notification;
	}

	void messaging_client::set_file_notification(const std::function<void(const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&)>& notification)
	{
		_received_file = notification;
	}

	void messaging_client::set_binary_notification(const std::function<void(const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&, const std::vector<unsigned char>&)>& notification)
	{
		_received_data = notification;
	}

	bool messaging_client::is_confirmed(void) const
	{
		return _confirm;
	}

	void messaging_client::start(const std::wstring& ip, const unsigned short& port, const unsigned short& high_priority, const unsigned short& normal_priority, const unsigned short& low_priority)
	{
		stop();

		_thread_pool = std::make_shared<threads::thread_pool>();

		_thread_pool->append(std::make_shared<thread_worker>(priorities::top), true);
		for (unsigned short high = 0; high < high_priority; ++high)
		{
			_thread_pool->append(std::make_shared<thread_worker>(priorities::high, std::vector<priorities> { priorities::normal, priorities::low }), true);
		}
		for (unsigned short normal = 0; normal < normal_priority; ++normal)
		{
			_thread_pool->append(std::make_shared<thread_worker>(priorities::normal, std::vector<priorities> { priorities::high, priorities::low }), true);
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
			connection_notification(false);
			return; 
		}
		catch (const std::runtime_error&) {
			connection_notification(false);
			return;
		}
		catch (const std::exception&) {
			connection_notification(false);
			return;
		}
		catch (...) {
			connection_notification(false);
			return;
		}

		_source_sub_id = fmt::format(L"{}:{}",
			converter::to_wstring(_socket->local_endpoint().address().to_string()), _socket->local_endpoint().port());
		_target_sub_id = fmt::format(L"{}:{}",
			converter::to_wstring(_socket->remote_endpoint().address().to_string()), _socket->remote_endpoint().port());

#ifdef ASIO_STANDALONE
		_thread = std::thread([this](std::shared_ptr<asio::io_context> context)
#else
		_thread = boost::thread([this](std::shared_ptr<boost::asio::io_context> context)
#endif
			{
				try
				{
					logger::handle().write(logging::logging_level::information, fmt::format(L"start messaging_client({})", _source_id));
					context->run();
					logger::handle().write(logging::logging_level::information, fmt::format(L"stop messaging_client({})", _source_id));
				}
				catch (const std::overflow_error&) { 
					if (_socket != nullptr) {
						logger::handle().write(logging::logging_level::exception, fmt::format(L"break messaging_client({}) with overflow error", _source_id));
					}
				}
				catch (const std::runtime_error&) { 
					if (_socket != nullptr) {
						logger::handle().write(logging::logging_level::exception, fmt::format(L"break messaging_client({}) with runtime error", _source_id));
					}
				}
				catch (const std::exception&) { 
					if (_socket != nullptr) {
						logger::handle().write(logging::logging_level::exception, fmt::format(L"break messaging_client({}) with exception", _source_id));
					}
				}
				catch (...) { 
					if (_socket != nullptr) {
						logger::handle().write(logging::logging_level::exception, fmt::format(L"break messaging_client({}) with error", _source_id));
					}
				}
				connection_notification(false);
			}, _io_context);

		read_start_code(_socket);
		send_connection();
	}

	void messaging_client::stop(void)
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

	void messaging_client::echo(void)
	{
		std::shared_ptr<container::value_container> container = std::make_shared<container::value_container>(_source_id, _source_sub_id, _target_id, _target_sub_id, L"echo",
			std::vector<std::shared_ptr<container::value>> {});

		send(container);
	}

	void messaging_client::send(const container::value_container& message)
	{
		send(std::make_shared<container::value_container>(message));
	}

	void messaging_client::send(std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}

		if (_socket == nullptr)
		{
			return;
		}

		if (message->source_id().empty())
		{
			message->set_source(_source_id, _source_sub_id);
		}

		if (_compress_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::high, message->serialize_array(), std::bind(&messaging_client::compress_packet, this, std::placeholders::_1)));

			return;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, message->serialize_array(), std::bind(&messaging_client::encrypt_packet, this, std::placeholders::_1)));

			return;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, message->serialize_array(), std::bind(&messaging_client::send_packet, this, std::placeholders::_1)));
	}

	void messaging_client::send_files(const container::value_container& message)
	{
		send_files(std::make_shared<container::value_container>(message));
	}

	void messaging_client::send_files(std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}

		if (_session_type != session_types::file_line)
		{
			return;
		}

		if (message->source_id().empty())
		{
			message->set_source(_source_id, _source_sub_id);
		}

		std::shared_ptr<container::value_container> container = message->copy(false);
		container->swap_header();
		container->set_message_type(L"request_file");

		std::vector<std::shared_ptr<container::value>> files = message->value_array(L"file");
		for (auto& file : files)
		{
			container << std::make_shared<container::string_value>(L"indication_id", message->get_value(L"indication_id")->to_string());
			container << std::make_shared<container::string_value>(L"source", (*file)[L"source"]->to_string());
			container << std::make_shared<container::string_value>(L"target", (*file)[L"target"]->to_string());

			_thread_pool->push(std::make_shared<job>(priorities::low, container->serialize_array(), std::bind(&messaging_client::load_file_packet, this, std::placeholders::_1)));
			container->clear_value();
		}
	}

	void messaging_client::send_binary(const std::wstring target_id, const std::wstring& target_sub_id, const std::vector<unsigned char>& data)
	{
		if (_session_type != session_types::binary_line)
		{
			return;
		}

		std::vector<unsigned char> result;
		append_binary_on_packet(result, converter::to_array(_source_id));
		append_binary_on_packet(result, converter::to_array(_source_sub_id));
		append_binary_on_packet(result, converter::to_array(target_id));
		append_binary_on_packet(result, converter::to_array(target_sub_id));
		append_binary_on_packet(result, data);

		if (_compress_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, result, std::bind(&messaging_client::compress_binary_packet, this, std::placeholders::_1)));

			return;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, result, std::bind(&messaging_client::encrypt_binary_packet, this, std::placeholders::_1)));

			return;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, result, std::bind(&messaging_client::send_binary_packet, this, std::placeholders::_1)));
	}

	void messaging_client::send_connection(void)
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

	void messaging_client::receive_on_tcp(const data_modes& data_mode, const std::vector<unsigned char>& data)
	{
		switch (data_mode)
		{
		case data_modes::packet_mode:
			_thread_pool->push(std::make_shared<job>(priorities::high, data, std::bind(&messaging_client::decrypt_packet, this, std::placeholders::_1))); 
			break;
		case data_modes::file_mode:
			_thread_pool->push(std::make_shared<job>(priorities::high, data, std::bind(&messaging_client::decrypt_file_packet, this, std::placeholders::_1)));
			break;
		}
	}

	void messaging_client::disconnected(void)
	{
		stop();

		connection_notification(false);
	}

	bool messaging_client::compress_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, compressor::compression(data), std::bind(&messaging_client::encrypt_packet, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, compressor::compression(data), std::bind(&messaging_client::send_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_client::encrypt_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, encryptor::encryption(data, _key, _iv), std::bind(&messaging_client::send_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_client::send_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		return send_on_tcp(_socket, data_modes::packet_mode, data);
	}

	bool messaging_client::decompress_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_compress_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, compressor::decompression(data), std::bind(&messaging_client::receive_packet, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::high, data, std::bind(&messaging_client::receive_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_client::decrypt_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::high, encryptor::decryption(data, _key, _iv), std::bind(&messaging_client::decompress_packet, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::high, data, std::bind(&messaging_client::decompress_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_client::receive_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		std::shared_ptr<container::value_container> message = std::make_shared<container::value_container>(data, true);
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

	bool messaging_client::load_file_packet(const std::vector<unsigned char>& data)
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
		append_binary_on_packet(result, converter::to_array(message->get_value(L"indication_id")->to_string()));
		append_binary_on_packet(result, converter::to_array(message->source_id()));
		append_binary_on_packet(result, converter::to_array(message->source_sub_id()));
		append_binary_on_packet(result, converter::to_array(message->target_id()));
		append_binary_on_packet(result, converter::to_array(message->target_sub_id()));
		append_binary_on_packet(result, converter::to_array(message->get_value(L"source")->to_string()));
		append_binary_on_packet(result, converter::to_array(message->get_value(L"target")->to_string()));
		append_binary_on_packet(result, file_handler::load(message->get_value(L"source")->to_string()));

		if (_compress_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, result, std::bind(&messaging_client::compress_file_packet, this, std::placeholders::_1)));

			return true;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, result, std::bind(&messaging_client::encrypt_file_packet, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, result, std::bind(&messaging_client::send_file_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_client::compress_file_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, compressor::compression(data), std::bind(&messaging_client::encrypt_file_packet, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, compressor::compression(data), std::bind(&messaging_client::send_file_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_client::encrypt_file_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, encryptor::encryption(data, _key, _iv), std::bind(&messaging_client::send_file_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_client::send_file_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		return send_on_tcp(_socket, data_modes::file_mode, data);
	}

	bool messaging_client::decompress_file_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_compress_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::low, compressor::decompression(data), std::bind(&messaging_client::receive_file_packet, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::low, data, std::bind(&messaging_client::receive_file_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_client::decrypt_file_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, encryptor::decryption(data, _key, _iv), std::bind(&messaging_client::decompress_file_packet, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::normal, data, std::bind(&messaging_client::decompress_file_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_client::receive_file_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}
		size_t index = 0;
		std::wstring indication_id = converter::to_wstring(devide_binary_on_packet(data, index));
		std::wstring source_id = converter::to_wstring(devide_binary_on_packet(data, index));
		std::wstring source_sub_id = converter::to_wstring(devide_binary_on_packet(data, index));
		std::wstring target_id = converter::to_wstring(devide_binary_on_packet(data, index));
		std::wstring target_sub_id = converter::to_wstring(devide_binary_on_packet(data, index));
		std::wstring source_path = converter::to_wstring(devide_binary_on_packet(data, index));
		std::wstring target_path = converter::to_wstring(devide_binary_on_packet(data, index));

		std::vector<unsigned char> result;
		append_binary_on_packet(result, converter::to_array(indication_id));
		append_binary_on_packet(result, converter::to_array(target_id));
		append_binary_on_packet(result, converter::to_array(target_sub_id));
		if (file_handler::save(target_path, devide_binary_on_packet(data, index)))
		{
			append_binary_on_packet(result, converter::to_array(target_path));
		}
		else
		{
			append_binary_on_packet(result, converter::to_array(L""));
		}

		_thread_pool->push(std::make_shared<job>(priorities::high, result, std::bind(&messaging_client::notify_file_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_client::notify_file_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		size_t index = 0;
		std::wstring indication_id = converter::to_wstring(devide_binary_on_packet(data, index));
		std::wstring target_id = converter::to_wstring(devide_binary_on_packet(data, index));
		std::wstring target_sub_id = converter::to_wstring(devide_binary_on_packet(data, index));
		std::wstring target_path = converter::to_wstring(devide_binary_on_packet(data, index));

		if (_received_file)
		{
			_received_file(target_id, target_sub_id, indication_id, target_path);
		}

		return true;
	}

	bool messaging_client::compress_binary_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, compressor::compression(data), std::bind(&messaging_client::encrypt_binary_packet, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, compressor::compression(data), std::bind(&messaging_client::send_binary_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_client::encrypt_binary_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, encryptor::encryption(data, _key, _iv), std::bind(&messaging_client::send_binary_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_client::send_binary_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		return send_on_tcp(_socket, data_modes::binary_mode, data);
	}

	bool messaging_client::decompress_binary_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_compress_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, compressor::decompression(data), std::bind(&messaging_client::receive_binary_packet, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::high, data, std::bind(&messaging_client::receive_binary_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_client::decrypt_binary_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::high, encryptor::decryption(data, _key, _iv), std::bind(&messaging_client::decompress_binary_packet, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::high, data, std::bind(&messaging_client::decompress_binary_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_client::receive_binary_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		size_t index = 0;
		std::wstring source_id = converter::to_wstring(devide_binary_on_packet(data, index));
		std::wstring source_sub_id = converter::to_wstring(devide_binary_on_packet(data, index));
		std::wstring target_id = converter::to_wstring(devide_binary_on_packet(data, index));
		std::wstring target_sub_id = converter::to_wstring(devide_binary_on_packet(data, index));
		std::vector<unsigned char> target_data = devide_binary_on_packet(data, index);
		if (_received_data)
		{
			_received_data(source_id, source_sub_id, target_id, target_sub_id, target_data);
		}

		return true;
	}

	bool messaging_client::normal_message(std::shared_ptr<container::value_container> message)
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

	bool messaging_client::confirm_message(std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return false;
		}

		_target_id = message->source_id();

		if (!message->get_value(L"confirm")->to_boolean())
		{
			connection_notification(false);

			return false;
		}

		_confirm = true;
		_key = message->get_value(L"key")->to_string();
		_iv = message->get_value(L"iv")->to_string();
		_encrypt_mode = message->get_value(L"encrypt_mode")->to_boolean();

		std::vector<std::shared_ptr<value>> snipping_targets = message->get_value(L"snipping_targets")->children();
		for (auto& snipping_target : snipping_targets)
		{
			if (snipping_target == nullptr)
			{
				continue;
			}

			if (snipping_target->name() != L"snipping_target")
			{
				continue;
			}

			logger::handle().write(logging::logging_level::information, fmt::format(L"accepted snipping target: {}", snipping_target->to_string()));
		}

		connection_notification(true);

		return true;
	}

	bool messaging_client::echo_message(std::shared_ptr<container::value_container> message)
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

		_thread_pool->push(std::make_shared<job>(priorities::top, message->serialize_array(), std::bind(&messaging_client::send_packet, this, std::placeholders::_1)));

		return true;
	}

	void messaging_client::connection_notification(const bool& condition)
	{
		if (!condition)
		{
			_confirm = false;
		}

		// Need to find out more efficient way
		std::thread thread([this](const bool& connection) 
			{
				if (_connection)
				{
					_connection(_target_id, _target_sub_id, connection);
				}
			}, condition);
		thread.detach();
	}
}