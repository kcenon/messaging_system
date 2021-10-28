#include "datetime_handler.h"

#include <ctime>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace datetime_handler
{
	std::wstring datetime::current_time(const bool& use_seperator, const unsigned short& places_of_decimal)
	{
		auto now = std::chrono::system_clock::now();
		auto in_time_t = std::chrono::system_clock::to_time_t(now);

		std::tm buf;
		localtime_s(&buf, &in_time_t);

		std::wostringstream oss;
		oss << std::put_time(&buf, (use_seperator) ? L"%H:%M:%S" : L"%H%M%S");

		if (places_of_decimal > 0)
		{
			auto base_time = now.time_since_epoch();

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
