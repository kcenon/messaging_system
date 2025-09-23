/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2021, 🍀☀🌕🌥 🌊
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

/**
 * @file messaging_integration_example.cpp
 * @brief Example demonstrating the new messaging integration features
 *
 * This example shows how to use the enhanced container system with
 * messaging-specific optimizations, performance monitoring, and
 * external system integration capabilities.
 */

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

// Include the container system with new integration features
#include "../container.h"

using namespace container_module;

#ifdef HAS_MESSAGING_FEATURES
using namespace container_module::integration;
#endif

void demonstrate_basic_usage() {
    std::cout << "\n=== Basic Container Usage ===\n";

    // Traditional way
    auto container = std::make_shared<value_container>();
    container->set_source("client_01", "session_123");
    container->set_target("server", "main_handler");
    container->set_message_type("user_data");

    // Add some values
    container->add(std::make_shared<long_value>("user_id", 12345));
    container->add(std::make_shared<string_value>("username", "john_doe"));
    container->add(std::make_shared<double_value>("balance", 1500.75));
    container->add(std::make_shared<bool_value>("active", true));

    // Count values - we know we added 4 values
    std::cout << "Created container with 4 values\n";
    std::cout << "Message type: " << container->message_type() << "\n";
    std::cout << "Source: " << container->source_id() << ":" << container->source_sub_id() << "\n";
    std::cout << "Target: " << container->target_id() << ":" << container->target_sub_id() << "\n";

    // Serialize
    std::string serialized = container->serialize();
    std::cout << "Serialized size: " << serialized.size() << " bytes\n";
}

#ifdef HAS_MESSAGING_FEATURES
void demonstrate_enhanced_features() {
    std::cout << "\n=== Enhanced Messaging Features ===\n";

    // Enhanced container creation
    auto container = messaging_integration::create_optimized_container("enhanced_message");
    std::cout << "Created optimized container for messaging\n";

    // Builder pattern usage
    auto built_container = messaging_container_builder()
        .source("enhanced_client", "session_456")
        .target("enhanced_server", "processing_unit")
        .message_type("enhanced_data")
        .add_value("request_id", 789)
        .add_value("priority", 1)
        .add_value("payload", std::string("Important data"))
        .add_value("timestamp", 1672531200L)
        .optimize_for_speed()
        .build();

    std::cout << "Built container using builder pattern\n";
    std::cout << "Message type: " << built_container->message_type() << "\n";
    // Builder added multiple values
    std::cout << "Values count: [multiple]\n";

    // Enhanced serialization
    {
        CONTAINER_PERF_MONITOR("enhanced_serialization");
        std::string serialized = messaging_integration::serialize_for_messaging(built_container);
        CONTAINER_PERF_SET_SIZE(4); // We added 4 values
        CONTAINER_PERF_SET_RESULT(serialized.size());

        std::cout << "Enhanced serialization completed\n";
        std::cout << "Serialized size: " << serialized.size() << " bytes\n";

        // Enhanced deserialization
        auto deserialized = messaging_integration::deserialize_from_messaging(serialized);
        if (deserialized) {
            std::cout << "Enhanced deserialization successful\n";
            std::cout << "Restored message type: " << deserialized->message_type() << "\n";
        }
    }
}
#endif

#ifdef HAS_PERFORMANCE_METRICS
void demonstrate_performance_monitoring() {
    std::cout << "\n=== Performance Monitoring ===\n";

    // Reset metrics for clean demonstration
    messaging_integration::reset_metrics();

    // Perform some operations to generate metrics
    for (int i = 0; i < 10; ++i) {
        auto container = messaging_integration::create_optimized_container("perf_test");

        auto built = messaging_container_builder()
            .message_type("performance_test")
            .add_value("iteration", i)
            .add_value("data", std::string(100, 'x'))  // 100-character string
            .build();

        std::string serialized = messaging_integration::serialize_for_messaging(built);
        auto deserialized = messaging_integration::deserialize_from_messaging(serialized);
    }

    // Display metrics
    std::cout << messaging_integration::get_metrics_summary();

    auto& metrics = messaging_integration::get_metrics();
    std::cout << "Total operations tracked: "
              << (metrics.containers_created.load() +
                  metrics.serializations_performed.load() +
                  metrics.deserializations_performed.load()) << "\n";
}
#endif

#ifdef HAS_EXTERNAL_INTEGRATION
void demonstrate_external_callbacks() {
    std::cout << "\n=== External System Integration ===\n";

    // Register callbacks for container operations
    messaging_integration::register_creation_callback([](const auto& container) {
        std::cout << "Callback: Container created with type '"
                  << container->message_type() << "'\n";
    });

    messaging_integration::register_serialization_callback(
        [](const std::shared_ptr<value_container>& container) {
            std::cout << "Callback: Container serialized\n";
        });

    // Create and serialize a container to trigger callbacks
    auto container = messaging_integration::create_optimized_container("callback_test");
    messaging_integration::serialize_for_messaging(container);

    // Clean up callbacks
    messaging_integration::unregister_callbacks();
    std::cout << "Callbacks unregistered\n";
}
#endif

void demonstrate_compatibility() {
    std::cout << "\n=== Messaging System Compatibility ===\n";

    // Show that the container system can be used with both aliases
    std::cout << "This container system provides compatibility aliases:\n";
    std::cout << "- ContainerSystem::container (standalone usage)\n";
    std::cout << "- MessagingSystem::container (messaging system integration)\n\n";

    // Demonstrate that the same container can be used in different contexts
    auto container = std::make_shared<value_container>();
    container->set_message_type("compatibility_test");

    std::cout << "Container can be used standalone or as part of messaging system\n";
    std::cout << "Type safety and performance remain consistent across usage patterns\n";
}

void performance_comparison() {
    std::cout << "\n=== Performance Comparison ===\n";

    const int iterations = 1000;

    // Standard serialization
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        auto container = std::make_shared<value_container>();
        container->set_message_type("standard_test");
        std::string serialized = container->serialize();
    }
    auto standard_time = std::chrono::high_resolution_clock::now() - start;

#ifdef HAS_MESSAGING_FEATURES
    // Enhanced serialization
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        auto container = messaging_integration::create_optimized_container("enhanced_test");
        std::string serialized = messaging_integration::serialize_for_messaging(container);
    }
    auto enhanced_time = std::chrono::high_resolution_clock::now() - start;

    auto standard_ms = std::chrono::duration_cast<std::chrono::milliseconds>(standard_time).count();
    auto enhanced_ms = std::chrono::duration_cast<std::chrono::milliseconds>(enhanced_time).count();

    std::cout << "Standard approach: " << standard_ms << " ms for " << iterations << " operations\n";
    std::cout << "Enhanced approach: " << enhanced_ms << " ms for " << iterations << " operations\n";

    if (enhanced_ms > 0) {
        double improvement = static_cast<double>(standard_ms) / enhanced_ms;
        std::cout << "Performance factor: " << std::fixed << improvement << "x\n";
    }
#else
    auto standard_ms = std::chrono::duration_cast<std::chrono::milliseconds>(standard_time).count();
    std::cout << "Standard approach: " << standard_ms << " ms for " << iterations << " operations\n";
    std::cout << "Enhanced features not enabled in this build\n";
#endif
}

int main() {
    std::cout << "Container System - Messaging Integration Example\n";
    std::cout << "================================================\n";

    std::cout << "Build configuration:\n";
#ifdef HAS_MESSAGING_FEATURES
    std::cout << "- Messaging Features: ENABLED\n";
#else
    std::cout << "- Messaging Features: DISABLED\n";
#endif

#ifdef HAS_PERFORMANCE_METRICS
    std::cout << "- Performance Metrics: ENABLED\n";
#else
    std::cout << "- Performance Metrics: DISABLED\n";
#endif

#ifdef HAS_EXTERNAL_INTEGRATION
    std::cout << "- External Integration: ENABLED\n";
#else
    std::cout << "- External Integration: DISABLED\n";
#endif

    try {
        // Demonstrate various features
        demonstrate_basic_usage();

#ifdef HAS_MESSAGING_FEATURES
        demonstrate_enhanced_features();
#endif

#ifdef HAS_PERFORMANCE_METRICS
        demonstrate_performance_monitoring();
#endif

#ifdef HAS_EXTERNAL_INTEGRATION
        demonstrate_external_callbacks();
#endif

        demonstrate_compatibility();
        performance_comparison();

        std::cout << "\n=== Example Completed Successfully ===\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}