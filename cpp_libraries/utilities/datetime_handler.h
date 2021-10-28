#pragma once

#include <string>
#include <chrono>

namespace datetime_handler
{
	class datetime
	{
	public:
		static std::wstring current_time(const bool& use_seperator = true, const unsigned short& places_of_decimal = 9);
		static std::wstring time(const std::chrono::system_clock::time_point& time, const bool& use_seperator = true, const unsigned short& places_of_decimal = 9);
	};
}
