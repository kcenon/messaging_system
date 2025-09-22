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

#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <thread>

#include <container_system/container.h>
#include <database_system/database_manager.h>
#include <network_system/network.h>

using namespace container_module;
using namespace database;
using namespace network_module;

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        container_ = std::make_shared<value_container>();
        database_mgr_ = std::make_unique<database_manager>();
        server_ = std::make_shared<messaging_server>("integration_server");
        client_ = std::make_shared<messaging_client>("integration_client");
        test_port_ = 16666;
    }

    void TearDown() override {
        if (server_) {
            server_->stop_server();
        }
        if (client_) {
            client_->stop_client();
        }
        if (database_mgr_) {
            database_mgr_->disconnect();
        }
        
        container_.reset();
        database_mgr_.reset();
        server_.reset();
        client_.reset();
    }

    std::shared_ptr<value_container> container_;
    std::unique_ptr<database_manager> database_mgr_;
    std::shared_ptr<messaging_server> server_;
    std::shared_ptr<messaging_client> client_;
    unsigned short test_port_;
};

TEST_F(IntegrationTest, ContainerAndDatabaseIntegration) {
    container_->set_source("db_source", "sub");
    container_->set_target("db_target", "sub");
    container_->set_message_type("database_message");
    
    database_mgr_->set_mode(database_types::postgres);
    
    std::string serialized_data = container_->serialize();
    EXPECT_FALSE(serialized_data.empty());
    
    auto new_container = std::make_shared<value_container>(serialized_data);
    EXPECT_EQ(new_container->source_id(), "db_source");
    EXPECT_EQ(new_container->target_id(), "db_target");
    EXPECT_EQ(new_container->message_type(), "database_message");
}

TEST_F(IntegrationTest, ContainerAndNetworkIntegration) {
    container_->set_source("net_source", "sub");
    container_->set_target("net_target", "sub");
    container_->set_message_type("network_message");
    
    server_->start_server(test_port_);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    client_->start_client("127.0.0.1", test_port_);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::string serialized = container_->serialize();
    EXPECT_FALSE(serialized.empty());
    
    client_->stop_client();
    
    server_->stop_server();
}

TEST_F(IntegrationTest, DatabaseAndNetworkIntegration) {
    database_mgr_->set_mode(database_types::postgres);
    
    server_->start_server(test_port_);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    client_->start_client("127.0.0.1", test_port_);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    client_->stop_client();
    
    server_->stop_server();
}

TEST_F(IntegrationTest, AllModulesIntegration) {
    container_->set_source("full_source", "full_sub");
    container_->set_target("full_target", "full_sub");
    container_->set_message_type("full_integration_message");
    
    database_mgr_->set_mode(database_types::postgres);
    
    server_->start_server(test_port_);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    client_->start_client("127.0.0.1", test_port_);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::string serialized = container_->serialize();
    EXPECT_FALSE(serialized.empty());
    
    auto new_container = std::make_shared<value_container>(serialized);
    EXPECT_EQ(new_container->source_id(), "full_source");
    EXPECT_EQ(new_container->target_id(), "full_target");
    EXPECT_EQ(new_container->message_type(), "full_integration_message");
    
    client_->stop_client();
    
    server_->stop_server();
}

TEST_F(IntegrationTest, ContainerSerializationCycle) {
    container_->set_source("cycle_source", "cycle_sub");
    container_->set_target("cycle_target", "cycle_sub");
    container_->set_message_type("cycle_message");
    
    std::string original_serialized = container_->serialize();
    
    auto temp_container = std::make_shared<value_container>(original_serialized);
    std::string round_trip_serialized = temp_container->serialize();
    
    auto final_container = std::make_shared<value_container>(round_trip_serialized);
    
    EXPECT_EQ(final_container->source_id(), "cycle_source");
    EXPECT_EQ(final_container->source_sub_id(), "cycle_sub");
    EXPECT_EQ(final_container->target_id(), "cycle_target");
    EXPECT_EQ(final_container->target_sub_id(), "cycle_sub");
    EXPECT_EQ(final_container->message_type(), "cycle_message");
}

TEST_F(IntegrationTest, MultipleClientSessions) {
    server_->start_server(test_port_);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto client1 = std::make_shared<messaging_client>("client_1");
    auto client2 = std::make_shared<messaging_client>("client_2");
    auto client3 = std::make_shared<messaging_client>("client_3");
    
    client1->start_client("127.0.0.1", test_port_);
    client2->start_client("127.0.0.1", test_port_);
    client3->start_client("127.0.0.1", test_port_);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    client1->stop_client();
    client2->stop_client();
    client3->stop_client();
    
    server_->stop_server();
}

TEST_F(IntegrationTest, DatabaseSingletonBehavior) {
    database_manager& mgr1 = database_manager::handle();
    database_manager& mgr2 = database_manager::handle();
    
    EXPECT_EQ(&mgr1, &mgr2);
    
    mgr1.set_mode(database_types::postgres);
    EXPECT_EQ(mgr2.database_type(), database_types::postgres);
    
    mgr2.set_mode(database_types::none);
    EXPECT_EQ(mgr1.database_type(), database_types::none);
}

TEST_F(IntegrationTest, ContainerHeaderSwapAndSerialization) {
    container_->set_source("original_source", "original_sub");
    container_->set_target("original_target", "original_sub");
    container_->set_message_type("swap_test");
    
    container_->swap_header();
    
    EXPECT_EQ(container_->source_id(), "original_target");
    EXPECT_EQ(container_->target_id(), "original_source");
    
    std::string serialized = container_->serialize();
    auto new_container = std::make_shared<value_container>(serialized);
    
    EXPECT_EQ(new_container->source_id(), "original_target");
    EXPECT_EQ(new_container->target_id(), "original_source");
    EXPECT_EQ(new_container->message_type(), "swap_test");
}