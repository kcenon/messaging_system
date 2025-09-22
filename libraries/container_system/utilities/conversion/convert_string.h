#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <optional>

// Full compatibility layer for Container module's expected convert_string API
namespace utility_module {

// Complete result wrapper that acts as a transparent proxy for the wrapped type
template<typename T>
class result_wrapper {
private:
    T value_;
    bool has_error_;

public:
    result_wrapper(T value, bool error = false) : value_(std::move(value)), has_error_(error) {}

    // std::optional-like interface
    bool has_value() const { return !has_error_; }
    const T& value() const { return value_; }
    T& value() { return value_; }
    operator bool() const { return !has_error_; }

    // Direct access operators for transparent usage
    const T& operator*() const { return value_; }
    T& operator*() { return value_; }
    const T* operator->() const { return &value_; }
    T* operator->() { return &value_; }

    // Conversion operators for direct assignment
    operator const T&() const { return value_; }
    operator T&() { return value_; }

    // Forward common methods to the wrapped type using SFINAE
    // Use concepts-like approach with enable_if for better type safety
    template<typename U = T, typename... Args>
    auto data(Args&&... args) const
        -> typename std::enable_if_t<
            !std::is_arithmetic_v<U> &&
            std::is_same_v<U, T>,
            decltype(std::declval<const U&>().data(std::forward<Args>(args)...))
        > {
        return value_.data(std::forward<Args>(args)...);
    }

    template<typename U = T, typename... Args>
    auto size(Args&&... args) const
        -> typename std::enable_if_t<
            !std::is_arithmetic_v<U> &&
            std::is_same_v<U, T>,
            decltype(std::declval<const U&>().size(std::forward<Args>(args)...))
        > {
        return value_.size(std::forward<Args>(args)...);
    }

    template<typename U = T, typename... Args>
    auto empty(Args&&... args) const
        -> typename std::enable_if_t<
            !std::is_arithmetic_v<U> &&
            std::is_same_v<U, T>,
            decltype(std::declval<const U&>().empty(std::forward<Args>(args)...))
        > {
        return value_.empty(std::forward<Args>(args)...);
    }

    template<typename U = T, typename... Args>
    auto begin(Args&&... args) const
        -> typename std::enable_if_t<
            !std::is_arithmetic_v<U> &&
            std::is_same_v<U, T>,
            decltype(std::declval<const U&>().begin(std::forward<Args>(args)...))
        > {
        return value_.begin(std::forward<Args>(args)...);
    }

    template<typename U = T, typename... Args>
    auto end(Args&&... args) const
        -> typename std::enable_if_t<
            !std::is_arithmetic_v<U> &&
            std::is_same_v<U, T>,
            decltype(std::declval<const U&>().end(std::forward<Args>(args)...))
        > {
        return value_.end(std::forward<Args>(args)...);
    }

    // Indexing operator
    template<typename U = T, typename Index>
    auto operator[](Index&& index) const
        -> typename std::enable_if_t<
            !std::is_arithmetic_v<U> &&
            std::is_same_v<U, T>,
            decltype(std::declval<const U&>()[std::forward<Index>(index)])
        > {
        return value_[std::forward<Index>(index)];
    }

    template<typename U = T, typename Index>
    auto operator[](Index&& index)
        -> typename std::enable_if_t<
            !std::is_arithmetic_v<U> &&
            std::is_same_v<U, T>,
            decltype(std::declval<U&>()[std::forward<Index>(index)])
        > {
        return value_[std::forward<Index>(index)];
    }
};

class convert_string {
public:
    // Container expects structured binding with result types that have .has_value() and .value()
    static std::tuple<result_wrapper<std::string>, result_wrapper<bool>> to_string(const std::vector<uint8_t>& data) {
        try {
            std::string result;
            result.reserve(data.size());
            for (auto byte : data) {
                result += static_cast<char>(byte);
            }
            return std::make_tuple(
                result_wrapper<std::string>(result, false),
                result_wrapper<bool>(false, false)
            );
        } catch (...) {
            return std::make_tuple(
                result_wrapper<std::string>("", true),
                result_wrapper<bool>(true, false)
            );
        }
    }

    // Generic template for other types
    template<typename T>
    static std::tuple<result_wrapper<std::string>, result_wrapper<bool>> to_string(const T& value) {
        try {
            std::ostringstream oss;
            oss << value;
            return std::make_tuple(
                result_wrapper<std::string>(oss.str(), false),
                result_wrapper<bool>(false, false)
            );
        } catch (...) {
            return std::make_tuple(
                result_wrapper<std::string>("", true),
                result_wrapper<bool>(true, false)
            );
        }
    }

    // Base64 encoding
    static std::tuple<result_wrapper<std::string>, result_wrapper<bool>> to_base64(const std::vector<uint8_t>& data) {
        try {
            const char base64_chars[] =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

            std::string result;
            int val = 0;
            int valb = -6;

            for (uint8_t c : data) {
                val = (val << 8) + c;
                valb += 8;
                while (valb >= 0) {
                    result.push_back(base64_chars[(val >> valb) & 0x3F]);
                    valb -= 6;
                }
            }

            if (valb > -6) {
                result.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
            }

            while (result.size() % 4) {
                result.push_back('=');
            }

            return std::make_tuple(
                result_wrapper<std::string>(result, false),
                result_wrapper<bool>(false, false)
            );
        } catch (...) {
            return std::make_tuple(
                result_wrapper<std::string>("", true),
                result_wrapper<bool>(true, false)
            );
        }
    }

    // Base64 decoding
    static std::tuple<result_wrapper<std::vector<uint8_t>>, result_wrapper<bool>> from_base64(const std::string& encoded) {
        try {
            std::vector<uint8_t> result;

            auto is_base64 = [](unsigned char c) {
                return (isalnum(c) || (c == '+') || (c == '/'));
            };

            int in_len = encoded.size();
            int i = 0;
            int in = 0;
            unsigned char char_array_4[4], char_array_3[3];

            while (in_len-- && (encoded[in] != '=') && is_base64(encoded[in])) {
                char_array_4[i++] = encoded[in]; in++;
                if (i == 4) {
                    for (i = 0; i < 4; i++) {
                        if (char_array_4[i] >= 'A' && char_array_4[i] <= 'Z')
                            char_array_4[i] = char_array_4[i] - 'A';
                        else if (char_array_4[i] >= 'a' && char_array_4[i] <= 'z')
                            char_array_4[i] = char_array_4[i] - 'a' + 26;
                        else if (char_array_4[i] >= '0' && char_array_4[i] <= '9')
                            char_array_4[i] = char_array_4[i] - '0' + 52;
                        else if (char_array_4[i] == '+')
                            char_array_4[i] = 62;
                        else if (char_array_4[i] == '/')
                            char_array_4[i] = 63;
                    }

                    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                    for (i = 0; (i < 3); i++)
                        result.push_back(char_array_3[i]);
                    i = 0;
                }
            }

            if (i) {
                for (int j = i; j < 4; j++)
                    char_array_4[j] = 0;

                for (int j = 0; j < 4; j++) {
                    if (char_array_4[j] >= 'A' && char_array_4[j] <= 'Z')
                        char_array_4[j] = char_array_4[j] - 'A';
                    else if (char_array_4[j] >= 'a' && char_array_4[j] <= 'z')
                        char_array_4[j] = char_array_4[j] - 'a' + 26;
                    else if (char_array_4[j] >= '0' && char_array_4[j] <= '9')
                        char_array_4[j] = char_array_4[j] - '0' + 52;
                    else if (char_array_4[j] == '+')
                        char_array_4[j] = 62;
                    else if (char_array_4[j] == '/')
                        char_array_4[j] = 63;
                }

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (int j = 0; (j < i - 1); j++) result.push_back(char_array_3[j]);
            }

            return std::make_tuple(
                result_wrapper<std::vector<uint8_t>>(result, false),
                result_wrapper<bool>(false, false)
            );
        } catch (...) {
            return std::make_tuple(
                result_wrapper<std::vector<uint8_t>>(std::vector<uint8_t>(), true),
                result_wrapper<bool>(true, false)
            );
        }
    }

    // String replacement
    static void replace(std::string& str, const std::string& from, const std::string& to) {
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    }

    // UTF-8 to system encoding conversion
    static std::tuple<result_wrapper<std::string>, result_wrapper<bool>> utf8_to_system(const std::string& utf8_str) {
        try {
            // On most modern systems, UTF-8 is the system encoding
            // For compatibility, we return the string as-is
            return std::make_tuple(
                result_wrapper<std::string>(utf8_str, false),
                result_wrapper<bool>(false, false)
            );
        } catch (...) {
            return std::make_tuple(
                result_wrapper<std::string>("", true),
                result_wrapper<bool>(true, false)
            );
        }
    }

    // Array conversion
    static std::tuple<result_wrapper<std::vector<uint8_t>>, result_wrapper<bool>> to_array(const std::string& str) {
        try {
            std::vector<uint8_t> result;
            result.reserve(str.size());
            for (char c : str) {
                result.push_back(static_cast<uint8_t>(c));
            }
            return std::make_tuple(
                result_wrapper<std::vector<uint8_t>>(result, false),
                result_wrapper<bool>(false, false)
            );
        } catch (...) {
            return std::make_tuple(
                result_wrapper<std::vector<uint8_t>>(std::vector<uint8_t>(), true),
                result_wrapper<bool>(true, false)
            );
        }
    }
};

} // namespace utility_module