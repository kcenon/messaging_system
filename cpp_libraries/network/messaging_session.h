#pragma once

#ifndef __USE_TYPE_CONTAINER__
#include "cpprest/json.h"
#else
#include "container.h"
#endif

#include "thread_pool.h"
#include "data_handling.h"
#include "session_types.h"

#include <map>
#include <memory>
#include <string>
#include <functional>

#include "asio.hpp"

using namespace std;

#ifndef __USE_TYPE_CONTAINER__
using namespace web;
#endif

namespace network
{
	class messaging_session : public enable_shared_from_this<messaging_session>, data_handling
	{
	public:
		messaging_session(const wstring& source_id, const wstring& connection_key, asio::ip::tcp::socket& socket);
		~messaging_session(void);

	public:
		shared_ptr<messaging_session> get_ptr(void);

	public:
		void set_kill_code(const bool& kill_code);
		void set_ignore_target_ids(const vector<wstring>& ignore_target_ids);
		void set_ignore_snipping_targets(const vector<wstring>& ignore_snipping_targets);
		void set_connection_notification(const function<void(shared_ptr<messaging_session>, const bool&)>& notification);

#ifndef __USE_TYPE_CONTAINER__
		void set_message_notification(const function<void(shared_ptr<json::value>)>& notification);
#else
		void set_message_notification(const function<void(shared_ptr<container::value_container>)>& notification);
#endif

		void set_file_notification(const function<void(const wstring&, const wstring&, const wstring&, const wstring&)>& notification);
		void set_binary_notification(const function<void(const wstring&, const wstring&, const wstring&, const wstring&, const vector<unsigned char>&)>& notification);

	public:
		const session_conditions get_confirom_status(void);
		const session_types get_session_type(void);
		const wstring target_id(void);
		const wstring target_sub_id(void);

	public:
		void start(const bool& encrypt_mode, const bool& compress_mode, const vector<session_types>& possible_session_types, 
			const unsigned short& high_priority = 8, const unsigned short& normal_priority = 8, const unsigned short& low_priority = 8);
		void stop(void);

	public:
		void echo(void);

#ifndef __USE_TYPE_CONTAINER__
		void send(shared_ptr<json::value> message);
		void send_files(shared_ptr<json::value> message);
#else
		void send(shared_ptr<container::value_container> message);
		void send_files(shared_ptr<container::value_container> message);
#endif

		void send_binary(const wstring target_id, const wstring& target_sub_id, const vector<unsigned char>& data);
		void send_binary(const wstring source_id, const wstring& source_sub_id, const wstring target_id, const wstring& target_sub_id, const vector<unsigned char>& data);

	protected:
		void receive_on_tcp(const data_modes& data_mode, const vector<unsigned char>& data) override;
		void disconnected(void) override;

	protected:
		bool check_confirm_condition(void);
		bool contained_snipping_target(const wstring& snipping_target);

		// packet
	private:
		void compress_packet(const vector<unsigned char>& data);
		void encrypt_packet(const vector<unsigned char>& data);
		void send_packet(const vector<unsigned char>& data);

	private:
		void decompress_packet(const vector<unsigned char>& data);
		void decrypt_packet(const vector<unsigned char>& data);
		void receive_packet(const vector<unsigned char>& data);

		// file
	private:
		void load_file_packet(const vector<unsigned char>& data);
		void compress_file_packet(const vector<unsigned char>& data);
		void encrypt_file_packet(const vector<unsigned char>& data);
		void send_file_packet(const vector<unsigned char>& data);

	private:
		void decompress_file_packet(const vector<unsigned char>& data);
		void decrypt_file_packet(const vector<unsigned char>& data);
		void receive_file_packet(const vector<unsigned char>& data);
		void notify_file_packet(const vector<unsigned char>& data);

		// binary
	private:
		void compress_binary_packet(const vector<unsigned char>& data);
		void encrypt_binary_packet(const vector<unsigned char>& data);
		void send_binary_packet(const vector<unsigned char>& data);

	private:
		void decompress_binary_packet(const vector<unsigned char>& data);
		void decrypt_binary_packet(const vector<unsigned char>& data);
		void receive_binary_packet(const vector<unsigned char>& data);

	private:
#ifndef __USE_TYPE_CONTAINER__
		void normal_message(shared_ptr<json::value> message);
		void connection_message(shared_ptr<json::value> message);
		void request_files(shared_ptr<json::value> message);
		void echo_message(shared_ptr<json::value> message);
#else
		void normal_message(shared_ptr<container::value_container> message);
		void connection_message(shared_ptr<container::value_container> message);
		void request_files(shared_ptr<container::value_container> message);
		void echo_message(shared_ptr<container::value_container> message);
#endif

	private:
		void generate_key(void);

#ifndef __USE_TYPE_CONTAINER__
		bool same_key_check(const json::value& key);
#else
		bool same_key_check(shared_ptr<container::value> key);
#endif

		bool same_id_check(void);

	private:
		session_conditions _confirm;
		bool _kill_code;
		bool _auto_echo;
		bool _bridge_line;
		session_types _session_type;
		wstring _source_id;
		wstring _source_sub_id;
		wstring _target_id;
		wstring _target_sub_id;
		wstring _connection_key;
		vector<wstring> _snipping_targets;
		vector<wstring> _ignore_target_ids;
		vector<wstring> _ignore_snipping_targets;
		vector<session_types> _possible_session_types;

	private:
		bool _compress_mode;
		bool _encrypt_mode;
		wstring _key;
		wstring _iv;

	private:
		function<void(shared_ptr<messaging_session>, const bool&)> _connection;

#ifndef __USE_TYPE_CONTAINER__
		function<void(shared_ptr<json::value>)> _received_message;
#else
		function<void(shared_ptr<container::value_container>)> _received_message;
#endif

		function<void(const wstring&, const wstring&, const wstring&, const wstring&)> _received_file;
		function<void(const wstring&, const wstring&, const wstring&, const wstring&, const vector<unsigned char>&)> _received_data;

	private:
		shared_ptr<asio::ip::tcp::socket> _socket;

	private:
		shared_ptr<threads::thread_pool> _thread_pool;

#ifndef __USE_TYPE_CONTAINER__
		map<wstring, function<void(shared_ptr<json::value>)>> _message_handlers;
#else
		map<wstring, function<void(shared_ptr<container::value_container>)>> _message_handlers;
#endif
	};
}