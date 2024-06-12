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
		bool start(const std::string& store_log_file_name = "log",
				   std::locale target_locale = std::locale(""),
				   const std::string& store_log_extention = "log",
				   const std::string& store_log_root_path = "",
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
			const std::function<void(const std::string&)>& notification);

	public:
		std::chrono::time_point<std::chrono::high_resolution_clock>
		chrono_start(void);
		void write(const logging_level& target_level,
				   const std::string& log_data,
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
		void set_log_flag(const std::string& flag);
		void backup_log(const std::string& target_path,
						const std::string& backup_path);

	private:
		std::string exception_log(
			const std::chrono::system_clock::time_point& time,
			const std::string& data);
		std::string error_log(const std::chrono::system_clock::time_point& time,
							  const std::string& data);
		std::string information_log(
			const std::chrono::system_clock::time_point& time,
			const std::string& data);
		std::string sequence_log(
			const std::chrono::system_clock::time_point& time,
			const std::string& data);
		std::string parameter_log(
			const std::chrono::system_clock::time_point& time,
			const std::string& data);
		std::string packet_log(
			const std::chrono::system_clock::time_point& time,
			const std::string& data);
		std::string make_log_string(
			const logging_level& target_level,
			const std::chrono::system_clock::time_point& time,
			const std::string& data,
			const std::string& type,
			const std::string& time_color,
			const std::string& type_color);

	private:
		std::vector<std::tuple<logging_level,
							   std::chrono::system_clock::time_point,
							   std::string>>
			_buffer;

	private:
		std::string _store_log_root_path;
		std::string _store_log_file_name;
		std::string _store_log_extention;
		logging_styles _logging_style;
		std::vector<logging_level> _write_console_levels;
		std::locale _locale;

	private:
		std::atomic<bool> _thread_stop{ true };
		std::atomic<bool> _write_date{ false };
		std::atomic<bool> _append_date_on_file_name{ true };
		std::atomic<size_t> _limit_log_file_size{ 2097152 };

	private:
		std::mutex mutex_;
		std::shared_ptr<std::thread> thread_;
		std::condition_variable _condition;
		std::function<void(const std::string&)> _backup_notification;
		std::map<logging_level,
				 std::function<std::string(
					 const std::chrono::system_clock::time_point&,
					 const std::string&)>>
			_log_datas;

#pragma region singleton
	public:
		static logger& handle(void);

	private:
		static std::unique_ptr<logger> handle_;
		static std::once_flag once_;
#pragma endregion
	};
} // namespace logging
