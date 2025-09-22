/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <kcenon/logger/writers/encrypted_writer.h>
#include <random>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>

namespace kcenon::logger {

encrypted_writer::encrypted_writer(std::unique_ptr<base_writer> wrapped_writer,
                                 const std::vector<uint8_t>& key,
                                 encryption_type type)
    : wrapped_writer_(std::move(wrapped_writer))
    , key_(key)
    , type_(type)
    , counter_(0) {
    
    if (key_.size() != 32) {
        throw std::invalid_argument("Encryption key must be 32 bytes for AES-256");
    }
    
    // Generate initial IV
    iv_ = generate_key(16);  // 16 bytes for AES block size
}

encrypted_writer::~encrypted_writer() = default;

result_void encrypted_writer::write(logger_system::log_level level,
                                    const std::string& message,
                                    const std::string& file,
                                    int line,
                                    const std::string& function,
                                    const std::chrono::system_clock::time_point& timestamp) {
    
    // Format the log entry
    std::string formatted = format_log_entry(level, message, file, line, function, timestamp);
    
    // Encrypt the formatted log
    std::string encrypted;
    try {
        encrypted = encrypt_data(formatted);
    } catch (const std::exception& e) {
        return make_logger_error(logger_error_code::encryption_failed, e.what());
    }
    
    // Write encrypted data as hex string (for demo purposes)
    // In production, write binary data with proper framing
    std::ostringstream hex_stream;
    hex_stream << std::hex << std::setfill('0');
    for (unsigned char c : encrypted) {
        hex_stream << std::setw(2) << static_cast<int>(c);
    }
    
    // Pass encrypted data to wrapped writer
    return wrapped_writer_->write(level, 
                                 "ENCRYPTED:" + hex_stream.str(),
                                 "", 0, "", timestamp);
}

result_void encrypted_writer::flush() {
    return wrapped_writer_->flush();
}

std::vector<uint8_t> encrypted_writer::generate_key(size_t size) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    std::vector<uint8_t> key(size);
    for (size_t i = 0; i < size; ++i) {
        key[i] = static_cast<uint8_t>(dis(gen));
    }
    
    return key;
}

bool encrypted_writer::save_key(const std::vector<uint8_t>& key, const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        return false;
    }
    
    // Write key with simple obfuscation (XOR with fixed pattern)
    // In production, use proper key storage (e.g., OS keychain)
    std::vector<uint8_t> obfuscated = key;
    const uint8_t pattern[] = {0xDE, 0xAD, 0xBE, 0xEF};
    for (size_t i = 0; i < obfuscated.size(); ++i) {
        obfuscated[i] ^= pattern[i % sizeof(pattern)];
    }
    
    file.write(reinterpret_cast<const char*>(obfuscated.data()), obfuscated.size());
    return file.good();
}

std::vector<uint8_t> encrypted_writer::load_key(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        return {};
    }
    
    // Get file size
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // Read obfuscated key
    std::vector<uint8_t> obfuscated(size);
    file.read(reinterpret_cast<char*>(obfuscated.data()), size);
    
    if (!file) {
        return {};
    }
    
    // De-obfuscate
    const uint8_t pattern[] = {0xDE, 0xAD, 0xBE, 0xEF};
    for (size_t i = 0; i < obfuscated.size(); ++i) {
        obfuscated[i] ^= pattern[i % sizeof(pattern)];
    }
    
    return obfuscated;
}

std::string encrypted_writer::encrypt_data(const std::string& plaintext) {
    // DEMO IMPLEMENTATION - NOT CRYPTOGRAPHICALLY SECURE!
    // In production, use OpenSSL, libsodium, or similar
    
    // Different behavior based on encryption type
    if (type_ == encryption_type::none) {
        return plaintext;  // No encryption
    }
    
    // Convert string to bytes
    std::vector<uint8_t> data(plaintext.begin(), plaintext.end());
    
    // Add simple padding
    size_t pad_len = 16 - (data.size() % 16);
    data.resize(data.size() + pad_len, static_cast<uint8_t>(pad_len));
    
    // Update IV with counter
    counter_++;
    for (size_t i = 0; i < 8 && i < iv_.size(); ++i) {
        iv_[i] = (counter_ >> (i * 8)) & 0xFF;
    }
    
    // Prepend IV to data
    std::vector<uint8_t> result = iv_;
    result.insert(result.end(), data.begin(), data.end());
    
    // XOR encrypt (DEMO ONLY) - for both aes_256_cbc and chacha20_poly1305 in demo
    xor_encrypt(result, key_);
    
    return std::string(result.begin(), result.end());
}

std::string encrypted_writer::decrypt_data(const std::string& ciphertext) {
    // DEMO IMPLEMENTATION - matches encrypt_data
    std::vector<uint8_t> data(ciphertext.begin(), ciphertext.end());
    
    // XOR decrypt
    xor_encrypt(data, key_);
    
    // Extract IV (first 16 bytes)
    if (data.size() < 16) {
        return "";
    }
    
    // Remove IV and padding
    data.erase(data.begin(), data.begin() + 16);
    if (!data.empty()) {
        uint8_t pad_len = data.back();
        if (pad_len <= 16 && pad_len <= data.size()) {
            data.resize(data.size() - pad_len);
        }
    }
    
    return std::string(data.begin(), data.end());
}

void encrypted_writer::xor_encrypt(std::vector<uint8_t>& data, const std::vector<uint8_t>& key) {
    // Simple XOR encryption (DEMO ONLY - NOT SECURE!)
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] ^= key[i % key.size()];
    }
}

} // namespace kcenon::logger