#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "base_writer.h"
#include <memory>
#include <vector>
#include <cstdint>

namespace logger_module {

/**
 * @class encrypted_writer
 * @brief Writer that encrypts log data before passing to another writer
 */
class encrypted_writer : public base_writer {
public:
    /**
     * @brief Encryption algorithm type
     */
    enum class encryption_type {
        aes_256_cbc,    // AES-256 with CBC mode
        aes_256_gcm,    // AES-256 with GCM mode (authenticated)
        chacha20_poly1305  // ChaCha20-Poly1305 (authenticated)
    };
    
    /**
     * @brief Constructor
     * @param wrapped_writer The writer to wrap with encryption
     * @param key Encryption key (must be 32 bytes for AES-256)
     * @param type Encryption algorithm to use
     */
    encrypted_writer(std::unique_ptr<base_writer> wrapped_writer,
                    const std::vector<uint8_t>& key,
                    encryption_type type = encryption_type::aes_256_gcm);
    
    /**
     * @brief Destructor
     */
    ~encrypted_writer() override;
    
    /**
     * @brief Write encrypted log entry
     */
    bool write(thread_module::log_level level,
               const std::string& message,
               const std::string& file,
               int line,
               const std::string& function,
               const std::chrono::system_clock::time_point& timestamp) override;
    
    /**
     * @brief Flush wrapped writer
     */
    void flush() override;
    
    /**
     * @brief Get writer name
     */
    std::string get_name() const override { 
        return "encrypted_" + wrapped_writer_->get_name(); 
    }
    
    /**
     * @brief Generate random encryption key
     * @param size Key size in bytes (32 for AES-256)
     * @return Random key
     */
    static std::vector<uint8_t> generate_key(size_t size = 32);
    
    /**
     * @brief Save key to file (securely)
     * @param key The key to save
     * @param filename File path
     * @return true if saved successfully
     */
    static bool save_key(const std::vector<uint8_t>& key, const std::string& filename);
    
    /**
     * @brief Load key from file
     * @param filename File path
     * @return The loaded key (empty if failed)
     */
    static std::vector<uint8_t> load_key(const std::string& filename);
    
private:
    // Simple encryption implementation (for demo purposes)
    // In production, use a proper crypto library like OpenSSL or libsodium
    std::string encrypt_data(const std::string& plaintext);
    std::string decrypt_data(const std::string& ciphertext);
    
    // XOR-based encryption (DEMO ONLY - NOT SECURE!)
    void xor_encrypt(std::vector<uint8_t>& data, const std::vector<uint8_t>& key);
    
private:
    std::unique_ptr<base_writer> wrapped_writer_;
    std::vector<uint8_t> key_;
    encryption_type type_;
    std::vector<uint8_t> iv_;  // Initialization vector
    uint64_t counter_;  // For unique IV generation
};

} // namespace logger_module