#include "job.h"

#include "logging.h"
#include "job_pool.h"
#include "converting.h"
#include "folder_handler.h"
#include "file_handler.h"

#include "fmt/xchar.h"
#include "fmt/format.h"

#include <chrono>
#include <future>

#ifdef __USE_CHAKRA_CORE__
#include "ChakraCore.h"
#else
#include "Python.h"
#endif

#ifdef __OSX__
#include <uuid/uuid.h>
#else
#include "crossguid/guid.hpp"
#endif

namespace threads
{
	using namespace logging;
	using namespace converting;
	using namespace file_handler;
	using namespace folder_handler;

	job::job(const priorities& priority)
		: _priority(priority), _working_callback(nullptr), _working_callback2(nullptr), _temporary_stored_path(L"")
	{
	}

	job::job(const priorities& priority, const vector<unsigned char>& data)
		: _priority(priority), _data(data), _working_callback(nullptr), _working_callback2(nullptr), _temporary_stored_path(L"")
	{
	}

	job::job(const priorities& priority, const function<void(void)>& working_callback)
		: _priority(priority), _working_callback(working_callback), _working_callback2(nullptr), _temporary_stored_path(L"")
	{
	}

	job::job(const priorities& priority, const vector<unsigned char>& data, const function<void(const vector<unsigned char>&)>& working_callback)
		: _priority(priority), _data(data), _working_callback(nullptr), _working_callback2(working_callback), _temporary_stored_path(L"")
	{
	}

	job::~job(void)
	{
	}

	shared_ptr<job> job::get_ptr(void)
	{
		return shared_from_this();
	}

	const priorities job::priority(void)
	{
		return _priority;
	}

	void job::set_job_pool(shared_ptr<job_pool> job_pool)
	{
		_job_pool = job_pool;
	}

	bool job::work(const priorities& worker_priority)
	{
		load();

		if (_working_callback != nullptr)
		{
			try
			{
				_working_callback();
			}
			catch (...)
			{
				logger::handle().write(logging_level::sequence,
					fmt::format(L"cannot complete working function on job: job priority[{}], worker priority[{}]", (int)_priority, (int)worker_priority));

				return false;
			}

			logger::handle().write(logging_level::sequence, 
				fmt::format(L"completed working callback function without value on job: job priority[{}], worker priority[{}]", (int)_priority, (int)worker_priority));

			return true;
		}

		if (_working_callback2 != nullptr)
		{
			try
			{
				_working_callback2(_data);
			}
			catch (...)
			{
				logger::handle().write(logging_level::sequence,
					fmt::format(L"cannot complete working function on job: job priority[{}], worker priority[{}]", (int)_priority, (int)worker_priority));

				return false;
			}

			logger::handle().write(logging_level::sequence, 
				fmt::format(L"completed working callback function with value on job: job priority[{}], worker priority[{}]", (int)_priority, (int)worker_priority));

			return true;
		}

		try
		{
			working(worker_priority);

			logger::handle().write(logging_level::sequence,
				fmt::format(L"completed working function on job: job priority[{}], worker priority[{}]", (int)_priority, (int)worker_priority));
		}
		catch (...) 
		{
			logger::handle().write(logging_level::sequence,
				fmt::format(L"cannot complete working function on job: job priority[{}], worker priority[{}]", (int)_priority, (int)worker_priority));

			return false;
		}

		return true;
	}

	void job::save(void)
	{
		if (_data.empty())
		{
			return;
		}

#ifdef __OSX__
		uuid_t uuidObj;
		uuid_generate(uuidObj);
		_temporary_stored_path = fmt::format(L"{}{}.job", folder::get_temporary_folder(), converter::to_wstring(string(uuidObj, uuidObj + 16)));
#else
		_temporary_stored_path = fmt::format(L"{}{}.job", folder::get_temporary_folder(), converter::to_wstring(xg::newGuid().str()));
#endif

		file::save(_temporary_stored_path, _data);
		_data.clear();
	}

	void job::working(const priorities& worker_priority)
	{
		auto start = logger::handle().chrono_start();

#ifndef __USE_TYPE_CONTAINER__
		shared_ptr<json::value> source_data = make_shared<json::value>(json::value::parse(converter::to_string(_data)));
#else
		shared_ptr<value_container> source_data = make_shared<value_container>(_data, true);
#endif
		if (source_data == nullptr)
		{
			return;
		}

#ifndef __USE_TYPE_CONTAINER__
#ifdef _WIN32
		wstring script = (*source_data)[L"data"][L"scripts"].as_string();
#else
		wstring script = converter::to_wstring((*source_data)["data"]["scripts"].as_string());
#endif
#else
		wstring script = source_data->get_value(L"scripts")->to_string();
#endif
		if (script.empty())
		{
			logger::handle().write(logging_level::information, do_script(converter::to_wstring(_data)), start);

			return;
		}

#ifndef __USE_TYPE_CONTAINER__
#ifdef _WIN32
		if ((*source_data)[L"header"][L"message_type"].as_string() == L"data_container")
#else
		if ((*source_data)["header"]["message_type"].as_string() == "data_container")
#endif
#else
		if (source_data->message_type() == L"data_container")
#endif
		{
			logger::handle().write(logging_level::information, do_script(script), start);
		}
		else
		{
			shared_ptr<job_pool> current_job_pool = _job_pool.lock();
			if (current_job_pool == nullptr)
			{
				return;
			}

#ifndef __USE_TYPE_CONTAINER__
			shared_ptr<json::value> target_data = make_shared<json::value>(json::value::parse(converter::to_string(_data)));
#ifdef _WIN32
			(*target_data)[L"header"][L"source_id"] = (*source_data)[L"header"][L"target_id"];
			(*target_data)[L"header"][L"source_sub_id"] = (*source_data)[L"header"][L"target_sub_id"];
			(*target_data)[L"header"][L"target_id"] = (*source_data)[L"header"][L"source_id"];
			(*target_data)[L"header"][L"target_sub_id"] = (*source_data)[L"header"][L"source_sub_id"];
			(*target_data)[L"data"][L"script_result"] = json::value::string(do_script(script));
#else
			(*target_data)["header"]["source_id"] = (*source_data)["header"]["target_id"];
			(*target_data)["header"]["source_sub_id"] = (*source_data)["header"]["target_sub_id"];
			(*target_data)["header"]["target_id"] = (*source_data)["header"]["source_id"];
			(*target_data)["header"]["target_sub_id"] = (*source_data)["header"]["source_sub_id"];
			(*target_data)["data"]["script_result"] = json::value::string(converter::to_string(do_script(script)));
#endif

			current_job_pool->push(make_shared<job>(_priority, converter::to_array(target_data->serialize())));
#else
			shared_ptr<value_container> target_data = source_data->copy(false);
			target_data->swap_header();
			target_data->add(make_shared<string_value>(L"script_result", do_script(script)));

			current_job_pool->push(make_shared<job>(_priority, target_data->serialize_array()));
#endif
			current_job_pool.reset();
		}
	}

	wstring job::do_script(const wstring& script)
	{
		try
		{
#ifdef __USE_CHAKRA_CORE__
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

			wstring resultW(resultWC);

			JsSetCurrentContext(JS_INVALID_REFERENCE);
			JsDisposeRuntime(runtime);

			return resultW;
#else
			Py_Initialize();

			PyRun_SimpleString(converter::to_string(script).c_str());

			Py_Finalize();

			return L"";
#endif
		}
		catch (...)
		{
			return L"";
		}
	}

	void job::load(void)
	{
		if (_temporary_stored_path.empty())
		{
			return;
		}

		_data = file::load(_temporary_stored_path);
		file::remove(_temporary_stored_path);
		_temporary_stored_path.clear();
	}
}
