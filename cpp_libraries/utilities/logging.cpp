#include "logging.h"

#include "file_handler.h"
#include "datetime_handler.h"
#include "converting.h"

#include "fmt/chrono.h"
#include "fmt/format.h"
#include "fmt/xchar.h"

#include <io.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <filesystem>

#include <codecvt>

namespace logging
{
	using namespace converting;
	using namespace file_handler;
	using namespace datetime_handler;

	logger::logger(void) : _target_level(logging_level::information), _store_log_root_path(L""), _store_log_file_name(L""), _store_log_extention(L"")
		, _places_of_decimal(7)
	{
		_log_datas.insert({ logging_level::exception, bind(&logger::exception_log, this, placeholders::_1, placeholders::_2) });
		_log_datas.insert({ logging_level::error, bind(&logger::error_log, this, placeholders::_1, placeholders::_2) });
		_log_datas.insert({ logging_level::information, bind(&logger::information_log, this, placeholders::_1, placeholders::_2) });
		_log_datas.insert({ logging_level::sequence, bind(&logger::sequence_log, this, placeholders::_1, placeholders::_2) });
		_log_datas.insert({ logging_level::parameter, bind(&logger::parameter_log, this, placeholders::_1, placeholders::_2) });
		_log_datas.insert({ logging_level::packet, bind(&logger::packet_log, this, placeholders::_1, placeholders::_2) });
	}

	logger::~logger(void)
	{

	}

	bool logger::start(const wstring& store_log_file_name, const wstring& store_log_extention, const wstring& store_log_root_path, 
		const bool& append_date_on_file_name, const unsigned short& places_of_decimal)
	{
		stop();

		_store_log_file_name = store_log_file_name;
		_store_log_extention = store_log_extention;
		_store_log_root_path = store_log_root_path;
		_append_date_on_file_name.store(append_date_on_file_name);
		_places_of_decimal = places_of_decimal;

		_thread = thread(&logger::run, this);

		return true;
	}

	bool logger::stop(void)
	{
		_thread_stop.store(true);

		if (_thread.joinable())
		{
			_thread.join();
		}

		_thread_stop.store(false);

		return true;
	}

	void logger::set_target_level(const logging_level& target_level)
	{
		_target_level = target_level;
	}

	void logger::set_write_console(const bool& write_console)
	{
		_write_console.store(write_console);
	}

	void logger::set_limit_log_file_size(const size_t& limit_log_file_size)
	{
		_limit_log_file_size.store(limit_log_file_size);
	}

	chrono::time_point<chrono::high_resolution_clock> logger::chrono_start(void)
	{
		return chrono::high_resolution_clock::now();
	}

	void logger::write(const logging_level& target_level, const wstring& log_data, const optional<chrono::time_point<chrono::high_resolution_clock>>& time)
	{
		if (target_level > _target_level)
		{
			return;
		}

		scoped_lock<mutex> guard(_mutex);

		if (!time.has_value())
		{
			_buffer.push_back({ target_level , chrono::system_clock::now(), log_data });

			_condition.notify_one();

			return;
		}

		auto end = chrono::high_resolution_clock::now();

		chrono::duration<double, milli> diff = end - time.value();

		_buffer.push_back({ target_level , chrono::system_clock::now(), fmt::format(L"{} [{} ms]", log_data, diff.count()) });

		_condition.notify_one();
	}

	void logger::write(const logging_level& target_level, const vector<unsigned char>& log_data, const optional<chrono::time_point<chrono::high_resolution_clock>>& time)
	{
		if (target_level > _target_level)
		{
			return;
		}

		write(target_level, converter::to_wstring(log_data), time);
	}

	void logger::run(void)
	{
		vector<tuple<logging_level, chrono::system_clock::time_point, wstring>> buffers;

		if (_setmode(_fileno(stdout), _O_U8TEXT) == -1)
		{
			return;
		}

		set_log_flag(L"START");

		while (!_thread_stop.load() || !_buffer.empty())
		{
			unique_lock<mutex> unique(_mutex);
			_condition.wait(unique, [this] { return _thread_stop.load() || !_buffer.empty(); });

			buffers.swap(_buffer);
			unique.unlock();

			filesystem::path target_path;
			if (_append_date_on_file_name.load())
			{
				target_path = fmt::format(L"{}{}_{:%Y-%m-%d}.{}", _store_log_root_path,
					_store_log_file_name, fmt::localtime(chrono::system_clock::now()), _store_log_extention);
			}
			else
			{
				target_path = fmt::format(L"{}{}.{}", _store_log_root_path, _store_log_file_name, _store_log_extention);
			}

			if (!target_path.parent_path().empty())
			{
				filesystem::create_directories(target_path.parent_path());
			}

			if (_append_date_on_file_name.load())
			{
				backup_log(target_path.wstring(), fmt::format(L"{}{}_{:%Y-%m-%d}_backup.{}", _store_log_root_path,
					_store_log_file_name, fmt::localtime(chrono::system_clock::now()), _store_log_extention));
			}
			else
			{
				backup_log(target_path.wstring(), fmt::format(L"{}{}_backup.{}", _store_log_root_path,
					_store_log_file_name, _store_log_extention));
			}
			
			int file;
			errno_t err;
			if (_append_date_on_file_name.load())
			{
				err = _wsopen_s(&file, fmt::format(L"{}{}_{:%Y-%m-%d}.{}", _store_log_root_path, _store_log_file_name, fmt::localtime(chrono::system_clock::now()), _store_log_extention).c_str(),
					_O_WRONLY | _O_CREAT | _O_APPEND | _O_BINARY, _SH_DENYWR, _S_IWRITE);
			}
			else
			{
				err = _wsopen_s(&file, fmt::format(L"{}{}.{}", _store_log_root_path, _store_log_file_name, _store_log_extention).c_str(),
					_O_WRONLY | _O_CREAT | _O_APPEND | _O_BINARY, _SH_DENYWR, _S_IWRITE);
			}

			if (err != 0)
			{
				return;
			}

			if (_setmode(file, _O_U8TEXT) == -1)
			{
				return;
			}

			for (auto& buffer : buffers)
			{
				auto iterator = _log_datas.find(get<0>(buffer));
				if (iterator == _log_datas.end())
				{
					continue;
				}

				store_log(file, iterator->second(get<1>(buffer), get<2>(buffer)));
			}

			_commit(file);
			_close(file);

			buffers.clear();
		}

		set_log_flag(L"END");
	}

	void logger::set_log_flag(const wstring& flag)
	{
		int file;

		errno_t err;
		if (_append_date_on_file_name.load())
		{
			err = _wsopen_s(&file, fmt::format(L"{}{}_{:%Y-%m-%d}.{}", _store_log_root_path, _store_log_file_name, fmt::localtime(chrono::system_clock::now()), _store_log_extention).c_str(),
				_O_WRONLY | _O_CREAT | _O_APPEND | _O_BINARY, _SH_DENYWR, _S_IWRITE);
		}
		else
		{
			err = _wsopen_s(&file, fmt::format(L"{}{}.{}", _store_log_root_path, _store_log_file_name, _store_log_extention).c_str(),
				_O_WRONLY | _O_CREAT | _O_APPEND | _O_BINARY, _SH_DENYWR, _S_IWRITE);
		}

		if (err != 0)
		{
			return;
		}

		if (_setmode(file, _O_U8TEXT) == -1)
		{
			return;
		}

		chrono::system_clock::time_point current = chrono::system_clock::now();
		auto time_string = datetime::time(current, true, _places_of_decimal);
		if (_write_date.load())
		{
			store_log(file, fmt::format(L"[{:%Y-%m-%d} {}][{}]\n", fmt::localtime(current), time_string, flag));
		}
		else
		{
			store_log(file, fmt::format(L"[{}][{}]\n", time_string, flag));
		}

		_commit(file);
		_close(file);
	}

	void logger::backup_log(const wstring& target_path, const wstring& backup_path)
	{
		if (!filesystem::exists(target_path))
		{
			return;
		}

		if (filesystem::file_size(target_path) < _limit_log_file_size.load())
		{
			return;
		}

		file::append(backup_path, file::load(target_path));
	}

	void logger::store_log(int& file_handle, const wstring& log)
	{
		if (log.empty())
		{
			return;
		}

		if (_write_console.load())
		{
			wcout << log;
		}

		_write(file_handle, log.data(), (unsigned int)(log.size() * sizeof(wchar_t)));
	}

	wstring logger::exception_log(const chrono::system_clock::time_point& time, const wstring& data)
	{
		auto time_string = datetime::time(time, true, _places_of_decimal);
		if (_write_date.load())
		{
			return fmt::format(L"[{:%Y-%m-%d} {}][EXCEPTION]: {}\n", fmt::localtime(time), time_string, data);
		}
		
		return fmt::format(L"[{}][EXCEPTION]: {}\n", time_string, data);
	}

	wstring logger::error_log(const chrono::system_clock::time_point& time, const wstring& data)
	{
		auto time_string = datetime::time(time, true, _places_of_decimal);
		if (_write_date.load())
		{
			return fmt::format(L"[{:%Y-%m-%d} {}][ERROR]: {}\n", fmt::localtime(time), time_string, data);
		}

		return fmt::format(L"[{}][ERROR]: {}\n", time_string, data);
	}

	wstring logger::information_log(const chrono::system_clock::time_point& time, const wstring& data)
	{
		auto time_string = datetime::time(time, true, _places_of_decimal);
		if (_write_date.load())
		{
			return fmt::format(L"[{:%Y-%m-%d} {}][INFORMATION]: {}\n", fmt::localtime(time), time_string, data);
		}

		return fmt::format(L"[{}][INFORMATION]: {}\n", time_string, data);
	}

	wstring logger::sequence_log(const chrono::system_clock::time_point& time, const wstring& data)
	{
		auto time_string = datetime::time(time, true, _places_of_decimal);
		if (_write_date.load())
		{
			return fmt::format(L"[{:%Y-%m-%d} {}][SEQUENCE]: {}\n", fmt::localtime(time), time_string, data);
		}

		return fmt::format(L"[{}][SEQUENCE]: {}\n", time_string, data);
	}

	wstring logger::parameter_log(const chrono::system_clock::time_point& time, const wstring& data)
	{
		auto time_string = datetime::time(time, true, _places_of_decimal);
		if (_write_date.load())
		{
			return fmt::format(L"[{:%Y-%m-%d} {}][PARAMETER]: {}\n", fmt::localtime(time), time_string, data);
		}

		return fmt::format(L"[{}][PARAMETER]: {}\n", time_string, data);
	}

	wstring logger::packet_log(const chrono::system_clock::time_point& time, const wstring& data)
	{
		auto time_string = datetime::time(time, true, _places_of_decimal);
		if (_write_date.load())
		{
			return fmt::format(L"[{:%Y-%m-%d} {}][PACKET]: {}\n", fmt::localtime(time), time_string, data);
		}

		return fmt::format(L"[{}][PACKET]: {}\n", time_string, data);
	}

#pragma region singleton
	unique_ptr<logger> logger::_handle;
	once_flag logger::_once;

	logger& logger::handle(void)
	{
		call_once(_once, []()
			{
				_handle.reset(new logger);
			});

		return *_handle.get();
	}
#pragma endregion
}
