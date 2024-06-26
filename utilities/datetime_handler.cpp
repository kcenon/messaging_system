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

#include "converting.h"

#include "fmt/chrono.h"
#include "fmt/format.h"
#include "fmt/xchar.h"

#include <ctime>
#include <iomanip>

namespace datetime_handler
{
	using namespace converting;

	auto datetime::date(const std::chrono::system_clock::time_point& time,
						const bool& use_seperator) -> std::string
	{
		auto in_time_t = std::chrono::system_clock::to_time_t(time);

		return fmt::format(((use_seperator) ? "{:%Y-%m-%d}" : "{:%Y%m%d}"),
						   fmt::localtime(in_time_t))
			.c_str();
	}

	auto datetime::time(const std::chrono::system_clock::time_point& time,
						const bool& use_seperator) -> std::string
	{
		auto in_time_t = std::chrono::system_clock::to_time_t(time);

		std::string result;

		// header
		fmt::format_to(std::back_inserter(result),
					   ((use_seperator) ? "{:%H:%M:%S}" : "{:%H%M%S}"),
					   fmt::localtime(in_time_t));
		if (use_seperator)
		{
			fmt::format_to(std::back_inserter(result), "{}", ".");
		}

		auto base_time = time.time_since_epoch();
		fmt::format_to(
			std::back_inserter(result), "{:03}",
			std::chrono::duration_cast<std::chrono::milliseconds>(base_time)
					.count()
				% 1000);
		fmt::format_to(
			std::back_inserter(result), "{:03}",
			std::chrono::duration_cast<std::chrono::microseconds>(base_time)
					.count()
				% 1000);
		fmt::format_to(
			std::back_inserter(result), "{:03}",
			std::chrono::duration_cast<std::chrono::nanoseconds>(base_time)
					.count()
				% 1000);

		return result;
	}
} // namespace datetime_handler
