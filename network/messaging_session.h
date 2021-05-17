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

namespace network
{
	class messaging_session : public std::enable_shared_from_this<messaging_session>, data_handling
	{
	public:
		messaging_session(const std::wstring& source_id, const std::wstring& connection_key, asio::ip::tcp::socket& socket);
		~messaging_session(void);

	public:
		std::shared_ptr<messaging_session> get_ptr(void);

	public:
		void set_kill_code(const bool& kill_code);
		void set_connection_notification(const std::function<void(std::shared_ptr<messaging_session>, const bool&)>& notification);
		void set_message_notification(const std::function<void(std::shared_ptr<container::value_container>)>& notification);
		void set_file_notification(const std::function<void(const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&)>& notification);
		void set_binary_notification(const std::function<void(const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&, const std::vector<unsigned char>&)>& notification);

	public:
		const session_types get_session_type(void);
		const std::wstring target_id(void);
		const std::wstring target_sub_id(void);

	public:
		void start(const bool& encrypt_mode, const bool& compress_mode, const std::vector<std::wstring>& ignore_snipping_targets, const unsigned short& high_priority = 8, const unsigned short& normal_priority = 8, const unsigned short& low_priority = 8);
		void stop(void);

	public:
		void echo(void);
		void send(std::shared_ptr<container::value_container> message);
		void send_files(std::shared_ptr<container::value_container> message);
		void send_binary(const std::wstring target_id, const std::wstring& target_sub_id, const std::vector<unsigned char>& data);
		void send_binary(const std::wstring source_id, const std::wstring& source_sub_id, const std::wstring target_id, const std::wstring& target_sub_id, const std::vector<unsigned char>& data);

	protected:
		void receive_on_tcp(const data_modes& data_mode, const std::vector<unsigned char>& data) override;
		void disconnected(void) override;

	protected:
		bool check_confirm_condition(void);
		bool contained_snipping_target(const std::wstring& snipping_target);

		// packet
	private:
		bool compress_packet(const std::vector<unsigned char>& data);
		bool encrypt_packet(const std::vector<unsigned char>& data);
		bool send_packet(const std::vector<unsigned char>& data);

	private:
		bool decompress_packet(const std::vector<unsigned char>& data);
		bool decrypt_packet(const std::vector<unsigned char>& data);
		bool receive_packet(const std::vector<unsigned char>& data);

		// file
	private:
		bool load_file_packet(const std::vector<unsigned char>& data);
		bool compress_file_packet(const std::vector<unsigned char>& data);
		bool encrypt_file_packet(const std::vector<unsigned char>& data);
		bool send_file_packet(const std::vector<unsigned char>& data);

	private:
		bool decompress_file_packet(const std::vector<unsigned char>& data);
		bool decrypt_file_packet(const std::vector<unsigned char>& data);
		bool receive_file_packet(const std::vector<unsigned char>& data);
		bool notify_file_packet(const std::vector<unsigned char>& data);

		// binary
	private:
		bool compress_binary_packet(const std::vector<unsigned char>& data);
		bool encrypt_binary_packet(const std::vector<unsigned char>& data);
		bool send_binary_packet(const std::vector<unsigned char>& data);

	private:
		bool decompress_binary_packet(const std::vector<unsigned char>& data);
		bool decrypt_binary_packet(const std::vector<unsigned char>& data);
		bool receive_binary_packet(const std::vector<unsigned char>& data);

	private:
		bool normal_message(std::shared_ptr<container::value_container> message);
		bool connection_message(std::shared_ptr<container::value_container> message);
		bool request_files(std::shared_ptr<container::value_container> message);
		bool echo_message(std::shared_ptr<container::value_container> message);

	private:
		void generate_key(void);
		bool same_key_check(std::shared_ptr<container::value> key);
		bool same_id_check(void);

	private:
		bool _confirm;
		bool _kill_code;
		bool _auto_echo;
		bool _bridge_line;
		session_types _session_type;
		std::wstring _source_id;
		std::wstring _source_sub_id;
		std::wstring _target_id;
		std::wstring _target_sub_id;
		std::wstring _connection_key;
		std::vector<std::wstring> _snipping_targets;
		std::vector<std::wstring> _ignore_snipping_targets;

	private:
		bool _compress_mode;
		bool _encrypt_mode;
		std::wstring _key;
		std::wstring _iv;

	private:
		std::function<void(std::shared_ptr<messaging_session>, const bool&)> _connection;
		std::function<void(std::shared_ptr<container::value_container>)> _received_message;
		std::function<void(const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&)> _received_file;
		std::function<void(const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&, const std::vector<unsigned char>&)> _received_data;

	private:
		std::shared_ptr<asio::ip::tcp::socket> _socket;

	private:
		std::shared_ptr<threads::thread_pool> _thread_pool;
		std::map<std::wstring, std::function<bool(std::shared_ptr<container::value_container>)>> _message_handlers;
	};
}