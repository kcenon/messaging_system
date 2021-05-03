#include "logging.h"

#include "file_handling.h"

#include "fmt/chrono.h"
#include "fmt/format.h"

#include <io.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <filesystem>

#include <codecvt>

namespace logging
{
	using namespace file_handling;

	logger::logger(void) : _target_level(logging_level::information), _store_log_root_path(L""), _store_log_file_name(L""), _store_log_extention(L"")
	{
		_log_datas.insert({ logging_level::exception, std::bind(&logger::exception_log, this, std::placeholders::_1, std::placeholders::_2) });
		_log_datas.insert({ logging_level::error, std::bind(&logger::error_log, this, std::placeholders::_1, std::placeholders::_2) });
		_log_datas.insert({ logging_level::information, std::bind(&logger::information_log, this, std::placeholders::_1, std::placeholders::_2) });
		_log_datas.insert({ logging_level::sequence, std::bind(&logger::sequence_log, this, std::placeholders::_1, std::placeholders::_2) });
		_log_datas.insert({ logging_level::parameter, std::bind(&logger::parameter_log, this, std::placeholders::_1, std::placeholders::_2) });
	}

	logger::~logger(void)
	{

	}

	bool logger::start(const std::wstring& store_log_file_name, const std::wstring& store_log_extention, const std::wstring& store_log_root_path, const bool& append_date_on_file_name)
	{
		stop();

		_store_log_file_name = store_log_file_name;
		_store_log_extention = store_log_extention;
		_store_log_root_path = store_log_root_path;
		_append_date_on_file_name.store(append_date_on_file_name);

		_thread = std::thread(&logger::run, this);

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

	void logger::set_store_latest_log_count(const size_t& store_latest_log_count)
	{
		_store_latest_log_count.store(store_latest_log_count);
	}

	void logger::set_limit_log_file_size(const size_t& limit_log_file_size)
	{
		_limit_log_file_size.store(limit_log_file_size);
	}

	const std::queue<std::wstring> logger::get_latest_logs(void)
	{
		return _latest_logs;
	}

	std::chrono::time_point<std::chrono::high_resolution_clock> logger::chrono_start(void)
	{
		return std::chrono::high_resolution_clock::now();
	}

	void logger::write(const logging_level& target_level, const std::wstring& log_data, const std::optional<std::chrono::time_point<std::chrono::high_resolution_clock>>& time)
	{
		if (target_level > _target_level)
		{
			return;
		}

		std::scoped_lock<std::mutex> guard(_mutex);

		if (!time.has_value())
		{
			_buffer.push_back({ target_level , std::chrono::system_clock::now(), log_data });

			_condition.notify_one();

			return;
		}

		auto end = std::chrono::high_resolution_clock::now();

		std::chrono::duration<double, std::milli> diff = end - time.value();

		_buffer.push_back({ target_level , std::chrono::system_clock::now(), fmt::format(L"{} [{} ms]", log_data, diff.count()) });

		_condition.notify_one();
	}

	void logger::run(void)
	{
		std::vector<std::tuple<logging_level, std::chrono::system_clock::time_point, std::wstring>> buffers;

		if (_setmode(_fileno(stdout), _O_U8TEXT) == -1)
		{
			return;
		}

		set_log_flag(L"START");

		while (!_thread_stop.load() || !_buffer.empty())
		{
			std::unique_lock<std::mutex> unique(_mutex);
			_condition.wait(unique, [this] { return _thread_stop.load() || !_buffer.empty(); });

			buffers.swap(_buffer);
			unique.unlock();

			std::filesystem::path target_path;
			if (_append_date_on_file_name.load())
			{
				target_path = fmt::format(L"{}{}_{:%Y-%m-%d}.{}", _store_log_root_path,
					_store_log_file_name, fmt::localtime(std::chrono::system_clock::now()), _store_log_extention);
			}
			else
			{
				target_path = fmt::format(L"{}{}.{}", _store_log_root_path, _store_log_file_name, _store_log_extention);
			}

			if (!target_path.parent_path().empty())
			{
				std::filesystem::create_directories(target_path.parent_path());
			}

			if (_append_date_on_file_name.load())
			{
				backup_log(target_path.wstring(), fmt::format(L"{}{}_{:%Y-%m-%d}_backup.{}", _store_log_root_path,
					_store_log_file_name, fmt::localtime(std::chrono::system_clock::now()), _store_log_extention));
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
				err = _wsopen_s(&file, fmt::format(L"{}{}_{:%Y-%m-%d}.{}", _store_log_root_path, _store_log_file_name, fmt::localtime(std::chrono::system_clock::now()), _store_log_extention).c_str(),
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
				auto iterator = _log_datas.find(std::get<0>(buffer));
				if (iterator == _log_datas.end())
				{
					continue;
				}

				store_log(file, iterator->second(std::get<1>(buffer), std::get<2>(buffer)));
			}

			_commit(file);
			_close(file);

			buffers.clear();
		}

		set_log_flag(L"END");
	}

	void logger::set_log_flag(const std::wstring& flag)
	{
		int file;

		errno_t err;
		if (_append_date_on_file_name.load())
		{
			err = _wsopen_s(&file, fmt::format(L"{}{}_{:%Y-%m-%d}.{}", _store_log_root_path, _store_log_file_name, fmt::localtime(std::chrono::system_clock::now()), _store_log_extention).c_str(),
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

		std::chrono::system_clock::time_point current = std::chrono::system_clock::now();
		auto seconds = get_under_seconds(current.time_since_epoch());
		if (_write_date.load())
		{
			store_log(file, fmt::format(L"[{:%Y-%m-%d %H:%M:%S}.{:0>3}{:0>3}][{}]\n", fmt::localtime(current), std::get<0>(seconds), std::get<1>(seconds), flag));
		}
		else
		{
			store_log(file, fmt::format(L"[{:%H:%M:%S}.{:0>3}{:0>3}][{}]\n", fmt::localtime(current), std::get<0>(seconds), std::get<1>(seconds), flag));
		}

		_commit(file);
		_close(file);
	}

	void logger::backup_log(const std::wstring& target_path, const std::wstring& backup_path)
	{
		if (!std::filesystem::exists(target_path))
		{
			return;
		}

		if (std::filesystem::file_size(target_path) < _limit_log_file_size.load())
		{
			return;
		}

		file_handler::append(backup_path, file_handler::load(target_path));
	}

	void logger::store_log(int& file_handle, const std::wstring& log)
	{
		if (log.empty())
		{
			return;
		}

		_latest_logs.push(log);
		while (_latest_logs.size() > _store_latest_log_count.load())
		{
			_latest_logs.pop();
		}

		if (_write_console.load())
		{
			std::wcout << log;
		}

		_write(file_handle, log.data(), (unsigned int)(log.size() * sizeof(wchar_t)));
	}

	std::wstring logger::exception_log(const std::chrono::system_clock::time_point& time, const std::wstring& data)
	{
		auto seconds = get_under_seconds(time.time_since_epoch());
		if (_write_date.load())
		{
			return fmt::format(L"[{:%Y-%m-%d %H:%M:%S}.{:0>3}{:0>3}][EXCEPTION]: {}\n", fmt::localtime(time), std::get<0>(seconds), std::get<1>(seconds), data);
		}
		
		return fmt::format(L"[{:%H:%M:%S}.{:0>3}{:0>3}][EXCEPTION]: {}\n", fmt::localtime(time), std::get<0>(seconds), std::get<1>(seconds), data);
	}

	std::wstring logger::error_log(const std::chrono::system_clock::time_point& time, const std::wstring& data)
	{
		auto seconds = get_under_seconds(time.time_since_epoch());
		if (_write_date.load())
		{
			return fmt::format(L"[{:%Y-%m-%d %H:%M:%S}.{:0>3}{:0>3}][ERROR]: {}\n", fmt::localtime(time), std::get<0>(seconds), std::get<1>(seconds), data);
		}

		return fmt::format(L"[{:%H:%M:%S}.{:0>3}{:0>3}][ERROR]: {}\n", fmt::localtime(time), std::get<0>(seconds), std::get<1>(seconds), data);
	}

	std::wstring logger::information_log(const std::chrono::system_clock::time_point& time, const std::wstring& data)
	{
		auto seconds = get_under_seconds(time.time_since_epoch());
		if (_write_date.load())
		{
			return fmt::format(L"[{:%Y-%m-%d %H:%M:%S}.{:0>3}{:0>3}][INFORMATION]: {}\n", fmt::localtime(time), std::get<0>(seconds), std::get<1>(seconds), data);
		}

		return fmt::format(L"[{:%H:%M:%S}.{:0>3}{:0>3}][INFORMATION]: {}\n", fmt::localtime(time), std::get<0>(seconds), std::get<1>(seconds), data);
	}

	std::wstring logger::sequence_log(const std::chrono::system_clock::time_point& time, const std::wstring& data)
	{
		auto seconds = get_under_seconds(time.time_since_epoch());
		if (_write_date.load())
		{
			return fmt::format(L"[{:%Y-%m-%d %H:%M:%S}.{:0>3}{:0>3}][SEQUENCE]: {}\n", fmt::localtime(time), std::get<0>(seconds), std::get<1>(seconds), data);
		}

		return fmt::format(L"[{:%H:%M:%S}.{:0>3}{:0>3}][SEQUENCE]: {}\n", fmt::localtime(time), std::get<0>(seconds), std::get<1>(seconds), data);
	}

	std::wstring logger::parameter_log(const std::chrono::system_clock::time_point& time, const std::wstring& data)
	{
		auto seconds = get_under_seconds(time.time_since_epoch());
		if (_write_date.load())
		{
			return fmt::format(L"[{:%Y-%m-%d %H:%M:%S}.{:0>3}{:0>3}][PARAMETER]: {}\n", fmt::localtime(time), std::get<0>(seconds), std::get<1>(seconds), data);
		}

		return fmt::format(L"[{:%H:%M:%S}.{:0>3}{:0>3}][PARAMETER]: {}\n", fmt::localtime(time), std::get<0>(seconds), std::get<1>(seconds), data);
	}

	std::tuple<long long, long long> logger::get_under_seconds(const std::chrono::system_clock::duration& duration)
	{
		return { 
			std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000 ,
			std::chrono::duration_cast<std::chrono::microseconds>(duration).count() % 1000 
		};
	}

#pragma region singleton
	std::unique_ptr<logger> logger::_handle;
	std::once_flag logger::_once;

	logger& logger::handle(void)
	{
		std::call_once(_once, []()
			{
				_handle.reset(new logger);
			});

		return *_handle.get();
	}
#pragma endregion
}
