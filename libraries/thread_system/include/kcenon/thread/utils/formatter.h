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

#include <string>
#include <string_view>
#include <type_traits>

#ifdef USE_STD_FORMAT
#include <format>
#else
// Use ostringstream as fallback when std::format is not available
#include <sstream>
#include <iomanip>
#endif

namespace utility_module
{
	/**
	 * @class enum_formatter
	 * @brief A generic formatter for enum types, using a user-provided converter functor.
	 *
	 * The @c enum_formatter template allows formatting an enum value by converting
	 * it to a string (via a custom @c Converter functor) and then passing that string
	 * to either C++20's @c std::format or the {fmt} library, depending on the build
	 * configuration.
	 *
	 * @tparam T The enum type to be formatted.
	 * @tparam Converter A callable object (e.g., a functor or lambda) that accepts
	 *                   an enum value of type @c T and returns its string representation.
	 *
	 * ### Example
	 * @code
	 * // 1) Define an enum and a corresponding converter functor:
	 * enum class color { Red, Green, Blue };
	 *
	 * struct color_converter {
	 *     std::string operator()(color c) const {
	 *         switch (c) {
	 *         case color::Red:   return "Red";
	 *         case color::Green: return "Green";
	 *         case color::Blue:  return "Blue";
	 *         }
	 *         return "Unknown";
	 *     }
	 * };
	 *
	 * // 2) Use enum_formatter<Color, color_converter> to format:
	 * // e.g. std::format("Color: {}", color::Green) // -> "Color: Green"
	 * @endcode
	 */
	template <typename T, typename Converter> class enum_formatter
	{
	private:
		/**
		 * @brief Internal helper that dispatches to @c std::format or {fmt} based on character
		 * type.
		 * @tparam CharT The character type (@c char or @c wchar_t).
		 * @param out An output iterator to which formatted text is appended.
		 * @param value The enum value to be converted to string and then formatted.
		 * @return An iterator pointing to the end of the inserted sequence.
		 */
		template <typename CharT> static auto do_format(auto& out, const T& value)
		{
#ifdef USE_STD_FORMAT
			if constexpr (std::is_same_v<CharT, wchar_t>)
			{
				return std::format_to(out, L"{}", Converter{}(value));
			}
			else
			{
				return std::format_to(out, "{}", Converter{}(value));
			}
#else
			// Fallback implementation using std::to_string
			auto converted = Converter{}(value);
			return std::copy(converted.begin(), converted.end(), out);
#endif
		}

	public:
		/**
		 * @brief A no-op parse function required by the formatting library.
		 * @param context The format parse context.
		 * @return An iterator pointing to the end of @p context.
		 *
		 * The formatter does not accept any custom formatting specifiers,
		 * so it simply returns @c context.begin().
		 */
		constexpr auto parse(auto& context) { return context.begin(); }

		/**
		 * @brief Formats the enum value into a provided format context.
		 * @tparam FormatContext The type of the format context (provided by either std::format or
		 * {fmt}).
		 * @param value The enum value to format.
		 * @param context The format context, which holds the output iterator and additional state.
		 * @return An iterator pointing to the end of the formatted output.
		 *
		 * Internally calls the helper @c do_format() to convert the enum to a string using @c
		 * Converter and then writes it.
		 */
		template <typename FormatContext> auto format(const T& value, FormatContext& context) const
		{
			using char_type =
				typename std::iterator_traits<typename FormatContext::iterator>::value_type;
			return do_format<char_type>(context.out(), value);
		}
	};

	/**
	 * @class formatter
	 * @brief Provides convenience methods for string formatting using either C++20 <format> or
	 * {fmt} library.
	 *
	 * The @c formatter class offers static functions to format strings (both narrow and wide)
	 * into a @c std::string / @c std::wstring, or directly to an output iterator. Which underlying
	 * implementation is used depends on whether @c USE_STD_FORMAT is defined.
	 *
	 * ### Example
	 * @code
	 * // Basic usage:
	 * std::string result = formatter::format("Hello, {}", "World");
	 *
	 * // Writing directly to a buffer:
	 * std::array<char, 50> buffer;
	 * auto it = formatter::format_to(buffer.begin(), "Number: {}", 42);
	 * // 'it' now points to the end of the written text
	 * @endcode
	 */
	class formatter
	{
	public:
		/**
		 * @brief Formats a narrow-character string with the given arguments.
		 * @tparam FormatArgs Parameter pack of argument types.
		 * @param formats A format string (e.g. "Name: {}, Age: {}").
		 * @param args The arguments to substitute into @p formats.
		 * @return A @c std::string containing the formatted result.
		 *
		 * If @c USE_STD_FORMAT is defined, uses C++20's @c std::format API.
		 * Otherwise, falls back to the {fmt} library.
		 */
		template <typename... FormatArgs>
		static auto format(const char* formats, const FormatArgs&... args) -> std::string
		{
#ifdef USE_STD_FORMAT
			return std::vformat(formats, std::make_format_args(args...));
#else
			// Fallback implementation using ostringstream
			std::ostringstream oss;
			format_impl(oss, formats, args...);
			return oss.str();
#endif
		}

	private:
		// Helper to output arguments with proper conversion
		template <typename Stream, typename T>
		static void output_arg(Stream& stream, const T& arg) {
			if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view> ||
						 std::is_same_v<T, const char*>) {
				stream << arg;
			} else if constexpr (std::is_same_v<T, std::wstring> || std::is_same_v<T, std::wstring_view> ||
							 std::is_same_v<T, const wchar_t*>) {
				stream << arg;
			} else if constexpr (std::is_array_v<T> && std::is_same_v<std::remove_extent_t<T>, char>) {
				// Handle char arrays (string literals)
				stream << arg;
			} else if constexpr (std::is_array_v<T> && std::is_same_v<std::remove_extent_t<T>, wchar_t>) {
				// Handle wchar_t arrays (wide string literals)
				stream << arg;
			} else if constexpr (std::is_arithmetic_v<T>) {
				stream << arg;
			} else if constexpr (std::is_enum_v<T>) {
				// For enum types, try to use to_string function
				stream << to_string(arg);
			} else {
				// For other custom types with to_string method
				stream << arg.to_string();
			}
		}

		// Simple fallback formatting implementation
		template <typename Stream>
		static void format_impl(Stream& stream, const char* format) {
			stream << format;
		}

		template <typename Stream, typename T, typename... Args>
		static void format_impl(Stream& stream, const char* format, const T& arg, const Args&... args) {
			while (*format) {
				if (*format == '{' && *(format + 1) == '}') {
					output_arg(stream, arg);
					format += 2;
					format_impl(stream, format, args...);
					return;
				}
				stream << *format++;
			}
		}

		// Wide character version
		template <typename Stream>
		static void wformat_impl(Stream& stream, const wchar_t* format) {
			stream << format;
		}

		template <typename Stream, typename T, typename... Args>
		static void wformat_impl(Stream& stream, const wchar_t* format, const T& arg, const Args&... args) {
			while (*format) {
				if (*format == L'{' && *(format + 1) == L'}') {
					output_arg(stream, arg);
					format += 2;
					wformat_impl(stream, format, args...);
					return;
				}
				stream << *format++;
			}
		}

			public:
		/**
		 * @brief Formats a wide-character string with the given arguments.
		 * @tparam FormatArgs Parameter pack of argument types.
		 * @param formats A wide format string (e.g. L"Count: {}, Value: {}").
		 * @param args The arguments to substitute into @p formats.
		 * @return A @c std::wstring containing the formatted result.
		 */
		template <typename... FormatArgs>
		static auto format(const wchar_t* formats, const FormatArgs&... args) -> std::wstring
		{
#ifdef USE_STD_FORMAT
			return std::vformat(formats, std::make_wformat_args(args...));
#else
			// Fallback implementation using wostringstream
			std::wostringstream woss;
			wformat_impl(woss, formats, args...);
			return woss.str();
#endif
		}

		/**
		 * @brief Formats a narrow-character string directly to an output iterator.
		 * @tparam OutputIt The type of the output iterator (e.g. a back_inserter for a container).
		 * @tparam FormatArgs Parameter pack of argument types.
		 * @param out The output iterator where formatted text will be written.
		 * @param formats The narrow format string.
		 * @param args The arguments to substitute into @p formats.
		 * @return The output iterator advanced to the end of the written text.
		 *
		 * This method is useful when writing to a buffer, a container, or custom iterators.
		 */
		template <typename OutputIt, typename... FormatArgs>
		static auto format_to(OutputIt out, const char* formats, const FormatArgs&... args)
			-> OutputIt
		{
#ifdef USE_STD_FORMAT
			return std::vformat_to(out, formats, std::make_format_args(args...));
#else
			// Fallback implementation
			std::string result = format(formats, args...);
			return std::copy(result.begin(), result.end(), out);
#endif
		}

		/**
		 * @brief Formats a wide-character string directly to an output iterator.
		 * @tparam OutputIt The type of the output iterator (e.g. a back_inserter for a wstring).
		 * @tparam FormatArgs Parameter pack of argument types.
		 * @param out The output iterator where formatted text will be written.
		 * @param formats The wide format string.
		 * @param args The arguments to substitute into @p formats.
		 * @return The output iterator advanced to the end of the written text.
		 */
		template <typename OutputIt, typename... FormatArgs>
		static auto format_to(OutputIt out, const wchar_t* formats, const FormatArgs&... args)
			-> OutputIt
		{
#ifdef USE_STD_FORMAT
			return std::vformat_to(out, formats, std::make_wformat_args(args...));
#else
			// Fallback implementation
			std::wstring result = format(formats, args...);
			return std::copy(result.begin(), result.end(), out);
#endif
		}
	};
} // namespace utility_module
