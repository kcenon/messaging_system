#include "messaging_client.h"

#ifdef __USE_TYPE_CONTAINER__
#include "value.h"
#include "values/bool_value.h"
#include "values/bytes_value.h"
#include "values/short_value.h"
#include "values/ushort_value.h"
#include "values/string_value.h"
#include "values/container_value.h"
#endif

#include "logging.h"
#include "converting.h"
#include "encrypting.h"
#include "compressing.h"
#include "thread_worker.h"
#include "job_pool.h"
#include "job.h"

#include "data_lengths.h"
#include "file_handler.h"

#include <functional>

#include "fmt/xchar.h"
#include "fmt/format.h"

namespace network
{
	using namespace logging;
	using namespace threads;

#ifdef __USE_TYPE_CONTAINER__
	using namespace container;
#endif

	using namespace converting;
	using namespace encrypting;
	using namespace compressing;
	using namespace file_handler;

	messaging_client::messaging_client(const wstring& source_id)
		: data_handling(246, 135), _confirm(false), _auto_echo(false), _compress_mode(false), _encrypt_mode(false), _bridge_line(false),
		_io_context(nullptr), _socket(nullptr), _key(L""), _iv(L""), _thread_pool(nullptr), _auto_echo_interval_seconds(1), _connection(nullptr),
		_connection_key(L"connection_key"), _source_id(source_id), _source_sub_id(L""), _target_id(L"unknown"), _target_sub_id(L"0.0.0.0:0"), _received_file(nullptr),
		_received_message(nullptr), _received_data(nullptr), _session_type(session_types::binary_line)
	{
		_message_handlers.insert({ L"confirm_connection", bind(&messaging_client::confirm_message, this, placeholders::_1) });
		_message_handlers.insert({ L"echo", bind(&messaging_client::echo_message, this, placeholders::_1) });
	}

	messaging_client::~messaging_client(void)
	{
		stop();
	}

	shared_ptr<messaging_client> messaging_client::get_ptr(void)
	{
		return shared_from_this();
	}

	wstring messaging_client::source_id(void) const
	{
		return _source_id;
	}

	wstring messaging_client::source_sub_id(void) const
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

	void messaging_client::set_connection_key(const wstring& connection_key)
	{
		_connection_key = connection_key;
	}

	void messaging_client::set_snipping_targets(const vector<wstring>& snipping_targets)
	{
		_snipping_targets = snipping_targets;
	}

	void messaging_client::set_connection_notification(const function<void(const wstring&, const wstring&, const bool&)>& notification)
	{
		_connection = notification;
	}

#ifndef __USE_TYPE_CONTAINER__
	void messaging_client::set_message_notification(const function<void(shared_ptr<json::value>)>& notification)
#else
	void messaging_client::set_message_notification(const function<void(shared_ptr<container::value_container>)>& notification)
#endif
	{
		_received_message = notification;
	}

	void messaging_client::set_file_notification(const function<void(const wstring&, const wstring&, const wstring&, const wstring&)>& notification)
	{
		_received_file = notification;
	}

	void messaging_client::set_binary_notification(const function<void(const wstring&, const wstring&, const wstring&, const wstring&, const vector<unsigned char>&)>& notification)
	{
		_received_data = notification;
	}

	bool messaging_client::is_confirmed(void) const
	{
		return _confirm;
	}

	void messaging_client::start(const wstring& ip, const unsigned short& port, const unsigned short& high_priority, const unsigned short& normal_priority, const unsigned short& low_priority)
	{
		stop();

		_thread_pool = make_shared<threads::thread_pool>();

		_thread_pool->append(make_shared<thread_worker>(priorities::top), true);
		for (unsigned short high = 0; high < high_priority; ++high)
		{
			_thread_pool->append(make_shared<thread_worker>(priorities::high, vector<priorities> { priorities::normal, priorities::low }), true);
		}
		for (unsigned short normal = 0; normal < normal_priority; ++normal)
		{
			_thread_pool->append(make_shared<thread_worker>(priorities::normal, vector<priorities> { priorities::high, priorities::low }), true);
		}
		for (unsigned short low = 0; low < low_priority; ++low)
		{
			_thread_pool->append(make_shared<thread_worker>(priorities::low, vector<priorities> { priorities::high, priorities::normal }), true);
		}

		logger::handle().write(logging_level::sequence, L"attempts to create io_context");

		_io_context = make_shared<asio::io_context>();

		logger::handle().write(logging_level::sequence, L"attempts to create socket");

		try
		{
			_socket = make_shared<asio::ip::tcp::socket>(*_io_context);
			_socket->open(asio::ip::tcp::v4());
			_socket->bind(asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 0));
			_socket->connect(asio::ip::tcp::endpoint(asio::ip::address::from_string(converter::to_string(ip)), port));

			_socket->set_option(asio::ip::tcp::no_delay(true));
			_socket->set_option(asio::socket_base::keep_alive(true));
			_socket->set_option(asio::socket_base::receive_buffer_size(buffer_size));
		}
		catch (const overflow_error&) {
			connection_notification(false);
			return; 
		}
		catch (const runtime_error&) {
			connection_notification(false);
			return;
		}
		catch (const exception&) {
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

		_thread = thread([&]()
			{
				try
				{
					logger::handle().write(logging_level::information, fmt::format(L"start messaging_client({})", _source_id));
					_io_context->run();
				}
				catch (const overflow_error&) { 
					if (_io_context != nullptr) {
						logger::handle().write(logging_level::exception, fmt::format(L"break messaging_client({}) with overflow error", _source_id));
					}
				}
				catch (const runtime_error&) { 
					if (_io_context != nullptr) {
						logger::handle().write(logging_level::exception, fmt::format(L"break messaging_client({}) with runtime error", _source_id));
					}
				}
				catch (const exception&) { 
					if (_io_context != nullptr) {
						logger::handle().write(logging_level::exception, fmt::format(L"break messaging_client({}) with exception", _source_id));
					}
				}
				catch (...) { 
					if (_io_context != nullptr) {
						logger::handle().write(logging_level::exception, fmt::format(L"break messaging_client({}) with error", _source_id));
					}
				}
				logger::handle().write(logging_level::information, fmt::format(L"stop messaging_client({})", _source_id));
				connection_notification(false);
			});

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
#ifndef __USE_TYPE_CONTAINER__
		shared_ptr<json::value> container = make_shared<json::value>();
#else
		shared_ptr<container::value_container> container = make_shared<container::value_container>(_source_id, _source_sub_id, _target_id, _target_sub_id, L"echo",
			vector<shared_ptr<container::value>> {});
#endif

		send(container);
	}

#ifndef __USE_TYPE_CONTAINER__
	void messaging_client::send(const json::value& message)
#else
	void messaging_client::send(const container::value_container& message)
#endif
	{
#ifndef __USE_TYPE_CONTAINER__
		send(make_shared<json::value>(message));
#else
		send(make_shared<container::value_container>(message));
#endif
	}

#ifndef __USE_TYPE_CONTAINER__
	void messaging_client::send(shared_ptr<json::value> message)
#else
	void messaging_client::send(shared_ptr<container::value_container> message)
#endif
	{
		if (_socket == nullptr)
		{
			return;
		}

		if (message == nullptr)
		{
			return;
		}

		if (_session_type == session_types::binary_line)
		{
			return;
		}

#ifndef __USE_TYPE_CONTAINER__
		if ((*message)[L"header"][L"source_id"].is_null())
		{
			(*message)[L"header"][L"source_id"] = json::value::string(_source_id);
			(*message)[L"header"][L"source_sub_id"] = json::value::string(_source_sub_id);
		}

		auto serialize_array = converter::to_array(message->serialize());
#else
		if (message->source_id().empty())
		{
			message->set_source(_source_id, _source_sub_id);
		}

		auto serialize_array = message->serialize_array();
#endif


		logger::handle().write(logging_level::packet, serialize_array);

		if (_compress_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::high, serialize_array, bind(&messaging_client::compress_packet, this, placeholders::_1)));

			return;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, serialize_array, bind(&messaging_client::encrypt_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::top, serialize_array, bind(&messaging_client::send_packet, this, placeholders::_1)));
	}

#ifndef __USE_TYPE_CONTAINER__
	void messaging_client::send_files(const json::value& message)
#else
	void messaging_client::send_files(const container::value_container& message)
#endif
	{
#ifndef __USE_TYPE_CONTAINER__
		send_files(make_shared<json::value>(message));
#else
		send_files(make_shared<container::value_container>(message));
#endif
	}

#ifndef __USE_TYPE_CONTAINER__
	void messaging_client::send_files(shared_ptr<json::value> message)
#else
	void messaging_client::send_files(shared_ptr<container::value_container> message)
#endif
	{
		if (_socket == nullptr)
		{
			return;
		}

		if (message == nullptr)
		{
			return;
		}

		if (_session_type != session_types::file_line)
		{
			return;
		}

#ifndef __USE_TYPE_CONTAINER__
		if ((*message)[L"header"][L"source_id"].is_null())
		{
			(*message)[L"header"][L"source_id"] = json::value::string(_source_id);
			(*message)[L"header"][L"source_sub_id"] = json::value::string(_source_sub_id);
		}

		auto& files = (*message)[L"data"][L"files"].as_array();
		for (int index = 0; index < files.size(); ++index)
		{
			shared_ptr<json::value> container = make_shared<json::value>();

			(*container)[L"header"][L"source_id"] = (*message)[L"header"][L"target_id"];
			(*container)[L"header"][L"source_sub_id"] = (*message)[L"header"][L"target_sub_id"];
			(*container)[L"header"][L"target_id"] = (*message)[L"header"][L"source_id"];
			(*container)[L"header"][L"target_sub_id"] = (*message)[L"header"][L"source_sub_id"];
			(*container)[L"header"][L"message_type"] = json::value::string(L"request_file");

			(*container)[L"data"][L"indication_id"] = (*message)[L"data"][L"indication_id"];
			(*container)[L"data"][L"source"] = files[index][L"source"];
			(*container)[L"data"][L"target"] = files[index][L"target"];

			_thread_pool->push(make_shared<job>(priorities::low, converter::to_array(container->serialize()), bind(&messaging_client::load_file_packet, this, placeholders::_1)));
		}
#else
		if (message->source_id().empty())
		{
			message->set_source(_source_id, _source_sub_id);
		}

		shared_ptr<container::value_container> container = message->copy(false);
		container->swap_header();
		container->set_message_type(L"request_file");

		vector<shared_ptr<container::value>> files = message->value_array(L"file");
		for (auto& file : files)
		{
			container << make_shared<container::string_value>(L"indication_id", message->get_value(L"indication_id")->to_string());
			container << make_shared<container::string_value>(L"source", (*file)[L"source"]->to_string());
			container << make_shared<container::string_value>(L"target", (*file)[L"target"]->to_string());

			_thread_pool->push(make_shared<job>(priorities::low, container->serialize_array(), bind(&messaging_client::load_file_packet, this, placeholders::_1)));
			container->clear_value();
		}
#endif
	}

	void messaging_client::send_binary(const wstring target_id, const wstring& target_sub_id, const vector<unsigned char>& data)
	{
		if (_socket == nullptr)
		{
			return;
		}

		if (_session_type != session_types::binary_line)
		{
			return;
		}

		vector<unsigned char> result;
		append_binary_on_packet(result, converter::to_array(_source_id));
		append_binary_on_packet(result, converter::to_array(_source_sub_id));
		append_binary_on_packet(result, converter::to_array(target_id));
		append_binary_on_packet(result, converter::to_array(target_sub_id));
		append_binary_on_packet(result, data);

		if (_compress_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, result, bind(&messaging_client::compress_binary_packet, this, placeholders::_1)));

			return;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, result, bind(&messaging_client::encrypt_binary_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::top, result, bind(&messaging_client::send_binary_packet, this, placeholders::_1)));
	}

	void messaging_client::send_connection(void)
	{
#ifndef __USE_TYPE_CONTAINER__
		shared_ptr<json::value> container = make_shared<json::value>();
		
		(*container)[L"header"][L"source_id"] = json::value::string(_source_id);
		(*container)[L"header"][L"source_sub_id"] = json::value::string(_source_sub_id);
		(*container)[L"header"][L"target_id"] = json::value::string(_target_id);
		(*container)[L"header"][L"target_sub_id"] = json::value::string(_target_sub_id);
		(*container)[L"header"][L"message_type"] = json::value::string(L"request_connection");

		(*container)[L"data"][L"connection_key"] = json::value::string(_connection_key);
		(*container)[L"data"][L"auto_echo"] = json::value::boolean(_auto_echo);
		(*container)[L"data"][L"auto_echo_interval_seconds"] = json::value::boolean(_auto_echo_interval_seconds);
		(*container)[L"data"][L"session_type"] = json::value::number((short)_session_type);
		(*container)[L"data"][L"bridge_mode"] = json::value::boolean(_bridge_line);

		int index = 0;
		for (auto& snipping_target : _snipping_targets)
		{
			(*container)[L"data"][L"snipping_targets"][index++] = json::value::string(snipping_target);
		}
#else
		shared_ptr<container::container_value> snipping_targets = make_shared<container::container_value>(L"snipping_targets");
		for (auto& snipping_target : _snipping_targets)
		{
			snipping_targets->add(make_shared<container::string_value>(L"snipping_target", snipping_target));
		}

		shared_ptr<container::value_container> container = make_shared<container::value_container>(_source_id, _source_sub_id, _target_id, _target_sub_id, L"request_connection",
			vector<shared_ptr<container::value>> {
				make_shared<container::string_value>(L"connection_key", _connection_key),
				make_shared<container::bool_value>(L"auto_echo", _auto_echo),
				make_shared<container::ushort_value>(L"auto_echo_interval_seconds", _auto_echo_interval_seconds),
				make_shared<container::short_value>(L"session_type", (short)_session_type),
				make_shared<container::bool_value>(L"bridge_mode", _bridge_line),
				snipping_targets
		});
#endif

		send(container);
	}

	void messaging_client::receive_on_tcp(const data_modes& data_mode, const vector<unsigned char>& data)
	{
		switch (data_mode)
		{
		case data_modes::packet_mode:
			_thread_pool->push(make_shared<job>(priorities::high, data, bind(&messaging_client::decrypt_packet, this, placeholders::_1))); 
			break;
		case data_modes::file_mode:
			_thread_pool->push(make_shared<job>(priorities::high, data, bind(&messaging_client::decrypt_file_packet, this, placeholders::_1)));
			break;
		}
	}

	void messaging_client::disconnected(void)
	{
		stop();

		connection_notification(false);
	}

	bool messaging_client::compress_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, compressor::compression(data), bind(&messaging_client::encrypt_packet, this, placeholders::_1)));

			return true;
		}

		_thread_pool->push(make_shared<job>(priorities::top, compressor::compression(data), bind(&messaging_client::send_packet, this, placeholders::_1)));

		return true;
	}

	bool messaging_client::encrypt_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		_thread_pool->push(make_shared<job>(priorities::top, encryptor::encryption(data, _key, _iv), bind(&messaging_client::send_packet, this, placeholders::_1)));

		return true;
	}

	bool messaging_client::send_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		return send_on_tcp(_socket, data_modes::packet_mode, data);
	}

	bool messaging_client::decompress_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_compress_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, compressor::decompression(data), bind(&messaging_client::receive_packet, this, placeholders::_1)));

			return true;
		}

		_thread_pool->push(make_shared<job>(priorities::high, data, bind(&messaging_client::receive_packet, this, placeholders::_1)));

		return true;
	}

	bool messaging_client::decrypt_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			// if encrypt_mode is true
			// 
			_thread_pool->push(make_shared<job>(priorities::high, encryptor::decryption(data, _key, _iv), bind(&messaging_client::decompress_packet, this, placeholders::_1)));

			return true;
		}

		_thread_pool->push(make_shared<job>(priorities::high, data, bind(&messaging_client::decompress_packet, this, placeholders::_1)));

		return true;
	}

	bool messaging_client::receive_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

#ifndef __USE_TYPE_CONTAINER__
		shared_ptr<json::value> message = make_shared<json::value>();
		message->parse(converter::to_wstring(data));
#else
		shared_ptr<container::value_container> message = make_shared<container::value_container>(data, true);
#endif
		if (message == nullptr)
		{
			return false;
		}

		logger::handle().write(logging_level::packet, data);

#ifndef __USE_TYPE_CONTAINER__
		auto target = _message_handlers.find((*message)[L"header"][L"message_type"].as_string());
#else
		auto target = _message_handlers.find(message->message_type());
#endif
		if (target == _message_handlers.end())
		{
			return normal_message(message);
		}

		return target->second(message);
	}

	bool messaging_client::load_file_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

#ifndef __USE_TYPE_CONTAINER__
		shared_ptr<json::value> message = make_shared<json::value>();
		message->parse(converter::to_wstring(data));
#else
		shared_ptr<container::value_container> message = make_shared<container::value_container>(data, true);
#endif
		if (message == nullptr)
		{
			return false;
		}

		vector<unsigned char> result;
#ifndef __USE_TYPE_CONTAINER__
		append_binary_on_packet(result, converter::to_array((*message)[L"data"][L"indication_id"].as_string()));
		append_binary_on_packet(result, converter::to_array((*message)[L"data"][L"source_id"].as_string()));
		append_binary_on_packet(result, converter::to_array((*message)[L"data"][L"source_sub_id"].as_string()));
		append_binary_on_packet(result, converter::to_array((*message)[L"data"][L"target_id"].as_string()));
		append_binary_on_packet(result, converter::to_array((*message)[L"data"][L"target_sub_id"].as_string()));
		append_binary_on_packet(result, converter::to_array((*message)[L"data"][L"source"].as_string()));
		append_binary_on_packet(result, converter::to_array((*message)[L"data"][L"target"].as_string()));
		append_binary_on_packet(result, file::load((*message)[L"data"][L"source"].as_string()));
#else
		append_binary_on_packet(result, converter::to_array(message->get_value(L"indication_id")->to_string()));
		append_binary_on_packet(result, converter::to_array(message->source_id()));
		append_binary_on_packet(result, converter::to_array(message->source_sub_id()));
		append_binary_on_packet(result, converter::to_array(message->target_id()));
		append_binary_on_packet(result, converter::to_array(message->target_sub_id()));
		append_binary_on_packet(result, converter::to_array(message->get_value(L"source")->to_string()));
		append_binary_on_packet(result, converter::to_array(message->get_value(L"target")->to_string()));
		append_binary_on_packet(result, file::load(message->get_value(L"source")->to_string()));
#endif

		if (_compress_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, result, bind(&messaging_client::compress_file_packet, this, placeholders::_1)));

			return true;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, result, bind(&messaging_client::encrypt_file_packet, this, placeholders::_1)));

			return true;
		}

		_thread_pool->push(make_shared<job>(priorities::top, result, bind(&messaging_client::send_file_packet, this, placeholders::_1)));

		return true;
	}

	bool messaging_client::compress_file_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, compressor::compression(data), bind(&messaging_client::encrypt_file_packet, this, placeholders::_1)));

			return true;
		}

		_thread_pool->push(make_shared<job>(priorities::top, compressor::compression(data), bind(&messaging_client::send_file_packet, this, placeholders::_1)));

		return true;
	}

	bool messaging_client::encrypt_file_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		_thread_pool->push(make_shared<job>(priorities::top, encryptor::encryption(data, _key, _iv), bind(&messaging_client::send_file_packet, this, placeholders::_1)));

		return true;
	}

	bool messaging_client::send_file_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		return send_on_tcp(_socket, data_modes::file_mode, data);
	}

	bool messaging_client::decompress_file_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_compress_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::low, compressor::decompression(data), bind(&messaging_client::receive_file_packet, this, placeholders::_1)));

			return true;
		}

		_thread_pool->push(make_shared<job>(priorities::low, data, bind(&messaging_client::receive_file_packet, this, placeholders::_1)));

		return true;
	}

	bool messaging_client::decrypt_file_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, encryptor::decryption(data, _key, _iv), bind(&messaging_client::decompress_file_packet, this, placeholders::_1)));

			return true;
		}

		_thread_pool->push(make_shared<job>(priorities::normal, data, bind(&messaging_client::decompress_file_packet, this, placeholders::_1)));

		return true;
	}

	bool messaging_client::receive_file_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}
		size_t index = 0;
		wstring indication_id = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring source_id = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring source_sub_id = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring target_id = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring target_sub_id = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring source_path = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring target_path = converter::to_wstring(devide_binary_on_packet(data, index));

		vector<unsigned char> result;
		append_binary_on_packet(result, converter::to_array(indication_id));
		append_binary_on_packet(result, converter::to_array(target_id));
		append_binary_on_packet(result, converter::to_array(target_sub_id));
		if (file::save(target_path, devide_binary_on_packet(data, index)))
		{
			append_binary_on_packet(result, converter::to_array(target_path));
		}
		else
		{
			append_binary_on_packet(result, converter::to_array(L""));
		}

		_thread_pool->push(make_shared<job>(priorities::high, result, bind(&messaging_client::notify_file_packet, this, placeholders::_1)));

		return true;
	}

	bool messaging_client::notify_file_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		size_t index = 0;
		wstring indication_id = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring target_id = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring target_sub_id = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring target_path = converter::to_wstring(devide_binary_on_packet(data, index));

		if (_received_file)
		{
			_received_file(target_id, target_sub_id, indication_id, target_path);
		}

		return true;
	}

	bool messaging_client::compress_binary_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, compressor::compression(data), bind(&messaging_client::encrypt_binary_packet, this, placeholders::_1)));

			return true;
		}

		_thread_pool->push(make_shared<job>(priorities::top, compressor::compression(data), bind(&messaging_client::send_binary_packet, this, placeholders::_1)));

		return true;
	}

	bool messaging_client::encrypt_binary_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		_thread_pool->push(make_shared<job>(priorities::top, encryptor::encryption(data, _key, _iv), bind(&messaging_client::send_binary_packet, this, placeholders::_1)));

		return true;
	}

	bool messaging_client::send_binary_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		return send_on_tcp(_socket, data_modes::binary_mode, data);
	}

	bool messaging_client::decompress_binary_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_compress_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, compressor::decompression(data), bind(&messaging_client::receive_binary_packet, this, placeholders::_1)));

			return true;
		}

		_thread_pool->push(make_shared<job>(priorities::high, data, bind(&messaging_client::receive_binary_packet, this, placeholders::_1)));

		return true;
	}

	bool messaging_client::decrypt_binary_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::high, encryptor::decryption(data, _key, _iv), bind(&messaging_client::decompress_binary_packet, this, placeholders::_1)));

			return true;
		}

		_thread_pool->push(make_shared<job>(priorities::high, data, bind(&messaging_client::decompress_binary_packet, this, placeholders::_1)));

		return true;
	}

	bool messaging_client::receive_binary_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		size_t index = 0;
		wstring source_id = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring source_sub_id = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring target_id = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring target_sub_id = converter::to_wstring(devide_binary_on_packet(data, index));
		vector<unsigned char> target_data = devide_binary_on_packet(data, index);
		if (_received_data)
		{
			_received_data(source_id, source_sub_id, target_id, target_sub_id, target_data);
		}

		return true;
	}

#ifndef __USE_TYPE_CONTAINER__
	bool messaging_client::normal_message(shared_ptr<json::value> message)
#else
	bool messaging_client::normal_message(shared_ptr<container::value_container> message)
#endif
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

#ifndef __USE_TYPE_CONTAINER__
	bool messaging_client::confirm_message(shared_ptr<json::value> message)
#else
	bool messaging_client::confirm_message(shared_ptr<container::value_container> message)
#endif
	{
		if (message == nullptr)
		{
			return false;
		}

#ifndef __USE_TYPE_CONTAINER__
		_target_id = (*message)[L"header"][L"source_id"].as_string();
#else
		_target_id = message->source_id();
#endif

#ifndef __USE_TYPE_CONTAINER__
		if (!(*message)[L"data"][L"confirm"].as_bool())
#else
		if (!message->get_value(L"confirm")->to_boolean())
#endif
		{
			connection_notification(false);

			return false;
		}

		_confirm = true;

#ifndef __USE_TYPE_CONTAINER__
		_key = (*message)[L"data"][L"key"].as_string();
		_iv = (*message)[L"data"][L"iv"].as_string();
		_encrypt_mode = (*message)[L"data"][L"encrypt_mode"].as_bool();

		auto& snipping_targets = (*message)[L"data"][L"snipping_targets"].as_array();
		for (int index = 0; index < snipping_targets.size(); ++index)
		{
			logger::handle().write(logging_level::information, fmt::format(L"accepted snipping target: {}", snipping_targets[index].as_string()));
		}
#else
		_key = message->get_value(L"key")->to_string();
		_iv = message->get_value(L"iv")->to_string();
		_encrypt_mode = message->get_value(L"encrypt_mode")->to_boolean();

		vector<shared_ptr<value>> snipping_targets = message->get_value(L"snipping_targets")->children();
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

			logger::handle().write(logging_level::information, fmt::format(L"accepted snipping target: {}", snipping_target->to_string()));
		}
#endif

		connection_notification(true);

		return true;
	}

#ifndef __USE_TYPE_CONTAINER__
	bool messaging_client::echo_message(shared_ptr<json::value> message)
#else
	bool messaging_client::echo_message(shared_ptr<container::value_container> message)
#endif
	{
		if (message == nullptr)
		{
			return false;
		}

		if (!_confirm)
		{
			return false;
		}

#ifndef __USE_TYPE_CONTAINER__
		if (!(*message)[L"data"][L"response"].is_null())
		{
			logger::handle().write(logging_level::information, fmt::format(L"received echo: {}", message->serialize()));

			return true;
		}

		shared_ptr<json::value> container = make_shared<json::value>();

		(*container)[L"header"][L"source_id"] = (*message)[L"header"][L"target_id"];
		(*container)[L"header"][L"source_sub_id"] = (*message)[L"header"][L"target_sub_id"];
		(*container)[L"header"][L"target_id"] = (*message)[L"header"][L"source_id"];
		(*container)[L"header"][L"target_sub_id"] = (*message)[L"header"][L"source_sub_id"];
		(*container)[L"header"][L"message_type"] = (*message)[L"header"][L"message_type"];

		(*container)[L"data"] = (*message)[L"data"];
		(*container)[L"data"][L"response"] = json::value::boolean(true);

		_thread_pool->push(make_shared<job>(priorities::low, converter::to_array(container->serialize()), bind(&messaging_client::send_packet, this, placeholders::_1)));
#else
		vector<shared_ptr<value>> response = (*message)[L"response"];
		if (!response.empty())
		{
			logger::handle().write(logging_level::information, fmt::format(L"received echo: {}", message->serialize()));

			return true;
		}

		message->swap_header();

		message << make_shared<bool_value>(L"response", true);

		_thread_pool->push(make_shared<job>(priorities::top, message->serialize_array(), bind(&messaging_client::send_packet, this, placeholders::_1)));
#endif

		return true;
	}

	void messaging_client::connection_notification(const bool& condition)
	{
		if (!condition)
		{
			_confirm = false;
		}

		// Need to find out more efficient way
		thread thread([this](const bool& connection) 
			{
				if (_connection)
				{
					_connection(_target_id, _target_sub_id, connection);
				}
			}, condition);
		thread.detach();
	}
}