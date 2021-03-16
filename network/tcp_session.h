#pragma once

#include "container.h"

#include <map>
#include <memory>
#include <string>
#include <functional>

#include "asio.hpp"

namespace network
{
	class tcp_session : public std::enable_shared_from_this<tcp_session>
	{
	public:
		tcp_session(asio::ip::tcp::socket& socket);
		~tcp_session(void);

	public:
		std::shared_ptr<tcp_session> get_ptr(void);

	public:
		void start(const unsigned short& high_priority, const unsigned short& normal_priority, const unsigned short& low_priority);
		void stop(void);

	public:
		void send(std::shared_ptr<container::value_container> message);

	private:
		bool send_packet(const std::vector<char>& data);
		bool receive_packet(const std::vector<char>& data);

	private:
		bool parsing_packet(const std::vector<char>& data);

	private:
		bool normal_message(std::shared_ptr<container::value_container> message);
		bool confirm_message(std::shared_ptr<container::value_container> message);
		bool echo_message(std::shared_ptr<container::value_container> message);
		bool disconnect_message(std::shared_ptr<container::value_container> message);

	private:
		bool _confirm;
		bool _bridge_line;
		size_t _buffer_size;
		std::wstring _source_id;
		std::wstring _source_sub_id;
		std::wstring _target_id;
		std::wstring _target_sub_id;

	private:
		std::shared_ptr<asio::ip::tcp::socket> _socket;
		std::map<std::wstring, std::function<bool(std::shared_ptr<container::value_container>)>> _message_handlers;
	};
}