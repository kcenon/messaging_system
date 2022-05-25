#include "datetime_handler.h"

#include <ctime>
#include <iomanip>
#include <sstream>

namespace datetime_handler
{
	wstring datetime::time(const chrono::system_clock::time_point& time, const bool& use_seperator, const unsigned short& places_of_decimal)
	{
		auto in_time_t = chrono::system_clock::to_time_t(time);

		tm buf;
#if WIN32
		localtime_s(&buf, &in_time_t);
#else
		localtime_r(&in_time_t, &buf);
#endif

		wostringstream oss;
		oss << put_time(&buf, (use_seperator) ? L"%H:%M:%S" : L"%H%M%S");

		if (places_of_decimal > 0)
		{
			auto base_time = time.time_since_epoch();

			wostringstream temp;
			temp << setfill(L'0') << setw(3) << chrono::duration_cast<chrono::milliseconds>(base_time).count() % 1000;
			temp << setfill(L'0') << setw(3) << chrono::duration_cast<chrono::microseconds>(base_time).count() % 1000;
			temp << setfill(L'0') << setw(3) << chrono::duration_cast<chrono::nanoseconds>(base_time).count() % 1000;

			if (use_seperator)
			{
				oss << '.';
			}

			oss << temp.str().substr(0, (places_of_decimal > 9) ? 9 : places_of_decimal);
		}

		return oss.str();
	}
}
