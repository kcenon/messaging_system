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

#include <vector>
#include <cstdint>
#include <functional>
#include <string_view>
#include <type_traits>

// Use nested namespace definition (C++17)
namespace network_module
{

	/*!
	 * \struct pipeline
	 * \brief Holds function objects for compressing, decompressing, encrypting,
	 * and decrypting data.
	 *
	 * Typically used by \c messaging_session or \c messaging_client to
	 * transform data buffers before sending or after receiving. The default
	 * stubs do no actual transformation.
	 *
	 * ### Example
	 * \code
	 * // Create a default pipeline
	 * pipeline p = make_default_pipeline();
	 *
	 * // Use p.compress(...) on data
	 * auto compressed = p.compress(original_data);
	 * // ...
	 * \endcode
	 */
	struct pipeline
	{
		/*!
		 * \brief Function object for compressing a data buffer.
		 * \param data A vector of bytes to compress.
		 * \return A new vector of bytes after compression.
		 */
		std::function<std::vector<uint8_t>(const std::vector<uint8_t>&)>
			compress;

		/*!
		 * \brief Function object for decompressing a data buffer.
		 * \param data A vector of bytes to decompress.
		 * \return A new vector of bytes after decompression.
		 */
		std::function<std::vector<uint8_t>(const std::vector<uint8_t>&)>
			decompress;

		/*!
		 * \brief Function object for encrypting a data buffer.
		 */
		std::function<std::vector<uint8_t>(const std::vector<uint8_t>&)>
			encrypt;

		/*!
		 * \brief Function object for decrypting a data buffer.
		 */
		std::function<std::vector<uint8_t>(const std::vector<uint8_t>&)>
			decrypt;
	};

	/*!
	 * \brief Creates a default pipeline with trivial (no-op) implementations
	 * for compress, decompress, encrypt, and decrypt.
	 * \return A \c pipeline struct with stubbed function objects.
	 */
	// Pipeline creation function - can't be constexpr because std::function is not a literal type
	auto make_default_pipeline() -> pipeline;

} // namespace network_module