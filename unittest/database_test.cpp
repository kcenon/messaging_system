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
#include <database/database_manager.h>
#include <database/database_types.h>
#include <memory>

using namespace database;

class DatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager_ = std::make_unique<database_manager>();
    }

    void TearDown() override {
        if (manager_) {
            manager_->disconnect();
        }
        manager_.reset();
    }

    std::unique_ptr<database_manager> manager_;
};

TEST_F(DatabaseTest, DefaultConstruction) {
    EXPECT_TRUE(manager_ != nullptr);
}

TEST_F(DatabaseTest, SetDatabaseMode) {
    bool result = manager_->set_mode(database_types::postgres);
    EXPECT_TRUE(result);
    
    database_types type = manager_->database_type();
    EXPECT_EQ(type, database_types::postgres);
}

TEST_F(DatabaseTest, SingletonAccess) {
    database_manager& handle1 = database_manager::handle();
    database_manager& handle2 = database_manager::handle();
    
    EXPECT_EQ(&handle1, &handle2);
}

TEST_F(DatabaseTest, ConnectWithInvalidString) {
    manager_->set_mode(database_types::postgres);
    bool result = manager_->connect("invalid_connection_string");
    EXPECT_FALSE(result);
}

TEST_F(DatabaseTest, DisconnectWithoutConnection) {
    bool result = manager_->disconnect();
    EXPECT_FALSE(result);
}

TEST_F(DatabaseTest, CreateQueryWithoutConnection) {
    bool result = manager_->create_query("SELECT * FROM test");
    EXPECT_FALSE(result);
}

TEST_F(DatabaseTest, InsertQueryWithoutConnection) {
    unsigned int result = manager_->insert_query("INSERT INTO test VALUES (1, 'test')");
    EXPECT_EQ(result, 0);
}

TEST_F(DatabaseTest, UpdateQueryWithoutConnection) {
    unsigned int result = manager_->update_query("UPDATE test SET name='updated' WHERE id=1");
    EXPECT_EQ(result, 0);
}

TEST_F(DatabaseTest, DeleteQueryWithoutConnection) {
    unsigned int result = manager_->delete_query("DELETE FROM test WHERE id=1");
    EXPECT_EQ(result, 0);
}

TEST_F(DatabaseTest, SelectQueryWithoutConnection) {
    auto result = manager_->select_query("SELECT * FROM test");
    EXPECT_TRUE(result == nullptr);
}

TEST_F(DatabaseTest, DatabaseTypeInitialization) {
    database_types initial_type = manager_->database_type();
    EXPECT_TRUE(initial_type == database_types::none || 
               initial_type == database_types::postgres);
}

TEST_F(DatabaseTest, MultipleSetModeOperations) {
    bool result1 = manager_->set_mode(database_types::postgres);
    EXPECT_TRUE(result1);
    EXPECT_EQ(manager_->database_type(), database_types::postgres);
    
    bool result2 = manager_->set_mode(database_types::none);
    EXPECT_FALSE(result2);
    EXPECT_EQ(manager_->database_type(), database_types::none);
    
    bool result3 = manager_->set_mode(database_types::postgres);
    EXPECT_TRUE(result3);
    EXPECT_EQ(manager_->database_type(), database_types::postgres);
}

TEST_F(DatabaseTest, EmptyQueryHandling) {
    manager_->set_mode(database_types::postgres);
    
    bool create_result = manager_->create_query("");
    EXPECT_FALSE(create_result);
    
    unsigned int insert_result = manager_->insert_query("");
    EXPECT_EQ(insert_result, 0);
    
    unsigned int update_result = manager_->update_query("");
    EXPECT_EQ(update_result, 0);
    
    unsigned int delete_result = manager_->delete_query("");
    EXPECT_EQ(delete_result, 0);
    
    auto select_result = manager_->select_query("");
    EXPECT_TRUE(select_result != nullptr);
}

TEST_F(DatabaseTest, SequentialOperations) {
    manager_->set_mode(database_types::postgres);
    
    bool connect_result = manager_->connect("host=localhost dbname=test");
    
    if (connect_result) {
        bool create_result = manager_->create_query("CREATE TABLE IF NOT EXISTS test_table (id INT, name VARCHAR(50))");
        
        unsigned int insert_result = manager_->insert_query("INSERT INTO test_table VALUES (1, 'test')");
        
        auto select_result = manager_->select_query("SELECT * FROM test_table");
        
        unsigned int update_result = manager_->update_query("UPDATE test_table SET name='updated' WHERE id=1");
        
        unsigned int delete_result = manager_->delete_query("DELETE FROM test_table WHERE id=1");
        
        bool disconnect_result = manager_->disconnect();
        EXPECT_TRUE(disconnect_result);
    }
}