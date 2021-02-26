#include "logging.h"

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
	util::util(void) : _target_level(logging_level::information), _store_log_root_path(L""), _store_log_file_name(L""), _store_log_extention(L"")
	{
		_log_datas.insert({ logging_level::exception, std::bind(&util::exception_log, this, std::placeholders::_1, std::placeholders::_2) });
		_log_datas.insert({ logging_level::error, std::bind(&util::error_log, this, std::placeholders::_1, std::placeholders::_2) });
		_log_datas.insert({ logging_level::information, std::bind(&util::information_log, this, std::placeholders::_1, std::placeholders::_2) });
		_log_datas.insert({ logging_level::sequence, std::bind(&util::sequence_log, this, std::placeholders::_1, std::placeholders::_2) });
		_log_datas.insert({ logging_level::parameter, std::bind(&util::parameter_log, this, std::placeholders::_1, std::placeholders::_2) });
	}

	util::~util(void)
	{

	}

	bool util::start(const std::wstring& store_log_file_name, const std::wstring& store_log_extention, const std::wstring& store_log_root_path)
	{
		stop();

		_store_log_file_name = store_log_file_name;
		_store_log_extention = store_log_extention;
		_store_log_root_path = store_log_root_path;

		_thread = std::thread(&util::run, this);

		return true;
	}

	bool util::stop(void)
	{
		_thread_stop.store(true);

		if (_thread.joinable())
		{
			_thread.join();
		}

		_thread_stop.store(false);

		return true;
	}

	void util::set_target_level(const logging_level& target_level)
	{
		_target_level = target_level;
	}

	void util::set_write_console(const bool& write_console)
	{
		_write_console.store(write_console);
	}

	void util::set_store_latest_log_count(const size_t& store_latest_log_count)
	{
		_store_latest_log_count.store(store_latest_log_count);
	}

	void util::set_limit_log_file_size(const size_t& limit_log_file_size)
	{
		_limit_log_file_size.store(limit_log_file_size);
	}

	const std::queue<std::wstring> util::get_latest_logs(void)
	{
		return _latest_logs;
	}

	std::chrono::time_point<std::chrono::high_resolution_clock> util::chrono_start(void)
	{
		return std::chrono::high_resolution_clock::now();
	}

	void util::write(const logging_level& target_level, const std::wstring& log_data, const std::optional<std::chrono::time_point<std::chrono::high_resolution_clock>>& time)
	{
		if (target_level > _target_level)
		{
			return;
		}

		std::lock_guard<std::mutex> guard(_mutex);

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

	void util::run(void)
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
			_condition.wait(unique, [this] { return !_thread_stop.load() || !_buffer.empty(); });

			buffers.swap(_buffer);
			unique.unlock();

			std::filesystem::path target_path(fmt::format(L"{}{}_{:%Y-%m-%d}.{}", _store_log_root_path,
				_store_log_file_name, fmt::localtime(std::chrono::system_clock::now()), _store_log_extention));
			if (!target_path.parent_path().empty())
			{
				std::filesystem::create_directories(target_path.parent_path());
			}

			backup_log(target_path.wstring(), fmt::format(L"{}{}_{:%Y-%m-%d}_backup.{}", _store_log_root_path,
				_store_log_file_name, fmt::localtime(std::chrono::system_clock::now()), _store_log_extention));
			
			int file;
			errno_t err = _wsopen_s(&file, fmt::format(L"{}{}_{:%Y-%m-%d}.{}", _store_log_root_path, _store_log_file_name, fmt::localtime(std::chrono::system_clock::now()), _store_log_extention).c_str(),
				_O_WRONLY | _O_CREAT | _O_APPEND | _O_BINARY, _SH_DENYWR, _S_IWRITE);
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

			_close(file);

			buffers.clear();
		}

		set_log_flag(L"END");
	}

	void util::set_log_flag(const std::wstring& flag)
	{
		int file;
		errno_t err = _wsopen_s(&file, fmt::format(L"{}{}_{:%Y-%m-%d}.{}", _store_log_root_path, _store_log_file_name, fmt::localtime(std::chrono::system_clock::now()), _store_log_extention).c_str(),
			_O_WRONLY | _O_CREAT | _O_APPEND | _O_BINARY, _SH_DENYWR, _S_IWRITE);
		if (err != 0)
		{
			return;
		}

		if (_setmode(file, _O_U8TEXT) == -1)
		{
			return;
		}

		std::chrono::system_clock::time_point current = std::chrono::system_clock::now();
		auto seconds = get_milli_micro_seconds(current.time_since_epoch());
		if (_write_date.load())
		{
			store_log(file, fmt::format(L"[{:%Y-%m-%d %H:%M:%S}.{:0>3}{:0>3}][{}]\n", fmt::localtime(current), std::get<0>(seconds), std::get<1>(seconds), flag));
		}
		else
		{
			store_log(file, fmt::format(L"[{:%H:%M:%S}.{:0>3}{:0>3}][{}]\n", fmt::localtime(current), std::get<0>(seconds), std::get<1>(seconds), flag));
		}
		
		_close(file);
	}

	void util::backup_log(const std::wstring& target_path, const std::wstring& backup_path)
	{
		if (!std::filesystem::exists(target_path))
		{
			return;
		}

		if (std::filesystem::file_size(target_path) < _limit_log_file_size.load())
		{
			return;
		}

		append(target_path, backup_path);
	}

	void util::store_log(int& file_handle, const std::wstring& log)
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

		if (!_write_file.load())
		{
			return;
		}

		_write(file_handle, log.data(), (unsigned int)(log.size() * sizeof(wchar_t)));
		_commit(file_handle);
	}

	std::vector<unsigned char> util::load(const std::wstring& path)
	{
		if (!std::filesystem::exists(path))
		{
			return std::vector<unsigned char>();
		}

		int file;
		errno_t err = _wsopen_s(&file, path.c_str(), _O_RDONLY | _O_BINARY, _SH_DENYRD, _S_IREAD);
		if (err != 0)
		{
			return std::vector<unsigned char>();
		}

		size_t file_size = _lseek(file, 0, SEEK_END);
		_lseek(file, 0, SEEK_SET);

		char* temp = new char[file_size];
		memset(temp, 0, file_size);

		file_size = _read(file, temp, (unsigned int)file_size);

		std::vector<unsigned char> target;
		target.reserve(file_size);
		target.insert(target.begin(), temp, temp + file_size);

		_close(file);

		delete[] temp;
		temp = nullptr;

		return target;
	}

	void util::append(const std::wstring& source, const std::wstring& target)
	{
		std::vector<unsigned char> data = load(source);

		int file;
		errno_t err = _wsopen_s(&file, target.c_str(), _O_WRONLY | _O_CREAT | _O_APPEND | _O_BINARY, _SH_DENYWR, _S_IWRITE);
		if (err != 0)
		{
			return;
		}

		_write(file, data.data(), (unsigned int)data.size());
		_close(file);

		std::filesystem::remove(source);
	}

	std::wstring util::exception_log(const std::chrono::system_clock::time_point& time, const std::wstring& data)
	{
		auto seconds = get_milli_micro_seconds(time.time_since_epoch());
		if (_write_date.load())
		{
			return fmt::format(L"[{:%Y-%m-%d %H:%M:%S}.{:0>3}{:0>3}][EXCEPTION]: {}\n", fmt::localtime(time), std::get<0>(seconds), std::get<1>(seconds), data);
		}
		
		return fmt::format(L"[{:%H:%M:%S}.{:0>3}{:0>3}][EXCEPTION]: {}\n", fmt::localtime(time), std::get<0>(seconds), std::get<1>(seconds), data);
	}

	std::wstring util::error_log(const std::chrono::system_clock::time_point& time, const std::wstring& data)
	{
		auto seconds = get_milli_micro_seconds(time.time_since_epoch());
		if (_write_date.load())
		{
			return fmt::format(L"[{:%Y-%m-%d %H:%M:%S}.{:0>3}{:0>3}][ERROR]: {}\n", fmt::localtime(time), std::get<0>(seconds), std::get<1>(seconds), data);
		}

		return fmt::format(L"[{:%H:%M:%S}.{:0>3}{:0>3}][ERROR]: {}\n", fmt::localtime(time), std::get<0>(seconds), std::get<1>(seconds), data);
	}

	std::wstring util::information_log(const std::chrono::system_clock::time_point& time, const std::wstring& data)
	{
		auto seconds = get_milli_micro_seconds(time.time_since_epoch());
		if (_write_date.load())
		{
			return fmt::format(L"[{:%Y-%m-%d %H:%M:%S}.{:0>3}{:0>3}][INFORMATION]: {}\n", fmt::localtime(time), std::get<0>(seconds), std::get<1>(seconds), data);
		}

		return fmt::format(L"[{:%H:%M:%S}.{:0>3}{:0>3}][INFORMATION]: {}\n", fmt::localtime(time), std::get<0>(seconds), std::get<1>(seconds), data);
	}

	std::wstring util::sequence_log(const std::chrono::system_clock::time_point& time, const std::wstring& data)
	{
		auto seconds = get_milli_micro_seconds(time.time_since_epoch());
		if (_write_date.load())
		{
			return fmt::format(L"[{:%Y-%m-%d %H:%M:%S}.{:0>3}{:0>3}][SEQUENCE]: {}\n", fmt::localtime(time), std::get<0>(seconds), std::get<1>(seconds), data);
		}

		return fmt::format(L"[{:%H:%M:%S}.{:0>3}{:0>3}][SEQUENCE]: {}\n", fmt::localtime(time), std::get<0>(seconds), std::get<1>(seconds), data);
	}

	std::wstring util::parameter_log(const std::chrono::system_clock::time_point& time, const std::wstring& data)
	{
		auto seconds = get_milli_micro_seconds(time.time_since_epoch());
		if (_write_date.load())
		{
			return fmt::format(L"[{:%Y-%m-%d %H:%M:%S}.{:0>3}{:0>3}][PARAMETER]: {}\n", fmt::localtime(time), std::get<0>(seconds), std::get<1>(seconds), data);
		}

		return fmt::format(L"[{:%H:%M:%S}.{:0>3}{:0>3}][PARAMETER]: {}\n", fmt::localtime(time), std::get<0>(seconds), std::get<1>(seconds), data);
	}

	std::tuple<long long, long long> util::get_milli_micro_seconds(const std::chrono::system_clock::duration& duration)
	{
		return { 
			std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000 ,
			std::chrono::duration_cast<std::chrono::microseconds>(duration).count() % 1000 
		};
	}

#pragma region singleton
	std::unique_ptr<util> util::_handle;
	std::once_flag util::_once;

	util& util::handle(void)
	{
		std::call_once(_once, []()
			{
				_handle.reset(new util);
			});

		return *_handle.get();
	}
#pragma endregion
}
