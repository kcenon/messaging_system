#include <iostream>
#include <monitoring/core/result_types.h>
#include <monitoring/core/error_codes.h>
#include <monitoring/interfaces/monitoring_interface.h>

using namespace monitoring_system;

/**
 * @brief Example demonstrating Result pattern usage
 */

// Example function that may fail
result<double> divide(double a, double b) {
    if (b == 0) {
        return make_error<double>(
            monitoring_error_code::invalid_configuration,
            "Division by zero"
        );
    }
    return make_success<double>(a / b);
}

// Example function using result_void
result_void validate_range(double value, double min, double max) {
    if (value < min || value > max) {
        return result_void(
            monitoring_error_code::invalid_configuration,
            "Value out of range [" + std::to_string(min) + ", " + std::to_string(max) + "]"
        );
    }
    return result_void::success();
}

// Example using monadic operations
result<std::string> process_metric(double value) {
    // Chain operations using map and and_then
    return divide(100.0, value)
        .map([](double x) { return x * 2; })
        .and_then([](double x) {
            if (x > 50) {
                return make_success<std::string>("High value: " + std::to_string(x));
            }
            return make_success<std::string>("Normal value: " + std::to_string(x));
        });
}

int main() {
    std::cout << "=== Result Pattern Example ===" << std::endl << std::endl;
    
    // Example 1: Successful operation
    std::cout << "Example 1: Successful division" << std::endl;
    auto result1 = divide(10.0, 2.0);
    if (result1) {
        std::cout << "  Result: " << result1.value() << std::endl;
    } else {
        std::cout << "  Error: " << result1.get_error().message << std::endl;
    }
    std::cout << std::endl;
    
    // Example 2: Failed operation
    std::cout << "Example 2: Division by zero" << std::endl;
    auto result2 = divide(10.0, 0.0);
    if (result2) {
        std::cout << "  Result: " << result2.value() << std::endl;
    } else {
        std::cout << "  Error: " << result2.get_error().message << std::endl;
    }
    std::cout << std::endl;
    
    // Example 3: Using value_or
    std::cout << "Example 3: Using value_or with default" << std::endl;
    auto result3 = divide(5.0, 0.0);
    double value = result3.value_or(-1.0);
    std::cout << "  Value (with default): " << value << std::endl;
    std::cout << std::endl;
    
    // Example 4: result_void usage
    std::cout << "Example 4: Validation with result_void" << std::endl;
    auto validation1 = validate_range(50.0, 0.0, 100.0);
    if (validation1) {
        std::cout << "  Validation passed" << std::endl;
    } else {
        std::cout << "  Validation failed: " << validation1.get_error().message << std::endl;
    }
    
    auto validation2 = validate_range(150.0, 0.0, 100.0);
    if (validation2) {
        std::cout << "  Validation passed" << std::endl;
    } else {
        std::cout << "  Validation failed: " << validation2.get_error().message << std::endl;
    }
    std::cout << std::endl;
    
    // Example 5: Monadic operations
    std::cout << "Example 5: Chaining operations" << std::endl;
    auto result4 = process_metric(4.0);
    if (result4) {
        std::cout << "  " << result4.value() << std::endl;
    } else {
        std::cout << "  Error: " << result4.get_error().message << std::endl;
    }
    
    auto result5 = process_metric(1.0);
    if (result5) {
        std::cout << "  " << result5.value() << std::endl;
    } else {
        std::cout << "  Error: " << result5.get_error().message << std::endl;
    }
    std::cout << std::endl;
    
    // Example 6: Metrics snapshot
    std::cout << "Example 6: Metrics snapshot" << std::endl;
    metrics_snapshot snapshot;
    snapshot.add_metric("cpu_usage", 65.5);
    snapshot.add_metric("memory_usage", 4096.0);
    snapshot.add_metric("disk_io", 150.25);
    
    std::cout << "  Metrics collected: " << snapshot.metrics.size() << std::endl;
    
    if (auto cpu = snapshot.get_metric("cpu_usage")) {
        std::cout << "  CPU Usage: " << cpu.value() << "%" << std::endl;
    }
    
    if (auto mem = snapshot.get_metric("memory_usage")) {
        std::cout << "  Memory Usage: " << mem.value() << " MB" << std::endl;
    }
    std::cout << std::endl;
    
    // Example 7: Configuration validation
    std::cout << "Example 7: Configuration validation" << std::endl;
    monitoring_config config;
    config.history_size = 1000;
    config.collection_interval = std::chrono::milliseconds(100);
    config.buffer_size = 5000;
    
    auto config_result = config.validate();
    if (config_result) {
        std::cout << "  Configuration is valid" << std::endl;
        std::cout << "  - History size: " << config.history_size << std::endl;
        std::cout << "  - Collection interval: " << config.collection_interval.count() << "ms" << std::endl;
        std::cout << "  - Buffer size: " << config.buffer_size << std::endl;
    } else {
        std::cout << "  Configuration error: " << config_result.get_error().message << std::endl;
    }
    
    return 0;
}