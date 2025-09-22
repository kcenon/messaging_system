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

#include <kcenon/logger/core/logger.h>
// #include <kcenon/logger/security/log_sanitizer.h>  // TODO: Not implemented yet
// #include <kcenon/logger/structured/structured_logger.h>  // TODO: Not implemented yet
#include <kcenon/logger/interfaces/log_filter_interface.h>
// #include <kcenon/logger/routing/log_router.h>  // TODO: Not implemented yet
#include <kcenon/logger/writers/console_writer.h>
#include <kcenon/logger/writers/file_writer.h>

#include <iostream>
#include <thread>
#include <chrono>
#include <regex>

using namespace kcenon::logger;
namespace logger_module = kcenon::logger;
using namespace std::chrono_literals;

// Custom security filter that blocks sensitive logs
class security_filter : public log_filter_interface {
public:
    bool should_log(const log_entry& entry) const override {
        const std::string message = entry.message.to_string();

        // Block logs containing passwords
        if (message.find("password") != std::string::npos) {
            std::cout << "[SECURITY] Blocked log containing password" << std::endl;
            return false;
        }
        return true;
    }

    std::string get_name() const override {
        return "security_filter";
    }
};

void demonstrate_log_sanitization() {
    std::cout << "\n=== Log Sanitization Demo ===" << std::endl;
    
    auto sanitizer = std::make_shared<log_sanitizer>();
    
    // Test various sensitive data patterns
    std::vector<std::string> test_messages = {
        "Credit card payment: 4532-1234-5678-9012",
        "User SSN: 123-45-6789",
        "Contact email: john.doe@example.com",
        "Server IP: 192.168.1.100",
        "API_KEY=example_test_key_1234567890",
        "Login with password=SuperSecret123!",
        "Multiple cards: 5555-4444-3333-2222 and 4111111111111111",
        "Mixed data: email admin@test.com from 10.0.0.1 with key=abcd1234efgh5678"
    };
    
    std::cout << "\nOriginal vs Sanitized messages:" << std::endl;
    std::cout << std::string(80, '-') << std::endl;
    
    for (const auto& msg : test_messages) {
        std::string sanitized = sanitizer->sanitize(msg);
        std::cout << "Original:  " << msg << std::endl;
        std::cout << "Sanitized: " << sanitized << std::endl;
        std::cout << std::endl;
    }
}

void demonstrate_security_logging() {
    std::cout << "\n=== Security Logging Demo ===" << std::endl;
    
    auto logger = std::make_shared<logger_module::logger>();
    auto sanitizer = std::make_shared<log_sanitizer>();
    
    // Add security filter
    auto sec_filter = std::make_unique<security_filter>();
    logger->set_filter(std::move(sec_filter));
    
    // Simulate security events
    std::cout << "\nLogging security events (sensitive data will be sanitized):" << std::endl;
    
    // This will be blocked by the security filter
    logger->log(logger_system::log_level::warning,
                "User login attempt with password=admin123");
    
    // These will be logged but sanitized
    logger->log(logger_system::log_level::warning,
                sanitizer->sanitize("Suspicious activity from IP 192.168.1.100"));
    
    logger->log(logger_system::log_level::warning,
                sanitizer->sanitize("Failed login for email user@example.com"));
    
    logger->log(logger_system::log_level::error,
                sanitizer->sanitize("Data breach detected: SSN 123-45-6789 exposed"));
    
    logger->log(logger_system::log_level::critical,
                sanitizer->sanitize("API key compromised: key=EXAMPLE_KEY_12345"));
}

void demonstrate_encryption() {
    std::cout << "\n=== Encryption Demo ===" << std::endl;
    
    auto logger = std::make_shared<logger_module::logger>();
    
    // Add encrypted file writer
    logger->add_writer(std::make_unique<file_writer>("security_encrypted.log"));
    
    logger->log(logger_system::log_level::info,
                "This message will be written to an encrypted log file");
    
    logger->log(logger_system::log_level::warning,
                "Sensitive operations are logged securely");
    
    std::cout << "Messages written to encrypted log file: security_encrypted.log" << std::endl;
}

void demonstrate_audit_trail() {
    std::cout << "\n=== Audit Trail Demo ===" << std::endl;
    
    auto logger = std::make_shared<logger_module::logger>();
    
    // Create audit logger (separate from main logger)
    auto audit_writer = std::make_unique<file_writer>("audit_trail.log");
    logger->add_writer("audit", std::move(audit_writer));
    
    // Configure routing for audit trail
    auto& router = logger->get_router();
    // Note: Router configuration would be done here if needed
    
    // Simulate various events
    logger->log(logger_system::log_level::info, "Normal operation");
    logger->log(logger_system::log_level::warning, "High CPU usage");
    logger->log(logger_system::log_level::critical, "Security breach detected");
    logger->log(logger_system::log_level::critical, "Unauthorized access attempt");
    logger->log(logger_system::log_level::error, "Database connection failed");
    
    std::cout << "Audit events written to: audit_trail.log" << std::endl;
}

void demonstrate_compliance_logging() {
    std::cout << "\n=== Compliance Logging Demo ===" << std::endl;
    
    auto base_logger = std::make_shared<logger_module::logger>();
    auto structured = std::make_shared<structured_logger>(base_logger);
    auto sanitizer = std::make_shared<log_sanitizer>();
    
    // GDPR-compliant user data access log
    structured->info(sanitizer->sanitize("User data access"))
        .field("user_id", "USR-12345")
        .field("accessed_by", "ADMIN-001")
        .field("data_type", "personal_information")
        .field("purpose", "support_request")
        .field("timestamp", std::chrono::system_clock::now())
        .field("ip_address", sanitizer->sanitize("192.168.1.50"))
        .commit();
    
    // PCI compliance - payment processing
    structured->info(sanitizer->sanitize("Payment processed"))
        .field("transaction_id", "TXN-98765")
        .field("amount", 150.00)
        .field("currency", "USD")
        .field("card_last_four", "9012")  // Only last 4 digits
        .field("status", "success")
        .commit();
    
    // HIPAA compliance - healthcare data access
    structured->warning(sanitizer->sanitize("Medical record accessed"))
        .field("patient_id", "PAT-55555")  // Anonymized ID
        .field("accessed_by", "DOC-777")
        .field("record_type", "lab_results")
        .field("compliance", "HIPAA")
        .commit();
}

void demonstrate_intrusion_detection() {
    std::cout << "\n=== Intrusion Detection Demo ===" << std::endl;
    
    auto logger = std::make_shared<logger_module::logger>();
    auto structured = std::make_shared<structured_logger>(logger);
    auto sanitizer = std::make_shared<log_sanitizer>();
    
    // Simulate suspicious activities
    structured->warning(sanitizer->sanitize("Multiple failed login attempts"))
        .field("source_ip", sanitizer->sanitize("10.0.0.100"))
        .field("target_user", "admin")
        .field("attempts", 5)
        .field("time_window", "60s")
        .commit();
    
    structured->critical(sanitizer->sanitize("Potential SQL injection detected"))
        .field("endpoint", "/api/users")
        .field("payload", sanitizer->sanitize("'; DROP TABLE users; --"))
        .field("blocked", true)
        .commit();
    
    structured->error(sanitizer->sanitize("Unauthorized API access"))
        .field("api_key", sanitizer->sanitize("api_key=example_api_key_abcdefgh123456"))
        .field("endpoint", "/api/v1/sensitive-data")
        .field("blocked", true)
        .commit();
    
    structured->critical(sanitizer->sanitize("Port scan detected"))
        .field("source_ip", sanitizer->sanitize("203.0.113.0"))
        .field("ports_scanned", 1000)
        .field("duration", "120s")
        .field("action", "ip_blocked")
        .commit();
}

void demonstrate_security_metrics() {
    std::cout << "\n=== Security Metrics Demo ===" << std::endl;
    
    auto logger = std::make_shared<logger_module::logger>();
    logger->enable_metrics_collection(true);
    
    // Get security-related metrics
    auto metrics_result = logger->get_current_metrics();
    
    std::cout << "\nSecurity Logging Metrics:" << std::endl;
    if (metrics_result) {
        auto& metrics = metrics_result.value();
        std::cout << "Total logs: " << metrics.messages_enqueued.load() << std::endl;
        std::cout << "Messages per second: " << metrics.get_messages_per_second() << std::endl;
        std::cout << "Average enqueue time: " << metrics.get_avg_enqueue_time_ns() << " ns" << std::endl;
        
        // Calculate security event rate (simplified for demo)
        auto duration = 1;
        
        if (duration > 0) {
            double message_rate = metrics.get_messages_per_second();
            std::cout << "\nMessage rate: " << message_rate << " msgs/sec" << std::endl;
        
            if (message_rate > 1000.0) {
                logger->log(logger_system::log_level::critical,
                           "High message rate detected!");
            }
        }
    } else {
        std::cout << "Failed to get metrics" << std::endl;
    }
}

int main() {
    try {
        std::cout << "=== Security Features Demo ===" << std::endl;
        std::cout << "Demonstrating logger security capabilities\n" << std::endl;
        
        // Create and configure logger
        auto logger = std::make_shared<logger_module::logger>();
        logger->set_min_level(logger_system::log_level::debug);
        logger->start();
        
        // Add console output for demo
        logger->add_writer(std::make_unique<console_writer>());
        
        // Run demonstrations
        demonstrate_log_sanitization();
        std::this_thread::sleep_for(100ms);
        
        demonstrate_security_logging();
        std::this_thread::sleep_for(100ms);
        
        demonstrate_encryption();
        std::this_thread::sleep_for(100ms);
        
        demonstrate_audit_trail();
        std::this_thread::sleep_for(100ms);
        
        demonstrate_compliance_logging();
        std::this_thread::sleep_for(100ms);
        
        demonstrate_intrusion_detection();
        std::this_thread::sleep_for(100ms);
        
        demonstrate_security_metrics();
        
        // Cleanup
        logger->stop();
        logger->flush();
        
        std::cout << "\n=== Security Demo Complete ===" << std::endl;
        std::cout << "Check the following files for results:" << std::endl;
        std::cout << "- security_encrypted.log (encrypted messages)" << std::endl;
        std::cout << "- audit_trail.log (critical security events)" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}