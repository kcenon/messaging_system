#pragma once

#include "logging_level.h"

#include <mutex>
#include <memory>
#include <atomic>
#include <thread>
#include <vector>
#include <string>
#include <chrono>

namespace logging
{
	class util
	{
	private:
		util(void);

	public:
		~util(void);

	public:
		bool start(void);
		bool stop(void);

	public:
		void set_target_level(const logging_level& target_level);
		void set_write_console(const bool& write_console);

	public:
		std::chrono::time_point<std::chrono::steady_clock> chrono_start(void);
		void write(const logging_level& target_level, const std::wstring& log_data);
		void write(const logging_level& target_level, const std::wstring& log_data, const std::chrono::time_point<std::chrono::steady_clock>& time);

	private:
		void run(void);

	private:
		std::vector<std::pair<logging_level, std::pair<std::chrono::system_clock::time_point, std::wstring>>> _buffer;

	private:
		logging_level _target_level;

	private:
		std::atomic<bool> _has_buffer{ false };
		std::atomic<bool> _transfer_buffer{ false };
		std::atomic<bool> _thread_stop{ false };
		std::atomic<bool> _write_console{ true };
		std::atomic<bool> _write_file{ true };

	private:
		std::thread _thread;

#pragma region singleton
	public:
		static util& handle(void);

	private:
		static std::unique_ptr<util> _handle;
		static std::once_flag _once;
#pragma endregion
	};
}


