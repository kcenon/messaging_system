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

// Database module (optional)
#include <database/database_manager.h>

// Thread system
#include <thread_pool/core/thread_pool.h>
#include <thread_pool/workers/thread_worker.h>

// Logger system
#include <logger/logger.h>

// Monitoring system
#include <monitoring/monitoring.h>

using namespace std::chrono_literals;

int main() {
    std::cout << "\n=== Messaging System Integration Test ===\n" << std::endl;
    
    try {
        // 1. Initialize logger
        auto logger = logger_module::logger::get_instance();
        logger->initialize();
        logger->log(logger_module::log_level::info, "Starting integration test");
        
        // 2. Initialize monitoring
        auto monitoring = monitoring_module::monitoring::create();
        monitoring->initialize({
            {"enable_thread_pool_analysis", true},
            {"enable_system_metrics", true},
            {"collection_interval_ms", 1000}
        });
        monitoring->start();
        
        // 3. Create thread pool with monitoring
        thread_module::thread_context context;
        context.set_logger(logger.get());
        context.set_monitoring(monitoring.get());
        
        auto pool = std::make_shared<thread_pool_module::thread_pool>("TestPool", context);
        
        // Add workers
        for (int i = 0; i < 2; ++i) {
            auto worker = std::make_unique<thread_pool_module::thread_worker>(
                "Worker-" + std::to_string(i)
            );
            pool->enqueue(std::move(worker));
        }
        pool->start();
        
        // 4. Test container
        container_module::value_container container;
        container.set_source("test_client", "integration");
        container.set_target("test_server", "integration");
        container.set_message_type("test_message");
        container.add_data("key1", "value1");
        container.add_data("key2", 42);
        container.add_data("key3", true);
        
        logger->log(logger_module::log_level::info, 
                   "Created container with " + std::to_string(container.data().size()) + " items");
        
        // 5. Test network (server)
        auto server = std::make_shared<network_module::messaging_server>("TestServer");
        server->set_callback([&logger](std::shared_ptr<network_module::messaging_session> session,
                                       const std::shared_ptr<container_module::value_container>& msg) {
            logger->log(logger_module::log_level::info, 
                       "Server received message from: " + msg->source_id());
            
            // Echo back
            auto response = std::make_shared<container_module::value_container>();
            response->set_source(msg->target_id(), msg->target_sub_id());
            response->set_target(msg->source_id(), msg->source_sub_id());
            response->set_message_type("echo_response");
            response->add_data("echo", "Message received");
            
            session->send(response);
        });
        
        server->start_server(12345);
        logger->log(logger_module::log_level::info, "Server started on port 12345");
        
        // 6. Test network (client)
        auto client = std::make_shared<network_module::messaging_client>("TestClient");
        client->set_callback([&logger](std::shared_ptr<network_module::messaging_session>,
                                      const std::shared_ptr<container_module::value_container>& msg) {
            logger->log(logger_module::log_level::info, 
                       "Client received response: " + msg->message_type());
        });
        
        client->start_client("127.0.0.1", 12345);
        logger->log(logger_module::log_level::info, "Client connected to server");
        
        // Send test message
        client->send(std::make_shared<container_module::value_container>(container));
        
        // 7. Submit some jobs to thread pool
        for (int i = 0; i < 5; ++i) {
            auto job = std::make_unique<thread_module::callback_job>(
                [i, &logger]() {
                    logger->log(logger_module::log_level::info, 
                               "Executing job " + std::to_string(i));
                    std::this_thread::sleep_for(100ms);
                }
            );
            pool->enqueue(std::move(job));
        }
        
        // 8. Test database (if PostgreSQL is available)
        try {
            auto& db_mgr = database::database_manager::handle();
            db_mgr.set_mode(database::database_types::postgres);
            logger->log(logger_module::log_level::info, "Database manager initialized");
        } catch (const std::exception& e) {
            logger->log(logger_module::log_level::warning, 
                       "Database test skipped: " + std::string(e.what()));
        }
        
        // Wait for operations to complete
        std::this_thread::sleep_for(2s);
        
        // 9. Get monitoring metrics
        auto snapshot = monitoring->get_snapshot();
        logger->log(logger_module::log_level::info, 
                   "System metrics collected: " + std::to_string(snapshot.system_metrics.size()));
        
        // Cleanup
        logger->log(logger_module::log_level::info, "Shutting down...");
        
        client->stop_client();
        server->stop_server();
        pool->stop();
        monitoring->stop();
        
        logger->log(logger_module::log_level::info, "Integration test completed successfully");
        logger->flush();
        
        std::cout << "\n✅ All tests completed successfully!\n" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}