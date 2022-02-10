#pragma once

#include "container.h"
#include "thread_pool.h"
#include "data_handling.h"
#include "session_types.h"

#include <map>
#include <memory>
#include <string>
#include <functional>

#include "asio.hpp"

using namespace std;

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
		void set_message_notification(const function<void(shared_ptr<container::value_container>)>& notification);
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
		void send(shared_ptr<container::value_container> message);
		void send_files(shared_ptr<container::value_container> message);
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
		bool compress_packet(const vector<unsigned char>& data);
		bool encrypt_packet(const vector<unsigned char>& data);
		bool send_packet(const vector<unsigned char>& data);

	private:
		bool decompress_packet(const vector<unsigned char>& data);
		bool decrypt_packet(const vector<unsigned char>& data);
		bool receive_packet(const vector<unsigned char>& data);

		// file
	private:
		bool load_file_packet(const vector<unsigned char>& data);
		bool compress_file_packet(const vector<unsigned char>& data);
		bool encrypt_file_packet(const vector<unsigned char>& data);
		bool send_file_packet(const vector<unsigned char>& data);

	private:
		bool decompress_file_packet(const vector<unsigned char>& data);
		bool decrypt_file_packet(const vector<unsigned char>& data);
		bool receive_file_packet(const vector<unsigned char>& data);
		bool notify_file_packet(const vector<unsigned char>& data);

		// binary
	private:
		bool compress_binary_packet(const vector<unsigned char>& data);
		bool encrypt_binary_packet(const vector<unsigned char>& data);
		bool send_binary_packet(const vector<unsigned char>& data);

	private:
		bool decompress_binary_packet(const vector<unsigned char>& data);
		bool decrypt_binary_packet(const vector<unsigned char>& data);
		bool receive_binary_packet(const vector<unsigned char>& data);

	private:
		bool normal_message(shared_ptr<container::value_container> message);
		bool connection_message(shared_ptr<container::value_container> message);
		bool request_files(shared_ptr<container::value_container> message);
		bool echo_message(shared_ptr<container::value_container> message);

	private:
		void generate_key(void);
		bool same_key_check(shared_ptr<container::value> key);
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
		function<void(shared_ptr<container::value_container>)> _received_message;
		function<void(const wstring&, const wstring&, const wstring&, const wstring&)> _received_file;
		function<void(const wstring&, const wstring&, const wstring&, const wstring&, const vector<unsigned char>&)> _received_data;

	private:
		shared_ptr<asio::ip::tcp::socket> _socket;

	private:
		shared_ptr<threads::thread_pool> _thread_pool;
		map<wstring, function<bool(shared_ptr<container::value_container>)>> _message_handlers;
	};
}