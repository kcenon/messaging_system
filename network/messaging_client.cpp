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
		_message_handlers.insert({ L"request_files", bind(&messaging_client::request_files, this, placeholders::_1) });
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

		_thread = make_shared<thread>([&]()
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
		if (_thread_pool != nullptr)
		{
			_thread_pool->stop();
			_thread_pool.reset();
		}

		if (_socket != nullptr)
		{
			if (_socket->is_open())
			{
				_socket->close();
				_socket.reset();
			}
		}

		if (_io_context != nullptr)
		{
			_io_context->reset();
			_io_context.reset();
		}

		if (_thread != nullptr)
		{
			if (_thread->joinable())
			{
				_thread->join();
			}
			_thread.reset();
		}
	}

	void messaging_client::echo(void)
	{
#ifndef __USE_TYPE_CONTAINER__
		shared_ptr<json::value> container = make_shared<json::value>(json::value::object(true));
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
		if ((*message)[HEADER][SOURCE_ID].is_null() ||
			(*message)[HEADER][SOURCE_ID].as_string().empty())
		{
#ifdef _WIN32
			(*message)[HEADER][SOURCE_ID] = json::value::string(_source_id);
			(*message)[HEADER][SOURCE_SUB_ID] = json::value::string(_source_sub_id);
#else
			(*message)[HEADER][SOURCE_ID] = json::value::string(converter::to_string(_source_id));
			(*message)[HEADER][SOURCE_SUB_ID] = json::value::string(converter::to_string(_source_sub_id));
#endif
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
		if ((*message)[HEADER][SOURCE_ID].is_null() ||
			(*message)[HEADER][SOURCE_ID].as_string().empty())
		{
#ifdef _WIN32
			(*message)[HEADER][SOURCE_ID] = json::value::string(_source_id);
			(*message)[HEADER][SOURCE_SUB_ID] = json::value::string(_source_sub_id);
#else
			(*message)[HEADER][SOURCE_ID] = json::value::string(converter::to_string(_source_id));
			(*message)[HEADER][SOURCE_SUB_ID] = json::value::string(converter::to_string(_source_sub_id));
#endif
		}

#ifdef _WIN32
		auto& files = (*message)[DATA][FILES].as_array();
#else
		auto& files = (*message)[DATA][FILES].as_array();
#endif
		for (int index = 0; index < files.size(); ++index)
		{
			shared_ptr<json::value> container = make_shared<json::value>(json::value::object(true));

			(*container)[HEADER][SOURCE_ID] = (*message)[HEADER][TARGET_ID];
			(*container)[HEADER][SOURCE_SUB_ID] = (*message)[HEADER][TARGET_SUB_ID];
			(*container)[HEADER][TARGET_ID] = (*message)[HEADER][SOURCE_ID];
			(*container)[HEADER][TARGET_SUB_ID] = (*message)[HEADER][SOURCE_SUB_ID];
			(*container)[HEADER][MESSAGE_TYPE] = json::value::string(REQUEST_FILE);

			(*container)[DATA][INDICATION_ID] = (*message)[DATA][INDICATION_ID];
#ifdef _WIN32
			(*container)[DATA][SOURCE] = files[index][SOURCE];
			(*container)[DATA][TARGET] = files[index][TARGET];
#else
			(*container)[DATA][SOURCE] = files[index][SOURCE];
			(*container)[DATA][TARGET] = files[index][TARGET];
#endif

			_thread_pool->push(make_shared<job>(priorities::low, converter::to_array(container->serialize()), bind(&messaging_client::load_file_packet, this, placeholders::_1)));
		}
#else
		if (message->source_id().empty())
		{
			message->set_source(_source_id, _source_sub_id);
		}

		shared_ptr<container::value_container> container = message->copy(false);
		container->swap_header();
		container->set_message_type(REQUEST_FILE);

		vector<shared_ptr<container::value>> files = message->value_array(L"file");
		for (auto& file : files)
		{
			container << make_shared<container::string_value>(INDICATION_ID, message->get_value(INDICATION_ID)->to_string());
			container << make_shared<container::string_value>(SOURCE, (*file)[SOURCE]->to_string());
			container << make_shared<container::string_value>(TARGET, (*file)[TARGET]->to_string());

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
		shared_ptr<json::value> container = make_shared<json::value>(json::value::object(true));

		(*container)[HEADER][MESSAGE_TYPE] = json::value::string(REQUEST_CONNECTION);
#ifdef _WIN32
		(*container)[HEADER][SOURCE_ID] = json::value::string(_source_id);
		(*container)[HEADER][SOURCE_SUB_ID] = json::value::string(_source_sub_id);
		(*container)[HEADER][TARGET_ID] = json::value::string(_target_id);
		(*container)[HEADER][TARGET_SUB_ID] = json::value::string(_target_sub_id);

		(*container)[DATA][CONNECTION_KEY] = json::value::string(_connection_key);
		(*container)[DATA][L"auto_echo"] = json::value::boolean(_auto_echo);
		(*container)[DATA][L"auto_echo_interval_seconds"] = json::value::number(_auto_echo_interval_seconds);
		(*container)[DATA][L"session_type"] = json::value::number((short)_session_type);
		(*container)[DATA][L"bridge_mode"] = json::value::boolean(_bridge_line);

		int index = 0;
		(*container)[DATA][SNIPPING_TARGETS] = json::value::array();
		for (auto& snipping_target : _snipping_targets)
		{
			(*container)[DATA][SNIPPING_TARGETS][index++] = json::value::string(snipping_target);
		}
#else
		(*container)[HEADER][SOURCE_ID] = json::value::string(converter::to_string(_source_id));
		(*container)[HEADER][SOURCE_SUB_ID] = json::value::string(converter::to_string(_source_sub_id));
		(*container)[HEADER][TARGET_ID] = json::value::string(converter::to_string(_target_id));
		(*container)[HEADER][TARGET_SUB_ID] = json::value::string(converter::to_string(_target_sub_id));

		(*container)[DATA][CONNECTION_KEY] = json::value::string(converter::to_string(_connection_key));
		(*container)[DATA]["auto_echo"] = json::value::boolean(_auto_echo);
		(*container)[DATA]["auto_echo_interval_seconds"] = json::value::number(_auto_echo_interval_seconds);
		(*container)[DATA]["session_type"] = json::value::number((short)_session_type);
		(*container)[DATA]["bridge_mode"] = json::value::boolean(_bridge_line);

		int index = 0;
		(*container)[DATA][SNIPPING_TARGETS] = json::value::array();
		for (auto& snipping_target : _snipping_targets)
		{
			(*container)[DATA][SNIPPING_TARGETS][index++] = json::value::string(converter::to_string(snipping_target));
		}
#endif
#else
		shared_ptr<container::container_value> snipping_targets = make_shared<container::container_value>(SNIPPING_TARGETS);
		for (auto& snipping_target : _snipping_targets)
		{
			snipping_targets->add(make_shared<container::string_value>(L"snipping_target", snipping_target));
		}

		shared_ptr<container::value_container> container = make_shared<container::value_container>(_source_id, _source_sub_id, _target_id, _target_sub_id, REQUEST_CONNECTION,
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
		default:
			break;
		}
	}

	void messaging_client::disconnected(void)
	{
		stop();

		connection_notification(false);
	}

	void messaging_client::compress_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, compressor::compression(data), bind(&messaging_client::encrypt_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::top, compressor::compression(data), bind(&messaging_client::send_packet, this, placeholders::_1)));
	}

	void messaging_client::encrypt_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		_thread_pool->push(make_shared<job>(priorities::top, encryptor::encryption(data, _key, _iv), bind(&messaging_client::send_packet, this, placeholders::_1)));
	}

	void messaging_client::send_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		send_on_tcp(_socket, data_modes::packet_mode, data);
	}

	void messaging_client::decompress_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_compress_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, compressor::decompression(data), bind(&messaging_client::receive_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::high, data, bind(&messaging_client::receive_packet, this, placeholders::_1)));
	}

	void messaging_client::decrypt_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_encrypt_mode)
		{
			// if encrypt_mode is true
			// 
			_thread_pool->push(make_shared<job>(priorities::high, encryptor::decryption(data, _key, _iv), bind(&messaging_client::decompress_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::high, data, bind(&messaging_client::decompress_packet, this, placeholders::_1)));
	}

	void messaging_client::receive_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

#ifndef __USE_TYPE_CONTAINER__
#ifdef _WIN32
		shared_ptr<json::value> message = make_shared<json::value>(json::value::parse(converter::to_wstring(data)));
#else
		shared_ptr<json::value> message = make_shared<json::value>(json::value::parse(converter::to_string(data)));
#endif
#else
		shared_ptr<container::value_container> message = make_shared<container::value_container>(data, true);
#endif
		if (message == nullptr)
		{
			return;
		}

		logger::handle().write(logging_level::packet, data);

#ifndef __USE_TYPE_CONTAINER__
#ifdef _WIN32
		auto target = _message_handlers.find((*message)[HEADER][MESSAGE_TYPE].as_string());
#else
		auto target = _message_handlers.find(converter::to_wstring((*message)[HEADER][MESSAGE_TYPE].as_string()));
#endif
#else
		auto target = _message_handlers.find(message->message_type());
#endif
		if (target == _message_handlers.end())
		{
			return normal_message(message);
		}

		target->second(message);
	}

	void messaging_client::load_file_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

#ifndef __USE_TYPE_CONTAINER__
#ifdef _WIN32
		shared_ptr<json::value> message = make_shared<json::value>(json::value::parse(converter::to_wstring(data)));
#else
		shared_ptr<json::value> message = make_shared<json::value>(json::value::parse(converter::to_string(data)));
#endif
#else
		shared_ptr<container::value_container> message = make_shared<container::value_container>(data, true);
#endif
		if (message == nullptr)
		{
			return;
		}

		vector<unsigned char> result;
#ifndef __USE_TYPE_CONTAINER__
		append_binary_on_packet(result, converter::to_array((*message)[DATA][INDICATION_ID].as_string()));
		append_binary_on_packet(result, converter::to_array((*message)[HEADER][SOURCE_ID].as_string()));
		append_binary_on_packet(result, converter::to_array((*message)[HEADER][SOURCE_SUB_ID].as_string()));
		append_binary_on_packet(result, converter::to_array((*message)[HEADER][TARGET_ID].as_string()));
		append_binary_on_packet(result, converter::to_array((*message)[HEADER][TARGET_SUB_ID].as_string()));
#ifdef _WIN32
		append_binary_on_packet(result, converter::to_array((*message)[DATA][SOURCE].as_string()));
		append_binary_on_packet(result, converter::to_array((*message)[DATA][TARGET].as_string()));
		append_binary_on_packet(result, file::load((*message)[DATA][SOURCE].as_string()));

		logger::handle().write(logging_level::parameter,
			fmt::format(L"load_file_packet: [{}] => [{}:{}] -> [{}:{}]", (*message)[DATA][INDICATION_ID].as_string(), 
				(*message)[HEADER][SOURCE_ID].as_string(), (*message)[HEADER][SOURCE_SUB_ID].as_string(),
				(*message)[HEADER][TARGET_ID].as_string(), (*message)[HEADER][TARGET_SUB_ID].as_string()));
#else
		append_binary_on_packet(result, converter::to_array((*message)[DATA][SOURCE].as_string()));
		append_binary_on_packet(result, converter::to_array((*message)[DATA][TARGET].as_string()));
		append_binary_on_packet(result, file::load(converter::to_wstring((*message)[DATA][SOURCE].as_string())));

		logger::handle().write(logging_level::parameter,
			converter::to_wstring(fmt::format("load_file_packet: [{}] => [{}:{}] -> [{}:{}]", (*message)[DATA][INDICATION_ID].as_string(),
				(*message)[HEADER][SOURCE_ID].as_string(), (*message)[HEADER][SOURCE_SUB_ID].as_string(),
				(*message)[HEADER][TARGET_ID].as_string(), (*message)[HEADER][TARGET_SUB_ID].as_string())));
#endif
#else
		append_binary_on_packet(result, converter::to_array(message->get_value(INDICATION_ID)->to_string()));
		append_binary_on_packet(result, converter::to_array(message->source_id()));
		append_binary_on_packet(result, converter::to_array(message->source_sub_id()));
		append_binary_on_packet(result, converter::to_array(message->target_id()));
		append_binary_on_packet(result, converter::to_array(message->target_sub_id()));
		append_binary_on_packet(result, converter::to_array(message->get_value(SOURCE)->to_string()));
		append_binary_on_packet(result, converter::to_array(message->get_value(TARGET)->to_string()));
		append_binary_on_packet(result, file::load(message->get_value(SOURCE)->to_string()));

		logger::handle().write(logging_level::parameter,
			fmt::format(L"load_file_packet: [{}] => [{}:{}] -> [{}:{}]", message->get_value(INDICATION_ID)->to_string(),
				message->source_id(), message->source_sub_id(), message->target_id(), message->target_sub_id()));
#endif

		if (_compress_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, result, bind(&messaging_client::compress_file_packet, this, placeholders::_1)));

			return;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, result, bind(&messaging_client::encrypt_file_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::top, result, bind(&messaging_client::send_file_packet, this, placeholders::_1)));
	}

	void messaging_client::compress_file_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, compressor::compression(data), bind(&messaging_client::encrypt_file_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::top, compressor::compression(data), bind(&messaging_client::send_file_packet, this, placeholders::_1)));
	}

	void messaging_client::encrypt_file_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		_thread_pool->push(make_shared<job>(priorities::top, encryptor::encryption(data, _key, _iv), bind(&messaging_client::send_file_packet, this, placeholders::_1)));
	}

	void messaging_client::send_file_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		send_on_tcp(_socket, data_modes::file_mode, data);
	}

	void messaging_client::decompress_file_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_compress_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::low, compressor::decompression(data), bind(&messaging_client::receive_file_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::low, data, bind(&messaging_client::receive_file_packet, this, placeholders::_1)));
	}

	void messaging_client::decrypt_file_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, encryptor::decryption(data, _key, _iv), bind(&messaging_client::decompress_file_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::normal, data, bind(&messaging_client::decompress_file_packet, this, placeholders::_1)));
	}

	void messaging_client::receive_file_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}
		size_t index = 0;
		wstring indication_id = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring source_id = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring source_sub_id = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring target_id = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring target_sub_id = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring source_path = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring target_path = converter::to_wstring(devide_binary_on_packet(data, index));

		logger::handle().write(logging_level::parameter, 
			fmt::format(L"receive_file_packet: [{}] => [{}:{}] -> [{}:{}]", indication_id, source_id, source_sub_id, target_id, target_sub_id));

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
	}

	void messaging_client::notify_file_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
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
	}

	void messaging_client::compress_binary_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, compressor::compression(data), bind(&messaging_client::encrypt_binary_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::top, compressor::compression(data), bind(&messaging_client::send_binary_packet, this, placeholders::_1)));
	}

	void messaging_client::encrypt_binary_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		_thread_pool->push(make_shared<job>(priorities::top, encryptor::encryption(data, _key, _iv), bind(&messaging_client::send_binary_packet, this, placeholders::_1)));
	}

	void messaging_client::send_binary_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		send_on_tcp(_socket, data_modes::binary_mode, data);
	}

	void messaging_client::decompress_binary_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_compress_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, compressor::decompression(data), bind(&messaging_client::receive_binary_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::high, data, bind(&messaging_client::receive_binary_packet, this, placeholders::_1)));
	}

	void messaging_client::decrypt_binary_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::high, encryptor::decryption(data, _key, _iv), bind(&messaging_client::decompress_binary_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::high, data, bind(&messaging_client::decompress_binary_packet, this, placeholders::_1)));
	}

	void messaging_client::receive_binary_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
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
	}

#ifndef __USE_TYPE_CONTAINER__
	void messaging_client::normal_message(shared_ptr<json::value> message)
#else
	void messaging_client::normal_message(shared_ptr<container::value_container> message)
#endif
	{
		if (message == nullptr)
		{
			return;
		}

		if (!_confirm)
		{
			return;
		}

		if (_received_message)
		{
			_received_message(message);
		}
	}

#ifndef __USE_TYPE_CONTAINER__
	void messaging_client::confirm_message(shared_ptr<json::value> message)
#else
	void messaging_client::confirm_message(shared_ptr<container::value_container> message)
#endif
	{
		if (message == nullptr)
		{
			return;
		}

#ifndef __USE_TYPE_CONTAINER__
#ifdef _WIN32
		_target_id = (*message)[HEADER][SOURCE_ID].as_string();
#else
		_target_id = converter::to_wstring((*message)[HEADER][SOURCE_ID].as_string());
#endif
#else
		_target_id = message->source_id();
#endif

#ifndef __USE_TYPE_CONTAINER__
#ifdef _WIN32
		if (!(*message)[DATA][L"confirm"].as_bool())
#else
		if (!(*message)[DATA]["confirm"].as_bool())
#endif
#else
		if (!message->get_value(L"confirm")->to_boolean())
#endif
		{
			connection_notification(false);

			return;
		}

		_confirm = true;

#ifndef __USE_TYPE_CONTAINER__
#ifdef _WIN32
		_key = (*message)[DATA][L"key"].as_string();
		_iv = (*message)[DATA][L"iv"].as_string();
#else
		_key = converter::to_wstring((*message)[DATA]["key"].as_string());
		_iv = converter::to_wstring((*message)[DATA]["iv"].as_string());
#endif
		_encrypt_mode = (*message)[DATA][ENCRYPT_MODE].as_bool();

		auto& snipping_targets = (*message)[DATA][SNIPPING_TARGETS].as_array();
		for (int index = 0; index < snipping_targets.size(); ++index)
		{
#ifdef _WIN32
			logger::handle().write(logging_level::information, fmt::format(L"accepted snipping target: {}", snipping_targets[index].as_string()));
#else
			logger::handle().write(logging_level::information, converter::to_wstring(fmt::format("accepted snipping target: {}", snipping_targets[index].as_string())));
#endif
		}
#else
		_key = message->get_value(L"key")->to_string();
		_iv = message->get_value(L"iv")->to_string();
		_encrypt_mode = message->get_value(ENCRYPT_MODE)->to_boolean();

		vector<shared_ptr<value>> snipping_targets = message->get_value(SNIPPING_TARGETS)->children();
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
	}

#ifndef __USE_TYPE_CONTAINER__
	void messaging_client::request_files(shared_ptr<json::value> message)
#else
	void messaging_client::request_files(shared_ptr<container::value_container> message)
#endif
	{
		if (message == nullptr)
		{
			return;
		}

		if (!_confirm)
		{
			return;
		}

#ifndef __USE_TYPE_CONTAINER__
		auto& files = (*message)[DATA][FILES].as_array();
		for (int index = 0; index < files.size(); ++index)
		{
			shared_ptr<json::value> container = make_shared<json::value>(json::value::object(true));

			(*container)[HEADER][SOURCE_ID] = (*message)[HEADER][TARGET_ID];
			(*container)[HEADER][SOURCE_SUB_ID] = (*message)[HEADER][TARGET_SUB_ID];
			(*container)[HEADER][TARGET_ID] = (*message)[HEADER][SOURCE_ID];
			(*container)[HEADER][TARGET_SUB_ID] = (*message)[HEADER][SOURCE_SUB_ID];
			(*container)[HEADER][MESSAGE_TYPE] = json::value::string(REQUEST_FILE);

			(*container)[DATA][INDICATION_ID] = (*message)[DATA][INDICATION_ID];
			(*container)[DATA][SOURCE] = files[index][SOURCE];
			(*container)[DATA][TARGET] = files[index][TARGET];

			_thread_pool->push(make_shared<job>(priorities::low, converter::to_array(container->serialize()), bind(&messaging_client::load_file_packet, this, placeholders::_1)));
		}
#else
		shared_ptr<container::value_container> container = message->copy(false);
		container->swap_header();
		container->set_message_type(REQUEST_FILE);

		vector<shared_ptr<container::value>> files = message->value_array(L"file");
		for (auto& file : files)
		{
			container << make_shared<container::string_value>(INDICATION_ID, message->get_value(INDICATION_ID)->to_string());
			container << make_shared<container::string_value>(SOURCE, (*file)[SOURCE]->to_string());
			container << make_shared<container::string_value>(TARGET, (*file)[TARGET]->to_string());

			_thread_pool->push(make_shared<job>(priorities::low, container->serialize_array(), bind(&messaging_client::load_file_packet, this, placeholders::_1)));
			container->clear_value();
		}
#endif
	}

#ifndef __USE_TYPE_CONTAINER__
	void messaging_client::echo_message(shared_ptr<json::value> message)
#else
	void messaging_client::echo_message(shared_ptr<container::value_container> message)
#endif
	{
		if (message == nullptr)
		{
			return;
		}

		if (!_confirm)
		{
			return;
		}

#ifndef __USE_TYPE_CONTAINER__
		if (!(*message)[DATA][RESPONSE].is_null())
		{
#ifdef _WIN32
			logger::handle().write(logging_level::information, fmt::format(L"received echo: {}", message->serialize()));
#else
			logger::handle().write(logging_level::information, converter::to_wstring(fmt::format("received echo: {}", message->serialize())));
#endif
			return;
		}

		shared_ptr<json::value> container = make_shared<json::value>(json::value::object(true));

		(*container)[HEADER][SOURCE_ID] = (*message)[HEADER][TARGET_ID];
		(*container)[HEADER][SOURCE_SUB_ID] = (*message)[HEADER][TARGET_SUB_ID];
		(*container)[HEADER][TARGET_ID] = (*message)[HEADER][SOURCE_ID];
		(*container)[HEADER][TARGET_SUB_ID] = (*message)[HEADER][SOURCE_SUB_ID];
		(*container)[HEADER][MESSAGE_TYPE] = (*message)[HEADER][MESSAGE_TYPE];

		(*container)[DATA] = (*message)[DATA];
		(*container)[DATA][RESPONSE] = json::value::boolean(true);

		_thread_pool->push(make_shared<job>(priorities::low, converter::to_array(container->serialize()), bind(&messaging_client::send_packet, this, placeholders::_1)));
#else
		vector<shared_ptr<value>> response = (*message)[RESPONSE];
		if (!response.empty())
		{
			logger::handle().write(logging_level::information, fmt::format(L"received echo: {}", message->serialize()));

			return;
		}

		message->swap_header();

		message << make_shared<bool_value>(RESPONSE, true);

		_thread_pool->push(make_shared<job>(priorities::top, message->serialize_array(), bind(&messaging_client::send_packet, this, placeholders::_1)));
#endif
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
