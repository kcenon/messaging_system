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

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include <container/container.h>

using namespace container_module;

class ContainerTest : public ::testing::Test {
protected:
    void SetUp() override {
        container_ = std::make_shared<value_container>();
    }

    void TearDown() override {
        container_.reset();
    }

    std::shared_ptr<value_container> container_;
};

TEST_F(ContainerTest, DefaultConstruction) {
    EXPECT_TRUE(container_ != nullptr);
    EXPECT_EQ(container_->source_id(), "");
    EXPECT_EQ(container_->target_id(), "");
    EXPECT_EQ(container_->message_type(), "data_container");
}

TEST_F(ContainerTest, SetAndGetSourceTarget) {
    container_->set_source("source_test", "sub_source");
    container_->set_target("target_test", "sub_target");
    
    EXPECT_EQ(container_->source_id(), "source_test");
    EXPECT_EQ(container_->source_sub_id(), "sub_source");
    EXPECT_EQ(container_->target_id(), "target_test");
    EXPECT_EQ(container_->target_sub_id(), "sub_target");
}

TEST_F(ContainerTest, MessageType) {
    container_->set_message_type("test_message");
    EXPECT_EQ(container_->message_type(), "test_message");
}

TEST_F(ContainerTest, SwapHeader) {
    container_->set_source("source_id", "source_sub");
    container_->set_target("target_id", "target_sub");
    
    container_->swap_header();
    
    EXPECT_EQ(container_->source_id(), "target_id");
    EXPECT_EQ(container_->source_sub_id(), "target_sub");
    EXPECT_EQ(container_->target_id(), "source_id");
    EXPECT_EQ(container_->target_sub_id(), "source_sub");
}

TEST_F(ContainerTest, ConstructWithMessageType) {
    std::vector<std::shared_ptr<value>> units;
    auto container = std::make_shared<value_container>("test_type", units);
    
    EXPECT_EQ(container->message_type(), "test_type");
}

TEST_F(ContainerTest, ConstructWithFullHeader) {
    std::vector<std::shared_ptr<value>> units;
    auto container = std::make_shared<value_container>(
        "src_id", "src_sub", "tgt_id", "tgt_sub", "msg_type", units);
    
    EXPECT_EQ(container->source_id(), "src_id");
    EXPECT_EQ(container->source_sub_id(), "src_sub");
    EXPECT_EQ(container->target_id(), "tgt_id");
    EXPECT_EQ(container->target_sub_id(), "tgt_sub");
    EXPECT_EQ(container->message_type(), "msg_type");
}

TEST_F(ContainerTest, Copy) {
    container_->set_source("src", "sub");
    container_->set_message_type("type");
    
    auto copied = container_->copy(false);
    
    EXPECT_EQ(copied->source_id(), "src");
    EXPECT_EQ(copied->source_sub_id(), "sub");
    EXPECT_EQ(copied->message_type(), "type");
}

TEST_F(ContainerTest, Serialization) {
    container_->set_source("test_source", "test_sub");
    container_->set_target("test_target", "test_target_sub");
    container_->set_message_type("test_message");
    
    std::string serialized = container_->serialize();
    EXPECT_FALSE(serialized.empty());
    
    auto new_container = std::make_shared<value_container>(serialized);
    EXPECT_EQ(new_container->source_id(), "test_source");
    EXPECT_EQ(new_container->target_id(), "test_target");
    EXPECT_EQ(new_container->message_type(), "test_message");
}

TEST_F(ContainerTest, ArraySerialization) {
    container_->set_message_type("array_test");
    
    std::vector<uint8_t> data = container_->serialize_array();
    EXPECT_FALSE(data.empty());
    
    auto new_container = std::make_shared<value_container>(data);
    EXPECT_EQ(new_container->message_type(), "array_test");
}

TEST_F(ContainerTest, Initialize) {
    container_->set_source("test", "test");
    container_->set_message_type("test");
    
    container_->initialize();
    
    EXPECT_EQ(container_->source_id(), "");
    EXPECT_EQ(container_->message_type(), "data_container");
}

TEST_F(ContainerTest, ClearValue) {
    container_->clear_value();
    EXPECT_TRUE(container_ != nullptr);
}

TEST_F(ContainerTest, MoveCopyConstructor) {
    container_->set_source("move_test", "sub");
    container_->set_message_type("move_message");
    
    value_container moved_container(std::move(*container_));
    
    EXPECT_EQ(moved_container.source_id(), "move_test");
    EXPECT_EQ(moved_container.source_sub_id(), "sub");
    EXPECT_EQ(moved_container.message_type(), "move_message");
}

TEST_F(ContainerTest, CopyConstructor) {
    container_->set_source("copy_source", "copy_sub");
    container_->set_target("copy_target", "copy_target_sub");
    container_->set_message_type("copy_message");
    
    value_container copied_container(*container_);
    
    EXPECT_EQ(copied_container.source_id(), "copy_source");
    EXPECT_EQ(copied_container.source_sub_id(), "copy_sub");
    EXPECT_EQ(copied_container.target_id(), "copy_target");
    EXPECT_EQ(copied_container.target_sub_id(), "copy_target_sub");
    EXPECT_EQ(copied_container.message_type(), "copy_message");
}

TEST_F(ContainerTest, XMLGeneration) {
    container_->set_message_type("xml_test");
    std::string xml = container_->to_xml();
    EXPECT_FALSE(xml.empty());
}

TEST_F(ContainerTest, JSONGeneration) {
    container_->set_message_type("json_test");
    std::string json = container_->to_json();
    EXPECT_FALSE(json.empty());
}