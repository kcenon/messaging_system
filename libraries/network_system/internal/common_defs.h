/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
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

#pragma once

#include <cstdint>
#include <string_view>

// Use nested namespace definition (C++17)
namespace network_system::internal
{
	/*!
	 * \enum data_mode
	 * \brief Represents a simple enumeration for differentiating data
	 * transmission modes.
	 *
	 * A higher-level code might use these to switch between packet-based,
	 * file-based, or binary data logic. They are optional stubs and can be
	 * extended as needed.
	 */
	enum class data_mode : std::uint8_t {
		packet_mode = 1, /*!< Regular messaging/packet mode. */
		file_mode = 2,	 /*!< File transfer mode. */
		binary_mode = 3	 /*!< Raw binary data mode. */
	};

	// Use inline variables for constants (C++17)
	inline constexpr std::size_t default_buffer_size = 4096;
	inline constexpr std::size_t default_timeout_ms = 5000;
	inline constexpr std::string_view default_client_id = "default_client";
	inline constexpr std::string_view default_server_id = "default_server";

} // namespace network_system::internal