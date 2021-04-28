#pragma once

#include "container.h"

#include <memory>
#include <vector>
#include <string>
#include <thread>
#include <future>

#ifdef ASIO_STANDALONE
#include "asio.hpp"
#else
#include <boost/asio.hpp>
#endif

namespace network
{
	class messaging_session;
	class messaging_server : public std::enable_shared_from_this<messaging_server>
	{
	public:
		messaging_server(const std::wstring& source_id);
		~messaging_server(void);

	public:
		std::shared_ptr<messaging_server> get_ptr(void);

	public:
		void set_encrypt_mode(const bool& encrypt_mode);
		void set_compress_mode(const bool& compress_mode);
		void set_broadcast_mode(const bool& broadcast_mode);
		void set_connection_key(const std::wstring& connection_key);

	public:
		void set_connection_notification(const std::function<void(const std::wstring&, const std::wstring&, const bool&)>& notification);
		void set_message_notification(const std::function<void(std::shared_ptr<container::value_container>)>& notification);
		void set_file_notification(const std::function<void(const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&)>& notification);
		void set_binary_notification(const std::function<void(const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&, const std::vector<unsigned char>&)>& notification);

	public:
		void start(const unsigned short& port, const unsigned short& high_priority = 8, const unsigned short& normal_priority = 8, const unsigned short& low_priority = 8);
		void wait_stop(const unsigned int& seconds = 0);
		void stop(void);

	public:
		void echo(void);
		void send(const container::value_container& message);
		void send(std::shared_ptr<container::value_container> message);
		void send_files(const container::value_container& message);
		void send_files(std::shared_ptr<container::value_container> message);
		void send_binary(const std::wstring target_id, const std::wstring& target_sub_id, const std::vector<unsigned char>& data);
		void send_binary(const std::wstring source_id, const std::wstring& source_sub_id, const std::wstring target_id, const std::wstring& target_sub_id, const std::vector<unsigned char>& data);

	protected:
		void wait_connection(void);
		void connect_condition(std::shared_ptr<messaging_session> target, const bool& condition);

	private:
		void received_message(std::shared_ptr<container::value_container> message);
		void received_binary(const std::wstring& source_id, const std::wstring& source_sub_id, const std::wstring& target_id, const std::wstring& target_sub_id, const std::vector<unsigned char>& data);

	private:
		bool _encrypt_mode;
		bool _compress_mode;
		bool _broadcast_mode;
		std::wstring _source_id;
		std::wstring _connection_key;
		unsigned short _high_priority;
		unsigned short _normal_priority;
		unsigned short _low_priority;

	private:
		std::thread _thread;
#ifdef ASIO_STANDALONE
		std::shared_ptr<asio::io_context> _io_context;
		std::shared_ptr<asio::ip::tcp::acceptor> _acceptor;
#else
		std::shared_ptr<boost::asio::io_context> _io_context;
		std::shared_ptr<boost::asio::ip::tcp::acceptor> _acceptor;
#endif

	private:
		std::promise<bool> _promise_status;
		std::future<bool> _future_status;
		std::vector<std::shared_ptr<messaging_session>> _sessions;

	private:
		std::function<void(const std::wstring&, const std::wstring&, const bool&)> _connection;
		std::function<void(std::shared_ptr<container::value_container>)> _received_message;
		std::function<void(const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&)> _received_file;
		std::function<void(const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&, const std::vector<unsigned char>&)> _received_data;
	};
}