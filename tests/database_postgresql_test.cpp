#include <gtest/gtest.h>
#include <satox/database/database_manager.hpp>
#include <nlohmann/json.hpp>
#include <thread>
#include <chrono>
#include <random>
#include <spdlog/spdlog.h>

using namespace satox::database;
using json = nlohmann::json;

class PostgreSQLDatabaseTest : public ::testing::Test {
protected:
    satox::database::DatabaseManager dbManager;
    std::string connectionId;
    
    void SetUp() override {
        // Initialize database manager
        satox::database::DatabaseConfig config;
        config.enableLogging = true;
        ASSERT_TRUE(dbManager.initialize(config));
        
        // PostgreSQL connection configuration
        json pgConfig = {
            {"host", "localhost"},
            {"port", 5432},
            {"database", "satox_test"},
            {"username", "postgres"},
            {"password", "password"},
            {"timeout", 30},
            {"enable_ssl", false},
            {"application_name", "satox-sdk-test"}
        };
        
        connectionId = dbManager.connect(DatabaseType::POSTGRESQL, pgConfig);
        ASSERT_FALSE(connectionId.empty());
        
        // Create test table
        json result;
        std::string createTable = R"(
            CREATE TABLE IF NOT EXISTS test_table (
                id SERIAL PRIMARY KEY,
                name VARCHAR(255) NOT NULL,
                age INTEGER,
                email VARCHAR(255),
                is_active BOOLEAN DEFAULT true,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                data JSONB
            )
        )";
        
        ASSERT_TRUE(dbManager.executeQuery(connectionId, createTable, result));
    }
    
    void TearDown() override {
        if (!connectionId.empty()) {
            // Clean up test table
            json result;
            dbManager.executeQuery(connectionId, "DROP TABLE IF EXISTS test_table", result);
            dbManager.disconnect(connectionId);
        }
        dbManager.shutdown();
    }
};

// Basic connection tests
TEST_F(PostgreSQLDatabaseTest, ConnectionTest) {
    EXPECT_FALSE(connectionId.empty());
    EXPECT_EQ(dbManager.getLastError(), "");
}

TEST_F(PostgreSQLDatabaseTest, InvalidConnectionTest) {
    json invalidConfig = {
        {"host", "invalid-host"},
        {"port", 5432},
        {"database", "nonexistent"},
        {"username", "invalid"},
        {"password", "invalid"}
    };
    
    std::string invalidId = dbManager.connect(DatabaseType::POSTGRESQL, invalidConfig);
    EXPECT_TRUE(invalidId.empty());
    EXPECT_FALSE(dbManager.getLastError().empty());
}

// Basic CRUD operations
TEST_F(PostgreSQLDatabaseTest, InsertTest) {
    json result;
    std::string insertQuery = R"(
        INSERT INTO test_table (name, age, email, is_active, data) 
        VALUES ('John Doe', 30, 'john@example.com', true, '{"key": "value"}')
    )";
    
    ASSERT_TRUE(dbManager.executeQuery(connectionId, insertQuery, result));
    EXPECT_TRUE(result["success"]);
    EXPECT_EQ(result["affected_rows"], 1);
    EXPECT_TRUE(result.contains("last_insert_id"));
}

TEST_F(PostgreSQLDatabaseTest, SelectTest) {
    // Insert test data
    json result;
    std::string insertQuery = R"(
        INSERT INTO test_table (name, age, email, is_active, data) 
        VALUES ('Jane Smith', 25, 'jane@example.com', true, '{"department": "engineering"}')
    )";
    ASSERT_TRUE(dbManager.executeQuery(connectionId, insertQuery, result));
    
    // Select data
    std::string selectQuery = "SELECT * FROM test_table WHERE name = 'Jane Smith'";
    ASSERT_TRUE(dbManager.executeQuery(connectionId, selectQuery, result));
    
    EXPECT_TRUE(result["success"]);
    EXPECT_EQ(result["affected_rows"], 1);
    EXPECT_EQ(result["rows"].size(), 1);
    
    json row = result["rows"][0];
    EXPECT_EQ(row["name"], "Jane Smith");
    EXPECT_EQ(row["age"], 25);
    EXPECT_EQ(row["email"], "jane@example.com");
    EXPECT_EQ(row["is_active"], true);
    EXPECT_EQ(row["data"]["department"], "engineering");
}

TEST_F(PostgreSQLDatabaseTest, UpdateTest) {
    // Insert test data
    json result;
    std::string insertQuery = R"(
        INSERT INTO test_table (name, age, email, is_active) 
        VALUES ('Bob Johnson', 35, 'bob@example.com', false)
    )";
    ASSERT_TRUE(dbManager.executeQuery(connectionId, insertQuery, result));
    
    // Update data
    std::string updateQuery = "UPDATE test_table SET age = 36, is_active = true WHERE name = 'Bob Johnson'";
    ASSERT_TRUE(dbManager.executeQuery(connectionId, updateQuery, result));
    
    EXPECT_TRUE(result["success"]);
    EXPECT_EQ(result["affected_rows"], 1);
    
    // Verify update
    std::string selectQuery = "SELECT * FROM test_table WHERE name = 'Bob Johnson'";
    ASSERT_TRUE(dbManager.executeQuery(connectionId, selectQuery, result));
    
    json row = result["rows"][0];
    EXPECT_EQ(row["age"], 36);
    EXPECT_EQ(row["is_active"], true);
}

TEST_F(PostgreSQLDatabaseTest, DeleteTest) {
    // Insert test data
    json result;
    std::string insertQuery = R"(
        INSERT INTO test_table (name, age, email) 
        VALUES ('Alice Brown', 28, 'alice@example.com')
    )";
    ASSERT_TRUE(dbManager.executeQuery(connectionId, insertQuery, result));
    
    // Delete data
    std::string deleteQuery = "DELETE FROM test_table WHERE name = 'Alice Brown'";
    ASSERT_TRUE(dbManager.executeQuery(connectionId, deleteQuery, result));
    
    EXPECT_TRUE(result["success"]);
    EXPECT_EQ(result["affected_rows"], 1);
    
    // Verify deletion
    std::string selectQuery = "SELECT * FROM test_table WHERE name = 'Alice Brown'";
    ASSERT_TRUE(dbManager.executeQuery(connectionId, selectQuery, result));
    
    EXPECT_EQ(result["affected_rows"], 0);
    EXPECT_EQ(result["rows"].size(), 0);
}

// Transaction tests
TEST_F(PostgreSQLDatabaseTest, TransactionSuccessTest) {
    std::vector<std::string> queries = {
        "INSERT INTO test_table (name, age, email) VALUES ('User1', 25, 'user1@example.com')",
        "INSERT INTO test_table (name, age, email) VALUES ('User2', 30, 'user2@example.com')",
        "UPDATE test_table SET age = 26 WHERE name = 'User1'"
    };
    
    json result;
    ASSERT_TRUE(dbManager.executeTransaction(connectionId, queries, result));
    
    EXPECT_TRUE(result["success"]);
    EXPECT_EQ(result["results"].size(), 3);
    
    // Verify all changes were committed
    json selectResult;
    std::string selectQuery = "SELECT COUNT(*) as count FROM test_table WHERE name IN ('User1', 'User2')";
    ASSERT_TRUE(dbManager.executeQuery(connectionId, selectQuery, selectResult));
    
    EXPECT_EQ(selectResult["rows"][0]["count"], 2);
}

TEST_F(PostgreSQLDatabaseTest, TransactionRollbackTest) {
    // Insert initial data
    json result;
    std::string insertQuery = "INSERT INTO test_table (name, age, email) VALUES ('Initial', 25, 'initial@example.com')";
    ASSERT_TRUE(dbManager.executeQuery(connectionId, insertQuery, result));
    
    // Transaction with invalid query (should rollback)
    std::vector<std::string> queries = {
        "INSERT INTO test_table (name, age, email) VALUES ('User3', 25, 'user3@example.com')",
        "INSERT INTO test_table (name, age, email) VALUES ('User4', 30, 'user4@example.com')",
        "INSERT INTO nonexistent_table (name) VALUES ('Invalid')" // This will fail
    };
    
    ASSERT_FALSE(dbManager.executeTransaction(connectionId, queries, result));
    EXPECT_FALSE(result["success"]);
    
    // Verify no changes were committed
    json selectResult;
    std::string selectQuery = "SELECT COUNT(*) as count FROM test_table WHERE name IN ('User3', 'User4')";
    ASSERT_TRUE(dbManager.executeQuery(connectionId, selectQuery, selectResult));
    
    EXPECT_EQ(selectResult["rows"][0]["count"], 0);
}

// Data type tests
TEST_F(PostgreSQLDatabaseTest, DataTypesTest) {
    json result;
    std::string insertQuery = R"(
        INSERT INTO test_table (name, age, email, is_active, data) 
        VALUES (
            'Type Test', 
            42, 
            'types@example.com', 
            false, 
            '{"nested": {"value": 123, "array": [1, 2, 3]}, "boolean": true, "null": null}'
        )
    )";
    
    ASSERT_TRUE(dbManager.executeQuery(connectionId, insertQuery, result));
    
    // Select and verify data types
    std::string selectQuery = "SELECT * FROM test_table WHERE name = 'Type Test'";
    ASSERT_TRUE(dbManager.executeQuery(connectionId, selectQuery, result));
    
    json row = result["rows"][0];
    EXPECT_EQ(row["name"], "Type Test");
    EXPECT_EQ(row["age"], 42);
    EXPECT_EQ(row["email"], "types@example.com");
    EXPECT_EQ(row["is_active"], false);
    
    // Verify JSON data
    EXPECT_EQ(row["data"]["nested"]["value"], 123);
    EXPECT_EQ(row["data"]["nested"]["array"][0], 1);
    EXPECT_EQ(row["data"]["nested"]["array"][1], 2);
    EXPECT_EQ(row["data"]["nested"]["array"][2], 3);
    EXPECT_EQ(row["data"]["boolean"], true);
    EXPECT_TRUE(row["data"]["null"].is_null());
}

// Error handling tests
TEST_F(PostgreSQLDatabaseTest, InvalidQueryTest) {
    json result;
    std::string invalidQuery = "SELECT * FROM nonexistent_table";
    
    ASSERT_FALSE(dbManager.executeQuery(connectionId, invalidQuery, result));
    EXPECT_FALSE(result["success"]);
    EXPECT_TRUE(result.contains("error"));
    EXPECT_FALSE(dbManager.getLastError().empty());
}

TEST_F(PostgreSQLDatabaseTest, SyntaxErrorTest) {
    json result;
    std::string syntaxError = "SELECT * FROM test_table WHERE";
    
    ASSERT_FALSE(dbManager.executeQuery(connectionId, syntaxError, result));
    EXPECT_FALSE(result["success"]);
    EXPECT_TRUE(result.contains("error"));
}

// Large data set tests
TEST_F(PostgreSQLDatabaseTest, LargeDataSetTest) {
    const int numRecords = 1000;
    
    // Insert large dataset
    for (int i = 0; i < numRecords; i++) {
        json result;
        std::string insertQuery = "INSERT INTO test_table (name, age, email) VALUES ('User" + 
                                 std::to_string(i) + "', " + std::to_string(20 + i % 50) + 
                                 ", 'user" + std::to_string(i) + "@example.com')";
        ASSERT_TRUE(dbManager.executeQuery(connectionId, insertQuery, result));
    }
    
    // Query large dataset
    json result;
    std::string selectQuery = "SELECT COUNT(*) as count FROM test_table";
    ASSERT_TRUE(dbManager.executeQuery(connectionId, selectQuery, result));
    
    EXPECT_GE(result["rows"][0]["count"], numRecords);
    
    // Test pagination
    std::string paginatedQuery = "SELECT * FROM test_table ORDER BY id LIMIT 10 OFFSET 100";
    ASSERT_TRUE(dbManager.executeQuery(connectionId, paginatedQuery, result));
    
    EXPECT_EQ(result["rows"].size(), 10);
}

// Concurrency tests
TEST_F(PostgreSQLDatabaseTest, ConcurrentAccessTest) {
    const int numThreads = 5;
    const int operationsPerThread = 100;
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};
    
    auto worker = [&](int threadId) {
        for (int i = 0; i < operationsPerThread; i++) {
            json result;
            std::string insertQuery = "INSERT INTO test_table (name, age, email) VALUES ('Thread" + 
                                     std::to_string(threadId) + "_" + std::to_string(i) + "', " + 
                                     std::to_string(threadId) + ", 'thread" + std::to_string(threadId) + 
                                     "_" + std::to_string(i) + "@example.com')";
            
            if (dbManager.executeQuery(connectionId, insertQuery, result)) {
                successCount++;
            }
        }
    };
    
    // Start concurrent threads
    for (int i = 0; i < numThreads; i++) {
        threads.emplace_back(worker, i);
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify all operations were successful
    EXPECT_EQ(successCount.load(), numThreads * operationsPerThread);
    
    // Verify total count
    json result;
    std::string countQuery = "SELECT COUNT(*) as count FROM test_table";
    ASSERT_TRUE(dbManager.executeQuery(connectionId, countQuery, result));
    
    EXPECT_GE(result["rows"][0]["count"], numThreads * operationsPerThread);
}

// Performance tests
TEST_F(PostgreSQLDatabaseTest, BatchInsertPerformanceTest) {
    const int numRecords = 1000;
    std::vector<std::string> queries;
    
    // Prepare batch insert queries
    for (int i = 0; i < numRecords; i++) {
        std::string insertQuery = "INSERT INTO test_table (name, age, email) VALUES ('Batch" + 
                                 std::to_string(i) + "', " + std::to_string(i % 100) + 
                                 ", 'batch" + std::to_string(i) + "@example.com')";
        queries.push_back(insertQuery);
    }
    
    // Execute batch transaction
    auto start = std::chrono::high_resolution_clock::now();
    json result;
    ASSERT_TRUE(dbManager.executeTransaction(connectionId, queries, result));
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_TRUE(result["success"]);
    EXPECT_EQ(result["results"].size(), numRecords);
    
    // Performance should be reasonable (less than 10 seconds for 1000 records)
    EXPECT_LT(duration.count(), 10000);
    
    spdlog::info("Batch insert of {} records took {} ms", numRecords, duration.count());
}

// Connection management tests
TEST_F(PostgreSQLDatabaseTest, ReconnectionTest) {
    // Disconnect and reconnect
    ASSERT_TRUE(dbManager.disconnect(connectionId));
    
    json pgConfig = {
        {"host", "localhost"},
        {"port", 5432},
        {"database", "satox_test"},
        {"username", "postgres"},
        {"password", "password"}
    };
    
    connectionId = dbManager.connect(DatabaseType::POSTGRESQL, pgConfig);
    ASSERT_FALSE(connectionId.empty());
    
    // Test query after reconnection
    json result;
    std::string testQuery = "SELECT 1 as test_value";
    ASSERT_TRUE(dbManager.executeQuery(connectionId, testQuery, result));
    
    EXPECT_TRUE(result["success"]);
    EXPECT_EQ(result["rows"][0]["test_value"], 1);
}

// SSL configuration test (if SSL is available)
TEST_F(PostgreSQLDatabaseTest, SSLConfigurationTest) {
    // Test with SSL enabled
    json sslConfig = {
        {"host", "localhost"},
        {"port", 5432},
        {"database", "satox_test"},
        {"username", "postgres"},
        {"password", "password"},
        {"enable_ssl", true}
    };
    
    std::string sslConnectionId = dbManager.connect(DatabaseType::POSTGRESQL, sslConfig);
    
    // SSL connection might fail if not configured, but shouldn't crash
    if (!sslConnectionId.empty()) {
        json result;
        std::string testQuery = "SELECT 1 as ssl_test";
        bool success = dbManager.executeQuery(sslConnectionId, testQuery, result);
        
        if (success) {
            EXPECT_TRUE(result["success"]);
            EXPECT_EQ(result["rows"][0]["ssl_test"], 1);
        }
        
        dbManager.disconnect(sslConnectionId);
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 