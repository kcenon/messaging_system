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

#include "logging.h"

#include "file_handler.h"
#include "datetime_handler.h"
#include "converting.h"

#include "fmt/chrono.h"
#include "fmt/format.h"
#include "fmt/xchar.h"

#include <fcntl.h>
#include <iostream>
#include <filesystem>
#include <codecvt>

namespace logging
{
	using namespace converting;
	using namespace file_handler;
	using namespace datetime_handler;

	logger::logger(void) : _store_log_root_path(L""), _store_log_file_name(L""), _store_log_extention(L"")
		, _locale(locale("")), _backup_notification(nullptr)
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

	bool logger::start(const wstring& store_log_file_name, locale target_locale, const wstring& store_log_extention, 
		const wstring& store_log_root_path, const bool& append_date_on_file_name)
	{
		stop();

		_store_log_file_name = store_log_file_name;
		_store_log_extention = store_log_extention;
		_store_log_root_path = store_log_root_path;
		_append_date_on_file_name.store(append_date_on_file_name);
		_locale = target_locale;

		wcout.imbue(_locale);
		
		_thread = make_shared<thread>(&logger::run, this);

		return true;
	}

	bool logger::stop(void)
	{
		_thread_stop.store(true);

		if (_thread != nullptr)
		{
			if (_thread->joinable())
			{
				_condition.notify_one();
				_thread->join();
			}
			_thread.reset();
		}

		_thread_stop.store(false);

		return true;
	}

	void logger::set_target_level(const logging_level& target_level)
	{
		_write_console_levels.clear();

		unsigned short max_value = (unsigned short)target_level;
		for (unsigned short index = 0; index <= max_value; ++index) 
		{
			_write_console_levels.push_back((logging_level)index);
		}
	}

	void logger::set_target_level(const vector<logging_level>& write_console_levels)
	{
		_write_console_levels = write_console_levels;
	}

	void logger::set_write_console(const logging_styles& logging_style)
	{
		_logging_style = logging_style;
	}

	void logger::set_write_date(const bool& write_date)
	{
		_write_date.store(write_date);
	}

	void logger::set_limit_log_file_size(const size_t& limit_log_file_size)
	{
		_limit_log_file_size.store(limit_log_file_size);
	}

	void logger::set_backup_notification(const function<void(const wstring&)>& notification)
	{
		_backup_notification = notification;
	}

	chrono::time_point<chrono::high_resolution_clock> logger::chrono_start(void)
	{
		return chrono::high_resolution_clock::now();
	}

	void logger::write(const logging_level& target_level, const wstring& log_data, const optional<chrono::time_point<chrono::high_resolution_clock>>& time)
	{
		if (!_write_console_levels.empty())
		{
			auto search = find(_write_console_levels.begin(), _write_console_levels.end(), target_level);
			if (search == _write_console_levels.end())
			{
				return;
			}
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

	void logger::write(const logging_level& target_level, const vector<uint8_t>& log_data, const optional<chrono::time_point<chrono::high_resolution_clock>>& time)
	{
		if (!_write_console_levels.empty())
		{
			auto search = find(_write_console_levels.begin(), _write_console_levels.end(), target_level);
			if (search == _write_console_levels.end())
			{
				return;
			}
		}

		write(target_level, converter::to_wstring(log_data), time);
	}

	void logger::run(void)
	{
		set_log_flag(L"START");

		wstring source = L"";
		while (!_thread_stop.load() || !_buffer.empty())
		{
			unique_lock<mutex> unique(_mutex);
			_condition.wait(unique, [this] { return _thread_stop.load() || !_buffer.empty(); });

			auto buffers = std::move(_buffer);
			unique.unlock();

			filesystem::path target_path;
			if (_append_date_on_file_name.load())
			{
				target_path = fmt::format(L"{}{}_{:%Y-%m-%d}.{}", _store_log_root_path,
					_store_log_file_name, fmt::localtime(chrono::system_clock::to_time_t(chrono::system_clock::now())), _store_log_extention);
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
					_store_log_file_name, fmt::localtime(chrono::system_clock::to_time_t(chrono::system_clock::now())), _store_log_extention));
			}
			else
			{
				backup_log(target_path.wstring(), fmt::format(L"{}{}_backup.{}", _store_log_root_path,
					_store_log_file_name, _store_log_extention));
			}
			
			if (_append_date_on_file_name.load())
			{
				source = fmt::format(L"{}{}_{:%Y-%m-%d}.{}", _store_log_root_path, _store_log_file_name, fmt::localtime(chrono::system_clock::to_time_t(chrono::system_clock::now())), _store_log_extention).c_str();
			}
			else
			{
				source = fmt::format(L"{}{}.{}", _store_log_root_path, _store_log_file_name, _store_log_extention).c_str();
			}

#ifdef _WIN32
			wfstream stream;
#else
			fstream stream;
#endif
			if (_logging_style >= logging_styles::file_and_console)
			{
#ifdef _WIN32
				stream.open(source, ios::out | ios::app);
#else
				stream.open(converter::to_string(source), ios::out | ios::app);
#endif
				if (!stream.is_open())
				{
					return;
				}
				stream.imbue(_locale);
			}

			source = L"";
			for (auto& buffer : buffers)
			{
				auto iterator = _log_datas.find(get<0>(buffer));
				if (iterator == _log_datas.end())
				{
					continue;
				}

				fmt::format_to(back_inserter(source), L"{}", iterator->second(get<1>(buffer), get<2>(buffer)));
			}
			buffers.clear();

			if (_logging_style >= logging_styles::file_and_console)
			{
#ifdef _WIN32
				stream << source;
#else
				stream << converter::to_string(source);
#endif
			}
			source = L"";

			if (_logging_style >= logging_styles::file_and_console)
			{
				stream.flush();
				stream.close();
			}
		}

		set_log_flag(L"END");
	}

	void logger::set_log_flag(const wstring& flag)
	{
		wstring source = L"";
		if (_append_date_on_file_name.load())
		{
			source = fmt::format(L"{}{}_{:%Y-%m-%d}.{}", _store_log_root_path, _store_log_file_name, fmt::localtime(chrono::system_clock::to_time_t(chrono::system_clock::now())), _store_log_extention).c_str();
		}
		else
		{
			source = fmt::format(L"{}{}.{}", _store_log_root_path, _store_log_file_name, _store_log_extention).c_str();
		}

#ifdef _WIN32
		wfstream stream;
#else
		fstream stream;
#endif
		if (_logging_style >= logging_styles::file_and_console)
		{
#ifdef _WIN32
			stream.open(source, ios::out | ios::app);
#else
			stream.open(converter::to_string(source), ios::out | ios::app);
#endif
			if (!stream.is_open())
			{
				return;
			}
			stream.imbue(_locale);
		}

		chrono::system_clock::time_point current = chrono::system_clock::now();
		auto time_string = datetime::time(current, true);
		if (_write_date.load())
		{
			wstring temp = fmt::format(L"[{:%Y-%m-%d} {}][{}]\n", fmt::localtime(chrono::system_clock::to_time_t(current)), time_string, flag);
			if (_logging_style < logging_styles::file_only)
			{
				wcout << fmt::format(L"[\033[0;94m{:%Y-%m-%d} {}\033[0m][\033[0;34m{}\033[0m]\n", fmt::localtime(chrono::system_clock::to_time_t(current)), time_string, flag);
			}

			if (_logging_style >= logging_styles::file_and_console)
			{
#ifdef _WIN32
				stream << temp;
#else
				stream << converter::to_string(temp);
#endif
			}
		}
		else
		{
			wstring temp = fmt::format(L"[{}][{}]\n", time_string, flag);
			if (_logging_style < logging_styles::file_only)
			{
				wcout << fmt::format(L"[\033[0;94m{}\033[0m][\033[0;34m{}\033[0m]\n", time_string, flag);
			}

			if (_logging_style >= logging_styles::file_and_console)
			{
#ifdef _WIN32
				stream << temp;
#else
				stream << converter::to_string(temp);
#endif
			}
		}

		if (_logging_style >= logging_styles::file_and_console)
		{
			stream.flush();
			stream.close();
		}
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
		filesystem::resize_file(target_path, 0);

		if (_backup_notification != nullptr)
		{
			_backup_notification(backup_path);
		}
	}

	wstring logger::exception_log(const chrono::system_clock::time_point& time, const wstring& data)
	{
		return make_log_string(logging_level::exception, time, data, L"EXCEPTION", L"\033[0;94m", L"\033[0;95m");
	}

	wstring logger::error_log(const chrono::system_clock::time_point& time, const wstring& data)
	{
		return make_log_string(logging_level::error, time, data, L"ERROR", L"\033[0;94m", L"\033[0;31m");
	}

	wstring logger::information_log(const chrono::system_clock::time_point& time, const wstring& data)
	{
		return make_log_string(logging_level::information, time, data, L"INFORMATION", L"\033[0;94m", L"\033[0;92m");
	}

	wstring logger::sequence_log(const chrono::system_clock::time_point& time, const wstring& data)
	{
		return make_log_string(logging_level::sequence, time, data, L"SEQUENCE", L"\033[0;94m", L"\033[0;90m");
	}

	wstring logger::parameter_log(const chrono::system_clock::time_point& time, const wstring& data)
	{
		return make_log_string(logging_level::parameter, time, data, L"PARAMETER", L"\033[0;94m", L"\033[0;97m");
	}

	wstring logger::packet_log(const chrono::system_clock::time_point& time, const wstring& data)
	{
		return make_log_string(logging_level::packet, time, data, L"PACKET", L"\033[0;94m", L"\033[0;91m");
	}

	wstring logger::make_log_string(const logging_level& target_level, const chrono::system_clock::time_point& time, const wstring& data, const wstring& type, 
		const wstring& time_color, const wstring& type_color)
	{
		auto time_string = datetime::time(time, true);
		if (_write_date.load())
		{
			if (_logging_style < logging_styles::file_only)
			{
				wcout << fmt::format(L"[{}{:%Y-%m-%d} {}\033[0m][{}{}\033[0m]: {}\n", 
					time_color, fmt::localtime(chrono::system_clock::to_time_t(time)), time_string, type_color, type, data);
			}

			return fmt::format(L"[{:%Y-%m-%d} {}][{}]: {}\n", fmt::localtime(chrono::system_clock::to_time_t(time)), time_string, type, data);
		}

		if (_logging_style < logging_styles::file_only)
		{
			wcout << fmt::format(L"[{}{}\033[0m][{}{}\033[0m]: {}\n", 
				time_color, time_string, type_color, type, data);
		}

		return fmt::format(L"[{}][{}]: {}\n", time_string, type, data);
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
