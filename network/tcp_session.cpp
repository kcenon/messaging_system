#include "tcp_session.h"

#include "values/bool_value.h"

#include "thread_pool.h"
#include "thread_worker.h"
#include "job_pool.h"
#include "job.h"

namespace network
{
	using namespace container;
	using namespace threads;

	tcp_session::tcp_session(asio::ip::tcp::socket& socket) : _confirm(false), _bridge_line(false), _buffer_size(1024), _socket(std::make_shared<asio::ip::tcp::socket>(std::move(socket)))
	{
		_socket->set_option(asio::ip::tcp::no_delay(true));
		_socket->set_option(asio::socket_base::keep_alive(true));
		_socket->set_option(asio::socket_base::receive_buffer_size(_buffer_size));

		_message_handlers.insert({ L"confirm", std::bind(&tcp_session::confirm_message, this, std::placeholders::_1) });
		_message_handlers.insert({ L"echo", std::bind(&tcp_session::echo_message, this, std::placeholders::_1) });
		_message_handlers.insert({ L"disconnect", std::bind(&tcp_session::disconnect_message, this, std::placeholders::_1) });
	}

	tcp_session::~tcp_session(void)
	{

	}

	std::shared_ptr<tcp_session> tcp_session::get_ptr(void)
	{
		return shared_from_this();
	}

	void tcp_session::start(const unsigned short& high_priority, const unsigned short& normal_priority, const unsigned short& low_priority)
	{
		thread_pool::handle().stop(true);
		thread_pool::handle().append(std::make_shared<thread_worker>(priorities::top), true);
		for (unsigned short high = 0; high < high_priority; ++high)
		{
			thread_pool::handle().append(std::make_shared<thread_worker>(priorities::high), true);
		}
		for (unsigned short normal = 0; normal < normal_priority; ++normal)
		{
			thread_pool::handle().append(std::make_shared<thread_worker>(priorities::normal, std::vector<priorities> { priorities::high }), true);
		}
		for (unsigned short low = 0; low < low_priority; ++low)
		{
			thread_pool::handle().append(std::make_shared<thread_worker>(priorities::low, std::vector<priorities> { priorities::high, priorities::normal }), true);
		}
	}

	void tcp_session::stop(void)
	{
		thread_pool::handle().stop();
	}

	void tcp_session::send(std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}

		if (_bridge_line)
		{
			job_pool::handle().push(std::make_shared<job>(priorities::top, message->serialize_array(), std::bind(&tcp_session::send_packet, this, std::placeholders::_1)));

			return;
		}

		if (message->target_id() != _target_id)
		{
			return;
		}

		if (!message->target_sub_id().empty() && message->target_sub_id() != _target_id)
		{
			return;
		}

		job_pool::handle().push(std::make_shared<job>(priorities::top, message->serialize_array(), std::bind(&tcp_session::send_packet, this, std::placeholders::_1)));
	}

	bool tcp_session::send_packet(const std::vector<char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		return true;
	}

	bool tcp_session::receive_packet(const std::vector<char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		job_pool::handle().push(std::make_shared<job>(priorities::high, data, std::bind(&tcp_session::parsing_packet, this, std::placeholders::_1)));

		return true;
	}

	bool tcp_session::parsing_packet(const std::vector<char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		std::shared_ptr<container::value_container> message = std::make_shared<container::value_container>(data);
		if (message == nullptr)
		{
			return false;
		}

		auto target = _message_handlers.find(message->message_type());
		if (target == _message_handlers.end())
		{
			return normal_message(message);
		}

		return target->second(message);
	}

	bool tcp_session::normal_message(std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return false;
		}

		return true;
	}

	bool tcp_session::confirm_message(std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return false;
		}

		return true;
	}

	bool tcp_session::echo_message(std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return false;
		}

		if (!_confirm)
		{
			return false;
		}

		std::shared_ptr<value> response = (*message)[L"response"];
		if (!response->is_null())
		{
			return true;
		}

		message->swap_header();

		message << std::make_shared<bool_value>(L"response", true);

		job_pool::handle().push(std::make_shared<job>(priorities::top, message->serialize_array(), std::bind(&tcp_session::send_packet, this, std::placeholders::_1)));

		return true;
	}

	bool tcp_session::disconnect_message(std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return false;
		}

		return true;
	}
}
