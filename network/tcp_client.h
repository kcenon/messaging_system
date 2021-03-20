#pragma once

#include "container.h"
#include "thread_pool.h"
#include "data_handling.h"
#include "session_types.h"

#include <map>
#include <memory>
#include <string>
#include <thread>
#include <functional>

#include "asio.hpp"

namespace network
{
	class tcp_client : public std::enable_shared_from_this<tcp_client>, public data_handling
	{
	public:
		tcp_client(const std::wstring& source_id, const std::wstring& connection_key);
		~tcp_client(void);

	public:
		std::shared_ptr<tcp_client> get_ptr(void);

	public:
		void set_auto_echo(const bool& auto_echo, const unsigned short& echo_interval);
		void set_bridge_line(const bool& bridge_line);
		void set_compress_mode(const bool& compress_mode);
		void set_session_types(const session_types& session_type);

	public:
		void start(const std::wstring& ip, const unsigned short& port, const unsigned short& high_priority, const unsigned short& normal_priority, const unsigned short& low_priority);
		void stop(void);

	public:
		void send(const container::value_container& message);
		void send(std::shared_ptr<container::value_container> message);

	protected:
		void send_connection(void);
		void receive_on_tcp(const data_modes& data_mode, const std::vector<char>& data) override;

	private:
		bool compress_packet(const std::vector<char>& data);
		bool encrypt_packet(const std::vector<char>& data);
		bool send_packet(const std::vector<char>& data);

	private:
		bool decompress_packet(const std::vector<char>& data);
		bool decrypt_packet(const std::vector<char>& data);
		bool receive_packet(const std::vector<char>& data);

	private:
		bool normal_message(std::shared_ptr<container::value_container> message);
		bool confirm_message(std::shared_ptr<container::value_container> message);
		bool echo_message(std::shared_ptr<container::value_container> message);

	private:
		bool _confirm;
		bool _auto_echo;
		bool _bridge_line;
		session_types _session_type;
		std::wstring _source_id;
		std::wstring _source_sub_id;
		std::wstring _target_id;
		std::wstring _target_sub_id;
		std::wstring _connection_key;
		unsigned short _auto_echo_interval_seconds;
		std::vector<std::wstring> _snipping_targets;

	private:
		bool _compress_mode;
		bool _encrypt_mode;
		std::wstring _key;
		std::wstring _iv;

	private:
		std::thread _thread;
		std::shared_ptr<asio::io_context> _io_context;
		std::shared_ptr<asio::ip::tcp::socket> _socket;
		std::shared_ptr<threads::thread_pool> _thread_pool;
		std::map<std::wstring, std::function<bool(std::shared_ptr<container::value_container>)>> _message_handlers;
	};
}
