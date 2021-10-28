#include "job.h"

#include "logging.h"
#include "job_pool.h"
#include "converting.h"
#include "folder_handler.h"
#include "file_handler.h"

#include "container.h"
#include "values/string_value.h"

#include "fmt/xchar.h"
#include "fmt/format.h"
#include "ChakraCore.h"

#include "crossguid/guid.hpp"

namespace threads
{
	using namespace logging;
	using namespace container;
	using namespace converting;
	using namespace file_handler;
	using namespace folder_handler;

	job::job(const priorities& priority)
		: _priority(priority), _working_callback(nullptr), _working_callback2(nullptr), _temporary_stored(false), _temporary_stored_path(L"")
	{
	}

	job::job(const priorities& priority, const std::vector<unsigned char>& data)
		: _priority(priority), _data(data), _working_callback(nullptr), _working_callback2(nullptr), _temporary_stored(false), _temporary_stored_path(L"")
	{
	}

	job::job(const priorities& priority, const std::function<bool(void)>& working_callback)
		: _priority(priority), _working_callback(working_callback), _working_callback2(nullptr), _temporary_stored(false), _temporary_stored_path(L"")
	{
	}

	job::job(const priorities& priority, const std::vector<unsigned char>& data, const std::function<bool(const std::vector<unsigned char>&)>& working_callback)
		: _priority(priority), _data(data), _working_callback(nullptr), _working_callback2(working_callback), _temporary_stored(false), _temporary_stored_path(L"")
	{
	}

	job::~job(void)
	{
	}

	std::shared_ptr<job> job::get_ptr(void)
	{
		return shared_from_this();
	}

	const priorities job::priority(void)
	{
		return _priority;
	}

	void job::set_job_pool(std::shared_ptr<job_pool> job_pool)
	{
		_job_pool = job_pool;
	}

	bool job::work(const priorities& worker_priority)
	{
		load();

		if (_working_callback != nullptr)
		{
			bool result = _working_callback();

			logger::handle().write(logging::logging_level::sequence, fmt::format(L"completed working callback function without value on job: job priority[{}], worker priority[{}]", _priority, worker_priority));

			return result;
		}

		if (_working_callback2 != nullptr)
		{
			bool result = _working_callback2(_data);

			logger::handle().write(logging::logging_level::sequence, fmt::format(L"completed working callback function with value on job: job priority[{}], worker priority[{}]", _priority, worker_priority));

			return result;
		}

		if (!working(worker_priority))
		{
			logger::handle().write(logging::logging_level::sequence, fmt::format(L"cannot complete working function on job: job priority[{}], worker priority[{}]", _priority, worker_priority));

			return false;
		}

		return true;
	}

	void job::save(void)
	{
		_temporary_stored = true;
		_temporary_stored_path = fmt::format(L"{}{}.job", folder::get_temporary_folder(), converter::to_wstring(xg::newGuid().str()));

		file::save(_temporary_stored_path, _data);
		_data.clear();
	}

	bool job::working(const priorities& worker_priority)
	{
		auto start = logger::handle().chrono_start();

		std::shared_ptr<value_container> source_data = std::make_shared<value_container>(_data);
		if (source_data == nullptr)
		{
			return false;
		}

		std::wstring script = source_data->get_value(L"scripts")->to_string();
		if (script.empty())
		{
			logger::handle().write(logging::logging_level::information, do_script(converter::to_wstring(_data)), start);

			return true;
		}

		if (source_data->message_type() == L"data_container")
		{
			logger::handle().write(logging::logging_level::information, do_script(script), start);
		}
		else
		{
			std::shared_ptr<job_pool> current_job_pool = _job_pool.lock();
			if (current_job_pool != nullptr)
			{
				std::shared_ptr<value_container> target_data = source_data->copy(false);
				target_data->swap_header();
				target_data->add(std::make_shared<string_value>(L"script_result", do_script(script)));

				current_job_pool->push(std::make_shared<job>(_priority, target_data->serialize_array()));
				current_job_pool.reset();
			}
		}

		return true;
	}

	std::wstring job::do_script(const std::wstring& script)
	{
		try
		{
			JsRuntimeHandle runtime;
			JsContextRef context;
			JsValueRef result;
			unsigned currentSourceContext = 0;

			JsCreateRuntime(JsRuntimeAttributeNone, nullptr, &runtime);

			JsCreateContext(runtime, &context);
			JsSetCurrentContext(context);

			JsRunScript(script.c_str(), currentSourceContext++, L"", &result);

			JsValueRef resultJSString;
			JsConvertValueToString(result, &resultJSString);

			const wchar_t* resultWC;
			size_t stringLength;
			JsStringToPointer(resultJSString, &resultWC, &stringLength);

			std::wstring resultW(resultWC);

			JsSetCurrentContext(JS_INVALID_REFERENCE);
			JsDisposeRuntime(runtime);

			return resultW;
		}
		catch (...)
		{
			return L"";
		}
	}

	void job::load(void)
	{
		if (!_temporary_stored)
		{
			return;
		}

		_data = file::load(_temporary_stored_path);
		file::remove(_temporary_stored_path);
	}
}