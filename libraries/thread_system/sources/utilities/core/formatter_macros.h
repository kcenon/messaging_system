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

/**
 * @file formatter_macros.h
 * @brief Provides macros for generating formatter specializations.
 * 
 * This file contains macros that generate formatter specializations for custom types
 * to work with both std::format (C++20) and fmt library. The macros eliminate code
 * duplication by generating the necessary boilerplate code for formatter specializations.
 * 
 * Usage example:
 * @code
 * // In your header file after class definition:
 * DECLARE_FORMATTER(my_namespace::my_class)
 * @endcode
 */

#include "formatter.h"
#include "../conversion/convert_string.h"

#ifdef USE_STD_FORMAT

/**
 * @brief Generates std::formatter specializations for narrow and wide characters.
 * 
 * This macro creates two formatter specializations:
 * 1. std::formatter<CLASS_NAME> for narrow character formatting
 * 2. std::formatter<CLASS_NAME, wchar_t> for wide character formatting
 * 
 * Requirements:
 * - The class must have a to_string() method that returns std::string
 * - The convert_string::to_wstring function must be available
 * 
 * @param CLASS_NAME The fully qualified class name (including namespace if any)
 */
#define DECLARE_FORMATTER(CLASS_NAME)                                                              \
template <> struct std::formatter<CLASS_NAME> : std::formatter<std::string_view>                   \
{                                                                                                  \
    template <typename FormatContext>                                                              \
    auto format(const CLASS_NAME& item, FormatContext& ctx) const                                  \
    {                                                                                              \
        return std::formatter<std::string_view>::format(item.to_string(), ctx);                   \
    }                                                                                              \
};                                                                                                 \
                                                                                                   \
template <>                                                                                        \
struct std::formatter<CLASS_NAME, wchar_t> : std::formatter<std::wstring_view, wchar_t>            \
{                                                                                                  \
    template <typename FormatContext>                                                              \
    auto format(const CLASS_NAME& item, FormatContext& ctx) const                                  \
    {                                                                                              \
        auto str = item.to_string();                                                               \
        auto wstr = utility_module::convert_string::to_wstring(str);                               \
        return std::formatter<std::wstring_view, wchar_t>::format(wstr, ctx);                     \
    }                                                                                              \
};

#else // USE_STD_FORMAT

/**
 * @brief Generates fmt::formatter specialization.
 * 
 * This macro creates a formatter specialization for the fmt library.
 * 
 * Requirements:
 * - The class must have a to_string() method that returns std::string
 * 
 * @param CLASS_NAME The fully qualified class name (including namespace if any)
 */
#define DECLARE_FORMATTER(CLASS_NAME)                                                              \
template <> struct fmt::formatter<CLASS_NAME> : fmt::formatter<std::string_view>                   \
{                                                                                                  \
    template <typename FormatContext>                                                              \
    auto format(const CLASS_NAME& item, FormatContext& ctx) const                                  \
    {                                                                                              \
        return fmt::formatter<std::string_view>::format(item.to_string(), ctx);                   \
    }                                                                                              \
};

#endif // USE_STD_FORMAT