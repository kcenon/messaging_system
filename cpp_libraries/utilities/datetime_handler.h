#pragma once

#include <string>

namespace datetime_handler
{
	class datetime
	{
	public:
		static std::wstring current_time(const bool& use_seperator = true, const unsigned short& places_of_decimal = 9);
	};
}
