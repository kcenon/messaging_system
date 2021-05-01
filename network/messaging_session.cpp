#include "messaging_session.h"

#include "values/bool_value.h"
#include "values/string_value.h"
#include "values/container_value.h"

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

#ifdef ASIO_STANDALONE
	messaging_session::messaging_session(const std::wstring& source_id, const std::wstring& connection_key, asio::ip::tcp::socket& socket)
#else
	messaging_session::messaging_session(const std::wstring& source_id, const std::wstring& connection_key, boost::asio::ip::tcp::socket& socket)
#endif
		: data_handling(246, 135), _confirm(false), _compress_mode(false), _encrypt_mode(false), _bridge_line(false), _received_message(nullptr),
		_key(L""), _iv(L""), _thread_pool(nullptr), _source_id(source_id), _source_sub_id(L""), _target_id(L""), _target_sub_id(L""), 
		_connection_key(connection_key), _received_file(nullptr), _received_data(nullptr), _connection(nullptr), 

#ifdef ASIO_STANDALONE
		_socket(std::make_shared<asio::ip::tcp::socket>(std::move(socket)))
#else
		_socket(std::make_shared<boost::asio::ip::tcp::socket>(std::move(socket)))
#endif
	{
#ifdef ASIO_STANDALONE
		_socket->set_option(asio::ip::tcp::no_delay(true));
		_socket->set_option(asio::socket_base::keep_alive(true));
		_socket->set_option(asio::socket_base::receive_buffer_size(buffer_size));
#else
		_socket->set_option(boost::asio::ip::tcp::no_delay(true));
		_socket->set_option(boost::asio::socket_base::keep_alive(true));
		_socket->set_option(boost::asio::socket_base::receive_buffer_size(buffer_size));
#endif

		_source_sub_id = fmt::format(L"{}:{}",
			converter::to_wstring(_socket->local_endpoint().address().to_string()), _socket->local_endpoint().port());
		_target_sub_id = fmt::format(L"{}:{}",
			converter::to_wstring(_socket->remote_endpoint().address().to_string()), _socket->remote_endpoint().port());

		_message_handlers.insert({ L"request_connection", std::bind(&messaging_session::connection_message, this, std::placeholders::_1) });
		_message_handlers.insert({ L"request_files", std::bind(&messaging_session::request_files, this, std::placeholders::_1) });
		_message_handlers.insert({ L"echo", std::bind(&messaging_session::echo_message, this, std::placeholders::_1) });
	}

	messaging_session::~messaging_session(void)
	{
		if (_socket != nullptr)
		{
			if (_socket->is_open())
			{
				_socket->close();
			}
			_socket.reset();
		}

		stop();
	}

	std::shared_ptr<messaging_session> messaging_session::get_ptr(void)
	{
		return shared_from_this();
	}

	void messaging_session::set_connection_notification(const std::function<void(std::shared_ptr<messaging_session>, const bool&)>& notification)
	{
		_connection = notification;
	}

	void messaging_session::set_message_notification(const std::function<void(std::shared_ptr<container::value_container>)>& notification)
	{
		_received_message = notification;
	}

	void messaging_session::set_file_notification(const std::function<void(const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&)>& notification)
	{
		_received_file = notification;
	}

	void messaging_session::set_binary_notification(const std::function<void(const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&, const std::vector<unsigned char>&)>& notification)
	{
		_received_data = notification;
	}

	const session_types messaging_session::get_session_type(void)
	{
		return _session_type;
	}

	const std::wstring messaging_session::target_id(void)
	{
		return _target_id;
	}

	const std::wstring messaging_session::target_sub_id(void)
	{
		return _target_sub_id;
	}

	void messaging_session::start(const bool& encrypt_mode, const bool& compress_mode, const std::vector<std::wstring>& ignore_snipping_targets, const unsigned short& high_priority, const unsigned short& normal_priority, const unsigned short& low_priority)
	{
		stop();

		_encrypt_mode = encrypt_mode;
		_compress_mode = compress_mode;
		_ignore_snipping_targets = ignore_snipping_targets;
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

		_thread_pool->push(std::make_shared<job>(priorities::high, std::bind(&messaging_session::check_confirm_condition, this)));

		read_start_code(_socket);

		logger::handle().write(logging::logging_level::information, fmt::format(L"started session: {}:{}", 
			converter::to_wstring(_socket->remote_endpoint().address().to_string()), _socket->remote_endpoint().port()));
	}

	void messaging_session::stop(void)
	{
		if (_thread_pool != nullptr)
		{
			_thread_pool->stop();
			_thread_pool.reset();
		}
	}

	void messaging_session::echo(void)
	{
		std::shared_ptr<container::value_container> container = std::make_shared<container::value_container>(_source_id, _source_sub_id, _target_id, _target_sub_id, L"echo",
			std::vector<std::shared_ptr<container::value>> {});

		send(container);
	}

	void messaging_session::send(std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}

		if (!_bridge_line && message->target_id() != _target_id && !contained_snipping_target(message->target_id()))
		{
			return;
		}

		if (!_bridge_line && !contained_snipping_target(message->target_id()) && !message->target_sub_id().empty() && message->target_sub_id() != _target_sub_id)
		{
			return;
		}

		if (_compress_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::high, message->serialize_array(), std::bind(&messaging_session::compress_packet, this, std::placeholders::_1)));

			return;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, message->serialize_array(), std::bind(&messaging_session::encrypt_packet, this, std::placeholders::_1)));

			return;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, message->serialize_array(), std::bind(&messaging_session::send_packet, this, std::placeholders::_1)));
	}

	void messaging_session::send_files(std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}

		if (_session_type != session_types::file_line)
		{
			return;
		}

		if (_target_id != message->source_id() && _target_sub_id != message->source_sub_id())
		{
			return;
		}

		if (message->source_id().empty())
		{
			message->set_source(_source_id, _source_sub_id);
		}

		std::shared_ptr<container::value_container> container = message->copy(false);
		container->swap_header();
		container->set_target(message->get_value(L"gateway_source_id")->to_string(), message->get_value(L"gateway_source_sub_id")->to_string());
		container->set_message_type(L"request_file");

		std::vector<std::shared_ptr<container::value>> files = message->value_array(L"file");
		for (auto& file : files)
		{
			container << std::make_shared<container::string_value>(L"indication_id", message->get_value(L"indication_id")->to_string());
			container << std::make_shared<container::string_value>(L"source", (*file)[L"source"]->to_string());
			container << std::make_shared<container::string_value>(L"target", (*file)[L"target"]->to_string());

			_thread_pool->push(std::make_shared<job>(priorities::low, container->serialize_array(), std::bind(&messaging_session::load_file_packet, this, std::placeholders::_1)));
			container->clear_value();
		}
	}

	void messaging_session::send_binary(const std::wstring target_id, const std::wstring& target_sub_id, const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_session_type != session_types::binary_line)
		{
			return;
		}

		if (!_bridge_line && target_id != _target_id)
		{
			return;
		}

		if (!_bridge_line && !target_sub_id.empty() && target_id != _target_sub_id)
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
			_thread_pool->push(std::make_shared<job>(priorities::normal, result, std::bind(&messaging_session::compress_binary_packet, this, std::placeholders::_1)));

			return;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, result, std::bind(&messaging_session::encrypt_binary_packet, this, std::placeholders::_1)));

			return;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, result, std::bind(&messaging_session::send_binary_packet, this, std::placeholders::_1)));
	}

	void messaging_session::send_binary(const std::wstring source_id, const std::wstring& source_sub_id, const std::wstring target_id, const std::wstring& target_sub_id, const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_session_type != session_types::binary_line)
		{
			return;
		}

		if (!_bridge_line && target_id != _target_id)
		{
			return;
		}

		if (!_bridge_line && !target_sub_id.empty() && target_id != _target_sub_id)
		{
			return;
		}

		std::vector<unsigned char> result;
		append_binary_on_packet(result, converter::to_array(source_id));
		append_binary_on_packet(result, converter::to_array(source_sub_id));
		append_binary_on_packet(result, converter::to_array(target_id));
		append_binary_on_packet(result, converter::to_array(target_sub_id));
		append_binary_on_packet(result, data);

		if (_compress_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, result, std::bind(&messaging_session::compress_binary_packet, this, std::placeholders::_1)));

			return;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, result, std::bind(&messaging_session::encrypt_binary_packet, this, std::placeholders::_1)));

			return;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, result, std::bind(&messaging_session::send_binary_packet, this, std::placeholders::_1)));
	}

	void messaging_session::receive_on_tcp(const data_modes& data_mode, const std::vector<unsigned char>& data)
	{
		switch (data_mode)
		{
		case data_modes::packet_mode:
			_thread_pool->push(std::make_shared<job>(priorities::high, data, std::bind(&messaging_session::decrypt_packet, this, std::placeholders::_1)));
			break;
		case data_modes::file_mode:
			_thread_pool->push(std::make_shared<job>(priorities::high, data, std::bind(&messaging_session::decrypt_file_packet, this, std::placeholders::_1)));
			break;
		}

	}

	void messaging_session::disconnected(void)
	{
		stop();

		if (_connection != nullptr)
		{
			_connection(get_ptr() , false);
		}
	}

	bool messaging_session::check_confirm_condition(void)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));

		if (!_confirm)
		{
			_socket->close();
		}

		return true;
	}

	bool messaging_session::contained_snipping_target(const std::wstring& snipping_target)
	{
		auto target = std::find(_snipping_targets.begin(), _snipping_targets.end(), snipping_target);
		if (target == _snipping_targets.end())
		{
			return false;
		}

		return true;
	}

	bool messaging_session::compress_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, compressor::compression(data), std::bind(&messaging_session::encrypt_packet, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, compressor::compression(data), std::bind(&messaging_session::send_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_session::encrypt_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, encryptor::encryption(data, _key, _iv), std::bind(&messaging_session::send_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_session::send_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		return send_on_tcp(_socket, data_modes::packet_mode, data);
	}

	bool messaging_session::decompress_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_compress_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, compressor::decompression(data), std::bind(&messaging_session::receive_packet, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::high, data, std::bind(&messaging_session::receive_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_session::decrypt_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::high, encryptor::decryption(data, _key, _iv), std::bind(&messaging_session::decompress_packet, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::high, data, std::bind(&messaging_session::decompress_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_session::receive_packet(const std::vector<unsigned char>& data)
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

	bool messaging_session::load_file_packet(const std::vector<unsigned char>& data)
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
			_thread_pool->push(std::make_shared<job>(priorities::normal, result, std::bind(&messaging_session::compress_file_packet, this, std::placeholders::_1)));

			return true;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, result, std::bind(&messaging_session::encrypt_file_packet, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, result, std::bind(&messaging_session::send_file_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_session::compress_file_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::high, compressor::compression(data), std::bind(&messaging_session::encrypt_file_packet, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, compressor::compression(data), std::bind(&messaging_session::send_file_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_session::encrypt_file_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, encryptor::encryption(data, _key, _iv), std::bind(&messaging_session::send_file_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_session::send_file_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		return send_on_tcp(_socket, data_modes::file_mode, data);
	}

	bool messaging_session::decompress_file_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_compress_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::low, compressor::decompression(data), std::bind(&messaging_session::receive_file_packet, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::low , data, std::bind(&messaging_session::receive_file_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_session::decrypt_file_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, encryptor::decryption(data, _key, _iv), std::bind(&messaging_session::decompress_file_packet, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::normal, data, std::bind(&messaging_session::decompress_file_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_session::receive_file_packet(const std::vector<unsigned char>& data)
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

		_thread_pool->push(std::make_shared<job>(priorities::high, result, std::bind(&messaging_session::notify_file_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_session::notify_file_packet(const std::vector<unsigned char>& data)
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

	bool messaging_session::compress_binary_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, compressor::compression(data), std::bind(&messaging_session::encrypt_binary_packet, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, compressor::compression(data), std::bind(&messaging_session::send_binary_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_session::encrypt_binary_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, encryptor::encryption(data, _key, _iv), std::bind(&messaging_session::send_binary_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_session::send_binary_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		return send_on_tcp(_socket, data_modes::binary_mode, data);
	}

	bool messaging_session::decompress_binary_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_compress_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::normal, compressor::decompression(data), std::bind(&messaging_session::receive_binary_packet, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::high, data, std::bind(&messaging_session::receive_binary_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_session::decrypt_binary_packet(const std::vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::high, encryptor::decryption(data, _key, _iv), std::bind(&messaging_session::decompress_binary_packet, this, std::placeholders::_1)));

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::high, data, std::bind(&messaging_session::decompress_binary_packet, this, std::placeholders::_1)));

		return true;
	}

	bool messaging_session::receive_binary_packet(const std::vector<unsigned char>& data)
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

	bool messaging_session::normal_message(std::shared_ptr<container::value_container> message)
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

	bool messaging_session::connection_message(std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return false;
		}

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

		_confirm = true;

		std::shared_ptr<value> acceptable_snipping_targets = std::make_shared<container::container_value>(L"snipping_targets");

		_snipping_targets.clear();		
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

			auto target = std::find(_ignore_snipping_targets.begin(), _ignore_snipping_targets.end(), snipping_target->to_string());
			if (target == _snipping_targets.end())
			{
				continue;
			}

			_snipping_targets.push_back(snipping_target->to_string());

			acceptable_snipping_targets->add(std::make_shared<container::string_value>(L"snipping_target", snipping_target->to_string()));
		}

		generate_key();

		std::shared_ptr<container::value_container> container = std::make_shared<container::value_container>(_source_id, _source_sub_id, _target_id, _target_sub_id, L"confirm_connection",
			std::vector<std::shared_ptr<container::value>> {
				std::make_shared<container::bool_value>(L"confirm", true),
				std::make_shared<container::string_value>(L"key", _key),
				std::make_shared<container::string_value>(L"iv", _iv),
				std::make_shared<container::bool_value>(L"encrypt_mode", _encrypt_mode),
				acceptable_snipping_targets
		});

		if (_compress_mode)
		{
			_thread_pool->push(std::make_shared<job>(priorities::high, compressor::compression(container->serialize_array()), std::bind(&messaging_session::send_packet, this, std::placeholders::_1)));

			if (_connection)
			{
				_connection(get_ptr(), true);
			}

			return true;
		}

		_thread_pool->push(std::make_shared<job>(priorities::top, container->serialize_array(), std::bind(&messaging_session::send_packet, this, std::placeholders::_1)));

		if (_connection)
		{
			_connection(get_ptr(), true);
		}

		return true;
	}

	bool messaging_session::request_files(std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return false;
		}

		if (!_confirm)
		{
			return false;
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

			_thread_pool->push(std::make_shared<job>(priorities::low, container->serialize_array(), std::bind(&messaging_session::load_file_packet, this, std::placeholders::_1)));
			container->clear_value();
		}

		return true;
	}

	bool messaging_session::echo_message(std::shared_ptr<container::value_container> message)
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

		_thread_pool->push(std::make_shared<job>(priorities::top, message->serialize_array(), std::bind(&messaging_session::send_packet, this, std::placeholders::_1)));

		return true;
	}

	void messaging_session::generate_key(void)
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

	bool messaging_session::same_key_check(std::shared_ptr<container::value> key)
	{
		if (key != nullptr && _connection_key == key->to_string())
		{
			return true;
		}

		logger::handle().write(logging::logging_level::information, L"ignored this line = \"unknown connection key\"");

		std::shared_ptr<container::value_container> container = std::make_shared<container::value_container>(_source_id, _source_sub_id, _target_id, _target_sub_id, L"confirm_connection",
			std::vector<std::shared_ptr<container::value>> {
				std::make_shared<container::bool_value>(L"confirm", false),
				std::make_shared<container::string_value>(L"reason", L"ignored this line = \"unknown connection key\"")
		});

		send(container);

		return false;
	}

	bool messaging_session::same_id_check(void)
	{
		if (_target_id != _source_id)
		{
			return true;
		}

		logger::handle().write(logging::logging_level::information, L"ignored this line = \"cannot use same id with server\"");

		std::shared_ptr<container::value_container> container = std::make_shared<container::value_container>(_source_id, _source_sub_id, _target_id, _target_sub_id, L"confirm_connection",
			std::vector<std::shared_ptr<container::value>> {
				std::make_shared<container::bool_value>(L"confirm", false),
				std::make_shared<container::string_value>(L"reason", L"ignored this line = \"cannot use same id with server\"")
		});

		send(container);

		return false;
	}
}
