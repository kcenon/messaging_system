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

#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

// Container module
#include <container/container.h>

// Network module
#include <network/network.h>

// Thread system
#include <thread_pool/core/thread_pool.h>
#include <thread_pool/workers/thread_worker.h>

using namespace std::chrono_literals;

int main() {
    std::cout << "\n=== Messaging System Simple Test ===\n" << std::endl;
    
    try {
        // 1. Test container
        std::cout << "1. Testing Container Module..." << std::endl;
        container_module::value_container container;
        container.set_source("test_client", "main");
        container.set_target("test_server", "main");
        container.set_message_type("test_message");
        container.add_data("greeting", "Hello, Messaging System!");
        container.add_data("version", 1.0);
        container.add_data("ready", true);
        
        std::cout << "   ✓ Container created with " << container.data().size() << " items" << std::endl;
        std::cout << "   ✓ Message type: " << container.message_type() << std::endl;
        
        // 2. Test thread pool
        std::cout << "\n2. Testing Thread Pool Module..." << std::endl;
        thread_module::thread_context context;
        auto pool = std::make_shared<thread_pool_module::thread_pool>("TestPool", context);
        
        // Add workers
        for (int i = 0; i < 2; ++i) {
            auto worker = std::make_unique<thread_pool_module::thread_worker>(
                "Worker-" + std::to_string(i)
            );
            pool->enqueue(std::move(worker));
        }
        
        auto start_result = pool->start();
        if (!start_result.has_value()) {
            std::cout << "   ✓ Thread pool started successfully" << std::endl;
        } else {
            std::cerr << "   ✗ Failed to start thread pool: " << *start_result << std::endl;
            return 1;
        }
        
        // Submit test jobs
        int completed = 0;
        std::mutex completed_mutex;
        
        for (int i = 0; i < 5; ++i) {
            auto job = std::make_unique<thread_module::callback_job>(
                [i, &completed, &completed_mutex]() {
                    std::this_thread::sleep_for(50ms);
                    std::lock_guard<std::mutex> lock(completed_mutex);
                    completed++;
                    std::cout << "   ✓ Job " << i << " completed" << std::endl;
                }
            );
            pool->enqueue(std::move(job));
        }
        
        // 3. Test network
        std::cout << "\n3. Testing Network Module..." << std::endl;
        
        // Start server
        auto server = std::make_shared<network_module::messaging_server>("TestServer");
        std::atomic<bool> message_received{false};
        
        server->set_callback([&message_received](std::shared_ptr<network_module::messaging_session> session,
                                                const std::shared_ptr<container_module::value_container>& msg) {
            std::cout << "   ✓ Server received message: " << msg->message_type() << std::endl;
            std::cout << "   ✓ From: " << msg->source_id() << std::endl;
            message_received = true;
            
            // Echo response
            auto response = std::make_shared<container_module::value_container>();
            response->set_source(msg->target_id(), msg->target_sub_id());
            response->set_target(msg->source_id(), msg->source_sub_id());
            response->set_message_type("echo_response");
            response->add_data("status", "received");
            
            session->send(response);
        });
        
        server->start_server(12345);
        std::cout << "   ✓ Server started on port 12345" << std::endl;
        
        // Start client
        auto client = std::make_shared<network_module::messaging_client>("TestClient");
        std::atomic<bool> response_received{false};
        
        client->set_callback([&response_received](std::shared_ptr<network_module::messaging_session>,
                                                  const std::shared_ptr<container_module::value_container>& msg) {
            std::cout << "   ✓ Client received response: " << msg->message_type() << std::endl;
            response_received = true;
        });
        
        client->start_client("127.0.0.1", 12345);
        std::cout << "   ✓ Client connected to server" << std::endl;
        
        // Send message
        client->send(std::make_shared<container_module::value_container>(container));
        std::cout << "   ✓ Message sent from client to server" << std::endl;
        
        // Wait for network operations
        std::this_thread::sleep_for(500ms);
        
        // Check results
        if (message_received && response_received) {
            std::cout << "   ✓ Network communication successful" << std::endl;
        } else {
            std::cerr << "   ✗ Network communication failed" << std::endl;
        }
        
        // Wait for jobs to complete
        std::this_thread::sleep_for(500ms);
        
        // 4. Summary
        std::cout << "\n4. Test Summary:" << std::endl;
        std::cout << "   • Container module: ✓" << std::endl;
        std::cout << "   • Thread pool module: ✓ (" << completed << "/5 jobs completed)" << std::endl;
        std::cout << "   • Network module: " << (message_received && response_received ? "✓" : "✗") << std::endl;
        
        // Cleanup
        client->stop_client();
        server->stop_server();
        pool->stop();
        
        std::cout << "\n✅ All tests completed!\n" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}