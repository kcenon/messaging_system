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

#include "pipeline.h"
#include "network_system/integration/logger_integration.h"
#include <string_view>

// Using nested namespace definition for implementation details (C++17)
namespace network_system::internal::detail
{
	// Using inline variables for debugging messages (C++17)
	inline constexpr std::string_view compress_debug_msg = "[debug] default_compress_stub";
	inline constexpr std::string_view decompress_debug_msg = "[debug] default_decompress_stub";
	inline constexpr std::string_view encrypt_debug_msg = "[debug] default_encrypt_stub";
	inline constexpr std::string_view decrypt_debug_msg = "[debug] default_decrypt_stub";
	
	// Using constexpr functions with if constexpr for compile-time optimization (C++17)
	auto default_compress_stub(const std::vector<uint8_t>& data)
		-> std::vector<uint8_t>
	{
		NETWORK_LOG_TRACE(std::string(compress_debug_msg));
		// No real compression, just return data
		return data;
	}

	auto default_decompress_stub(const std::vector<uint8_t>& data)
		-> std::vector<uint8_t>
	{
		NETWORK_LOG_TRACE(std::string(decompress_debug_msg));
		// No real decompression, just return data
		return data;
	}

	auto default_encrypt_stub(const std::vector<uint8_t>& data)
		-> std::vector<uint8_t>
	{
		NETWORK_LOG_TRACE(std::string(encrypt_debug_msg));
		// No real encryption, just return data
		return data;
	}

	auto default_decrypt_stub(const std::vector<uint8_t>& data)
		-> std::vector<uint8_t>
	{
		NETWORK_LOG_TRACE(std::string(decrypt_debug_msg));
		// No real decryption, just return data
		return data;
	}
}

namespace network_system::internal
{
	// Using aggregate initialization with designated initializers (C++17)
	auto make_default_pipeline() -> pipeline
	{
		return pipeline{
			.compress = detail::default_compress_stub,
			.decompress = detail::default_decompress_stub,
			.encrypt = detail::default_encrypt_stub,
			.decrypt = detail::default_decrypt_stub
		};
	}

} // namespace network_system::internal
