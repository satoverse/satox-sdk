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
// #include <gmock/gmock.h>  // Removed to avoid compilation issues
#include <memory>
#include <mutex>
#include <thread>
#include <chrono>
#include <random>
#include <fstream>
#include <string>
#include "satox/database/database_manager.hpp"

using namespace satox::database;
// using namespace testing;  // Removed to avoid Google Mock conflicts

class DatabaseManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary test database
        std::string configPath = "test_config.json";
        nlohmann::json config = {
            {"maxConnections", 10},
            {"connectionTimeout", 5000}
        };
        std::ofstream configFile(configPath);
        configFile << config.dump(4);
        
        DatabaseConfig dbConfig;
        dbConfig.name = "test_db";
        dbConfig.maxConnections = 10;
        dbConfig.connectionTimeout = 5000;
        
        ASSERT_TRUE(dbManager.initialize(dbConfig));
        ASSERT_TRUE(dbManager.connect(""));
        ASSERT_TRUE(dbManager.createDatabase("test_db"));
        ASSERT_TRUE(dbManager.useDatabase("test_db"));
    }

    void TearDown() override {
        dbManager.shutdown();
        std::filesystem::remove("test_config.json");
    }

    DatabaseManager& dbManager = DatabaseManager::getInstance();
};

// Helper function to check if vector contains element
template<typename T>
bool contains(const std::vector<T>& vec, const T& element) {
    return std::find(vec.begin(), vec.end(), element) != vec.end();
}

// Basic Functionality Tests
TEST_F(DatabaseManagerTest, Initialization) {
    EXPECT_TRUE(dbManager.isConnected());
    EXPECT_EQ(dbManager.getMaxConnections(), 10);
    EXPECT_EQ(dbManager.getConnectionTimeout(), 5000);
}

TEST_F(DatabaseManagerTest, DatabaseOperations) {
    EXPECT_TRUE(dbManager.createDatabase("test_db2"));
    EXPECT_TRUE(dbManager.databaseExists("test_db2"));
    
    auto databases = dbManager.listDatabases();
    EXPECT_TRUE(contains(databases, std::string("test_db")));
    EXPECT_TRUE(contains(databases, std::string("test_db2")));
    
    EXPECT_TRUE(dbManager.deleteDatabase("test_db2"));
    EXPECT_FALSE(dbManager.databaseExists("test_db2"));
}

// Table Operations Tests
TEST_F(DatabaseManagerTest, TableOperations) {
    nlohmann::json schema = {
        {"fields", {
            {"id", "string"},
            {"name", "string"},
            {"age", "integer"}
        }},
        {"required", {"id", "name"}}
    };
    
    EXPECT_TRUE(dbManager.createTable("users", schema));
    EXPECT_TRUE(dbManager.tableExists("users"));
    
    auto tables = dbManager.listTables();
    EXPECT_TRUE(contains(tables, std::string("users")));
    
    auto retrievedSchema = dbManager.getTableSchema("users");
    EXPECT_EQ(retrievedSchema, schema);
    
    EXPECT_TRUE(dbManager.deleteTable("users"));
    EXPECT_FALSE(dbManager.tableExists("users"));
}

// Data Operations Tests
TEST_F(DatabaseManagerTest, DataOperations) {
    // Create test table
    nlohmann::json schema = {
        {"fields", {
            {"id", "string"},
            {"name", "string"},
            {"age", "integer"}
        }},
        {"required", {"id", "name"}}
    };
    ASSERT_TRUE(dbManager.createTable("users", schema));
    
    // Test insert
    nlohmann::json data = {
        {"id", "1"},
        {"name", "John"},
        {"age", 30}
    };
    EXPECT_TRUE(dbManager.insert("users", data));
    
    // Test find
    auto found = dbManager.find("users", "1");
    EXPECT_EQ(found["name"], "John");
    EXPECT_EQ(found["age"], 30);
    
    // Test update
    nlohmann::json update = {
        {"age", 31}
    };
    EXPECT_TRUE(dbManager.update("users", "1", update));
    
    found = dbManager.find("users", "1");
    EXPECT_EQ(found["age"], 31);
    
    // Test query
    auto results = dbManager.query("users", {{"age", 31}});
    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0]["name"], "John");
    
    // Test remove
    EXPECT_TRUE(dbManager.remove("users", "1"));
    found = dbManager.find("users", "1");
    EXPECT_TRUE(found.empty());
}

// Transaction Tests
TEST_F(DatabaseManagerTest, TransactionOperations) {
    ASSERT_TRUE(dbManager.createTable("accounts", {
        {"fields", {
            {"id", "string"},
            {"balance", "integer"}
        }},
        {"required", {"id", "balance"}}
    }));
    
    // Test successful transaction
    ASSERT_TRUE(dbManager.beginTransaction());
    EXPECT_TRUE(dbManager.insert("accounts", {{"id", "1"}, {"balance", 100}}));
    EXPECT_TRUE(dbManager.insert("accounts", {{"id", "2"}, {"balance", 200}}));
    EXPECT_TRUE(dbManager.commitTransaction());
    
    auto results = dbManager.query("accounts", {});
    EXPECT_EQ(results.size(), 2);
    
    // Test rollback
    ASSERT_TRUE(dbManager.beginTransaction());
    EXPECT_TRUE(dbManager.insert("accounts", {{"id", "3"}, {"balance", 300}}));
    EXPECT_TRUE(dbManager.rollbackTransaction());
    
    results = dbManager.query("accounts", {});
    EXPECT_EQ(results.size(), 2);
}

// Index Tests
TEST_F(DatabaseManagerTest, IndexOperations) {
    ASSERT_TRUE(dbManager.createTable("products", {
        {"fields", {
            {"id", "string"},
            {"name", "string"},
            {"price", "integer"}
        }},
        {"required", {"id", "name", "price"}}
    }));
    
    EXPECT_TRUE(dbManager.createIndex("products", "price"));
    auto indexes = dbManager.listIndexes("products");
    EXPECT_TRUE(contains(indexes, std::string("price")));
    
    EXPECT_TRUE(dbManager.dropIndex("products", "price"));
    indexes = dbManager.listIndexes("products");
    EXPECT_FALSE(contains(indexes, std::string("price")));
}

// Backup and Restore Tests
TEST_F(DatabaseManagerTest, BackupAndRestore) {
    // Create test data
    ASSERT_TRUE(dbManager.createTable("test_data", {
        {"fields", {
            {"id", "string"},
            {"value", "string"}
        }},
        {"required", {"id", "value"}}
    }));
    
    ASSERT_TRUE(dbManager.insert("test_data", {{"id", "1"}, {"value", "test1"}}));
    ASSERT_TRUE(dbManager.insert("test_data", {{"id", "2"}, {"value", "test2"}}));
    
    // Test backup
    EXPECT_TRUE(dbManager.createBackup("test_backup"));
    
    // Modify data
    ASSERT_TRUE(dbManager.remove("test_data", "1"));
    
    // Test restore - using the correct method name
    EXPECT_TRUE(dbManager.restoreFromBackup("test_backup"));
    
    // Verify data is restored
    auto found = dbManager.find("test_data", "1");
    EXPECT_EQ(found["value"], "test1");
}

// Concurrency Tests
TEST_F(DatabaseManagerTest, Concurrency) {
    ASSERT_TRUE(dbManager.createTable("concurrent_test", {
        {"fields", {
            {"id", "string"},
            {"value", "integer"}
        }},
        {"required", {"id", "value"}}
    }));
    
    const int numThreads = 10;
    const int operationsPerThread = 100;
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&, i]() {
            for (int j = 0; j < operationsPerThread; ++j) {
                std::string id = "thread_" + std::to_string(i) + "_" + std::to_string(j);
                nlohmann::json data = {{"id", id}, {"value", j}};
                if (dbManager.insert("concurrent_test", data)) {
                    successCount++;
                }
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(successCount.load(), numThreads * operationsPerThread);
}

// Performance Tests
TEST_F(DatabaseManagerTest, Performance) {
    ASSERT_TRUE(dbManager.createTable("perf_test", {
        {"fields", {
            {"id", "string"},
            {"data", "string"}
        }},
        {"required", {"id", "data"}}
    }));
    
    const int numRecords = 1000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < numRecords; ++i) {
        nlohmann::json data = {
            {"id", "perf_" + std::to_string(i)},
            {"data", "data_" + std::to_string(i)}
        };
        ASSERT_TRUE(dbManager.insert("perf_test", data));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Performance should be reasonable (less than 1 second for 1000 records)
    EXPECT_LT(duration.count(), 1000);
}

// Edge Cases Tests
TEST_F(DatabaseManagerTest, EdgeCases) {
    // Test with empty data
    ASSERT_TRUE(dbManager.createTable("edge_test", {
        {"fields", {
            {"id", "string"}
        }},
        {"required", {"id"}}
    }));
    
    nlohmann::json emptyData = {{"id", "empty"}};
    EXPECT_TRUE(dbManager.insert("edge_test", emptyData));
    
    // Test with non-existent table
    EXPECT_FALSE(dbManager.insert("non_existent", {{"id", "test"}}));
    EXPECT_FALSE(dbManager.find("non_existent", "test").is_object());
    
    // Test with invalid JSON
    EXPECT_FALSE(dbManager.insert("edge_test", nlohmann::json()));
}

// Recovery Tests
TEST_F(DatabaseManagerTest, Recovery) {
    ASSERT_TRUE(dbManager.createTable("recovery_test", {
        {"fields", {
            {"id", "string"},
            {"value", "string"}
        }},
        {"required", {"id", "value"}}
    }));
    
    ASSERT_TRUE(dbManager.insert("recovery_test", {{"id", "1"}, {"value", "test"}}));
    
    // Test health check - using the correct method name and property
    auto health = dbManager.getHealth();
    EXPECT_TRUE(health.healthy);
    
    // Test statistics - using the correct method name and property
    auto stats = dbManager.getStats();
    EXPECT_GE(stats.totalOperations, 0);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 