/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <logger/logger.h>
#include <logger/writers/console_writer.h>
#include <logger/writers/file_writer.h>
#include <logger/writers/encrypted_writer.h>
#include <logger/security/log_sanitizer.h>
#include <logger/structured/structured_logger.h>
#include <thread>
#include <iostream>
#include <fstream>

using namespace logger_module;

void test_encryption() {
    std::cout << "\n=== Testing Log Encryption ===\n" << std::endl;
    
    // Generate encryption key
    auto key = encrypted_writer::generate_key(32);
    
    // Save key to file
    if (encrypted_writer::save_key(key, "logger.key")) {
        std::cout << "Encryption key saved to logger.key" << std::endl;
    }
    
    // Create encrypted logger
    auto logger = std::make_shared<logger_module::logger>();
    
    // Add encrypted file writer
    auto file_writer = std::make_unique<logger_module::file_writer>("encrypted_logs.dat");
    auto encrypted = std::make_unique<encrypted_writer>(std::move(file_writer), key);
    logger->add_writer("encrypted", std::move(encrypted));
    
    // Log some messages
    logger->log(thread_module::log_level::info, "Starting secure application");
    logger->log(thread_module::log_level::warning, "User authentication required");
    logger->log(thread_module::log_level::error, "Failed login attempt from 192.168.1.100");
    
    logger->flush();
    
    std::cout << "Encrypted logs written to encrypted_logs.dat" << std::endl;
    std::cout << "Note: Logs are encrypted with XOR cipher (demo only)" << std::endl;
}

void test_sanitization() {
    std::cout << "\n=== Testing Log Sanitization ===\n" << std::endl;
    
    // Create logger with console output
    auto logger = std::make_shared<logger_module::logger>();
    logger->add_writer(std::make_unique<console_writer>());
    
    // Create sanitizer
    auto sanitizer = std::make_shared<log_sanitizer>();
    
    // Test various sensitive data patterns
    std::vector<std::string> test_messages = {
        "Credit card payment: 4532-1234-5678-9012",
        "User SSN: 123-45-6789",
        "Contact email: john.doe@example.com",
        "Server IP: 192.168.1.100",
        "API_KEY=sk_test_1234567890abcdefghijklmnop",
        "Login with password=SuperSecret123!",
        "Multiple cards: 5555-4444-3333-2222 and 4111111111111111",
        "Mixed data: email admin@test.com from 10.0.0.1 with key=abcd1234efgh5678"
    };
    
    std::cout << "\nOriginal vs Sanitized messages:\n" << std::endl;
    
    for (const auto& msg : test_messages) {
        std::string sanitized = sanitizer->sanitize(msg);
        std::cout << "Original:  " << msg << std::endl;
        std::cout << "Sanitized: " << sanitized << std::endl;
        std::cout << std::endl;
    }
}

void test_sanitizing_filter() {
    std::cout << "\n=== Testing Sanitizing Filter ===\n" << std::endl;
    
    // Create logger
    auto logger = std::make_shared<logger_module::logger>();
    logger->add_writer(std::make_unique<console_writer>());
    
    // Create and apply sanitizing filter
    auto sanitizer = std::make_shared<log_sanitizer>();
    auto filter = std::make_unique<sanitizing_filter>(sanitizer);
    logger->set_filter(std::move(filter));
    
    // Log messages with sensitive data
    std::cout << "\nLogging with automatic sanitization:\n" << std::endl;
    
    logger->log(thread_module::log_level::info, 
                "User login successful for email: alice@company.com");
    
    logger->log(thread_module::log_level::warning,
                "Payment failed for card 4532-1234-5678-9012");
    
    logger->log(thread_module::log_level::error,
                "API call failed with api_key=sk_live_[EXAMPLE_KEY_REDACTED]");
    
    // Note: The filter sanitizes before passing to logger,
    // but the logger interface doesn't support message modification
    // In a real implementation, you'd need a custom writer that uses
    // the sanitizer or modify the logger interface
}

void test_access_control() {
    std::cout << "\n=== Testing Access Control ===\n" << std::endl;
    
    // Create logger
    auto logger = std::make_shared<logger_module::logger>();
    logger->add_writer(std::make_unique<console_writer>());
    
    // Create access control filter
    auto access_filter = std::make_unique<access_control_filter>(
        access_control_filter::permission_level::write_info
    );
    
    // Set file-specific permissions
    access_filter->set_file_permission(".*secure.*", 
        access_control_filter::permission_level::admin);
    access_filter->set_file_permission(".*public.*", 
        access_control_filter::permission_level::write_all);
    
    logger->set_filter(std::move(access_filter));
    
    // Test with different user contexts
    std::cout << "\nTesting different user permissions:\n" << std::endl;
    
    // Note: Logger doesn't have get_filter() method, so we'll demonstrate the filter directly
    auto test_filter = std::make_unique<access_control_filter>(
        access_control_filter::permission_level::write_info
    );
    
    // Admin user
    test_filter->set_user_context("admin", access_control_filter::permission_level::admin);
    std::cout << "\nAdmin user:" << std::endl;
    if (test_filter->should_log(thread_module::log_level::debug, "Debug message", 
                               "secure_module.cpp", 10, "test")) {
        std::cout << "  [ALLOWED] Debug message from secure_module.cpp" << std::endl;
        logger->log(thread_module::log_level::debug, "Debug message", "secure_module.cpp", 10, "test");
    }
    if (test_filter->should_log(thread_module::log_level::error, "Error message",
                               "secure_module.cpp", 20, "test")) {
        std::cout << "  [ALLOWED] Error message from secure_module.cpp" << std::endl;
        logger->log(thread_module::log_level::error, "Error message", "secure_module.cpp", 20, "test");
    }
    
    // Regular user
    test_filter->set_user_context("user", access_control_filter::permission_level::write_info);
    std::cout << "\nRegular user (write_info permission):" << std::endl;
    if (test_filter->should_log(thread_module::log_level::info, "Info allowed",
                               "public_module.cpp", 30, "test")) {
        std::cout << "  [ALLOWED] Info message from public_module.cpp" << std::endl;
    }
    if (!test_filter->should_log(thread_module::log_level::debug, "Debug blocked",
                                "public_module.cpp", 40, "test")) {
        std::cout << "  [BLOCKED] Debug message from public_module.cpp" << std::endl;
    }
    if (!test_filter->should_log(thread_module::log_level::error, "Error blocked",
                                "secure_module.cpp", 50, "test")) {
        std::cout << "  [BLOCKED] Error message from secure_module.cpp (insufficient permission)" << std::endl;
    }
    
    // Read-only user
    test_filter->set_user_context("viewer", access_control_filter::permission_level::read_only);
    std::cout << "\nRead-only user:" << std::endl;
    if (!test_filter->should_log(thread_module::log_level::info, "This should be blocked",
                                "any_module.cpp", 60, "test")) {
        std::cout << "  [BLOCKED] All write operations blocked for read-only user" << std::endl;
    }
}

void test_combined_security() {
    std::cout << "\n=== Testing Combined Security Features ===\n" << std::endl;
    
    // Create logger with structured output
    auto logger = std::make_shared<logger_module::logger>();
    
    // Add console writer for demo
    logger->add_writer("console", std::make_unique<console_writer>());
    
    // Add encrypted file writer
    auto key = encrypted_writer::generate_key();
    auto secure_file = std::make_unique<file_writer>("secure_audit.log");
    auto encrypted = std::make_unique<encrypted_writer>(std::move(secure_file), key);
    logger->add_writer("secure", std::move(encrypted));
    
    // Create structured logger with sanitizer
    auto sanitizer = std::make_shared<log_sanitizer>();
    auto structured = std::make_shared<structured_logger>(
        logger, 
        structured_logger::output_format::json
    );
    
    // Log security events
    std::cout << "\nLogging security events with sanitization and encryption:\n" << std::endl;
    
    // Simulate authentication events
    structured->info(sanitizer->sanitize("User login attempt"))
        .field("user_email", sanitizer->sanitize("john.doe@company.com"))
        .field("source_ip", sanitizer->sanitize("192.168.1.100"))
        .field("timestamp", std::chrono::system_clock::now())
        .commit();
    
    // Simulate payment processing
    structured->warning(sanitizer->sanitize("Payment processing failed"))
        .field("card_number", sanitizer->sanitize("4532-1234-5678-9012"))
        .field("amount", 99.99)
        .field("error", "Insufficient funds")
        .commit();
    
    // Simulate API access
    structured->error(sanitizer->sanitize("Unauthorized API access"))
        .field("api_key", sanitizer->sanitize("api_key=sk_test_abcdefghijklmnop123456"))
        .field("endpoint", "/api/v1/sensitive-data")
        .field("blocked", true)
        .commit();
    
    logger->flush();
    
    std::cout << "\nSecure audit log written to secure_audit.log (encrypted)" << std::endl;
    std::cout << "Encryption key saved for this session" << std::endl;
}

int main() {
    std::cout << "Logger Security Features Demo" << std::endl;
    std::cout << "============================" << std::endl;
    
    // Test individual security features
    test_encryption();
    test_sanitization();
    test_sanitizing_filter();
    test_access_control();
    test_combined_security();
    
    std::cout << "\n=== Security Demo Complete ===" << std::endl;
    std::cout << "\nIMPORTANT NOTES:" << std::endl;
    std::cout << "1. The encryption uses XOR cipher for demo only - use proper crypto in production" << std::endl;
    std::cout << "2. Always store encryption keys securely (HSM, key vault, etc.)" << std::endl;
    std::cout << "3. Sanitization rules should be customized for your specific use case" << std::endl;
    std::cout << "4. Access control should integrate with your authentication system" << std::endl;
    
    return 0;
}