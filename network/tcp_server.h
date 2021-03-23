#pragma once

#include "container.h"

#include <memory>
#include <vector>
#include <string>
#include <thread>

#include "asio.hpp"

namespace network
{
	class tcp_session;
	class tcp_server : public std::enable_shared_from_this<tcp_server>
	{
	public:
		tcp_server(const std::wstring& source_id, const std::wstring& connection_key);
		~tcp_server(void);

	public:
		std::shared_ptr<tcp_server> get_ptr(void);

	public:
		void set_encrypt_mode(const bool& encrypt_mode);
		void set_compress_mode(const bool& compress_mode);

	public:
		void set_connection_notification(const std::function<void(const std::wstring&, const std::wstring&, const bool&)>& notification);
		void set_message_notification(const std::function<void(std::shared_ptr<container::value_container>)>& notification);
		void set_file_notification(const std::function<void(const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&)>& notification);

	public:
		void start(const unsigned short& port, const unsigned short& high_priority, const unsigned short& normal_priority, const unsigned short& low_priority);
		void stop(void);

	public:
		void send(const container::value_container& message);
		void send(std::shared_ptr<container::value_container> message);

	protected:
		void wait_connection(void);
		void connect_condition(std::shared_ptr<tcp_session> target, const bool& condition);

	private:
		bool _encrypt_mode;
		bool _compress_mode;
		std::wstring _source_id;
		std::wstring _connection_key;
		unsigned short _high_priority;
		unsigned short _normal_priority;
		unsigned short _low_priority;

	private:
		std::thread _thread;
		std::shared_ptr<asio::io_context> _io_context;
		std::shared_ptr<asio::ip::tcp::acceptor> _acceptor;
		std::vector<std::shared_ptr<tcp_session>> _sessions;

	private:
		std::function<void(const std::wstring&, const std::wstring&, const bool&)> _connection;
		std::function<void(std::shared_ptr<container::value_container>)> _received_message;
		std::function<void(const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&)> _received_file;
	};
}