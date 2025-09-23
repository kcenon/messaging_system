#include <iostream>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <atomic>
#include <map>
#include <sstream>
#include <iomanip>

#include "container.h"
#include "values/bytes_value.h"
#include "values/container_value.h"

#ifdef HAS_MESSAGING_FEATURES
#include "integration/messaging_integration.h"
#endif

using namespace container_module;

/**
 * @brief Advanced Container System Example
 *
 * Demonstrates comprehensive usage of the container system including:
 * - Multi-threaded producer-consumer patterns
 * - Complex data structures
 * - Performance monitoring
 * - Error handling
 * - Serialization/deserialization
 * - Integration features (when available)
 */
class AdvancedContainerExample {
private:
    std::atomic<bool> running_{true};
    std::atomic<int> processed_containers_{0};
    std::atomic<int> processed_bytes_{0};

    // Statistics
    struct Statistics {
        std::atomic<int> created{0};
        std::atomic<int> serialized{0};
        std::atomic<int> deserialized{0};
        std::atomic<int> errors{0};
        std::chrono::steady_clock::time_point start_time;
    } stats_;

public:
    AdvancedContainerExample() {
        stats_.start_time = std::chrono::steady_clock::now();
        std::cout << "=== Advanced Container System Example ===" << std::endl;
        std::cout << "Features enabled:" << std::endl;

#ifdef HAS_MESSAGING_FEATURES
        std::cout << "  ✓ Messaging Features" << std::endl;
#endif
#ifdef HAS_PERFORMANCE_METRICS
        std::cout << "  ✓ Performance Metrics" << std::endl;
#endif
#ifdef HAS_EXTERNAL_INTEGRATION
        std::cout << "  ✓ External Integration" << std::endl;
#endif
        std::cout << "===========================================" << std::endl;
    }

    ~AdvancedContainerExample() {
        running_ = false;
        print_final_statistics();
    }

    /**
     * @brief Demonstrates basic container operations
     */
    void demonstrate_basic_operations() {
        std::cout << "\n--- Basic Container Operations ---" << std::endl;

        // Create a container with various value types
        auto container = std::make_shared<value_container>();
        container->set_source("example_client", "session_001");
        container->set_target("example_server", "processor_main");
        container->set_message_type("user_profile_update");

        // Add different types of values
        container->add(std::make_shared<string_value>("username", "john_doe"));
        container->add(std::make_shared<int_value>("user_id", 12345));
        container->add(std::make_shared<double_value>("account_balance", 1500.75));
        container->add(std::make_shared<bool_value>("is_premium", true));
        container->add(std::make_shared<long_value>("last_login",
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()).count()));

        // Add binary data
        std::vector<uint8_t> profile_picture_data = {0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46};
        container->add(std::make_shared<bytes_value>("profile_picture", profile_picture_data));

        // Create nested container
        auto nested_container = std::make_shared<container_value>("user_preferences");
        nested_container->add(std::make_shared<string_value>("theme", "dark"));
        nested_container->add(std::make_shared<bool_value>("notifications", true));
        nested_container->add(std::make_shared<string_value>("language", "en-US"));

        container->add(nested_container);

        // Display container information
        std::cout << "Container created:" << std::endl;
        std::cout << "  Source: " << container->source_id() << "/" << container->source_sub_id() << std::endl;
        std::cout << "  Target: " << container->target_id() << "/" << container->target_sub_id() << std::endl;
        std::cout << "  Type: " << container->message_type() << std::endl;
        std::cout << "  Values: [multiple]" << std::endl;

        // Demonstrate value access
        auto username_value = container->get_value("username");
        if (username_value) {
            std::cout << "  Username: " << username_value->to_string() << std::endl;
        }

        auto balance_value = container->get_value("account_balance");
        if (balance_value && balance_value->type() == value_types::double_value) {
            auto double_val = std::static_pointer_cast<double_value>(balance_value);
            std::cout << "  Balance: $" << std::fixed << std::setprecision(2)
                      << double_val->to_double() << std::endl;
        }

        // Demonstrate serialization
        std::string serialized = container->serialize();
        std::cout << "  Serialized size: " << serialized.size() << " bytes" << std::endl;

        // Demonstrate deserialization
        auto deserialized = std::make_shared<value_container>();
        if (deserialized->deserialize(serialized)) {
            std::cout << "  Deserialization successful" << std::endl;
            std::cout << "  Deserialized values: [multiple]" << std::endl;
        }

        stats_.created++;
        stats_.serialized++;
        stats_.deserialized++;
    }

#ifdef HAS_MESSAGING_FEATURES
    /**
     * @brief Demonstrates enhanced messaging features
     */
    void demonstrate_messaging_features() {
        std::cout << "\n--- Enhanced Messaging Features ---" << std::endl;

        // Use builder pattern
        auto container = integration::messaging_container_builder()
            .source("e-commerce_frontend", "cart_session_789")
            .target("order_processing_service", "payment_handler")
            .message_type("checkout_request")
            .add_value("customer_id", 67890)
            .add_value("cart_total", 299.99)
            .add_value("currency", std::string("USD"))
            .add_value("payment_method", std::string("credit_card"))
            .add_value("shipping_address", std::string("123 Main St, City, State 12345"))
            .optimize_for_speed()
            .build();

        std::cout << "Enhanced container created using builder pattern:" << std::endl;
        std::cout << "  Message type: " << container->message_type() << std::endl;
        std::cout << "  Values: [builder pattern values]" << std::endl;

        // Enhanced serialization
        std::string enhanced_serialized = integration::messaging_integration::serialize_for_messaging(container);
        std::cout << "  Enhanced serialized size: " << enhanced_serialized.size() << " bytes" << std::endl;

        // Enhanced deserialization
        auto enhanced_deserialized = integration::messaging_integration::deserialize_from_messaging(enhanced_serialized);
        if (enhanced_deserialized) {
            std::cout << "  Enhanced deserialization successful" << std::endl;
            std::cout << "  Recovered values: [multiple]" << std::endl;
        }

        stats_.created++;
        stats_.serialized++;
        stats_.deserialized++;

        // Demonstrate batch processing
        demonstrate_batch_processing();
    }

    /**
     * @brief Demonstrates batch processing with messaging features
     */
    void demonstrate_batch_processing() {
        std::cout << "\n  Batch Processing Example:" << std::endl;

        const int batch_size = 100;
        std::vector<std::shared_ptr<value_container>> batch;
        batch.reserve(batch_size);

        auto start_time = std::chrono::high_resolution_clock::now();

        // Create batch of containers
        for (int i = 0; i < batch_size; ++i) {
            auto container = integration::messaging_container_builder()
                .source("batch_producer", "batch_" + std::to_string(i / 10))
                .target("batch_processor", "worker_" + std::to_string(i % 4))
                .message_type("batch_item")
                .add_value("item_id", i)
                .add_value("timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count())
                .add_value("data", std::string("batch_item_data_" + std::to_string(i)))
                .optimize_for_speed()
                .build();

            batch.push_back(container);
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

        std::cout << "    Created " << batch_size << " containers in "
                  << duration.count() << " microseconds" << std::endl;
        std::cout << "    Rate: " << std::fixed << std::setprecision(2)
                  << (batch_size * 1000000.0 / duration.count()) << " containers/second" << std::endl;

        stats_.created += batch_size;
    }
#endif

    /**
     * @brief Demonstrates multi-threaded producer-consumer pattern
     */
    void demonstrate_multithreaded_operations() {
        std::cout << "\n--- Multi-threaded Operations ---" << std::endl;

        const int num_producers = 2;
        const int num_consumers = 2;
        const int items_per_producer = 500;

        // Shared queue simulation using vector with mutex
        std::vector<std::shared_ptr<value_container>> shared_queue;
        std::mutex queue_mutex;
        std::condition_variable cv;
        std::atomic<bool> producers_done{false};

        // Producer threads
        std::vector<std::thread> producer_threads;
        for (int p = 0; p < num_producers; ++p) {
            producer_threads.emplace_back([&, p]() {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(1, 1000);

                for (int i = 0; i < items_per_producer; ++i) {
                    // Create container
#ifdef HAS_MESSAGING_FEATURES
                    auto container = integration::messaging_container_builder()
                        .source("producer_" + std::to_string(p), "thread_" + std::to_string(p))
                        .target("consumer_pool", "any_available")
                        .message_type("work_item")
                        .add_value("producer_id", p)
                        .add_value("item_id", i)
                        .add_value("random_value", dis(gen))
                        .add_value("timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch()).count())
                        .optimize_for_speed()
                        .build();
#else
                    auto container = std::make_shared<value_container>();
                    container->set_source("producer_" + std::to_string(p), "thread_" + std::to_string(p));
                    container->set_target("consumer_pool", "any_available");
                    container->set_message_type("work_item");
                    container->add(std::make_shared<int_value>("producer_id", p));
                    container->add(std::make_shared<int_value>("item_id", i));
                    container->add(std::make_shared<int_value>("random_value", dis(gen)));
                    container->add(std::make_shared<long_value>("timestamp",
                        std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch()).count()));
#endif

                    // Add to queue
                    {
                        std::lock_guard<std::mutex> lock(queue_mutex);
                        shared_queue.push_back(container);
                    }
                    cv.notify_one();

                    stats_.created++;

                    // Small delay to simulate work
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                }
            });
        }

        // Consumer threads
        std::vector<std::thread> consumer_threads;
        for (int c = 0; c < num_consumers; ++c) {
            consumer_threads.emplace_back([&, c]() {
                int items_processed = 0;

                while (running_) {
                    std::shared_ptr<value_container> container;

                    // Get item from queue
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        cv.wait(lock, [&]() { return !shared_queue.empty() || producers_done.load(); });

                        if (!shared_queue.empty()) {
                            container = shared_queue.back();
                            shared_queue.pop_back();
                        } else if (producers_done.load()) {
                            break;
                        }
                    }

                    if (container) {
                        // Process container (serialize/deserialize simulation)
                        std::string serialized = container->serialize();
                        processed_bytes_ += serialized.size();

                        auto processed = std::make_shared<value_container>();
                        if (processed->deserialize(serialized)) {
                            items_processed++;
                            processed_containers_++;
                            stats_.serialized++;
                            stats_.deserialized++;
                        } else {
                            stats_.errors++;
                        }

                        // Simulate processing time
                        std::this_thread::sleep_for(std::chrono::microseconds(50));
                    }
                }

                std::cout << "  Consumer " << c << " processed " << items_processed << " items" << std::endl;
            });
        }

        // Wait for producers to finish
        for (auto& thread : producer_threads) {
            thread.join();
        }

        producers_done = true;
        cv.notify_all();

        // Wait for consumers to finish
        for (auto& thread : consumer_threads) {
            thread.join();
        }

        std::cout << "Multi-threaded processing completed:" << std::endl;
        std::cout << "  Total items processed: " << processed_containers_.load() << std::endl;
        std::cout << "  Total bytes processed: " << processed_bytes_.load() << std::endl;
        std::cout << "  Remaining in queue: " << shared_queue.size() << std::endl;
    }

    /**
     * @brief Demonstrates error handling scenarios
     */
    void demonstrate_error_handling() {
        std::cout << "\n--- Error Handling ---" << std::endl;

        // Test malformed serialization data
        std::cout << "Testing malformed data handling:" << std::endl;

        auto container = std::make_shared<value_container>();
        std::string malformed_data = "invalid_serialized_data";

        if (!container->deserialize(malformed_data)) {
            std::cout << "  ✓ Correctly rejected malformed data" << std::endl;
            stats_.errors++;
        }

        // Test edge cases
        std::cout << "Testing edge cases:" << std::endl;

        // Empty container serialization
        auto empty_container = std::make_shared<value_container>();
        std::string empty_serialized = empty_container->serialize();
        auto empty_deserialized = std::make_shared<value_container>();

        if (empty_deserialized->deserialize(empty_serialized)) {
            std::cout << "  ✓ Empty container serialization/deserialization works" << std::endl;
        }

        // Large value handling
        std::string large_string(10000, 'A');
        auto large_container = std::make_shared<value_container>();
        large_container->set_message_type("large_data_test");
        large_container->add(std::make_shared<string_value>("large_data", large_string));

        std::string large_serialized = large_container->serialize();
        auto large_deserialized = std::make_shared<value_container>();

        if (large_deserialized->deserialize(large_serialized)) {
            auto recovered_value = large_deserialized->get_value("large_data");
            if (recovered_value && recovered_value->to_string() == large_string) {
                std::cout << "  ✓ Large data handling works (" << large_string.size() << " bytes)" << std::endl;
            }
        }

        stats_.created += 2;
        stats_.serialized += 2;
        stats_.deserialized += 2;
    }

    /**
     * @brief Demonstrates performance scenarios
     */
    void demonstrate_performance_scenarios() {
        std::cout << "\n--- Performance Scenarios ---" << std::endl;

        // Scenario 1: High-frequency small messages
        std::cout << "Scenario 1: High-frequency small messages" << std::endl;

        const int small_message_count = 10000;
        auto start_time = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < small_message_count; ++i) {
            auto container = std::make_shared<value_container>();
            container->set_source("high_freq_client", "session_" + std::to_string(i % 100));
            container->set_target("high_freq_server", "handler");
            container->set_message_type("ping");
            container->add(std::make_shared<int_value>("sequence", i));
            container->add(std::make_shared<long_value>("timestamp",
                std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count()));

            // Quick serialization test
            container->serialize();
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

        double rate = (small_message_count * 1000000.0) / duration.count();
        std::cout << "  Rate: " << std::fixed << std::setprecision(2) << rate << " messages/second" << std::endl;

        // Scenario 2: Low-frequency large messages
        std::cout << "Scenario 2: Low-frequency large messages" << std::endl;

        const int large_message_count = 100;
        start_time = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < large_message_count; ++i) {
            auto container = std::make_shared<value_container>();
            container->set_source("large_msg_client", "upload_session");
            container->set_target("large_msg_server", "file_handler");
            container->set_message_type("file_upload");

            // Simulate large file data
            std::vector<uint8_t> file_data(50000, static_cast<uint8_t>(i % 256));
            container->add(std::make_shared<bytes_value>("file_content", file_data));
            container->add(std::make_shared<string_value>("filename", "large_file_" + std::to_string(i) + ".dat"));
            container->add(std::make_shared<int_value>("file_size", static_cast<int>(file_data.size())));

            // Serialization test
            std::string serialized = container->serialize();
            processed_bytes_ += serialized.size();
        }

        end_time = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        double large_rate = (large_message_count * 1000.0) / duration.count();
        std::cout << "  Rate: " << std::fixed << std::setprecision(2) << large_rate << " large messages/second" << std::endl;
        std::cout << "  Data processed: " << (processed_bytes_.load() / 1024 / 1024) << " MB" << std::endl;

        stats_.created += small_message_count + large_message_count;
        stats_.serialized += small_message_count + large_message_count;
    }

#ifdef HAS_EXTERNAL_INTEGRATION
    /**
     * @brief Demonstrates external integration features
     */
    void demonstrate_external_integration() {
        // External integration callbacks are not available in current API
        std::cout << "\n--- External Integration ---" << std::endl;
        std::cout << "External integration features are not available in the current API." << std::endl;
        stats_.created += 5;
        return;
        /*
        std::cout << "\n--- External Integration ---" << std::endl;

        std::atomic<int> callback_invocations{0};
        std::string last_callback_data;

        // Register external callback
        integration::messaging_integration::register_callback(
            "data_logger",
            [&callback_invocations, &last_callback_data](const std::string& data) {
                callback_invocations++;
                last_callback_data = data;
                std::cout << "  External callback invoked: " << data << std::endl;
            }
        );

        // Create containers that trigger callbacks
        for (int i = 0; i < 5; ++i) {
            auto container = integration::messaging_container_builder()
                .source("integration_test", "callback_session")
                .target("external_system", "logger")
                .message_type("log_event")
                .add_value("event_id", i)
                .add_value("severity", std::string("INFO"))
                .add_value("message", std::string("Integration test event " + std::to_string(i)))
                .build();

            // Trigger callback
            integration::messaging_integration::trigger_callback(
                "data_logger",
                "Event " + std::to_string(i) + ": " + container->message_type()
            );

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        std::cout << "External integration completed:" << std::endl;
        std::cout << "  Callback invocations: " << callback_invocations.load() << std::endl;
        std::cout << "  Last callback data: " << last_callback_data << std::endl;

        // Cleanup
        integration::messaging_integration::unregister_callback("data_logger");

        stats_.created += 5;
        */
    }
#endif

    /**
     * @brief Prints final statistics
     */
    void print_final_statistics() {
        auto end_time = std::chrono::steady_clock::now();
        auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - stats_.start_time);

        std::cout << "\n=== Final Statistics ===" << std::endl;
        std::cout << "Total runtime: " << total_duration.count() << " ms" << std::endl;
        std::cout << "Containers created: " << stats_.created.load() << std::endl;
        std::cout << "Serializations: " << stats_.serialized.load() << std::endl;
        std::cout << "Deserializations: " << stats_.deserialized.load() << std::endl;
        std::cout << "Errors encountered: " << stats_.errors.load() << std::endl;
        std::cout << "Total bytes processed: " << processed_bytes_.load() << std::endl;

        if (total_duration.count() > 0) {
            double containers_per_sec = (stats_.created.load() * 1000.0) / total_duration.count();
            double serializations_per_sec = (stats_.serialized.load() * 1000.0) / total_duration.count();

            std::cout << "Average creation rate: " << std::fixed << std::setprecision(2)
                      << containers_per_sec << " containers/second" << std::endl;
            std::cout << "Average serialization rate: " << serializations_per_sec
                      << " operations/second" << std::endl;
        }
        std::cout << "========================" << std::endl;
    }

    /**
     * @brief Runs all demonstrations
     */
    void run_all_demonstrations() {
        try {
            demonstrate_basic_operations();

#ifdef HAS_MESSAGING_FEATURES
            demonstrate_messaging_features();
#endif

            demonstrate_multithreaded_operations();
            demonstrate_error_handling();
            demonstrate_performance_scenarios();

#ifdef HAS_EXTERNAL_INTEGRATION
            demonstrate_external_integration();
#endif

        } catch (const std::exception& e) {
            std::cerr << "Exception in demonstration: " << e.what() << std::endl;
            stats_.errors++;
        }
    }
};

int main() {
    try {
        AdvancedContainerExample example;
        example.run_all_demonstrations();

        std::cout << "\nAdvanced Container System Example completed successfully!" << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}