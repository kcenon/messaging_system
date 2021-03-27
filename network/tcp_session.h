#pragma once

#include "container.h"
#include "thread_pool.h"
#include "data_handling.h"
#include "session_types.h"

#include <map>
#include <memory>
#include <string>
#include <functional>

#ifdef ASIO_STANDALONE
#include "asio.hpp"
#else
#include <boost/asio.hpp>
#endif

namespace network
{
	class tcp_session : public std::enable_shared_from_this<tcp_session>, data_handling
	{
	public:
#ifdef ASIO_STANDALONE
		tcp_session(const std::wstring& source_id, const std::wstring& connection_key, asio::ip::tcp::socket& socket);
#else
		tcp_session(const std::wstring& source_id, const std::wstring& connection_key, boost::asio::ip::tcp::socket& socket);
#endif
		~tcp_session(void);

	public:
		std::shared_ptr<tcp_session> get_ptr(void);

	public:
		void set_connection_notification(const std::function<void(std::shared_ptr<tcp_session>, const bool&)>& notification);
		void set_message_notification(const std::function<void(std::shared_ptr<container::value_container>)>& notification);
		void set_file_notification(const std::function<void(const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&)>& notification);
		void set_binary_notification(const std::function<void(const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&, const std::vector<unsigned char>&)>& notification);

	public:
		const session_types get_session_type(void);
		const std::wstring target_id(void);
		const std::wstring target_sub_id(void);

	public:
		void start(const bool& encrypt_mode, const bool& compress_mode, const unsigned short& high_priority, const unsigned short& normal_priority, const unsigned short& low_priority);
		void stop(void);

	public:
		void echo(void);
		void send(std::shared_ptr<container::value_container> message);
		void send(const std::wstring target_id, const std::wstring& target_sub_id, const std::vector<unsigned char>& data);
		void send(const std::wstring source_id, const std::wstring& source_sub_id, const std::wstring target_id, const std::wstring& target_sub_id, const std::vector<unsigned char>& data);

	protected:
		void receive_on_tcp(const data_modes& data_mode, const std::vector<unsigned char>& data) override;
		void disconnected(void) override;

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
		bool load_file(const std::vector<unsigned char>& data);
		bool compress_file(const std::vector<unsigned char>& data);
		bool encrypt_file(const std::vector<unsigned char>& data);
		bool send_file(const std::vector<unsigned char>& data);

	private:
		bool decompress_file(const std::vector<unsigned char>& data);
		bool decrypt_file(const std::vector<unsigned char>& data);
		bool receive_file(const std::vector<unsigned char>& data);

		// binary
	private:
		bool compress_binary(const std::vector<unsigned char>& data);
		bool encrypt_binary(const std::vector<unsigned char>& data);
		bool send_binary(const std::vector<unsigned char>& data);

	private:
		bool decompress_binary(const std::vector<unsigned char>& data);
		bool decrypt_binary(const std::vector<unsigned char>& data);
		bool receive_binary(const std::vector<unsigned char>& data);

	private:
		bool normal_message(std::shared_ptr<container::value_container> message);
		bool connection_message(std::shared_ptr<container::value_container> message);
		bool echo_message(std::shared_ptr<container::value_container> message);

	private:
		void generate_key(void);
		bool same_key_check(std::shared_ptr<container::value> key);
		bool same_id_check(void);

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

	private:
		bool _compress_mode;
		bool _encrypt_mode;
		std::wstring _key;
		std::wstring _iv;

	private:
		std::function<void(std::shared_ptr<tcp_session>, const bool&)> _connection;
		std::function<void(std::shared_ptr<container::value_container>)> _received_message;
		std::function<void(const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&)> _received_file;
		std::function<void(const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&, const std::vector<unsigned char>&)> _received_data;

	private:
#ifdef ASIO_STANDALONE
		std::shared_ptr<asio::ip::tcp::socket> _socket;
#else
		std::shared_ptr<boost::asio::ip::tcp::socket> _socket;
#endif

	private:
		std::shared_ptr<threads::thread_pool> _thread_pool;
		std::map<std::wstring, std::function<bool(std::shared_ptr<container::value_container>)>> _message_handlers;
	};
}