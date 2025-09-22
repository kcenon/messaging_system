#pragma once

#include <string>
#include <sstream>

#ifdef USE_STD_FORMAT
    #include <format>
#else
    #include <fmt/format.h>
#endif

namespace utility_module {

/**
 * @brief Simple formatter wrapper around fmt library or std::format
 */
class formatter {
public:
    template<typename... Args>
    static std::string format(const std::string& format_str, Args&&... args) {
        try {
#ifdef USE_STD_FORMAT
            return std::vformat(format_str, std::make_format_args(args...));
#else
            return fmt::vformat(format_str, fmt::make_format_args(args...));
#endif
        } catch (const std::exception&) {
            return format_str; // Return original string if formatting fails
        }
    }

    template<typename OutputIt, typename... Args>
    static void format_to(OutputIt out, const std::string& format_str, Args&&... args) {
        try {
#ifdef USE_STD_FORMAT
            std::vformat_to(out, format_str, std::make_format_args(args...));
#else
            fmt::vformat_to(out, format_str, fmt::make_format_args(args...));
#endif
        } catch (const std::exception&) {
            // Fallback: just copy the format string
            std::copy(format_str.begin(), format_str.end(), out);
        }
    }

    static std::string format(const std::string& format_str) {
        return format_str;
    }
};

} // namespace utility_module