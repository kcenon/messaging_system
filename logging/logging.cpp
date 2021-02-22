#include "logging.h"

#include "fmt/chrono.h"
#include "fmt/format.h"

#include <iostream>

namespace logging
{
	util::util(void)
	{

	}

	util::~util(void)
	{

	}

	bool util::start(void)
	{
		stop();

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

	void util::write(const logging_level& target_level, const std::wstring& log_data)
	{
		if ((unsigned short)target_level > (unsigned short)_target_level)
		{
			return;
		}

		while (_transfer_buffer.load())
		{
			std::this_thread::yield();
		}

		_buffer.push_back({ target_level , { std::chrono::system_clock::now(), log_data } });
		_has_buffer.store(true);
	}

	void util::run(void)
	{
		fmt::wmemory_buffer result;
		std::vector<std::pair<logging_level, std::pair<std::chrono::system_clock::time_point, std::wstring>>> buffers;

		while (!_thread_stop.load() || !_buffer.empty())
		{
			if (!_has_buffer.load())
			{
				std::this_thread::yield();
				continue;
			}
			
			_transfer_buffer.store(true);
			if (!_buffer.empty())
			{
				buffers.swap(_buffer);
				_has_buffer.store(false);
			}
			_transfer_buffer.store(false);

			for (auto& buffer : buffers)
			{
				auto milli_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(buffer.second.first.time_since_epoch()).count() % 1000;
				auto micro_seconds = std::chrono::duration_cast<std::chrono::microseconds>(buffer.second.first.time_since_epoch()).count() % 1000;
				
				result.clear();
				fmt::format_to(std::back_inserter(result), L"[{:%Y-%m-%d %H:%M:%S}.{:0>3}{:0>3}]", fmt::localtime(buffer.second.first), milli_seconds, micro_seconds);
				switch (buffer.first)
				{
				case logging_level::exception: fmt::format_to(std::back_inserter(result), L"{}", L"[EXCEPTION]"); break;
				case logging_level::error: fmt::format_to(std::back_inserter(result), L"{}", L"[ERROR]"); break;
				case logging_level::information: fmt::format_to(std::back_inserter(result), L"{}", L"[INFORMATION]"); break;
				case logging_level::sequence: fmt::format_to(std::back_inserter(result), L"{}", L"[SEQUENCE]"); break;
				case logging_level::paramete: fmt::format_to(std::back_inserter(result), L"{}", L"[PARAMETER]"); break;
				}
				fmt::format_to(std::back_inserter(result), L": {}\r\n", buffer.second.second);

				if (_write_console.load())
				{
					std::wcout << result.data();
				}
			}

			buffers.clear();
		}
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
