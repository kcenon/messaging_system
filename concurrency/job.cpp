#include "job.h"

namespace concurrency
{
	job::job(const std::function<void(void)>& notification) 
		: _priority(priorities::normal), _notification(notification)
	{
	}

	job::~job(void)
	{
	}

	std::shared_ptr<job> job::get_ptr(void)
	{
		return shared_from_this();
	}

	void job::set_priority(const priorities& priority)
	{
		_priority = priority;
	}

	const priorities job::priority(void)
	{
		return _priority;
	}

	void job::work(void)
	{
		if (_notification != nullptr)
		{
			_notification();

			return;
		}
	}
}