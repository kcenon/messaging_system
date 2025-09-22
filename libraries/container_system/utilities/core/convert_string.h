#pragma once

#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace utility_module {

/**
 * @brief String conversion utilities
 */
class convert_string {
public:
    /**
     * @brief Convert string to byte array
     */
    static std::pair<std::vector<uint8_t>, std::string> to_array(const std::string& str) {
        try {
            std::vector<uint8_t> result(str.begin(), str.end());
            return {result, ""};
        } catch (const std::exception& e) {
            return {{}, e.what()};
        }
    }

    /**
     * @brief Convert byte array to string
     */
    static std::pair<std::string, std::string> to_string(const std::vector<uint8_t>& arr) {
        try {
            std::string result(arr.begin(), arr.end());
            return {result, ""};
        } catch (const std::exception& e) {
            return {"", e.what()};
        }
    }

    /**
     * @brief Convert data to base64
     */
    static std::pair<std::string, std::string> to_base64(const std::vector<uint8_t>& data) {
        try {
            const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            std::string result;
            int val = 0;
            int valb = -6;

            for (uint8_t c : data) {
                val = (val << 8) + c;
                valb += 8;
                while (valb >= 0) {
                    result.push_back(chars[(val >> valb) & 0x3F]);
                    valb -= 6;
                }
            }

            if (valb > -6) {
                result.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
            }

            while (result.size() % 4) {
                result.push_back('=');
            }

            return {result, ""};
        } catch (const std::exception& e) {
            return {"", e.what()};
        }
    }

    /**
     * @brief Convert from base64
     */
    static std::pair<std::vector<uint8_t>, std::string> from_base64(const std::string& str) {
        try {
            const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            std::vector<uint8_t> result;
            int val = 0;
            int valb = -8;

            for (char c : str) {
                if (c == '=') break;
                size_t pos = chars.find(c);
                if (pos == std::string::npos) continue;

                val = (val << 6) + pos;
                valb += 6;
                if (valb >= 0) {
                    result.push_back((val >> valb) & 0xFF);
                    valb -= 8;
                }
            }

            return {result, ""};
        } catch (const std::exception& e) {
            return {{}, e.what()};
        }
    }

    /**
     * @brief Replace all occurrences of a substring
     */
    static void replace(std::string& str, const std::string& from, const std::string& to) {
        if (from.empty()) return;

        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    }

    /**
     * @brief Convert string to string (identity function for compatibility)
     */
    static std::pair<std::string, std::string> to_string(const std::string& str) {
        return {str, ""};
    }
};

} // namespace utility_module