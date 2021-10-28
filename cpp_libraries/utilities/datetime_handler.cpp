#include "datetime_handler.h"

#include <ctime>
#include <iomanip>
#include <sstream>

namespace datetime_handler
{
	std::wstring datetime::current_time(const bool& use_seperator, const unsigned short& places_of_decimal)
	{
		return time(std::chrono::system_clock::now(), use_seperator, places_of_decimal);
	}

	std::wstring datetime::time(const std::chrono::system_clock::time_point& time, const bool& use_seperator, const unsigned short& places_of_decimal)
	{
		auto in_time_t = std::chrono::system_clock::to_time_t(time);

		std::tm buf;
		localtime_s(&buf, &in_time_t);

		std::wostringstream oss;
		oss << std::put_time(&buf, (use_seperator) ? L"%H:%M:%S" : L"%H%M%S");

		if (places_of_decimal > 0)
		{
			auto base_time = time.time_since_epoch();

			std::wostringstream temp;
			temp << std::setfill(L'0') << std::setw(3) << std::chrono::duration_cast<std::chrono::milliseconds>(base_time).count() % 1000;
			temp << std::setfill(L'0') << std::setw(3) << std::chrono::duration_cast<std::chrono::microseconds>(base_time).count() % 1000;
			temp << std::setfill(L'0') << std::setw(3) << std::chrono::duration_cast<std::chrono::nanoseconds>(base_time).count() % 1000;

			if (use_seperator)
			{
				oss << '.';
			}

			oss << temp.str().substr(0, (places_of_decimal > 9) ? 9 : places_of_decimal);
		}

		return oss.str();
	}
}
