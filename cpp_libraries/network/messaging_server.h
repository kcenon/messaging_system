#pragma once

#include "container.h"
#include "session_types.h"
#include "thread_pool.h"

#include <memory>
#include <vector>
#include <string>
#include <thread>
#include <future>

#include "asio.hpp"

using namespace std;

namespace network
{
	class messaging_session;
	class messaging_server : public enable_shared_from_this<messaging_server>
	{
	public:
		messaging_server(const wstring& source_id);
		~messaging_server(void);

	public:
		shared_ptr<messaging_server> get_ptr(void);

	public:
		void set_encrypt_mode(const bool& encrypt_mode);
		void set_compress_mode(const bool& compress_mode);
		void set_connection_key(const wstring& connection_key);
		void set_ignore_target_ids(const vector<wstring>& ignore_target_ids);
		void set_ignore_snipping_targets(const vector<wstring>& ignore_snipping_targets);
		void set_possible_session_types(const vector<session_types>& possible_session_types);
		void set_session_limit_count(const bool& session_limit_count);

	public:
		void set_connection_notification(const function<void(const wstring&, const wstring&, const bool&)>& notification);
		void set_message_notification(const function<void(shared_ptr<container::value_container>)>& notification);
		void set_file_notification(const function<void(const wstring&, const wstring&, const wstring&, const wstring&)>& notification);
		void set_binary_notification(const function<void(const wstring&, const wstring&, const wstring&, const wstring&, const vector<unsigned char>&)>& notification);

	public:
		void start(const unsigned short& port, const unsigned short& high_priority = 8, const unsigned short& normal_priority = 8, const unsigned short& low_priority = 8);
		void wait_stop(const unsigned int& seconds = 0);
		void stop(void);

	public:
		void echo(void);
		void send(const container::value_container& message);
		void send(shared_ptr<container::value_container> message);
		void send_files(const container::value_container& message);
		void send_files(shared_ptr<container::value_container> message);
		void send_binary(const wstring target_id, const wstring& target_sub_id, const vector<unsigned char>& data);
		void send_binary(const wstring source_id, const wstring& source_sub_id, const wstring target_id, const wstring& target_sub_id, const vector<unsigned char>& data);

	protected:
		void wait_connection(void);
		bool check_confirm_condition(void);
		void connect_condition(shared_ptr<messaging_session> target, const bool& condition);

	private:
		void received_message(shared_ptr<container::value_container> message);
		void received_binary(const wstring& source_id, const wstring& source_sub_id, const wstring& target_id, const wstring& target_sub_id, const vector<unsigned char>& data);

	private:
		bool _encrypt_mode;
		bool _compress_mode;
		wstring _source_id;
		wstring _connection_key;
		unsigned short _high_priority;
		unsigned short _normal_priority;
		unsigned short _low_priority;
		size_t _session_limit_count;
		vector<wstring> _ignore_target_ids;
		vector<wstring> _ignore_snipping_targets;
		vector<session_types> _possible_session_types;

	private:
		thread _thread;
		shared_ptr<asio::io_context> _io_context;
		shared_ptr<asio::ip::tcp::acceptor> _acceptor;

	private:
		promise<bool> _promise_status;
		future<bool> _future_status;
		vector<shared_ptr<messaging_session>> _sessions;

	private:
		function<void(const wstring&, const wstring&, const bool&)> _connection;
		function<void(shared_ptr<container::value_container>)> _received_message;
		function<void(const wstring&, const wstring&, const wstring&, const wstring&)> _received_file;
		function<void(const wstring&, const wstring&, const wstring&, const wstring&, const vector<unsigned char>&)> _received_data;

	private:
		shared_ptr<threads::thread_pool> _thread_pool;
	};
}