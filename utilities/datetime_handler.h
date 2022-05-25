#pragma once

#include <string>
#include <chrono>

using namespace std;

namespace datetime_handler
{
	class datetime
	{
	public:
		static wstring time(const chrono::system_clock::time_point& time, const bool& use_seperator = true, const unsigned short& places_of_decimal = 9);
	};
}
