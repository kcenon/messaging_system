#pragma once

#include "logging_level.h"

#include <queue>
#include <mutex>
#include <memory>
#include <atomic>
#include <thread>
#include <vector>
#include <string>
#include <chrono>
#include <iostream>
#include <condition_variable>

namespace logging
{
	class util
	{
	private:
		util(void);

	public:
		~util(void);

	public:
		bool start(const std::wstring& store_log_file_name = L"log", const std::wstring& store_log_extention = L"log", const std::wstring& store_log_root_path = L"");
		bool stop(void);

	public:
		void set_target_level(const logging_level& target_level);
		void set_write_console(const bool& write_console);
		void set_store_latest_log_count(const size_t& store_latest_log_count);
		void set_limit_log_file_size(const size_t& limit_log_file_size);

	public:
		const std::queue<std::wstring> get_latest_logs(void);

	public:
		std::chrono::time_point<std::chrono::steady_clock> chrono_start(void);
		void write(const logging_level& target_level, const std::wstring& log_data);
		void write(const logging_level& target_level, const std::wstring& log_data, const std::chrono::time_point<std::chrono::steady_clock>& time);

	protected:
		void run(void);

	private:
		void start_log(void);
		void backup_log(const std::wstring& target_path, const std::wstring& backup_path);
		void store_log(int& file_handle, const std::wstring& log);
		void end_log(void);

	private:
		std::vector<unsigned char> load(const std::wstring& path);
		void append(const std::wstring& source, const std::wstring& target);

	private:
		std::vector<std::pair<logging_level, std::pair<std::chrono::system_clock::time_point, std::wstring>>> _buffer;

	private:
		logging_level _target_level;
		std::wstring _store_log_root_path;
		std::wstring _store_log_file_name;
		std::wstring _store_log_extention;

	private:
		std::atomic<bool> _thread_stop{ false };
		std::atomic<bool> _write_date{ false };
		std::atomic<bool> _write_console{ true };
		std::atomic<bool> _write_file{ true };
		std::atomic<size_t> _store_latest_log_count{ 1000 };
		std::atomic<size_t> _limit_log_file_size{ 2097152 };

	private:
		std::mutex _mutex;
		std::thread _thread;
		std::condition_variable _condition;
		std::queue<std::wstring> _latest_logs;

#pragma region singleton
	public:
		static util& handle(void);

	private:
		static std::unique_ptr<util> _handle;
		static std::once_flag _once;
#pragma endregion
	};
}


