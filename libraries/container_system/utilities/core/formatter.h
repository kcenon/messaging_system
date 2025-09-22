#pragma once

#include <string>
#include <sstream>
#include <memory>

#ifdef USE_STD_FORMAT
    #include <format>
#else
    // Use fmt if available, otherwise fallback to basic implementation
    #ifdef FMT_VERSION
        #include <fmt/format.h>
    #endif
#endif

// Complete formatter implementation for Container module compatibility
namespace utility_module {

class formatter {
public:
    // Basic format function for strings
    template<typename... Args>
    static std::string format(const std::string& format_str, Args&&... args) {
        try {
#ifdef USE_STD_FORMAT
            return std::vformat(format_str, std::make_format_args(args...));
#elif defined(FMT_VERSION)
            return fmt::vformat(format_str, fmt::make_format_args(args...));
#else
            // Fallback: simple string substitution for basic cases
            return basic_format(format_str, std::forward<Args>(args)...);
#endif
        } catch (const std::exception&) {
            return format_str; // Return original string if formatting fails
        }
    }

    // Simple format without arguments
    static std::string format(const std::string& format_str) {
        return format_str;
    }

    // Format to output iterator
    template<typename OutputIt, typename... Args>
    static void format_to(OutputIt out, const std::string& format_str, Args&&... args) {
        try {
#ifdef USE_STD_FORMAT
            std::vformat_to(out, format_str, std::make_format_args(args...));
#elif defined(FMT_VERSION)
            fmt::vformat_to(out, format_str, fmt::make_format_args(args...));
#else
            auto result = basic_format(format_str, std::forward<Args>(args)...);
            std::copy(result.begin(), result.end(), out);
#endif
        } catch (const std::exception&) {
            // Fallback: just copy the format string
            std::copy(format_str.begin(), format_str.end(), out);
        }
    }

private:
    // Basic format implementation as fallback
    template<typename T>
    static std::string basic_format(const std::string& format_str, T&& value) {
        std::string result = format_str;

        // Simple substitution for {} placeholder
        size_t pos = result.find("{}");
        if (pos != std::string::npos) {
            std::ostringstream oss;
            oss << value;
            result.replace(pos, 2, oss.str());
        }

        return result;
    }

    template<typename T, typename... Args>
    static std::string basic_format(const std::string& format_str, T&& first, Args&&... rest) {
        auto result = basic_format(format_str, std::forward<T>(first));
        return basic_format(result, std::forward<Args>(rest)...);
    }

    // Base case for recursion
    static std::string basic_format(const std::string& format_str) {
        return format_str;
    }
};

} // namespace utility_module