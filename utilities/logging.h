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

#include <map>
#include <queue>
#include <tuple>
#include <mutex>
#include <atomic>
#include <memory>
#include <thread>
#include <vector>
#include <string>
#include <chrono>
#include <fstream>
#include <optional>
#include <iostream>
#include <functional>
#include <condition_variable>

using namespace std;

namespace logging
{
	class logger
	{
	private:
		logger(void);

	public:
		~logger(void);

	public:
		bool start(const wstring& store_log_file_name = L"log", locale target_locale = locale(""), 
			const wstring& store_log_extention = L"log", const wstring& store_log_root_path = L"",
			const bool& append_date_on_file_name = true);
		bool stop(void);

	public:
		void set_target_level(const logging_level& target_level);
		void set_target_level(const vector<logging_level>& write_console_levels);
		void set_write_console(const logging_styles& logging_style = logging_styles::file_only);
		void set_write_date(const bool& write_date);
		void set_limit_log_file_size(const size_t& limit_log_file_size);
		void set_backup_notification(const function<void(const wstring&)>& notification);

	public:
		chrono::time_point<chrono::high_resolution_clock> chrono_start(void);
		void write(const logging_level& target_level, const wstring& log_data, const optional<chrono::time_point<chrono::high_resolution_clock>>& time = nullopt);
		void write(const logging_level& target_level, const vector<uint8_t>& log_data, const optional<chrono::time_point<chrono::high_resolution_clock>>& time = nullopt);

	protected:
		void run(void);

	private:
		void set_log_flag(const wstring& flag);
		void backup_log(const wstring& target_path, const wstring& backup_path);

	private:
		wstring exception_log(const chrono::system_clock::time_point& time, const wstring& data);
		wstring error_log(const chrono::system_clock::time_point& time, const wstring& data);
		wstring information_log(const chrono::system_clock::time_point& time, const wstring& data);
		wstring sequence_log(const chrono::system_clock::time_point& time, const wstring& data);
		wstring parameter_log(const chrono::system_clock::time_point& time, const wstring& data);
		wstring packet_log(const chrono::system_clock::time_point& time, const wstring& data);
		wstring make_log_string(const logging_level& target_level, const chrono::system_clock::time_point& time, 
			const wstring& data, const wstring& type, const wstring& time_color, const wstring& type_color);

	private:
		vector<tuple<logging_level, chrono::system_clock::time_point,wstring>> _buffer;

	private:
		wstring _store_log_root_path;
		wstring _store_log_file_name;
		wstring _store_log_extention;
		logging_styles _logging_style;
		vector<logging_level> _write_console_levels;
		locale _locale;

	private:
		atomic<bool> _thread_stop{ true };
		atomic<bool> _write_date{ false };
		atomic<bool> _append_date_on_file_name{ true };
		atomic<size_t> _limit_log_file_size{ 2097152 };

	private:
		mutex _mutex;
		shared_ptr<thread> _thread;
		condition_variable _condition;
		function<void(const wstring&)> _backup_notification;
		map<logging_level, function<wstring(const chrono::system_clock::time_point&, const wstring&)>> _log_datas;

#pragma region singleton
	public:
		static logger& handle(void);

	private:
		static unique_ptr<logger> _handle;
		static once_flag _once;
#pragma endregion
	};
}


