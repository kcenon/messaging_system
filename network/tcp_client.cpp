#include "tcp_client.h"

#include "thread_pool.h"
#include "thread_worker.h"
#include "job_pool.h"
#include "job.h"

#include <functional>

namespace network
{
	using namespace concurrency;

	tcp_client::tcp_client(void) : _confirm(false), _bridge_line(false)
	{

	}

	tcp_client::~tcp_client(void)
	{

	}

	std::shared_ptr<tcp_client> tcp_client::get_ptr(void)
	{
		return shared_from_this();
	}

	void tcp_client::start(const unsigned short& high_priority, const unsigned short& normal_priority, const unsigned short& low_priority)
	{
		thread_pool::handle().clear();
		thread_pool::handle().append(std::make_shared<thread_worker>(priorities::top));
		for (unsigned short high = 0; high < high_priority; ++high)
		{
			thread_pool::handle().append(std::make_shared<thread_worker>(priorities::high));
		}
		for (unsigned short normal = 0; normal < normal_priority; ++normal)
		{
			thread_pool::handle().append(std::make_shared<thread_worker>(priorities::normal, std::vector<priorities> { priorities::high }));
		}
		for (unsigned short low = 0; low < low_priority; ++low)
		{
			thread_pool::handle().append(std::make_shared<thread_worker>(priorities::low, std::vector<priorities> { priorities::high, priorities::normal }));
		}
		thread_pool::handle().start();
	}

	void tcp_client::stop(void)
	{
		thread_pool::handle().stop();
	}

	void tcp_client::send(const container::value_container& message)
	{
		send(std::make_shared<container::value_container>(message));
	}

	void tcp_client::send(std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return;
		}

		job_pool::handle().push(std::make_shared<job>(priorities::top, message->serialize_array(), std::bind(&tcp_client::send_packet, this, std::placeholders::_1)));
	}

	bool tcp_client::send_packet(const std::vector<char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		return true;
	}

	bool tcp_client::receive_packet(const std::vector<char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		job_pool::handle().push(std::make_shared<job>(priorities::high, data, std::bind(&tcp_client::parsing_packet, this, std::placeholders::_1)));

		return true;
	}

	bool tcp_client::parsing_packet(const std::vector<char>& data)
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

	bool tcp_client::normal_message(std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return false;
		}

		if (!_confirm)
		{
			return false;
		}

		return true;
	}

	bool tcp_client::confirm_message(std::shared_ptr<container::value_container> message)
	{
		if (message == nullptr)
		{
			return false;
		}

		return true;
	}
}