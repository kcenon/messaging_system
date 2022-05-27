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
