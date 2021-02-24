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

	std::chrono::time_point<std::chrono::steady_clock> util::chrono_start(void)
	{
		return std::chrono::steady_clock::now();
	}

	void util::write(const logging_level& target_level, const std::wstring& log_data)
	{
		if (target_level > _target_level)
		{
			return;
		}
		
		std::lock_guard<std::mutex> guard(_mutex);

		_buffer.push_back({ target_level , { std::chrono::system_clock::now(), log_data } });

		_condition.notify_one();
	}

	void util::write(const logging_level& target_level, const std::wstring& log_data, const std::chrono::time_point<std::chrono::steady_clock>& time)
	{
		auto end = std::chrono::steady_clock::now();

		std::chrono::duration<double> diff = end - time;

		write(target_level, fmt::format(L"{} [{:0>3} ms]", log_data, (diff.count() * 1000)));
	}

	void util::run(void)
	{
		std::wstring result;
		std::vector<std::pair<logging_level, std::pair<std::chrono::system_clock::time_point, std::wstring>>> buffers;

		std::wclog.imbue(std::locale("C.UTF-8"));

		start_log();

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

			for (auto& buffer : buffers)
			{
				auto milli_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(buffer.second.first.time_since_epoch()).count() % 1000;
				auto micro_seconds = std::chrono::duration_cast<std::chrono::microseconds>(buffer.second.first.time_since_epoch()).count() % 1000;
				if (_write_date.load())
				{
					fmt::format_to(std::back_inserter(result), L"[{:%Y-%m-%d %H:%M:%S}.{:0>3}{:0>3}]", fmt::localtime(buffer.second.first), milli_seconds, micro_seconds);
				}
				else
				{
					fmt::format_to(std::back_inserter(result), L"[{:%H:%M:%S}.{:0>3}{:0>3}]", fmt::localtime(buffer.second.first), milli_seconds, micro_seconds);
				}

				switch (buffer.first)
				{
				case logging_level::exception: fmt::format_to(std::back_inserter(result), L"{}", L"[EXCEPTION]"); break;
				case logging_level::error: fmt::format_to(std::back_inserter(result), L"{}", L"[ERROR]"); break;
				case logging_level::information: fmt::format_to(std::back_inserter(result), L"{}", L"[INFORMATION]"); break;
				case logging_level::sequence: fmt::format_to(std::back_inserter(result), L"{}", L"[SEQUENCE]"); break;
				case logging_level::paramete: fmt::format_to(std::back_inserter(result), L"{}", L"[PARAMETER]"); break;
				}
				fmt::format_to(std::back_inserter(result), L": {}\r\n", buffer.second.second);

				store_log(file, std::move(result));
			}

			_close(file);

			buffers.clear();
		}

		end_log();
	}

	std::ostream& operator<<(std::ostream& os, logging_level level)
	{
		switch (level)
		{
		case logging_level::exception: os << L"EXCEPTION"; break;
		case logging_level::error: os << L"ERROR"; break;
		case logging_level::information: os << L"INFORMATION"; break;
		case logging_level::sequence: os << L"SEQUENCE"; break;
		case logging_level::paramete: os << L"PARAMETER"; break;
		}

		return os;
	}

	void util::start_log(void)
	{
		std::wstring result;

		std::chrono::system_clock::time_point current = std::chrono::system_clock::now();

		auto milli_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(current.time_since_epoch()).count() % 1000;
		auto micro_seconds = std::chrono::duration_cast<std::chrono::microseconds>(current.time_since_epoch()).count() % 1000;
		if (_write_date.load())
		{
			fmt::format_to(std::back_inserter(result), L"[{:%Y-%m-%d %H:%M:%S}.{:0>3}{:0>3}][START]\r\n", fmt::localtime(current), milli_seconds, micro_seconds);
		}
		else
		{
			fmt::format_to(std::back_inserter(result), L"[{:%H:%M:%S}.{:0>3}{:0>3}][START]\r\n", fmt::localtime(current), milli_seconds, micro_seconds);
		}

		int file;
		errno_t err = _wsopen_s(&file, fmt::format(L"{}{}_{:%Y-%m-%d}.{}", _store_log_root_path, _store_log_file_name, fmt::localtime(std::chrono::system_clock::now()), _store_log_extention).c_str(),
			_O_WRONLY | _O_CREAT | _O_APPEND | _O_BINARY, _SH_DENYWR, _S_IWRITE);
		if (err != 0)
		{
			return;
		}

		store_log(file, std::move(result));
		
		_close(file);
	}

	void util::backup_log(const std::wstring& target_path, const std::wstring& backup_path)
	{
		if (!std::filesystem::exists(target_path))
		{
			return;
		}

		size_t size1 = std::filesystem::file_size(target_path);
		size_t size2 = _limit_log_file_size.load();
		if (size1 < size2)
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
			std::wclog << log;
		}

		if (!_write_file.load())
		{
			return;
		}

		_write(file_handle, log.data(), (unsigned int)(log.size() * sizeof(wchar_t)));
		_commit(file_handle);
	}

	void util::end_log(void)
	{
		std::wstring result;

		std::chrono::system_clock::time_point current = std::chrono::system_clock::now();

		auto milli_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(current.time_since_epoch()).count() % 1000;
		auto micro_seconds = std::chrono::duration_cast<std::chrono::microseconds>(current.time_since_epoch()).count() % 1000;
		if (_write_date.load())
		{
			fmt::format_to(std::back_inserter(result), L"[{:%Y-%m-%d %H:%M:%S}.{:0>3}{:0>3}][END]\r\n", fmt::localtime(current), milli_seconds, micro_seconds);
		}
		else
		{
			fmt::format_to(std::back_inserter(result), L"[{:%H:%M:%S}.{:0>3}{:0>3}][END]\r\n", fmt::localtime(current), milli_seconds, micro_seconds);
		}

		int file;
		errno_t err = _wsopen_s(&file, fmt::format(L"{}{}_{:%Y-%m-%d}.{}", _store_log_root_path, _store_log_file_name, fmt::localtime(std::chrono::system_clock::now()), _store_log_extention).c_str(),
			_O_WRONLY | _O_CREAT | _O_APPEND | _O_BINARY, _SH_DENYWR, _S_IWRITE);
		if (err != 0)
		{
			result.clear();
			return;
		}

		store_log(file, std::move(result));

		_close(file);

		result.clear();
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
