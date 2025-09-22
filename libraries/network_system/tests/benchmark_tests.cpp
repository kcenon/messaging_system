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

#include <benchmark/benchmark.h>
#include <memory>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <random>
#include <sstream>

#include "../network.h"
#include <container.h>

using namespace network_module;
using namespace container_module;
using namespace std::chrono_literals;

// Global server for benchmarks
static std::shared_ptr<messaging_server> g_benchmark_server;
static unsigned short g_server_port = 0;

// Helper to find an available port
static unsigned short FindAvailablePort(unsigned short start = 6000) {
    for (unsigned short port = start; port < 65535; ++port) {
        try {
            asio::io_context io_context;
            asio::ip::tcp::acceptor acceptor(io_context);
            asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);
            acceptor.open(endpoint.protocol());
            acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
            acceptor.bind(endpoint);
            acceptor.close();
            return port;
        } catch (...) {
            // Port is in use, try next
        }
    }
    return 0;
}

// Setup and teardown for benchmarks
class NetworkBenchmarkFixture : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override {
        if (!g_benchmark_server && g_server_port == 0) {
            g_server_port = FindAvailablePort();
            if (g_server_port == 0) {
                state.SkipWithError("No available port found");
                return;
            }
            
            g_benchmark_server = std::make_shared<messaging_server>("benchmark_server");
            g_benchmark_server->start_server(g_server_port);
            
            // Give server time to start
            std::this_thread::sleep_for(100ms);
        }
    }
    
    void TearDown(const ::benchmark::State& state) override {
        // Server cleanup is done in main()
    }
};

// Connection benchmarks
BENCHMARK_DEFINE_F(NetworkBenchmarkFixture, BM_ClientConnection)(benchmark::State& state) {
    for (auto _ : state) {
        asio::io_context io_context;
        auto client = std::make_shared<messaging_client>(
            io_context, 
            "bench_client", 
            "bench_key"
        );
        
        auto connected = client->connect("127.0.0.1", g_server_port);
        benchmark::DoNotOptimize(connected);
        
        if (connected) {
            client->disconnect();
        }
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(NetworkBenchmarkFixture, BM_ClientConnection);

// Message creation benchmarks
static void BM_MessageCreation(benchmark::State& state) {
    const int field_count = state.range(0);
    
    for (auto _ : state) {
        auto message = std::make_shared<value_container>();
        
        for (int i = 0; i < field_count; ++i) {
            message->add_value(std::make_shared<string_value>(
                "field_" + std::to_string(i),
                "value_" + std::to_string(i)
            ));
        }
        
        benchmark::DoNotOptimize(message);
    }
    
    state.SetItemsProcessed(state.iterations() * field_count);
}
BENCHMARK(BM_MessageCreation)
    ->RangeMultiplier(10)
    ->Range(1, 1000);

// Message sending benchmarks
BENCHMARK_DEFINE_F(NetworkBenchmarkFixture, BM_MessageSending)(benchmark::State& state) {
    const int message_size = state.range(0);
    
    asio::io_context io_context;
    auto client = std::make_shared<messaging_client>(
        io_context, 
        "send_bench_client",
        "send_bench_key"
    );
    
    if (!client->connect("127.0.0.1", g_server_port)) {
        state.SkipWithError("Failed to connect to server");
        return;
    }
    
    // Pre-create message
    auto message = std::make_shared<value_container>();
    message->add_value(std::make_shared<string_value>("type", "benchmark"));
    
    // Add payload of specified size
    std::string payload(message_size, 'X');
    message->add_value(std::make_shared<string_value>("payload", payload));
    
    // Start io_context in background
    std::atomic<bool> stop_io{false};
    std::thread io_thread([&io_context, &stop_io]() {
        while (!stop_io) {
            io_context.run_one_for(10ms);
        }
    });
    
    for (auto _ : state) {
        auto sent = client->send(message);
        benchmark::DoNotOptimize(sent);
    }
    
    stop_io = true;
    io_thread.join();
    
    client->disconnect();
    
    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * message_size);
}
BENCHMARK_REGISTER_F(NetworkBenchmarkFixture, BM_MessageSending)
    ->RangeMultiplier(10)
    ->Range(100, 1000000);

// Round-trip latency benchmark
BENCHMARK_DEFINE_F(NetworkBenchmarkFixture, BM_RoundTripLatency)(benchmark::State& state) {
    asio::io_context io_context;
    auto client = std::make_shared<messaging_client>(
        io_context, 
        "latency_client",
        "latency_key"
    );
    
    if (!client->connect("127.0.0.1", g_server_port)) {
        state.SkipWithError("Failed to connect to server");
        return;
    }
    
    // Setup echo handler
    std::promise<void> response_promise;
    std::atomic<bool> waiting{false};
    
    client->set_message_handler([&](std::shared_ptr<value_container> msg) {
        if (waiting) {
            response_promise.set_value();
        }
    });
    
    // Start io_context in background
    std::atomic<bool> stop_io{false};
    std::thread io_thread([&io_context, &stop_io]() {
        while (!stop_io) {
            io_context.run_one_for(1ms);
        }
    });
    
    auto message = std::make_shared<value_container>();
    message->add_value(std::make_shared<string_value>("type", "echo"));
    message->add_value(std::make_shared<string_value>("data", "test"));
    
    for (auto _ : state) {
        response_promise = std::promise<void>();
        auto response_future = response_promise.get_future();
        
        auto start = std::chrono::high_resolution_clock::now();
        
        waiting = true;
        client->send(message);
        
        // Wait for response (with timeout)
        if (response_future.wait_for(100ms) == std::future_status::ready) {
            auto end = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
            state.SetIterationTime(elapsed.count() / 1e9);
        } else {
            state.SkipWithError("Timeout waiting for response");
            break;
        }
        
        waiting = false;
    }
    
    stop_io = true;
    io_thread.join();
    
    client->disconnect();
}
BENCHMARK_REGISTER_F(NetworkBenchmarkFixture, BM_RoundTripLatency)
    ->UseManualTime();

// Concurrent connections benchmark
static void BM_ConcurrentConnections(benchmark::State& state) {
    const int connection_count = state.range(0);
    
    for (auto _ : state) {
        std::vector<asio::io_context> io_contexts(connection_count);
        std::vector<std::shared_ptr<messaging_client>> clients;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Create and connect all clients
        for (int i = 0; i < connection_count; ++i) {
            auto client = std::make_shared<messaging_client>(
                io_contexts[i],
                "concurrent_" + std::to_string(i),
                "key_" + std::to_string(i)
            );
            
            if (client->connect("127.0.0.1", g_server_port)) {
                clients.push_back(client);
            }
        }
        
        // Disconnect all
        for (auto& client : clients) {
            client->disconnect();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        state.SetIterationTime(elapsed.count() / 1e6);
    }
    
    state.SetItemsProcessed(state.iterations() * connection_count);
}
BENCHMARK(BM_ConcurrentConnections)
    ->UseManualTime()
    ->RangeMultiplier(2)
    ->Range(1, 64);

// Throughput benchmark
BENCHMARK_DEFINE_F(NetworkBenchmarkFixture, BM_MessageThroughput)(benchmark::State& state) {
    const int batch_size = state.range(0);
    
    asio::io_context io_context;
    auto client = std::make_shared<messaging_client>(
        io_context, 
        "throughput_client",
        "throughput_key"
    );
    
    if (!client->connect("127.0.0.1", g_server_port)) {
        state.SkipWithError("Failed to connect to server");
        return;
    }
    
    // Pre-create messages
    std::vector<std::shared_ptr<value_container>> messages;
    for (int i = 0; i < batch_size; ++i) {
        auto message = std::make_shared<value_container>();
        message->add_value(std::make_shared<string_value>("type", "throughput"));
        message->add_value(std::make_shared<int32_value>("sequence", i));
        message->add_value(std::make_shared<string_value>("data", "payload_data"));
        messages.push_back(message);
    }
    
    // Start io_context in background
    std::atomic<bool> stop_io{false};
    std::thread io_thread([&io_context, &stop_io]() {
        while (!stop_io) {
            io_context.run_one_for(1ms);
        }
    });
    
    for (auto _ : state) {
        int sent_count = 0;
        
        for (const auto& message : messages) {
            if (client->send(message)) {
                sent_count++;
            }
        }
        
        benchmark::DoNotOptimize(sent_count);
    }
    
    stop_io = true;
    io_thread.join();
    
    client->disconnect();
    
    state.SetItemsProcessed(state.iterations() * batch_size);
}
BENCHMARK_REGISTER_F(NetworkBenchmarkFixture, BM_MessageThroughput)
    ->RangeMultiplier(10)
    ->Range(1, 1000);

// Message serialization benchmark
static void BM_MessageSerialization(benchmark::State& state) {
    const int field_count = state.range(0);
    
    // Create a complex message
    auto message = std::make_shared<value_container>();
    
    for (int i = 0; i < field_count; ++i) {
        message->add_value(std::make_shared<string_value>(
            "string_" + std::to_string(i),
            "value_" + std::to_string(i)
        ));
        message->add_value(std::make_shared<int32_value>(
            "int_" + std::to_string(i),
            i * 42
        ));
        message->add_value(std::make_shared<double_value>(
            "double_" + std::to_string(i),
            i * 3.14159
        ));
    }
    
    for (auto _ : state) {
        auto serialized = message->serialize();
        benchmark::DoNotOptimize(serialized);
    }
    
    state.SetItemsProcessed(state.iterations() * field_count * 3); // 3 fields per iteration
}
BENCHMARK(BM_MessageSerialization)
    ->RangeMultiplier(10)
    ->Range(1, 100);

// Message deserialization benchmark
static void BM_MessageDeserialization(benchmark::State& state) {
    const int field_count = state.range(0);
    
    // Create and serialize a message
    auto original = std::make_shared<value_container>();
    
    for (int i = 0; i < field_count; ++i) {
        original->add_value(std::make_shared<string_value>(
            "field_" + std::to_string(i),
            "value_" + std::to_string(i)
        ));
    }
    
    auto serialized = original->serialize();
    
    for (auto _ : state) {
        value_container deserialized;
        auto result = deserialized.deserialize(serialized);
        benchmark::DoNotOptimize(result);
    }
    
    state.SetItemsProcessed(state.iterations() * field_count);
    state.SetBytesProcessed(state.iterations() * serialized.size());
}
BENCHMARK(BM_MessageDeserialization)
    ->RangeMultiplier(10)
    ->Range(1, 100);

// Memory allocation benchmark
static void BM_ClientAllocation(benchmark::State& state) {
    for (auto _ : state) {
        asio::io_context io_context;
        auto client = std::make_shared<messaging_client>(
            io_context,
            "alloc_client",
            "alloc_key"
        );
        benchmark::DoNotOptimize(client);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_ClientAllocation);

// Multi-threaded stress test
static void BM_MultiThreadedStress(benchmark::State& state) {
    const int thread_count = state.range(0);
    const int messages_per_thread = 100;
    
    for (auto _ : state) {
        std::vector<std::thread> threads;
        std::atomic<int> total_sent{0};
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int t = 0; t < thread_count; ++t) {
            threads.emplace_back([&, t]() {
                asio::io_context io_context;
                auto client = std::make_shared<messaging_client>(
                    io_context,
                    "stress_" + std::to_string(t),
                    "key_" + std::to_string(t)
                );
                
                if (client->connect("127.0.0.1", g_server_port)) {
                    // Start io processing
                    std::thread io_thread([&io_context]() {
                        io_context.run_for(500ms);
                    });
                    
                    // Send messages
                    for (int i = 0; i < messages_per_thread; ++i) {
                        auto message = std::make_shared<value_container>();
                        message->add_value(std::make_shared<string_value>("thread", std::to_string(t)));
                        message->add_value(std::make_shared<int32_value>("msg", i));
                        
                        if (client->send(message)) {
                            total_sent++;
                        }
                    }
                    
                    io_thread.join();
                    client->disconnect();
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        state.SetIterationTime(elapsed.count() / 1e3);
        benchmark::DoNotOptimize(total_sent.load());
    }
    
    state.SetItemsProcessed(state.iterations() * thread_count * messages_per_thread);
}
BENCHMARK(BM_MultiThreadedStress)
    ->UseManualTime()
    ->RangeMultiplier(2)
    ->Range(1, 16);

// Compression efficiency benchmark (if compression is enabled)
static void BM_CompressionEfficiency(benchmark::State& state) {
    const int data_size = state.range(0);
    
    // Create a message with compressible data
    auto message = std::make_shared<value_container>();
    
    // Add repetitive data (highly compressible)
    std::string repetitive_data;
    for (int i = 0; i < data_size / 10; ++i) {
        repetitive_data += "COMPRESS_ME";
    }
    message->add_value(std::make_shared<string_value>("data", repetitive_data));
    
    asio::io_context io_context;
    auto client = std::make_shared<messaging_client>(
        io_context, 
        "compress_client",
        "compress_key"
    );
    
    if (!client->connect("127.0.0.1", g_server_port)) {
        state.SkipWithError("Failed to connect to server");
        return;
    }
    
    // Start io_context in background
    std::atomic<bool> stop_io{false};
    std::thread io_thread([&io_context, &stop_io]() {
        while (!stop_io) {
            io_context.run_one_for(1ms);
        }
    });
    
    for (auto _ : state) {
        auto sent = client->send(message);
        benchmark::DoNotOptimize(sent);
    }
    
    stop_io = true;
    io_thread.join();
    
    client->disconnect();
    
    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * data_size);
}
BENCHMARK(BM_CompressionEfficiency)
    ->RangeMultiplier(10)
    ->Range(1000, 100000);

int main(int argc, char** argv) {
    // Initialize benchmark
    ::benchmark::Initialize(&argc, argv);
    
    // Start global server for benchmarks
    g_server_port = FindAvailablePort();
    if (g_server_port == 0) {
        std::cerr << "Error: No available port found for benchmark server\n";
        return 1;
    }
    
    g_benchmark_server = std::make_shared<messaging_server>("benchmark_server");
    g_benchmark_server->start_server(g_server_port);
    
    // Give server time to start
    std::this_thread::sleep_for(200ms);
    
    std::cout << "Benchmark server started on port " << g_server_port << "\n";
    
    // Run benchmarks
    ::benchmark::RunSpecifiedBenchmarks();
    
    // Cleanup
    if (g_benchmark_server) {
        g_benchmark_server->stop_server();
        g_benchmark_server->wait_for_stop();
    }
    
    return 0;
}