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
#include "satox/core/database_manager.hpp"
#include <nlohmann/json.hpp>
#include <filesystem>
#include <chrono>
#include <thread>

using namespace satox::core;
using json = nlohmann::json;

class RocksDBTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Clean up any existing test database
        std::filesystem::remove_all("./test_rocksdb");
        
        // Initialize database manager
        DatabaseConfig config;
        config.type = DatabaseType::ROCKSDB;
        config.database = "./test_rocksdb";
        config.additionalOptions["create_if_missing"] = true;
        config.additionalOptions["enable_compression"] = true;
        config.additionalOptions["compression_type"] = "snappy";
        
        manager = &DatabaseManager::getInstance();
        EXPECT_TRUE(manager->initialize(config));
        
        // Connect to RocksDB
        connectionId = manager->connect(DatabaseType::ROCKSDB, {
            {"path", "./test_rocksdb"},
            {"create_if_missing", true}
        });
        EXPECT_FALSE(connectionId.empty());
    }

    void TearDown() override {
        if (!connectionId.empty()) {
            manager->disconnect(connectionId);
        }
        manager->shutdown();
        
        // Clean up test database
        std::filesystem::remove_all("./test_rocksdb");
    }

    DatabaseManager* manager;
    std::string connectionId;
};

TEST_F(RocksDBTest, BasicOperations) {
    // Test PUT operation
    json putQuery = {
        {"operation", "put"},
        {"key", "test_key"},
        {"value", "test_value"}
    };
    
    json result;
    EXPECT_TRUE(manager->executeQuery(connectionId, putQuery.dump(), result));
    EXPECT_TRUE(result["success"].get<bool>());
    EXPECT_EQ(result["affected_rows"].get<int>(), 1);

    // Test GET operation
    json getQuery = {
        {"operation", "get"},
        {"key", "test_key"}
    };
    
    result.clear();
    EXPECT_TRUE(manager->executeQuery(connectionId, getQuery.dump(), result));
    EXPECT_TRUE(result["success"].get<bool>());
    EXPECT_EQ(result["rows"][0]["key"].get<std::string>(), "test_key");
    EXPECT_EQ(result["rows"][0]["value"].get<std::string>(), "test_value");

    // Test DELETE operation
    json deleteQuery = {
        {"operation", "delete"},
        {"key", "test_key"}
    };
    
    result.clear();
    EXPECT_TRUE(manager->executeQuery(connectionId, deleteQuery.dump(), result));
    EXPECT_TRUE(result["success"].get<bool>());
    EXPECT_EQ(result["affected_rows"].get<int>(), 1);

    // Verify deletion
    result.clear();
    EXPECT_TRUE(manager->executeQuery(connectionId, getQuery.dump(), result));
    EXPECT_FALSE(result["success"].get<bool>());
    EXPECT_EQ(result["error"].get<std::string>(), "Key not found");
}

TEST_F(RocksDBTest, MultiOperations) {
    // Test PUT_MULTI operation
    json putMultiQuery = {
        {"operation", "put_multi"},
        {"key_values", {
            {"key1", "value1"},
            {"key2", "value2"},
            {"key3", "value3"}
        }}
    };
    
    json result;
    EXPECT_TRUE(manager->executeQuery(connectionId, putMultiQuery.dump(), result));
    EXPECT_TRUE(result["success"].get<bool>());
    EXPECT_EQ(result["affected_rows"].get<int>(), 3);

    // Test GET_MULTI operation
    json getMultiQuery = {
        {"operation", "get_multi"},
        {"keys", {"key1", "key2", "key3", "nonexistent"}}
    };
    
    result.clear();
    EXPECT_TRUE(manager->executeQuery(connectionId, getMultiQuery.dump(), result));
    EXPECT_TRUE(result["success"].get<bool>());
    EXPECT_EQ(result["rows"].size(), 4);
    
    // Check that existing keys are found
    bool foundKey1 = false, foundKey2 = false, foundKey3 = false;
    for (const auto& row : result["rows"]) {
        std::string key = row["key"].get<std::string>();
        if (key == "key1" && row["value"].get<std::string>() == "value1") foundKey1 = true;
        if (key == "key2" && row["value"].get<std::string>() == "value2") foundKey2 = true;
        if (key == "key3" && row["value"].get<std::string>() == "value3") foundKey3 = true;
    }
    EXPECT_TRUE(foundKey1);
    EXPECT_TRUE(foundKey2);
    EXPECT_TRUE(foundKey3);

    // Test DELETE_MULTI operation
    json deleteMultiQuery = {
        {"operation", "delete_multi"},
        {"keys", {"key1", "key2"}}
    };
    
    result.clear();
    EXPECT_TRUE(manager->executeQuery(connectionId, deleteMultiQuery.dump(), result));
    EXPECT_TRUE(result["success"].get<bool>());
    EXPECT_EQ(result["affected_rows"].get<int>(), 2);
}

TEST_F(RocksDBTest, ScanOperations) {
    // Insert test data
    json putMultiQuery = {
        {"operation", "put_multi"},
        {"key_values", {
            {"user:1", "{\"name\":\"Alice\"}"},
            {"user:2", "{\"name\":\"Bob\"}"},
            {"user:3", "{\"name\":\"Charlie\"}"},
            {"post:1", "{\"title\":\"Post 1\"}"},
            {"post:2", "{\"title\":\"Post 2\"}"}
        }}
    };
    
    json result;
    EXPECT_TRUE(manager->executeQuery(connectionId, putMultiQuery.dump(), result));

    // Test scan with prefix
    json scanQuery = {
        {"operation", "scan"},
        {"start_key", "user:"},
        {"end_key", "user:999"},
        {"limit", 10}
    };
    
    result.clear();
    EXPECT_TRUE(manager->executeQuery(connectionId, scanQuery.dump(), result));
    EXPECT_TRUE(result["success"].get<bool>());
    EXPECT_EQ(result["count"].get<int>(), 3);
    EXPECT_EQ(result["rows"].size(), 3);

    // Test scan with limit
    json scanLimitQuery = {
        {"operation", "scan"},
        {"start_key", ""},
        {"limit", 2}
    };
    
    result.clear();
    EXPECT_TRUE(manager->executeQuery(connectionId, scanLimitQuery.dump(), result));
    EXPECT_TRUE(result["success"].get<bool>());
    EXPECT_EQ(result["count"].get<int>(), 2);
    EXPECT_EQ(result["rows"].size(), 2);
}

TEST_F(RocksDBTest, ExistsOperation) {
    // Insert a key
    json putQuery = {
        {"operation", "put"},
        {"key", "test_exists"},
        {"value", "test_value"}
    };
    
    json result;
    EXPECT_TRUE(manager->executeQuery(connectionId, putQuery.dump(), result));

    // Test exists for existing key
    json existsQuery = {
        {"operation", "exists"},
        {"key", "test_exists"}
    };
    
    result.clear();
    EXPECT_TRUE(manager->executeQuery(connectionId, existsQuery.dump(), result));
    EXPECT_TRUE(result["success"].get<bool>());
    EXPECT_TRUE(result["exists"].get<bool>());

    // Test exists for non-existing key
    json notExistsQuery = {
        {"operation", "exists"},
        {"key", "non_existent_key"}
    };
    
    result.clear();
    EXPECT_TRUE(manager->executeQuery(connectionId, notExistsQuery.dump(), result));
    EXPECT_TRUE(result["success"].get<bool>());
    EXPECT_FALSE(result["exists"].get<bool>());
}

TEST_F(RocksDBTest, DatabaseMaintenance) {
    // Insert some data first
    json putMultiQuery = {
        {"operation", "put_multi"},
        {"key_values", {
            {"key1", "value1"},
            {"key2", "value2"},
            {"key3", "value3"}
        }}
    };
    
    json result;
    EXPECT_TRUE(manager->executeQuery(connectionId, putMultiQuery.dump(), result));

    // Test flush operation
    json flushQuery = {
        {"operation", "flush"}
    };
    
    result.clear();
    EXPECT_TRUE(manager->executeQuery(connectionId, flushQuery.dump(), result));
    EXPECT_TRUE(result["success"].get<bool>());

    // Test compact operation
    json compactQuery = {
        {"operation", "compact"}
    };
    
    result.clear();
    EXPECT_TRUE(manager->executeQuery(connectionId, compactQuery.dump(), result));
    EXPECT_TRUE(result["success"].get<bool>());
}

TEST_F(RocksDBTest, GetProperties) {
    // Test get database properties
    json propertyQuery = {
        {"operation", "get_property"},
        {"property", "rocksdb.stats"}
    };
    
    json result;
    EXPECT_TRUE(manager->executeQuery(connectionId, propertyQuery.dump(), result));
    EXPECT_TRUE(result["success"].get<bool>());
    EXPECT_EQ(result["property"].get<std::string>(), "rocksdb.stats");
    EXPECT_FALSE(result["value"].get<std::string>().empty());
}

TEST_F(RocksDBTest, Transactions) {
    // Test transaction with multiple operations
    std::vector<std::string> transactionQueries = {
        R"({"operation":"put","key":"tx_key1","value":"tx_value1"})",
        R"({"operation":"put","key":"tx_key2","value":"tx_value2"})",
        R"({"operation":"put","key":"tx_key3","value":"tx_value3"})"
    };
    
    json result;
    EXPECT_TRUE(manager->executeTransaction(connectionId, transactionQueries, result));
    EXPECT_TRUE(result["success"].get<bool>());
    EXPECT_EQ(result["affected_rows"].get<int>(), 3);

    // Verify all keys were inserted
    json getMultiQuery = {
        {"operation", "get_multi"},
        {"keys", {"tx_key1", "tx_key2", "tx_key3"}}
    };
    
    result.clear();
    EXPECT_TRUE(manager->executeQuery(connectionId, getMultiQuery.dump(), result));
    EXPECT_TRUE(result["success"].get<bool>());
    EXPECT_EQ(result["rows"].size(), 3);
}

TEST_F(RocksDBTest, PerformanceConfiguration) {
    // Test with high-performance configuration
    std::string highPerfConnectionId = manager->connect(DatabaseType::ROCKSDB, {
        {"path", "./test_rocksdb_high_perf"},
        {"create_if_missing", true},
        {"max_background_jobs", 8},
        {"write_buffer_size", 128 * 1024 * 1024}, // 128MB
        {"max_write_buffer_number", 4},
        {"target_file_size_base", 128 * 1024 * 1024}, // 128MB
        {"max_bytes_for_level_base", 512 * 1024 * 1024}, // 512MB
        {"enable_compression", true},
        {"compression_type", "lz4"}
    });
    
    EXPECT_FALSE(highPerfConnectionId.empty());

    // Test operations with high-performance config
    json putQuery = {
        {"operation", "put"},
        {"key", "perf_test"},
        {"value", "perf_value"}
    };
    
    json result;
    EXPECT_TRUE(manager->executeQuery(highPerfConnectionId, putQuery.dump(), result));
    EXPECT_TRUE(result["success"].get<bool>());

    // Cleanup
    manager->disconnect(highPerfConnectionId);
    std::filesystem::remove_all("./test_rocksdb_high_perf");
}

TEST_F(RocksDBTest, ErrorHandling) {
    // Test invalid operation
    json invalidQuery = {
        {"operation", "invalid_operation"},
        {"key", "test"}
    };
    
    json result;
    EXPECT_FALSE(manager->executeQuery(connectionId, invalidQuery.dump(), result));
    EXPECT_FALSE(result["success"].get<bool>());
    EXPECT_FALSE(result["error"].get<std::string>().empty());

    // Test invalid JSON
    EXPECT_FALSE(manager->executeQuery(connectionId, "invalid json", result));
    EXPECT_FALSE(result["success"].get<bool>());

    // Test missing required fields
    json incompleteQuery = {
        {"operation", "put"}
        // Missing key and value
    };
    
    result.clear();
    EXPECT_FALSE(manager->executeQuery(connectionId, incompleteQuery.dump(), result));
    EXPECT_FALSE(result["success"].get<bool>());
}

TEST_F(RocksDBTest, ConcurrentAccess) {
    const int numThreads = 10;
    const int operationsPerThread = 100;
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};
    std::atomic<int> failureCount{0};

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([this, i, operationsPerThread, &successCount, &failureCount]() {
            for (int j = 0; j < operationsPerThread; ++j) {
                std::string key = "thread_" + std::to_string(i) + "_key_" + std::to_string(j);
                std::string value = "value_" + std::to_string(i) + "_" + std::to_string(j);
                
                json putQuery = {
                    {"operation", "put"},
                    {"key", key},
                    {"value", value}
                };
                
                json result;
                if (manager->executeQuery(connectionId, putQuery.dump(), result)) {
                    successCount++;
                } else {
                    failureCount++;
                }
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(successCount.load(), numThreads * operationsPerThread);
    EXPECT_EQ(failureCount.load(), 0);
}

TEST_F(RocksDBTest, LargeDataHandling) {
    // Test with large values
    std::string largeValue(1024 * 1024, 'A'); // 1MB value
    
    json putQuery = {
        {"operation", "put"},
        {"key", "large_key"},
        {"value", largeValue}
    };
    
    json result;
    EXPECT_TRUE(manager->executeQuery(connectionId, putQuery.dump(), result));
    EXPECT_TRUE(result["success"].get<bool>());

    // Retrieve the large value
    json getQuery = {
        {"operation", "get"},
        {"key", "large_key"}
    };
    
    result.clear();
    EXPECT_TRUE(manager->executeQuery(connectionId, getQuery.dump(), result));
    EXPECT_TRUE(result["success"].get<bool>());
    EXPECT_EQ(result["rows"][0]["value"].get<std::string>(), largeValue);
}

TEST_F(RocksDBTest, CompressionTypes) {
    // Test different compression types
    std::vector<std::string> compressionTypes = {"snappy", "lz4", "zstd", "zlib"};
    
    for (const auto& compressionType : compressionTypes) {
        std::string testPath = "./test_rocksdb_" + compressionType;
        std::filesystem::remove_all(testPath);
        
        std::string testConnectionId = manager->connect(DatabaseType::ROCKSDB, {
            {"path", testPath},
            {"create_if_missing", true},
            {"enable_compression", true},
            {"compression_type", compressionType}
        });
        
        EXPECT_FALSE(testConnectionId.empty());

        // Test operations with this compression
        json putQuery = {
            {"operation", "put"},
            {"key", "compression_test"},
            {"value", "test_value_with_" + compressionType + "_compression"}
        };
        
        json result;
        EXPECT_TRUE(manager->executeQuery(testConnectionId, putQuery.dump(), result));
        EXPECT_TRUE(result["success"].get<bool>());

        // Cleanup
        manager->disconnect(testConnectionId);
        std::filesystem::remove_all(testPath);
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 