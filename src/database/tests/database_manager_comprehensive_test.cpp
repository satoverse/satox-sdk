/**
 * @file $(basename "$1")
 * @brief $(basename "$1" | sed 's/\./_/g' | tr '[:lower:]' '[:upper:]')
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <gtest/gtest.h>
#include "satox/database/database_manager.hpp"
#include <thread>
#include <chrono>
#include <future>
#include <vector>
#include <memory>
#include <random>
#include <filesystem>
#include <fstream>
#include <string>
#include <algorithm>

using namespace satox::database;
using namespace testing;
using json = nlohmann::json;

class DatabaseManagerComprehensiveTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test configuration
        std::string configPath = "test_config.json";
        json config = {
            {"host", "localhost"},
            {"port", 5432},
            {"username", "test_user"},
            {"password", "test_pass"},
            {"max_connections", 10},
            {"connection_timeout", 5000}
        };
        std::ofstream configFile(configPath);
        configFile << config.dump(4);
        configFile.close();

        DatabaseConfig dbConfig;
        dbConfig.name = "test_db";
        dbConfig.maxConnections = 10;
        dbConfig.connectionTimeout = 5000;
        
        ASSERT_TRUE(DatabaseManager::getInstance().initialize(dbConfig));
    }

    void TearDown() override {
        DatabaseManager::getInstance().shutdown();
        std::filesystem::remove("test_config.json");
    }

    // Helper functions
    json createTestSchema() {
        return {
            {"fields", {
                {"id", {{"type", "string"}, {"primary", true}}},
                {"name", {{"type", "string"}, {"required", true}}},
                {"age", {{"type", "integer"}, {"required", false}}},
                {"email", {{"type", "string"}, {"required", true}}}
            }}
        };
    }

    json createTestData() {
        return {
            {"id", "test_id"},
            {"name", "Test User"},
            {"age", 25},
            {"email", "test@example.com"}
        };
    }
};

// Helper function to check if vector contains element
template<typename T>
bool contains(const std::vector<T>& vec, const T& element) {
    return std::find(vec.begin(), vec.end(), element) != vec.end();
}

// Basic Functionality Tests
TEST_F(DatabaseManagerComprehensiveTest, Initialization) {
    EXPECT_TRUE(DatabaseManager::getInstance().isConnected());
    EXPECT_NO_THROW(DatabaseManager::getInstance().shutdown());
    
    DatabaseConfig dbConfig;
    dbConfig.name = "test_db";
    dbConfig.maxConnections = 10;
    dbConfig.connectionTimeout = 5000;
    EXPECT_NO_THROW(DatabaseManager::getInstance().initialize(dbConfig));
}

// Database Operations Tests
TEST_F(DatabaseManagerComprehensiveTest, DatabaseOperations) {
    // Test database creation
    EXPECT_TRUE(DatabaseManager::getInstance().createDatabase("test_db"));
    EXPECT_TRUE(DatabaseManager::getInstance().databaseExists("test_db"));
    
    // Test database listing
    auto databases = DatabaseManager::getInstance().listDatabases();
    EXPECT_TRUE(contains(databases, std::string("test_db")));
    
    // Test database selection
    EXPECT_TRUE(DatabaseManager::getInstance().useDatabase("test_db"));
    
    // Test database deletion
    EXPECT_TRUE(DatabaseManager::getInstance().deleteDatabase("test_db"));
    EXPECT_FALSE(DatabaseManager::getInstance().databaseExists("test_db"));
}

// Table Operations Tests
TEST_F(DatabaseManagerComprehensiveTest, TableOperations) {
    // Create test database
    ASSERT_TRUE(DatabaseManager::getInstance().createDatabase("test_db"));
    ASSERT_TRUE(DatabaseManager::getInstance().useDatabase("test_db"));
    
    // Test table creation
    auto schema = createTestSchema();
    EXPECT_TRUE(DatabaseManager::getInstance().createTable("test_table", schema));
    EXPECT_TRUE(DatabaseManager::getInstance().tableExists("test_table"));
    
    // Test table listing
    auto tables = DatabaseManager::getInstance().listTables();
    EXPECT_TRUE(contains(tables, std::string("test_table")));
    
    // Test schema retrieval
    auto retrievedSchema = DatabaseManager::getInstance().getTableSchema("test_table");
    EXPECT_EQ(retrievedSchema, schema);
    
    // Test table deletion
    EXPECT_TRUE(DatabaseManager::getInstance().deleteTable("test_table"));
    EXPECT_FALSE(DatabaseManager::getInstance().tableExists("test_table"));
}

// Data Operations Tests
TEST_F(DatabaseManagerComprehensiveTest, DataOperations) {
    // Setup test environment
    ASSERT_TRUE(DatabaseManager::getInstance().createDatabase("test_db"));
    ASSERT_TRUE(DatabaseManager::getInstance().useDatabase("test_db"));
    ASSERT_TRUE(DatabaseManager::getInstance().createTable("test_table", createTestSchema()));
    
    // Test data insertion
    auto data = createTestData();
    EXPECT_TRUE(DatabaseManager::getInstance().insert("test_table", data));
    
    // Test data retrieval
    auto retrieved = DatabaseManager::getInstance().find("test_table", "test_id");
    EXPECT_EQ(retrieved["name"], data["name"]);
    EXPECT_EQ(retrieved["age"], data["age"]);
    
    // Test data update
    data["age"] = 26;
    EXPECT_TRUE(DatabaseManager::getInstance().update("test_table", "test_id", data));
    retrieved = DatabaseManager::getInstance().find("test_table", "test_id");
    EXPECT_EQ(retrieved["age"], 26);
    
    // Test data deletion
    EXPECT_TRUE(DatabaseManager::getInstance().remove("test_table", "test_id"));
    retrieved = DatabaseManager::getInstance().find("test_table", "test_id");
    EXPECT_TRUE(retrieved.empty());
}

// Transaction Tests
TEST_F(DatabaseManagerComprehensiveTest, TransactionOperations) {
    // Setup test environment
    ASSERT_TRUE(DatabaseManager::getInstance().createDatabase("test_db"));
    ASSERT_TRUE(DatabaseManager::getInstance().useDatabase("test_db"));
    ASSERT_TRUE(DatabaseManager::getInstance().createTable("test_table", createTestSchema()));
    
    // Test transaction begin
    EXPECT_TRUE(DatabaseManager::getInstance().beginTransaction());
    EXPECT_TRUE(DatabaseManager::getInstance().isInTransaction());
    
    // Test transaction operations
    auto data = createTestData();
    EXPECT_TRUE(DatabaseManager::getInstance().insert("test_table", data));
    
    // Test transaction commit
    EXPECT_TRUE(DatabaseManager::getInstance().commitTransaction());
    EXPECT_FALSE(DatabaseManager::getInstance().isInTransaction());
    
    // Verify committed data
    auto retrieved = DatabaseManager::getInstance().find("test_table", "test_id");
    EXPECT_EQ(retrieved["name"], data["name"]);
    
    // Test transaction rollback
    EXPECT_TRUE(DatabaseManager::getInstance().beginTransaction());
    data["name"] = "Updated Name";
    EXPECT_TRUE(DatabaseManager::getInstance().update("test_table", "test_id", data));
    EXPECT_TRUE(DatabaseManager::getInstance().rollbackTransaction());
    
    // Verify rollback
    retrieved = DatabaseManager::getInstance().find("test_table", "test_id");
    EXPECT_EQ(retrieved["name"], "Test User");
}

// Index Operations Tests
TEST_F(DatabaseManagerComprehensiveTest, IndexOperations) {
    // Setup test environment
    ASSERT_TRUE(DatabaseManager::getInstance().createDatabase("test_db"));
    ASSERT_TRUE(DatabaseManager::getInstance().useDatabase("test_db"));
    ASSERT_TRUE(DatabaseManager::getInstance().createTable("test_table", createTestSchema()));
    
    // Test index creation
    EXPECT_TRUE(DatabaseManager::getInstance().createIndex("test_table", "email"));
    
    // Test index listing
    auto indexes = DatabaseManager::getInstance().listIndexes("test_table");
    EXPECT_TRUE(contains(indexes, std::string("email")));
    
    // Test index deletion
    EXPECT_TRUE(DatabaseManager::getInstance().dropIndex("test_table", "email"));
    indexes = DatabaseManager::getInstance().listIndexes("test_table");
    EXPECT_FALSE(contains(indexes, std::string("email")));
}

// Backup and Restore Tests
TEST_F(DatabaseManagerComprehensiveTest, BackupAndRestore) {
    // Setup test environment
    ASSERT_TRUE(DatabaseManager::getInstance().createDatabase("test_db"));
    ASSERT_TRUE(DatabaseManager::getInstance().useDatabase("test_db"));
    ASSERT_TRUE(DatabaseManager::getInstance().createTable("test_table", createTestSchema()));
    ASSERT_TRUE(DatabaseManager::getInstance().insert("test_table", createTestData()));
    
    // Test backup creation
    EXPECT_TRUE(DatabaseManager::getInstance().createBackup("test_backup.db"));
    
    // Test database deletion
    ASSERT_TRUE(DatabaseManager::getInstance().deleteDatabase("test_db"));
    
    // Test restore from backup
    EXPECT_TRUE(DatabaseManager::getInstance().restoreFromBackup("test_backup.db"));
    EXPECT_TRUE(DatabaseManager::getInstance().databaseExists("test_db"));
    
    // Verify restored data
    ASSERT_TRUE(DatabaseManager::getInstance().useDatabase("test_db"));
    auto retrieved = DatabaseManager::getInstance().find("test_table", "test_id");
    EXPECT_EQ(retrieved["name"], "Test User");
}

// Connection Management Tests
TEST_F(DatabaseManagerComprehensiveTest, ConnectionManagement) {
    // Test connection settings
    DatabaseManager::getInstance().setMaxConnections(20);
    EXPECT_EQ(DatabaseManager::getInstance().getMaxConnections(), 20);
    
    DatabaseManager::getInstance().setConnectionTimeout(10000);
    EXPECT_EQ(DatabaseManager::getInstance().getConnectionTimeout(), 10000);
    
    // Test connection callback
    bool callback_called = false;
    DatabaseManager::getInstance().setConnectionCallback(
        [&](bool connected, const std::string& details) {
            callback_called = true;
        }
    );
    
    // Test reconnection
    EXPECT_TRUE(DatabaseManager::getInstance().reconnect());
    EXPECT_TRUE(callback_called);
}

// Error Handling Tests
TEST_F(DatabaseManagerComprehensiveTest, ErrorHandling) {
    // Test invalid database name
    EXPECT_FALSE(DatabaseManager::getInstance().createDatabase(""));
    
    // Test invalid table name
    ASSERT_TRUE(DatabaseManager::getInstance().createDatabase("test_db"));
    ASSERT_TRUE(DatabaseManager::getInstance().useDatabase("test_db"));
    EXPECT_FALSE(DatabaseManager::getInstance().createTable("", createTestSchema()));
    
    // Test invalid schema
    json invalidSchema = {{"invalid", "schema"}};
    EXPECT_FALSE(DatabaseManager::getInstance().createTable("test_table", invalidSchema));
    
    // Test invalid data
    ASSERT_TRUE(DatabaseManager::getInstance().createTable("test_table", createTestSchema()));
    json invalidData = {{"invalid", "data"}};
    EXPECT_FALSE(DatabaseManager::getInstance().insert("test_table", invalidData));
}

// Concurrency Tests
TEST_F(DatabaseManagerComprehensiveTest, Concurrency) {
    // Setup test environment
    ASSERT_TRUE(DatabaseManager::getInstance().createDatabase("test_db"));
    ASSERT_TRUE(DatabaseManager::getInstance().useDatabase("test_db"));
    ASSERT_TRUE(DatabaseManager::getInstance().createTable("test_table", createTestSchema()));
    
    std::vector<std::future<bool>> futures;
    
    // Test concurrent inserts
    for (int i = 0; i < 10; ++i) {
        futures.push_back(std::async(std::launch::async, [&, i]() {
            json data = createTestData();
            data["id"] = "test_id_" + std::to_string(i);
            return DatabaseManager::getInstance().insert("test_table", data);
        }));
    }
    
    for (auto& future : futures) {
        EXPECT_TRUE(future.get());
    }
    
    // Test concurrent queries
    futures.clear();
    for (int i = 0; i < 10; ++i) {
        futures.push_back(std::async(std::launch::async, [&, i]() {
            return !DatabaseManager::getInstance().find("test_table", "test_id_" + std::to_string(i)).empty();
        }));
    }
    
    for (auto& future : futures) {
        EXPECT_TRUE(future.get());
    }
}

// Edge Case Tests
TEST_F(DatabaseManagerComprehensiveTest, EdgeCases) {
    // Test maximum database name length
    std::string longName(256, 'a');
    EXPECT_FALSE(DatabaseManager::getInstance().createDatabase(longName));
    
    // Test maximum table name length
    ASSERT_TRUE(DatabaseManager::getInstance().createDatabase("test_db"));
    ASSERT_TRUE(DatabaseManager::getInstance().useDatabase("test_db"));
    EXPECT_FALSE(DatabaseManager::getInstance().createTable(longName, createTestSchema()));
    
    // Test maximum data size
    ASSERT_TRUE(DatabaseManager::getInstance().createTable("test_table", createTestSchema()));
    json largeData = createTestData();
    largeData["large_field"] = std::string(1024 * 1024, 'a'); // 1MB of data
    EXPECT_FALSE(DatabaseManager::getInstance().insert("test_table", largeData));
}

// Cleanup Tests
TEST_F(DatabaseManagerComprehensiveTest, Cleanup) {
    // Create test resources
    ASSERT_TRUE(DatabaseManager::getInstance().createDatabase("test_db"));
    ASSERT_TRUE(DatabaseManager::getInstance().useDatabase("test_db"));
    ASSERT_TRUE(DatabaseManager::getInstance().createTable("test_table", createTestSchema()));
    ASSERT_TRUE(DatabaseManager::getInstance().insert("test_table", createTestData()));
    
    // Test shutdown
    DatabaseManager::getInstance().shutdown();
    
    // Verify cleanup
    EXPECT_FALSE(DatabaseManager::getInstance().isConnected());
    EXPECT_FALSE(DatabaseManager::getInstance().databaseExists("test_db"));
} 