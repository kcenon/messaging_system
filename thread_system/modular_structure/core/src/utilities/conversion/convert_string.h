/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this
   list of conditions and the following disclaimer in the documentation
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

#include <tuple>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include <errno.h>
#include <optional>
#include <algorithm>
#include <string_view>

namespace utility_module
{
	/**
	 * @class convert_string
	 * @brief Provides utilities for string encoding conversion, Base64 encoding/decoding,
	 *        and substring operations like splitting or replacing.
	 *
	 * This class relies on platform-specific APIs or @c iconv (when available) to convert strings
	 * between different encodings such as UTF-8, UTF-16, UTF-32, and the local system encoding.
	 * It also provides methods to handle Base64 encoding/decoding and some basic string
	 * manipulation (split, replace).
	 *
	 * ### Encoding Conversions
	 * - Typically returns a tuple containing either a successful result (@c std::optional<T>)
	 *   and an empty error (@c std::nullopt), or @c std::nullopt with an error message
	 *   describing the failure.
	 * - BOM (Byte Order Mark) is handled in certain functions for UTF-8, UTF-16, and UTF-32.
	 *
	 * ### Base64
	 * - Encodes and decodes raw byte arrays (@c std::vector<uint8_t>).
	 *
	 * ### Usage Example
	 * @code
	 * // Converting a wstring to system-encoded string
	 * auto [strOpt, errMsg] = convert_string::to_string(L"안녕하세요");
	 * if (!strOpt) {
	 *     std::cerr << "Error: " << *errMsg << std::endl;
	 * } else {
	 *     std::cout << "Converted string: " << *strOpt << std::endl;
	 * }
	 * @endcode
	 */
	class convert_string
	{
	public:
		/**
		 * @brief Converts a @c std::wstring to a @c std::string using the system encoding.
		 * @param value The wide-string input.
		 * @return A tuple of:
		 *         - @c std::optional<std::string>: The converted narrow string on success,
		 *           or @c std::nullopt on failure.
		 *         - @c std::optional<std::string>: The error message if conversion fails,
		 *           otherwise @c std::nullopt.
		 *
		 * On Windows, this typically uses the current code page. On Unix-like systems,
		 * it may use @c iconv or locale-based conversion functions.
		 */
		static auto to_string(const std::wstring& value)
			-> std::tuple<std::optional<std::string>, std::optional<std::string>>;

		/**
		 * @brief Converts a @c std::wstring_view to a @c std::string using the system encoding.
		 * @param value The wide-string view.
		 * @return Same tuple semantics as @c to_string(const std::wstring&).
		 *
		 * This is a convenience overload for handling string views without copying
		 * the entire wide string first.
		 */
		static auto to_string(std::wstring_view value)
			-> std::tuple<std::optional<std::string>, std::optional<std::string>>;

		/**
		 * @brief Converts a @c std::string (system-encoded) to a @c std::wstring.
		 * @param value The narrow string in system encoding.
		 * @return A tuple containing:
		 *         - @c std::optional<std::wstring>: The converted wide string on success.
		 *         - @c std::optional<std::string>: An error message if conversion fails.
		 */
		static auto to_wstring(const std::string& value)
			-> std::tuple<std::optional<std::wstring>, std::optional<std::string>>;

		/**
		 * @brief Converts a @c std::string_view (system-encoded) to a @c std::wstring.
		 * @param value The narrow string view in system encoding.
		 * @return Same tuple semantics as @c to_wstring(const std::string&).
		 */
		static auto to_wstring(std::string_view value)
			-> std::tuple<std::optional<std::wstring>, std::optional<std::string>>;

		/**
		 * @brief Retrieves the system code page used for conversions.
		 * @return An integer representing the system code page (e.g., CP_ACP on Windows).
		 *
		 * On Unix-like systems using @c iconv, this may return an approximation based
		 * on locale info, or a default if detection fails.
		 */
		static auto get_system_code_page() -> int;

		/**
		 * @brief Converts a system-encoded string to UTF-8.
		 * @param value The input string in system encoding.
		 * @return A tuple of:
		 *         - @c std::optional<std::string>: The UTF-8 encoded string on success.
		 *         - @c std::optional<std::string>: An error message on failure.
		 */
		static auto system_to_utf8(const std::string& value)
			-> std::tuple<std::optional<std::string>, std::optional<std::string>>;

		/**
		 * @brief Converts a UTF-8 encoded string to the system encoding.
		 * @param value The UTF-8 encoded input string.
		 * @return Same tuple semantics as @c system_to_utf8().
		 */
		static auto utf8_to_system(const std::string& value)
			-> std::tuple<std::optional<std::string>, std::optional<std::string>>;

		/**
		 * @brief Splits a string by a given delimiter.
		 * @param source The source string to split.
		 * @param token The delimiter substring.
		 * @return A tuple of:
		 *         - @c std::optional<std::vector<std::string>>: A vector of tokens on success.
		 *         - @c std::optional<std::string>: An error message if splitting fails.
		 *
		 * If @p token is empty, splitting cannot proceed and an error is returned.
		 */
		static auto split(const std::string& source, const std::string& token)
			-> std::tuple<std::optional<std::vector<std::string>>, std::optional<std::string>>;

		/**
		 * @brief Converts a system-encoded string to a UTF-8 byte array.
		 * @param value The input string in system encoding.
		 * @return A tuple of:
		 *         - @c std::optional<std::vector<uint8_t>>: The UTF-8 byte array on success.
		 *         - @c std::optional<std::string>: An error message on failure.
		 */
		static auto to_array(const std::string& value)
			-> std::tuple<std::optional<std::vector<uint8_t>>, std::optional<std::string>>;

		/**
		 * @brief Converts a UTF-8 byte array to a system-encoded string.
		 * @param value The input byte array.
		 * @return A tuple with:
		 *         - @c std::optional<std::string>: The system-encoded string on success.
		 *         - @c std::optional<std::string>: An error message on failure.
		 */
		static auto to_string(const std::vector<uint8_t>& value)
			-> std::tuple<std::optional<std::string>, std::optional<std::string>>;

		/**
		 * @brief Encodes a byte array into a Base64 string.
		 * @param value The raw byte array to encode.
		 * @return A tuple of:
		 *         - @c std::optional<std::string>: The Base64-encoded string on success.
		 *         - @c std::optional<std::string>: An error message if encoding fails.
		 *
		 * Typically, Base64 encoding should not fail unless the input is extremely large
		 * and memory allocation fails.
		 */
		static auto to_base64(const std::vector<uint8_t>& value)
			-> std::tuple<std::optional<std::string>, std::optional<std::string>>;

		/**
		 * @brief Decodes a Base64 string into a byte array.
		 * @param base64_str The Base64-encoded string.
		 * @return A tuple of:
		 *         - @c std::vector<uint8_t>: The decoded data. May be empty if decoding fails.
		 *         - @c std::optional<std::string>: An error message on failure, or @c std::nullopt
		 * on success.
		 */
		static auto from_base64(const std::string& base64_str)
			-> std::tuple<std::vector<uint8_t>, std::optional<std::string>>;

		/**
		 * @brief Replaces all occurrences of @p token in @p source with @p target, in place.
		 * @param source The string to modify.
		 * @param token The substring to find.
		 * @param target The substring to replace @p token with.
		 * @return @c std::optional<std::string> containing an error message if replacement fails,
		 *         or @c std::nullopt on success.
		 *
		 * This function modifies @p source directly. If @p token is empty, it returns an error.
		 */
		static auto replace(std::string& source,
							const std::string& token,
							const std::string& target) -> std::optional<std::string>;

		/**
		 * @brief Replaces all occurrences of @p token in @p source with @p target, returning a new
		 * string.
		 * @param source The source string (unchanged).
		 * @param token The substring to find.
		 * @param target The substring to replace @p token with.
		 * @return A tuple of:
		 *         - @c std::optional<std::string>: The modified string on success.
		 *         - @c std::optional<std::string>: An error message if replacement fails.
		 */
		static auto replace2(const std::string& source,
							 const std::string& token,
							 const std::string& target)
			-> std::tuple<std::optional<std::string>, std::optional<std::string>>;

	private:
		/// @brief Possible endianness values for UTF-16 or UTF-32 data.
		enum class endian_types
		{
			little, ///< Little-endian
			big,	///< Big-endian
			unknown ///< Unknown or not applicable
		};

		/// @brief Supported encoding types used in iconv-based conversions.
		enum class encoding_types
		{
			utf8,  ///< UTF-8 encoding
			utf16, ///< UTF-16 encoding
			utf32  ///< UTF-32 encoding
		};

		/**
		 * @brief Retrieves a textual name for a code page (e.g., "CP_ACP" or a locale-based name).
		 * @param code_page The code page integer.
		 * @return A string representing the code page name, or an empty string if unknown.
		 */
		static auto get_code_page_name(int code_page) -> std::string;

		/**
		 * @brief Determines the encoding name used by iconv based on @p encoding and @p endian.
		 * @param encoding The base encoding (UTF-8, UTF-16, or UTF-32).
		 * @param endian The byte order (little, big) if applicable.
		 * @return A string recognized by iconv, such as "UTF-8" or "UTF-16LE".
		 */
		static auto get_encoding_name(encoding_types encoding,
									  endian_types endian = endian_types::little) -> std::string;

		/**
		 * @brief Derives the wchar_t encoding name based on its size and endianness.
		 * @param endian The byte order. Defaults to little-endian.
		 * @return For example, "UTF-16LE" on Windows or "UTF-32LE" on some Unix-like systems.
		 */
		static auto get_wchar_encoding(endian_types endian = endian_types::little) -> std::string;

		/**
		 * @brief Converts a string (or wide string) to a @c std::vector<char> for iconv processing.
		 * @tparam T The character type of the input string (char, wchar_t, char16_t, etc.).
		 * @param value The input string.
		 * @return A vector containing the raw byte representation of @p value.
		 */
		template <typename T> static auto string_to_vector(const T& value) -> std::vector<char>
		{
			return std::vector<char>(reinterpret_cast<const char*>(value.data()),
									 reinterpret_cast<const char*>(value.data() + value.size()));
		}

		/**
		 * @brief Converts from one encoding to another using @c iconv or equivalent APIs.
		 * @tparam FromType The input string type.
		 * @tparam ToType The output string type.
		 * @param value The input string.
		 * @param from_encoding Source encoding name (e.g., "UTF-16LE").
		 * @param to_encoding Target encoding name (e.g., "UTF-8").
		 * @return A tuple:
		 *         - @c std::optional<ToType>: The converted string on success,
		 *           or @c std::nullopt on failure.
		 *         - @c std::optional<std::string>: The error message if conversion fails.
		 */
		template <typename FromType, typename ToType>
		static auto convert(const FromType& value,
							const std::string& from_encoding,
							const std::string& to_encoding)
			-> std::tuple<std::optional<ToType>, std::optional<std::string>>;

		/**
		 * @brief Detects the endianness of a UTF-16 string by checking for BOM or content patterns.
		 * @param value The UTF-16 string.
		 * @return The detected endianness (little, big, or unknown).
		 */
		static auto detect_endian(const std::u16string& value) -> endian_types;

		/**
		 * @brief Detects the endianness of a UTF-32 string by checking for BOM or content patterns.
		 * @param value The UTF-32 string.
		 * @return The detected endianness (little, big, or unknown).
		 */
		static auto detect_endian(const std::u32string& value) -> endian_types;

		/**
		 * @brief Checks if a string has a UTF-8 BOM (Byte Order Mark).
		 * @param value The input string (UTF-8 encoded).
		 * @return @c true if the string starts with the UTF-8 BOM (0xEF,0xBB,0xBF), else @c false.
		 */
		static auto has_utf8_bom(const std::string& value) -> bool;

		/**
		 * @brief Removes a leading UTF-8 BOM from a string, if present.
		 * @param value The input string (UTF-8 encoded).
		 * @return A new string without the BOM. If no BOM was present, returns the original string.
		 */
		static auto remove_utf8_bom(const std::string& value) -> std::string;

		/**
		 * @brief Adds a UTF-8 BOM to a string if it doesn't already have one.
		 * @param value The input string (UTF-8 encoded).
		 * @return A new string guaranteed to have the UTF-8 BOM at the start.
		 */
		static auto add_utf8_bom(const std::string& value) -> std::string;

		/**
		 * @brief Encodes a byte array into a Base64 string.
		 * @param data The raw byte array.
		 * @return The Base64-encoded representation.
		 */
		static auto base64_encode(const std::vector<uint8_t>& data) -> std::string;

		/**
		 * @brief Decodes a Base64 string into a byte array.
		 * @param base64_str The Base64 string.
		 * @return A tuple of:
		 *         - @c std::vector<uint8_t>: The decoded bytes (empty on failure).
		 *         - @c std::optional<std::string>: An error message if decoding fails, otherwise @c
		 * std::nullopt.
		 */
		static auto base64_decode(const std::string& base64_str)
			-> std::tuple<std::vector<uint8_t>, std::optional<std::string>>;
	};
} // namespace utility_module