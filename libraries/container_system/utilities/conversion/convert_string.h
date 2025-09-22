#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <cstdint>
#include <type_traits>

namespace utilities {
namespace conversion {

/**
 * @brief String conversion utilities
 */
class convert_string {
public:
    // Convert to string
    template<typename T>
    static std::string to_string(const T& value) {
        if constexpr (std::is_arithmetic_v<T>) {
            return std::to_string(value);
        } else {
            std::ostringstream oss;
            oss << value;
            return oss.str();
        }
    }

    // Specialization for bool
    static std::string to_string(bool value) {
        return value ? "true" : "false";
    }

    // Specialization for string (no conversion needed)
    static const std::string& to_string(const std::string& value) {
        return value;
    }

    // Convert from string
    template<typename T>
    static T from_string(const std::string& str) {
        if constexpr (std::is_same_v<T, std::string>) {
            return str;
        } else if constexpr (std::is_same_v<T, int>) {
            return std::stoi(str);
        } else if constexpr (std::is_same_v<T, long>) {
            return std::stol(str);
        } else if constexpr (std::is_same_v<T, long long>) {
            return std::stoll(str);
        } else if constexpr (std::is_same_v<T, float>) {
            return std::stof(str);
        } else if constexpr (std::is_same_v<T, double>) {
            return std::stod(str);
        } else if constexpr (std::is_same_v<T, bool>) {
            return str == "true" || str == "1" || str == "yes";
        } else {
            std::istringstream iss(str);
            T value;
            iss >> value;
            return value;
        }
    }

    // Convert bytes to hex string
    static std::string bytes_to_hex(const std::vector<uint8_t>& bytes) {
        std::ostringstream oss;
        oss << std::hex;
        for (uint8_t byte : bytes) {
            oss << static_cast<int>(byte);
        }
        return oss.str();
    }

    // Convert hex string to bytes
    static std::vector<uint8_t> hex_to_bytes(const std::string& hex) {
        std::vector<uint8_t> bytes;
        bytes.reserve(hex.length() / 2);

        for (size_t i = 0; i < hex.length(); i += 2) {
            std::string byte_str = hex.substr(i, 2);
            uint8_t byte = static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16));
            bytes.push_back(byte);
        }

        return bytes;
    }

    // Trim whitespace
    static std::string trim(const std::string& str) {
        size_t start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";

        size_t end = str.find_last_not_of(" \t\r\n");
        return str.substr(start, end - start + 1);
    }

    // Split string by delimiter
    static std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::istringstream iss(str);
        std::string token;

        while (std::getline(iss, token, delimiter)) {
            tokens.push_back(token);
        }

        return tokens;
    }

    // Join strings with delimiter
    static std::string join(const std::vector<std::string>& strings, const std::string& delimiter) {
        if (strings.empty()) return "";

        std::ostringstream oss;
        oss << strings[0];

        for (size_t i = 1; i < strings.size(); ++i) {
            oss << delimiter << strings[i];
        }

        return oss.str();
    }
};

} // namespace conversion
} // namespace utilities