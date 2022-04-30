#pragma once

#include "logging_level.h"

#include <map>
#include <queue>
#include <tuple>
#include <mutex>
#include <atomic>
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
			const bool& append_date_on_file_name = true, const unsigned short& places_of_decimal = 7);
		bool stop(void);

	public:
		void set_target_level(const logging_level& target_level);
		void set_write_console(const bool& write_console);
		void set_write_date(const bool& write_date);
		void set_limit_log_file_size(const size_t& limit_log_file_size);
		void set_backup_notification(const function<void(const wstring&)>& notification);

	public:
		chrono::time_point<chrono::high_resolution_clock> chrono_start(void);
		void write(const logging_level& target_level, const wstring& log_data, const optional<chrono::time_point<chrono::high_resolution_clock>>& time = nullopt);
		void write(const logging_level& target_level, const vector<unsigned char>& log_data, const optional<chrono::time_point<chrono::high_resolution_clock>>& time = nullopt);

	protected:
		void run(void);

	private:
		void set_log_flag(const wstring& flag);
		void backup_log(const wstring& target_path, const wstring& backup_path);
		void store_log(wfstream& buffer, const wstring& log);

	private:
		wstring exception_log(const chrono::system_clock::time_point& time, const wstring& data);
		wstring error_log(const chrono::system_clock::time_point& time, const wstring& data);
		wstring information_log(const chrono::system_clock::time_point& time, const wstring& data);
		wstring sequence_log(const chrono::system_clock::time_point& time, const wstring& data);
		wstring parameter_log(const chrono::system_clock::time_point& time, const wstring& data);
		wstring packet_log(const chrono::system_clock::time_point& time, const wstring& data);

	private:
		vector<tuple<logging_level, chrono::system_clock::time_point,wstring>> _buffer;

	private:
		logging_level _target_level;
		wstring _store_log_root_path;
		wstring _store_log_file_name;
		wstring _store_log_extention;
		unsigned short _places_of_decimal;
		locale _locale;

	private:
		atomic<bool> _thread_stop{ true };
		atomic<bool> _write_date{ false };
		atomic<bool> _write_console{ false };
		atomic<bool> _append_date_on_file_name{ true };
		atomic<size_t> _limit_log_file_size{ 2097152 };

	private:
		mutex _mutex;
		thread _thread;
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


