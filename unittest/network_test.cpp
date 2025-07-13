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
#include <network/network.h>
#include <memory>
#include <thread>
#include <chrono>

using namespace network_module;

class NetworkTest : public ::testing::Test {
protected:
    void SetUp() override {
        server_ = std::make_shared<messaging_server>("test_server");
        client_ = std::make_shared<messaging_client>("test_client");
        test_port_ = 15555;
    }

    void TearDown() override {
        if (server_) {
            server_->stop_server();
        }
        if (client_) {
            client_->stop_client();
        }
        server_.reset();
        client_.reset();
    }

    std::shared_ptr<messaging_server> server_;
    std::shared_ptr<messaging_client> client_;
    unsigned short test_port_;
};

TEST_F(NetworkTest, ServerConstruction) {
    EXPECT_TRUE(server_ != nullptr);
    auto test_server = std::make_shared<messaging_server>("construction_test");
    EXPECT_TRUE(test_server != nullptr);
}

TEST_F(NetworkTest, ClientConstruction) {
    EXPECT_TRUE(client_ != nullptr);
    auto test_client = std::make_shared<messaging_client>("construction_test");
    EXPECT_TRUE(test_client != nullptr);
}

TEST_F(NetworkTest, ServerStartStop) {
    server_->start_server(test_port_);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    server_->stop_server();
}

TEST_F(NetworkTest, ServerDoubleStart) {
    server_->start_server(test_port_);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    server_->start_server(test_port_);
    
    server_->stop_server();
}

TEST_F(NetworkTest, ServerStopWithoutStart) {
    server_->stop_server();
}

TEST_F(NetworkTest, ServerWaitForStop) {
    std::thread server_thread([this]() {
        server_->start_server(test_port_);
        server_->wait_for_stop();
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    server_->stop_server();
    
    if (server_thread.joinable()) {
        server_thread.join();
    }
}

TEST_F(NetworkTest, ClientConnectToInvalidAddress) {
    client_->start_client("invalid_host", 12345);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    client_->stop_client();
}

TEST_F(NetworkTest, ClientConnectToValidButClosedPort) {
    client_->start_client("127.0.0.1", 54321);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    client_->stop_client();
}

TEST_F(NetworkTest, ClientDisconnectWithoutConnection) {
    client_->stop_client();
}

TEST_F(NetworkTest, ServerClientIntegration) {
    server_->start_server(test_port_);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    client_->start_client("127.0.0.1", test_port_);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    client_->stop_client();
    
    server_->stop_server();
}

TEST_F(NetworkTest, MultipleClients) {
    server_->start_server(test_port_);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    auto client1 = std::make_shared<messaging_client>("client1");
    auto client2 = std::make_shared<messaging_client>("client2");
    auto client3 = std::make_shared<messaging_client>("client3");
    
    client1->start_client("127.0.0.1", test_port_);
    client2->start_client("127.0.0.1", test_port_);
    client3->start_client("127.0.0.1", test_port_);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    client1->stop_client();
    client2->stop_client();
    client3->stop_client();
    
    server_->stop_server();
}

TEST_F(NetworkTest, ServerPortBinding) {
    unsigned short port1 = test_port_;
    unsigned short port2 = test_port_ + 1;
    
    auto server1 = std::make_shared<messaging_server>("server1");
    auto server2 = std::make_shared<messaging_server>("server2");
    
    server1->start_server(port1);
    server2->start_server(port2);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    client_->start_client("127.0.0.1", port1);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    client_->stop_client();
    
    client_->start_client("127.0.0.1", port2);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    client_->stop_client();
    
    server1->stop_server();
    server2->stop_server();
}

TEST_F(NetworkTest, ServerQuickStartStop) {
    for (int i = 0; i < 5; ++i) {
        server_->start_server(test_port_ + i);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        server_->stop_server();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

TEST_F(NetworkTest, ClientQuickConnectDisconnect) {
    server_->start_server(test_port_);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    for (int i = 0; i < 5; ++i) {
        client_->start_client("127.0.0.1", test_port_);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        client_->stop_client();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    server_->stop_server();
}