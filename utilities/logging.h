/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2021, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#pragma once

#include "logging_level.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

namespace logging
{
	class logger
	{
	private:
		logger(void);

	public:
		~logger(void);

	public:
		bool start(const std::wstring& store_log_file_name = L"log",
				   std::locale target_locale = std::locale(""),
				   const std::wstring& store_log_extention = L"log",
				   const std::wstring& store_log_root_path = L"",
				   const bool& append_date_on_file_name = true);
		bool stop(void);

	public:
		void set_target_level(const logging_level& target_level);
		void set_target_level(
			const std::vector<logging_level>& write_console_levels);
		void set_write_console(const logging_styles& logging_style
							   = logging_styles::file_only);
		void set_write_date(const bool& write_date);
		void set_limit_log_file_size(const size_t& limit_log_file_size);
		void set_backup_notification(
			const std::function<void(const std::wstring&)>& notification);

	public:
		std::chrono::time_point<std::chrono::high_resolution_clock>
		chrono_start(void);
		void write(const logging_level& target_level,
				   const std::wstring& log_data,
				   const std::optional<std::chrono::time_point<
					   std::chrono::high_resolution_clock>>& time
				   = std::nullopt);
		void write(const logging_level& target_level,
				   const std::vector<uint8_t>& log_data,
				   const std::optional<std::chrono::time_point<
					   std::chrono::high_resolution_clock>>& time
				   = std::nullopt);

	protected:
		void run(void);

	private:
		void set_log_flag(const std::wstring& flag);
		void backup_log(const std::wstring& target_path,
						const std::wstring& backup_path);

	private:
		std::wstring exception_log(
			const std::chrono::system_clock::time_point& time,
			const std::wstring& data);
		std::wstring error_log(
			const std::chrono::system_clock::time_point& time,
			const std::wstring& data);
		std::wstring information_log(
			const std::chrono::system_clock::time_point& time,
			const std::wstring& data);
		std::wstring sequence_log(
			const std::chrono::system_clock::time_point& time,
			const std::wstring& data);
		std::wstring parameter_log(
			const std::chrono::system_clock::time_point& time,
			const std::wstring& data);
		std::wstring packet_log(
			const std::chrono::system_clock::time_point& time,
			const std::wstring& data);
		std::wstring make_log_string(
			const logging_level& target_level,
			const std::chrono::system_clock::time_point& time,
			const std::wstring& data,
			const std::wstring& type,
			const std::wstring& time_color,
			const std::wstring& type_color);

	private:
		std::vector<std::tuple<logging_level,
							   std::chrono::system_clock::time_point,
							   std::wstring>>
			_buffer;

	private:
		std::wstring _store_log_root_path;
		std::wstring _store_log_file_name;
		std::wstring _store_log_extention;
		logging_styles _logging_style;
		std::vector<logging_level> _write_console_levels;
		std::locale _locale;

	private:
		std::atomic<bool> _thread_stop{ true };
		std::atomic<bool> _write_date{ false };
		std::atomic<bool> _append_date_on_file_name{ true };
		std::atomic<size_t> _limit_log_file_size{ 2097152 };

	private:
		std::mutex _mutex;
		std::shared_ptr<std::thread> _thread;
		std::condition_variable _condition;
		std::function<void(const std::wstring&)> _backup_notification;
		std::map<logging_level,
				 std::function<std::wstring(
					 const std::chrono::system_clock::time_point&,
					 const std::wstring&)>>
			_log_datas;

#pragma region singleton
	public:
		static logger& handle(void);

	private:
		static std::unique_ptr<logger> _handle;
		static std::once_flag _once;
#pragma endregion
	};
} // namespace logging
