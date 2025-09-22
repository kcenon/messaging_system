/**
 * @file benchmark_tests.cpp
 * @brief Performance benchmarks for the container system
 * 
 * Measures performance characteristics including:
 * - Serialization/deserialization speed
 * - Memory usage patterns
 * - SIMD optimization effectiveness
 * - Thread scalability
 * - Type conversion overhead
 */

#include <benchmark/benchmark.h>
#include <container.h>
#include <values/bool_value.h>
#include <values/string_value.h>
#include <values/bytes_value.h>
#include <values/numeric_value.h>
#include <internal/thread_safe_container.h>
#include <thread>
#include <vector>
#include <random>
#include <sstream>
#include <cstring>

using namespace container_module;

// Helper function to generate random string
std::string generate_random_string(size_t length) {
    static const char charset[] = 
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);
    
    std::string result;
    result.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        result += charset[dist(rng)];
    }
    return result;
}

// ============================================================================
// Value Creation Benchmarks
// ============================================================================

static void BM_ValueCreation_Null(benchmark::State& state) {
    for (auto _ : state) {
        auto val = std::make_shared<value>("test", value_types::null_value, "");
        benchmark::DoNotOptimize(val);
    }
}
BENCHMARK(BM_ValueCreation_Null);

static void BM_ValueCreation_Bool(benchmark::State& state) {
    for (auto _ : state) {
        auto val = std::make_shared<bool_value>("test", true);
        benchmark::DoNotOptimize(val);
    }
}
BENCHMARK(BM_ValueCreation_Bool);

static void BM_ValueCreation_Int32(benchmark::State& state) {
    for (auto _ : state) {
        auto val = std::make_shared<int_value>("test", 42);
        benchmark::DoNotOptimize(val);
    }
}
BENCHMARK(BM_ValueCreation_Int32);

static void BM_ValueCreation_Double(benchmark::State& state) {
    for (auto _ : state) {
        auto val = std::make_shared<double_value>("test", 3.14159);
        benchmark::DoNotOptimize(val);
    }
}
BENCHMARK(BM_ValueCreation_Double);

static void BM_ValueCreation_String(benchmark::State& state) {
    std::string data = generate_random_string(state.range(0));
    for (auto _ : state) {
        auto val = std::make_shared<string_value>("test", data);
        benchmark::DoNotOptimize(val);
    }
    state.SetBytesProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_ValueCreation_String)->Range(8, 8<<10);

static void BM_ValueCreation_Bytes(benchmark::State& state) {
    std::vector<uint8_t> data(state.range(0), 0xFF);
    
    for (auto _ : state) {
        auto val = std::make_shared<bytes_value>("test", data);
        benchmark::DoNotOptimize(val);
    }
    state.SetBytesProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_ValueCreation_Bytes)->Range(8, 8<<10);

// ============================================================================
// Value Conversion Benchmarks
// ============================================================================

static void BM_ValueConversion_StringToInt(benchmark::State& state) {
    auto val = std::make_shared<string_value>("test", "12345");
    
    for (auto _ : state) {
        int result = val->to_int();
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_ValueConversion_StringToInt);

static void BM_ValueConversion_IntToString(benchmark::State& state) {
    auto val = std::make_shared<int_value>("test", 12345);
    
    for (auto _ : state) {
        std::string result = val->to_string();
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_ValueConversion_IntToString);

static void BM_ValueConversion_DoubleToString(benchmark::State& state) {
    auto val = std::make_shared<double_value>("test", 3.14159265358979);
    
    for (auto _ : state) {
        std::string result = val->to_string();
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_ValueConversion_DoubleToString);

// ============================================================================
// Container Operation Benchmarks
// ============================================================================

static void BM_ContainerCreation_Empty(benchmark::State& state) {
    for (auto _ : state) {
        auto container = std::make_unique<value_container>();
        benchmark::DoNotOptimize(container);
    }
}
BENCHMARK(BM_ContainerCreation_Empty);

static void BM_ContainerAddValue(benchmark::State& state) {
    auto container = std::make_unique<value_container>();
    auto val = std::make_shared<string_value>("test", "data");
    
    for (auto _ : state) {
        state.PauseTiming();
        // Reset container by removing all values
        while (container->get_value("test") && !container->get_value("test")->is_null()) {
            container->remove("test");
        }
        state.ResumeTiming();
        
        container->add(val);
    }
}
BENCHMARK(BM_ContainerAddValue);

static void BM_ContainerAddMultipleValues(benchmark::State& state) {
    auto container = std::make_unique<value_container>();
    std::vector<std::shared_ptr<value>> values;
    
    // Pre-create values
    for (int i = 0; i < state.range(0); ++i) {
        values.push_back(std::make_shared<string_value>(
            "key" + std::to_string(i), 
            "value" + std::to_string(i)
        ));
    }
    
    for (auto _ : state) {
        state.PauseTiming();
        // Clear container
        container = std::make_unique<value_container>();
        state.ResumeTiming();
        
        for (const auto& val : values) {
            container->add(val);
        }
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_ContainerAddMultipleValues)->Range(10, 1000);

static void BM_ContainerGetValue(benchmark::State& state) {
    auto container = std::make_unique<value_container>();
    
    // Add values
    for (int i = 0; i < state.range(0); ++i) {
        container->add(std::make_shared<string_value>(
            "key" + std::to_string(i), 
            "value" + std::to_string(i)
        ));
    }
    
    // Lookup middle value
    std::string lookup_key = "key" + std::to_string(state.range(0) / 2);
    
    for (auto _ : state) {
        auto val = container->get_value(lookup_key);
        benchmark::DoNotOptimize(val);
    }
}
BENCHMARK(BM_ContainerGetValue)->Range(10, 1000);

// ============================================================================
// Serialization Benchmarks
// ============================================================================

static void BM_ContainerSerialize(benchmark::State& state) {
    auto container = std::make_unique<value_container>();
    container->set_source("src", "sub");
    container->set_target("tgt", "sub2");
    container->set_message_type("benchmark");
    
    // Add values
    for (int i = 0; i < state.range(0); ++i) {
        container->add(std::make_shared<string_value>(
            "key" + std::to_string(i), 
            "value" + std::to_string(i)
        ));
    }
    
    for (auto _ : state) {
        std::string serialized = container->serialize();
        benchmark::DoNotOptimize(serialized);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_ContainerSerialize)->Range(1, 1000);

static void BM_ContainerDeserialize(benchmark::State& state) {
    auto container = std::make_unique<value_container>();
    container->set_source("src", "sub");
    container->set_target("tgt", "sub2");
    container->set_message_type("benchmark");
    
    // Add values
    for (int i = 0; i < state.range(0); ++i) {
        container->add(std::make_shared<string_value>(
            "key" + std::to_string(i), 
            "value" + std::to_string(i)
        ));
    }
    
    std::string serialized = container->serialize();
    
    for (auto _ : state) {
        auto new_container = std::make_unique<value_container>(serialized);
        benchmark::DoNotOptimize(new_container);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_ContainerDeserialize)->Range(1, 1000);

// ============================================================================
// Format Conversion Benchmarks
// ============================================================================

static void BM_ContainerToJSON(benchmark::State& state) {
    auto container = std::make_unique<value_container>();
    container->set_message_type("benchmark");
    
    // Add values
    for (int i = 0; i < state.range(0); ++i) {
        container->add(std::make_shared<string_value>(
            "key" + std::to_string(i), 
            "value" + std::to_string(i)
        ));
    }
    
    for (auto _ : state) {
        std::string json = container->to_json();
        benchmark::DoNotOptimize(json);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_ContainerToJSON)->Range(1, 100);

static void BM_ContainerToXML(benchmark::State& state) {
    auto container = std::make_unique<value_container>();
    container->set_message_type("benchmark");
    
    // Add values
    for (int i = 0; i < state.range(0); ++i) {
        container->add(std::make_shared<string_value>(
            "key" + std::to_string(i), 
            "value" + std::to_string(i)
        ));
    }
    
    for (auto _ : state) {
        std::string xml = container->to_xml();
        benchmark::DoNotOptimize(xml);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_ContainerToXML)->Range(1, 100);

// ============================================================================
// Large Data Benchmarks
// ============================================================================

static void BM_LargeStringHandling(benchmark::State& state) {
    std::string large_data = generate_random_string(state.range(0));
    
    for (auto _ : state) {
        auto container = std::make_unique<value_container>();
        container->add(std::make_shared<string_value>("large", large_data));
        
        std::string serialized = container->serialize();
        auto restored = std::make_unique<value_container>(serialized);
        
        benchmark::DoNotOptimize(restored);
    }
    state.SetBytesProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_LargeStringHandling)->Range(1024, 1024*1024);

static void BM_LargeBinaryHandling(benchmark::State& state) {
    std::vector<uint8_t> binary_data(state.range(0));
    std::generate(binary_data.begin(), binary_data.end(), std::rand);
    std::string data_str(binary_data.begin(), binary_data.end());
    
    for (auto _ : state) {
        auto container = std::make_unique<value_container>();
        container->add(std::make_shared<bytes_value>("binary", binary_data));
        
        std::string serialized = container->serialize();
        auto restored = std::make_unique<value_container>(serialized);
        
        benchmark::DoNotOptimize(restored);
    }
    state.SetBytesProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_LargeBinaryHandling)->Range(1024, 1024*1024);

// ============================================================================
// Thread Scalability Benchmarks
// ============================================================================

static void BM_ThreadSafeContainer_SingleThread(benchmark::State& state) {
    auto safe_container = std::make_shared<thread_safe_container>();
    
    for (auto _ : state) {
        // Add values
        for (int i = 0; i < 100; ++i) {
            std::string key = "key" + std::to_string(i);
            std::string value = "value" + std::to_string(i);
            safe_container->set(key, value);
        }
        
        // Read values
        for (int i = 0; i < 100; ++i) {
            std::string key = "key" + std::to_string(i);
            auto val = safe_container->get_typed<std::string>(key);
            benchmark::DoNotOptimize(val);
        }
        
        // Clear container
        safe_container->clear();
    }
    state.SetItemsProcessed(state.iterations() * 200); // 100 sets + 100 gets
}
BENCHMARK(BM_ThreadSafeContainer_SingleThread);

static void BM_ThreadSafeContainer_MultiThread(benchmark::State& state) {
    auto safe_container = std::make_shared<thread_safe_container>();
    
    const int num_threads = state.range(0);
    const int ops_per_thread = 100;
    
    for (auto _ : state) {
        std::vector<std::thread> threads;
        
        // Start writer threads
        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back([safe_container, t, ops_per_thread]() {
                for (int i = 0; i < ops_per_thread; ++i) {
                    std::string key = "thread" + std::to_string(t) + "_" + std::to_string(i);
                    int value = t * 1000 + i;
                    safe_container->set(key, value);
                }
            });
        }
        
        // Start reader threads
        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back([safe_container, t, ops_per_thread]() {
                for (int i = 0; i < ops_per_thread; ++i) {
                    std::string key = "thread" + std::to_string(t) + "_" + std::to_string(i);
                    auto val = safe_container->get_typed<int>(key);
                    benchmark::DoNotOptimize(val);
                }
            });
        }
        
        // Wait for all threads
        for (auto& t : threads) {
            t.join();
        }
        
        safe_container->clear();
    }
    state.SetItemsProcessed(state.iterations() * num_threads * ops_per_thread * 2);
}
BENCHMARK(BM_ThreadSafeContainer_MultiThread)->Range(1, 8);

// ============================================================================
// Memory Usage Patterns
// ============================================================================

static void BM_MemoryPattern_SmallValues(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<std::shared_ptr<value>> values;
        values.reserve(state.range(0));
        
        for (int i = 0; i < state.range(0); ++i) {
            values.push_back(std::make_shared<int_value>(
                "k", 1
            ));
        }
        
        benchmark::DoNotOptimize(values);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_MemoryPattern_SmallValues)->Range(100, 10000);

static void BM_MemoryPattern_LargeValues(benchmark::State& state) {
    std::string large_string(1024, 'X'); // 1KB string
    
    for (auto _ : state) {
        std::vector<std::shared_ptr<value>> values;
        values.reserve(state.range(0));
        
        for (int i = 0; i < state.range(0); ++i) {
            values.push_back(std::make_shared<string_value>(
                "key", large_string
            ));
        }
        
        benchmark::DoNotOptimize(values);
    }
    state.SetBytesProcessed(state.iterations() * state.range(0) * 1024);
}
BENCHMARK(BM_MemoryPattern_LargeValues)->Range(10, 1000);

// ============================================================================
// Nested Container Benchmarks
// ============================================================================

static void BM_NestedContainer_Create(benchmark::State& state) {
    const int depth = state.range(0);
    
    for (auto _ : state) {
        auto root = std::make_unique<value_container>();
        root->set_message_type("root");
        
        auto current = root.get();
        for (int i = 0; i < depth; ++i) {
            auto nested = std::make_unique<value_container>();
            nested->set_message_type("level_" + std::to_string(i));
            nested->add(std::make_shared<string_value>("data", "value"));
            
            // Serialize nested container
            std::string nested_data = nested->serialize();
            current->add(std::make_shared<value>("child", value_types::container_value, nested_data));
            
            if (i < depth - 1) {
                auto child_val = current->get_value("child");
                current = new value_container(child_val->data());
            }
        }
        
        benchmark::DoNotOptimize(root);
    }
}
BENCHMARK(BM_NestedContainer_Create)->Range(1, 10);

static void BM_NestedContainer_Serialize(benchmark::State& state) {
    const int depth = state.range(0);
    
    // Create nested structure
    auto root = std::make_unique<value_container>();
    root->set_message_type("root");
    
    std::function<void(value_container*, int)> create_nested = 
        [&](value_container* parent, int level) {
        if (level >= depth) return;
        
        auto nested = std::make_unique<value_container>();
        nested->set_message_type("level_" + std::to_string(level));
        nested->add(std::make_shared<string_value>("data", 
                                         "value_at_level_" + std::to_string(level)));
        
        // Serialize nested container and add as container value
        std::string nested_data = nested->serialize();
        parent->add(std::make_shared<value>("child", value_types::container_value, nested_data));
    };
    
    create_nested(root.get(), 0);
    
    for (auto _ : state) {
        std::string serialized = root->serialize();
        benchmark::DoNotOptimize(serialized);
    }
}
BENCHMARK(BM_NestedContainer_Serialize)->Range(1, 10);

// ============================================================================
// SIMD Optimization Benchmarks (if available)
// ============================================================================

static void BM_SIMD_StringSearch(benchmark::State& state) {
    auto container = std::make_unique<value_container>();
    
    // Add many string values
    for (int i = 0; i < 1000; ++i) {
        container->add(std::make_shared<string_value>(
            "key" + std::to_string(i), 
            generate_random_string(64)
        ));
    }
    
    // Search for values containing specific pattern
    for (auto _ : state) {
        int count = 0;
        auto values = container->value_array("key500");
        if (!values.empty()) {
            count++;
        }
        benchmark::DoNotOptimize(count);
    }
}
BENCHMARK(BM_SIMD_StringSearch);

// ============================================================================
// Worst Case Scenarios
// ============================================================================

static void BM_WorstCase_ManyDuplicateKeys(benchmark::State& state) {
    auto container = std::make_unique<value_container>();
    
    // Add many values with the same key
    for (int i = 0; i < state.range(0); ++i) {
        container->add(std::make_shared<string_value>(
            "duplicate_key", 
            "value_" + std::to_string(i)
        ));
    }
    
    for (auto _ : state) {
        auto values = container->value_array("duplicate_key");
        benchmark::DoNotOptimize(values);
    }
}
BENCHMARK(BM_WorstCase_ManyDuplicateKeys)->Range(10, 1000);

static void BM_WorstCase_DeepNesting(benchmark::State& state) {
    auto container = std::make_unique<value_container>();
    container->set_message_type("root");
    
    // Create deeply nested structure
    for (int i = 0; i < 100; ++i) {
        auto nested = std::make_unique<value_container>();
        nested->set_message_type("nested_" + std::to_string(i));
        
        for (int j = 0; j < 10; ++j) {
            nested->add(std::make_shared<string_value>(
                "data_" + std::to_string(j), 
                "value"
            ));
        }
        
        // Serialize nested container and add
        std::string nested_data = nested->serialize();
        container->add(std::make_shared<value>("container_" + std::to_string(i), value_types::container_value, nested_data));
    }
    
    for (auto _ : state) {
        std::string serialized = container->serialize();
        auto restored = std::make_unique<value_container>(serialized);
        benchmark::DoNotOptimize(restored);
    }
}
BENCHMARK(BM_WorstCase_DeepNesting);

// Main function
BENCHMARK_MAIN();