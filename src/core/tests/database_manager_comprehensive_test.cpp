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
#include <chrono>
#include <thread>
#include <vector>
#include <random>
#include <memory>
#include <future>
#include "satox/core/database_manager.hpp"
#include <fstream>
#include <filesystem>

using namespace satox::core;

class DatabaseManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = &DatabaseManager::getInstance();
        manager->shutdown(); // Ensure clean state
        manager->initialize(nlohmann::json::object());
        
        // Create test directory
        std::filesystem::create_directory("test_data");
    }

    void TearDown() override {
        manager->shutdown();
        std::filesystem::remove_all("test_data");
    }

    DatabaseManager* manager;
};

// Initialization Tests
TEST_F(DatabaseManagerTest, Initialization) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    EXPECT_FALSE(manager->initialize(nlohmann::json::object())); // Should fail on second init
}

// SQLite Tests
TEST_F(DatabaseManagerTest, SQLiteOperations) {
    // Connect to SQLite
    nlohmann::json config = {
        {"path", "test_data/test.db"}
    };
    std::string id = manager->connect(DatabaseType::SQLITE, config);
    EXPECT_FALSE(id.empty());

    // Create table
    nlohmann::json result;
    EXPECT_TRUE(manager->executeQuery(id, 
        "CREATE TABLE test (id INTEGER PRIMARY KEY, name TEXT, value REAL)", result));

    // Insert data
    EXPECT_TRUE(manager->executeQuery(id,
        "INSERT INTO test (name, value) VALUES ('test1', 1.0)", result));
    EXPECT_TRUE(manager->executeQuery(id,
        "INSERT INTO test (name, value) VALUES ('test2', 2.0)", result));

    // Query data
    EXPECT_TRUE(manager->executeQuery(id, "SELECT * FROM test", result));
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0]["name"], "test1");
    EXPECT_EQ(result[1]["name"], "test2");

    // Transaction
    std::vector<std::string> queries = {
        "INSERT INTO test (name, value) VALUES ('test3', 3.0)",
        "INSERT INTO test (name, value) VALUES ('test4', 4.0)"
    };
    EXPECT_TRUE(manager->executeTransaction(id, queries, result));

    // Backup and restore
    EXPECT_TRUE(manager->backup(id, "test_data/backup.db"));
    EXPECT_TRUE(manager->restore(id, "test_data/backup.db"));

    // Disconnect
    EXPECT_TRUE(manager->disconnect(id));
}

// PostgreSQL Tests
TEST_F(DatabaseManagerTest, PostgreSQLOperations) {
    // Connect to PostgreSQL
    nlohmann::json config = {
        {"host", "localhost"},
        {"port", 5432},
        {"database", "testdb"},
        {"username", "testuser"},
        {"password", "testpass"}
    };
    std::string id = manager->connect(DatabaseType::POSTGRESQL, config);
    if (id.empty()) {
        GTEST_SKIP() << "PostgreSQL not available";
    }

    // Create table
    nlohmann::json result;
    EXPECT_TRUE(manager->executeQuery(id,
        "CREATE TABLE test (id SERIAL PRIMARY KEY, name VARCHAR(255), value DOUBLE PRECISION)",
        result));

    // Insert data
    EXPECT_TRUE(manager->executeQuery(id,
        "INSERT INTO test (name, value) VALUES ('test1', 1.0)", result));
    EXPECT_TRUE(manager->executeQuery(id,
        "INSERT INTO test (name, value) VALUES ('test2', 2.0)", result));

    // Query data
    EXPECT_TRUE(manager->executeQuery(id, "SELECT * FROM test", result));
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0]["name"], "test1");
    EXPECT_EQ(result[1]["name"], "test2");

    // Transaction
    std::vector<std::string> queries = {
        "INSERT INTO test (name, value) VALUES ('test3', 3.0)",
        "INSERT INTO test (name, value) VALUES ('test4', 4.0)"
    };
    EXPECT_TRUE(manager->executeTransaction(id, queries, result));

    // Backup and restore
    EXPECT_TRUE(manager->backup(id, "test_data/backup.sql"));
    EXPECT_TRUE(manager->restore(id, "test_data/backup.sql"));

    // Disconnect
    EXPECT_TRUE(manager->disconnect(id));
}

// Redis Tests
TEST_F(DatabaseManagerTest, RedisOperations) {
    // Connect to Redis
    nlohmann::json config = {
        {"host", "localhost"},
        {"port", 6379}
    };
    std::string id = manager->connect(DatabaseType::REDIS, config);
    if (id.empty()) {
        GTEST_SKIP() << "Redis not available";
    }

    // Set and get
    nlohmann::json result;
    EXPECT_TRUE(manager->executeQuery(id, "SET test1 value1", result));
    EXPECT_TRUE(manager->executeQuery(id, "GET test1", result));
    EXPECT_EQ(result, "value1");

    // List operations
    EXPECT_TRUE(manager->executeQuery(id, "LPUSH testlist value1", result));
    EXPECT_TRUE(manager->executeQuery(id, "LPUSH testlist value2", result));
    EXPECT_TRUE(manager->executeQuery(id, "LRANGE testlist 0 -1", result));
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "value2");
    EXPECT_EQ(result[1], "value1");

    // Hash operations
    EXPECT_TRUE(manager->executeQuery(id, "HSET testhash field1 value1", result));
    EXPECT_TRUE(manager->executeQuery(id, "HGET testhash field1", result));
    EXPECT_EQ(result, "value1");

    // Set operations
    EXPECT_TRUE(manager->executeQuery(id, "SADD testset value1", result));
    EXPECT_TRUE(manager->executeQuery(id, "SADD testset value2", result));
    EXPECT_TRUE(manager->executeQuery(id, "SMEMBERS testset", result));
    EXPECT_EQ(result.size(), 2);

    // Backup and restore
    EXPECT_TRUE(manager->backup(id, "test_data/backup.rdb"));
    EXPECT_TRUE(manager->restore(id, "test_data/backup.rdb"));

    // Disconnect
    EXPECT_TRUE(manager->disconnect(id));
}

// Statistics Tests
TEST_F(DatabaseManagerTest, Statistics) {
    // Connect to SQLite
    nlohmann::json config = {
        {"path", "test_data/test.db"}
    };
    std::string id = manager->connect(DatabaseType::SQLITE, config);
    EXPECT_FALSE(id.empty());

    // Enable statistics
    manager->enableStats(id, true);

    // Execute some queries
    nlohmann::json result;
    EXPECT_TRUE(manager->executeQuery(id,
        "CREATE TABLE test (id INTEGER PRIMARY KEY, name TEXT)", result));
    EXPECT_TRUE(manager->executeQuery(id,
        "INSERT INTO test (name) VALUES ('test1')", result));
    EXPECT_TRUE(manager->executeQuery(id,
        "SELECT * FROM test", result));

    // Check statistics
    auto stats = manager->getStats(id);
    EXPECT_EQ(stats.totalOperations, 3);
    EXPECT_EQ(stats.successfulOperations, 3);
    EXPECT_EQ(stats.failedOperations, 0);
    EXPECT_GT(stats.totalOperationTime, 0);
    EXPECT_GT(stats.averageOperationTime, 0);

    // Reset statistics
    manager->resetStats(id);
    stats = manager->getStats(id);
    EXPECT_EQ(stats.totalOperations, 0);
    EXPECT_EQ(stats.successfulOperations, 0);
    EXPECT_EQ(stats.failedOperations, 0);
    EXPECT_EQ(stats.totalOperationTime, 0);
    EXPECT_EQ(stats.averageOperationTime, 0);

    // Disconnect
    EXPECT_TRUE(manager->disconnect(id));
}

// Callback Tests
TEST_F(DatabaseManagerTest, Callbacks) {
    // Connect to SQLite
    nlohmann::json config = {
        {"path", "test_data/test.db"}
    };
    std::string id = manager->connect(DatabaseType::SQLITE, config);
    EXPECT_FALSE(id.empty());

    // Register callback
    bool callbackCalled = false;
    manager->registerCallback(id, [&](const std::string& query, bool success) {
        callbackCalled = true;
    });

    // Execute query
    nlohmann::json result;
    EXPECT_TRUE(manager->executeQuery(id,
        "CREATE TABLE test (id INTEGER PRIMARY KEY, name TEXT)", result));
    EXPECT_TRUE(callbackCalled);

    // Unregister callback
    manager->unregisterCallback(id);
    callbackCalled = false;
    EXPECT_TRUE(manager->executeQuery(id,
        "INSERT INTO test (name) VALUES ('test1')", result));
    EXPECT_FALSE(callbackCalled);

    // Disconnect
    EXPECT_TRUE(manager->disconnect(id));
}

// Error Handling Tests
TEST_F(DatabaseManagerTest, ErrorHandling) {
    // Test invalid connection
    EXPECT_FALSE(manager->executeQuery("invalid_id", "SELECT 1", nlohmann::json()));
    EXPECT_FALSE(manager->getLastError().empty());

    // Test invalid query
    nlohmann::json config = {
        {"path", "test_data/test.db"}
    };
    std::string id = manager->connect(DatabaseType::SQLITE, config);
    EXPECT_FALSE(id.empty());

    EXPECT_FALSE(manager->executeQuery(id, "INVALID QUERY", nlohmann::json()));
    EXPECT_FALSE(manager->getLastError().empty());

    // Test invalid transaction
    std::vector<std::string> queries = {"INVALID QUERY"};
    EXPECT_FALSE(manager->executeTransaction(id, queries, nlohmann::json()));
    EXPECT_FALSE(manager->getLastError().empty());

    // Disconnect
    EXPECT_TRUE(manager->disconnect(id));
}

// Concurrency Tests
TEST_F(DatabaseManagerTest, Concurrency) {
    // Connect to SQLite
    nlohmann::json config = {
        {"path", "test_data/test.db"}
    };
    std::string id = manager->connect(DatabaseType::SQLITE, config);
    EXPECT_FALSE(id.empty());

    // Create table
    nlohmann::json result;
    EXPECT_TRUE(manager->executeQuery(id,
        "CREATE TABLE test (id INTEGER PRIMARY KEY, name TEXT)", result));

    // Execute concurrent queries
    const int numThreads = 10;
    const int numQueriesPerThread = 100;
    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([this, id, i, numQueriesPerThread]() {
            for (int j = 0; j < numQueriesPerThread; ++j) {
                nlohmann::json result;
                std::string query = "INSERT INTO test (name) VALUES ('test_" +
                                  std::to_string(i) + "_" + std::to_string(j) + "')";
                manager->executeQuery(id, query, result);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // Verify results
    EXPECT_TRUE(manager->executeQuery(id, "SELECT COUNT(*) as count FROM test", result));
    EXPECT_EQ(result[0]["count"], numThreads * numQueriesPerThread);

    // Disconnect
    EXPECT_TRUE(manager->disconnect(id));
}

// Edge Cases Tests
TEST_F(DatabaseManagerTest, EdgeCases) {
    // Connect to SQLite
    nlohmann::json config = {
        {"path", "test_data/test.db"}
    };
    std::string id = manager->connect(DatabaseType::SQLITE, config);
    EXPECT_FALSE(id.empty());

    // Create table
    nlohmann::json result;
    EXPECT_TRUE(manager->executeQuery(id,
        "CREATE TABLE test (id INTEGER PRIMARY KEY, name TEXT, value REAL)", result));

    // Test empty values
    EXPECT_TRUE(manager->executeQuery(id,
        "INSERT INTO test (name, value) VALUES ('', NULL)", result));

    // Test special characters
    EXPECT_TRUE(manager->executeQuery(id,
        "INSERT INTO test (name, value) VALUES ('test!@#$%^&*()', 1.0)", result));

    // Test large values
    std::string largeValue(1000, 'a');
    EXPECT_TRUE(manager->executeQuery(id,
        "INSERT INTO test (name, value) VALUES (?, 1.0)", result));

    // Test concurrent connections
    std::string id2 = manager->connect(DatabaseType::SQLITE, config);
    EXPECT_FALSE(id2.empty());
    EXPECT_TRUE(manager->disconnect(id2));

    // Disconnect
    EXPECT_TRUE(manager->disconnect(id));
}

// Cleanup Tests
TEST_F(DatabaseManagerTest, Cleanup) {
    // Connect to SQLite
    nlohmann::json config = {
        {"path", "test_data/test.db"}
    };
    std::string id = manager->connect(DatabaseType::SQLITE, config);
    EXPECT_FALSE(id.empty());

    // Create table and insert data
    nlohmann::json result;
    EXPECT_TRUE(manager->executeQuery(id,
        "CREATE TABLE test (id INTEGER PRIMARY KEY, name TEXT)", result));
    EXPECT_TRUE(manager->executeQuery(id,
        "INSERT INTO test (name) VALUES ('test1')", result));

    // Disconnect
    EXPECT_TRUE(manager->disconnect(id));

    // Verify cleanup
    EXPECT_FALSE(manager->executeQuery(id, "SELECT * FROM test", result));
}

// Stress Tests
TEST_F(DatabaseManagerTest, StressTest) {
    // Connect to SQLite
    nlohmann::json config = {
        {"path", "test_data/test.db"}
    };
    std::string id = manager->connect(DatabaseType::SQLITE, config);
    EXPECT_FALSE(id.empty());

    // Create table
    nlohmann::json result;
    EXPECT_TRUE(manager->executeQuery(id,
        "CREATE TABLE test (id INTEGER PRIMARY KEY, name TEXT, value REAL)", result));

    // Insert many rows
    const int numRows = 10000;
    for (int i = 0; i < numRows; ++i) {
        EXPECT_TRUE(manager->executeQuery(id,
            "INSERT INTO test (name, value) VALUES (?, ?)", result));
    }

    // Query all rows
    EXPECT_TRUE(manager->executeQuery(id, "SELECT * FROM test", result));
    EXPECT_EQ(result.size(), numRows);

    // Complex query
    EXPECT_TRUE(manager->executeQuery(id,
        "SELECT name, COUNT(*) as count, AVG(value) as avg_value "
        "FROM test GROUP BY name ORDER BY count DESC LIMIT 10",
        result));

    // Disconnect
    EXPECT_TRUE(manager->disconnect(id));
}

// MongoDB Tests
// TEST_F(DatabaseManagerTest, MongoDBOperations) {
//     // Connect to MongoDB
//     nlohmann::json config = {
//         {"host", "localhost"},
//         {"port", 27017}
//     };
//     std::string id = manager->connect(DatabaseType::MONGODB, config);
//     GTEST_SKIP() << "MongoDB not available";
// }

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 