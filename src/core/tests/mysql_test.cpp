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
#include <chrono>
#include <thread>

using namespace satox::core;
using json = nlohmann::json;

class MySQLTest : public ::testing::Test {
protected:
    DatabaseManager& manager = DatabaseManager::getInstance();
    std::string connectionId;
    
    void SetUp() override {
        // Initialize database manager
        DatabaseConfig config;
        config.type = DatabaseType::MYSQL;
        config.host = "localhost";
        config.port = 3306;
        config.database = "satox_test";
        config.username = "satox_user";
        config.password = "satox_password";
        config.enableLogging = true;
        
        ASSERT_TRUE(manager.initialize(config));
        
        // Connect to MySQL
        json connConfig = {
            {"host", "localhost"},
            {"port", 3306},
            {"database", "satox_test"},
            {"username", "satox_user"},
            {"password", "satox_password"},
            {"timeout", 30},
            {"enable_ssl", false}
        };
        
        connectionId = manager.connect(DatabaseType::MYSQL, connConfig);
        ASSERT_FALSE(connectionId.empty()) << "Failed to connect to MySQL: " << manager.getLastError();
    }
    
    void TearDown() override {
        if (!connectionId.empty()) {
            manager.disconnect(connectionId);
        }
        manager.shutdown();
    }
};

TEST_F(MySQLTest, BasicConnection) {
    EXPECT_TRUE(manager.isConnected());
    EXPECT_FALSE(manager.getLastError().empty() && manager.getLastError() != "No error");
}

TEST_F(MySQLTest, CreateTable) {
    std::string createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS test_users (
            id INT AUTO_INCREMENT PRIMARY KEY,
            name VARCHAR(255) NOT NULL,
            email VARCHAR(255) UNIQUE NOT NULL,
            age INT,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    json result;
    EXPECT_TRUE(manager.executeQuery(connectionId, createTableQuery, result));
    EXPECT_TRUE(result["success"].get<bool>());
}

TEST_F(MySQLTest, InsertData) {
    // First create the table
    std::string createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS test_users (
            id INT AUTO_INCREMENT PRIMARY KEY,
            name VARCHAR(255) NOT NULL,
            email VARCHAR(255) UNIQUE NOT NULL,
            age INT,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    json result;
    EXPECT_TRUE(manager.executeQuery(connectionId, createTableQuery, result));
    
    // Insert data
    std::string insertQuery = R"(
        INSERT INTO test_users (name, email, age) VALUES 
        ('John Doe', 'john@example.com', 30),
        ('Jane Smith', 'jane@example.com', 25),
        ('Bob Johnson', 'bob@example.com', 35)
    )";
    
    EXPECT_TRUE(manager.executeQuery(connectionId, insertQuery, result));
    EXPECT_TRUE(result["success"].get<bool>());
    EXPECT_EQ(result["affected_rows"].get<int>(), 3);
    EXPECT_GT(result["last_insert_id"].get<int64_t>(), 0);
}

TEST_F(MySQLTest, SelectData) {
    // Setup: Create table and insert data
    std::string createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS test_users (
            id INT AUTO_INCREMENT PRIMARY KEY,
            name VARCHAR(255) NOT NULL,
            email VARCHAR(255) UNIQUE NOT NULL,
            age INT,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    json result;
    EXPECT_TRUE(manager.executeQuery(connectionId, createTableQuery, result));
    
    std::string insertQuery = R"(
        INSERT INTO test_users (name, email, age) VALUES 
        ('John Doe', 'john@example.com', 30),
        ('Jane Smith', 'jane@example.com', 25)
    )";
    
    EXPECT_TRUE(manager.executeQuery(connectionId, insertQuery, result));
    
    // Select data
    std::string selectQuery = "SELECT * FROM test_users ORDER BY id";
    EXPECT_TRUE(manager.executeQuery(connectionId, selectQuery, result));
    EXPECT_TRUE(result["success"].get<bool>());
    
    auto rows = result["rows"];
    EXPECT_EQ(rows.size(), 2);
    
    // Check first row
    EXPECT_EQ(rows[0]["name"].get<std::string>(), "John Doe");
    EXPECT_EQ(rows[0]["email"].get<std::string>(), "john@example.com");
    EXPECT_EQ(rows[0]["age"].get<int>(), 30);
    
    // Check second row
    EXPECT_EQ(rows[1]["name"].get<std::string>(), "Jane Smith");
    EXPECT_EQ(rows[1]["email"].get<std::string>(), "jane@example.com");
    EXPECT_EQ(rows[1]["age"].get<int>(), 25);
}

TEST_F(MySQLTest, UpdateData) {
    // Setup: Create table and insert data
    std::string createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS test_users (
            id INT AUTO_INCREMENT PRIMARY KEY,
            name VARCHAR(255) NOT NULL,
            email VARCHAR(255) UNIQUE NOT NULL,
            age INT,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    json result;
    EXPECT_TRUE(manager.executeQuery(connectionId, createTableQuery, result));
    
    std::string insertQuery = R"(
        INSERT INTO test_users (name, email, age) VALUES 
        ('John Doe', 'john@example.com', 30)
    )";
    
    EXPECT_TRUE(manager.executeQuery(connectionId, insertQuery, result));
    int64_t userId = result["last_insert_id"].get<int64_t>();
    
    // Update data
    std::string updateQuery = "UPDATE test_users SET age = 31 WHERE id = " + std::to_string(userId);
    EXPECT_TRUE(manager.executeQuery(connectionId, updateQuery, result));
    EXPECT_TRUE(result["success"].get<bool>());
    EXPECT_EQ(result["affected_rows"].get<int>(), 1);
    
    // Verify update
    std::string selectQuery = "SELECT * FROM test_users WHERE id = " + std::to_string(userId);
    EXPECT_TRUE(manager.executeQuery(connectionId, selectQuery, result));
    EXPECT_TRUE(result["success"].get<bool>());
    
    auto rows = result["rows"];
    EXPECT_EQ(rows.size(), 1);
    EXPECT_EQ(rows[0]["age"].get<int>(), 31);
}

TEST_F(MySQLTest, DeleteData) {
    // Setup: Create table and insert data
    std::string createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS test_users (
            id INT AUTO_INCREMENT PRIMARY KEY,
            name VARCHAR(255) NOT NULL,
            email VARCHAR(255) UNIQUE NOT NULL,
            age INT,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    json result;
    EXPECT_TRUE(manager.executeQuery(connectionId, createTableQuery, result));
    
    std::string insertQuery = R"(
        INSERT INTO test_users (name, email, age) VALUES 
        ('John Doe', 'john@example.com', 30),
        ('Jane Smith', 'jane@example.com', 25)
    )";
    
    EXPECT_TRUE(manager.executeQuery(connectionId, insertQuery, result));
    
    // Delete data
    std::string deleteQuery = "DELETE FROM test_users WHERE name = 'John Doe'";
    EXPECT_TRUE(manager.executeQuery(connectionId, deleteQuery, result));
    EXPECT_TRUE(result["success"].get<bool>());
    EXPECT_EQ(result["affected_rows"].get<int>(), 1);
    
    // Verify deletion
    std::string selectQuery = "SELECT * FROM test_users";
    EXPECT_TRUE(manager.executeQuery(connectionId, selectQuery, result));
    EXPECT_TRUE(result["success"].get<bool>());
    
    auto rows = result["rows"];
    EXPECT_EQ(rows.size(), 1);
    EXPECT_EQ(rows[0]["name"].get<std::string>(), "Jane Smith");
}

TEST_F(MySQLTest, TransactionSupport) {
    // Create table
    std::string createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS test_accounts (
            id INT AUTO_INCREMENT PRIMARY KEY,
            account_number VARCHAR(50) UNIQUE NOT NULL,
            balance DECIMAL(10,2) DEFAULT 0.00,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    json result;
    EXPECT_TRUE(manager.executeQuery(connectionId, createTableQuery, result));
    
    // Insert initial data
    std::string insertQuery = R"(
        INSERT INTO test_accounts (account_number, balance) VALUES 
        ('ACC001', 1000.00),
        ('ACC002', 500.00)
    )";
    
    EXPECT_TRUE(manager.executeQuery(connectionId, insertQuery, result));
    
    // Execute transaction
    std::vector<std::string> transactionQueries = {
        "UPDATE test_accounts SET balance = balance - 100 WHERE account_number = 'ACC001'",
        "UPDATE test_accounts SET balance = balance + 100 WHERE account_number = 'ACC002'"
    };
    
    EXPECT_TRUE(manager.executeTransaction(connectionId, transactionQueries, result));
    EXPECT_TRUE(result["success"].get<bool>());
    
    // Verify transaction results
    auto results = result["results"];
    EXPECT_EQ(results.size(), 2);
    EXPECT_EQ(results[0]["affected_rows"].get<int>(), 1);
    EXPECT_EQ(results[1]["affected_rows"].get<int>(), 1);
    
    // Verify final balances
    std::string selectQuery = "SELECT * FROM test_accounts ORDER BY account_number";
    EXPECT_TRUE(manager.executeQuery(connectionId, selectQuery, result));
    EXPECT_TRUE(result["success"].get<bool>());
    
    auto rows = result["rows"];
    EXPECT_EQ(rows.size(), 2);
    EXPECT_DOUBLE_EQ(rows[0]["balance"].get<double>(), 900.00);
    EXPECT_DOUBLE_EQ(rows[1]["balance"].get<double>(), 600.00);
}

TEST_F(MySQLTest, TransactionRollback) {
    // Create table
    std::string createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS test_accounts (
            id INT AUTO_INCREMENT PRIMARY KEY,
            account_number VARCHAR(50) UNIQUE NOT NULL,
            balance DECIMAL(10,2) DEFAULT 0.00,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    json result;
    EXPECT_TRUE(manager.executeQuery(connectionId, createTableQuery, result));
    
    // Insert initial data
    std::string insertQuery = R"(
        INSERT INTO test_accounts (account_number, balance) VALUES 
        ('ACC001', 1000.00)
    )";
    
    EXPECT_TRUE(manager.executeQuery(connectionId, insertQuery, result));
    
    // Execute transaction that will fail (violating unique constraint)
    std::vector<std::string> transactionQueries = {
        "UPDATE test_accounts SET balance = balance - 100 WHERE account_number = 'ACC001'",
        "INSERT INTO test_accounts (account_number, balance) VALUES ('ACC001', 500.00)" // This will fail
    };
    
    EXPECT_FALSE(manager.executeTransaction(connectionId, transactionQueries, result));
    EXPECT_FALSE(result["success"].get<bool>());
    
    // Verify rollback - balance should be unchanged
    std::string selectQuery = "SELECT * FROM test_accounts WHERE account_number = 'ACC001'";
    EXPECT_TRUE(manager.executeQuery(connectionId, selectQuery, result));
    EXPECT_TRUE(result["success"].get<bool>());
    
    auto rows = result["rows"];
    EXPECT_EQ(rows.size(), 1);
    EXPECT_DOUBLE_EQ(rows[0]["balance"].get<double>(), 1000.00);
}

TEST_F(MySQLTest, DataTypes) {
    // Create table with various data types
    std::string createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS test_data_types (
            id INT AUTO_INCREMENT PRIMARY KEY,
            tiny_int_field TINYINT,
            small_int_field SMALLINT,
            int_field INT,
            big_int_field BIGINT,
            float_field FLOAT,
            double_field DOUBLE,
            decimal_field DECIMAL(10,2),
            bit_field BIT(1),
            varchar_field VARCHAR(255),
            text_field TEXT,
            date_field DATE,
            datetime_field DATETIME,
            timestamp_field TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    json result;
    EXPECT_TRUE(manager.executeQuery(connectionId, createTableQuery, result));
    
    // Insert data with various types
    std::string insertQuery = R"(
        INSERT INTO test_data_types (
            tiny_int_field, small_int_field, int_field, big_int_field,
            float_field, double_field, decimal_field, bit_field,
            varchar_field, text_field, date_field, datetime_field
        ) VALUES (
            127, 32767, 2147483647, 9223372036854775807,
            3.14159, 2.718281828, 123.45, 1,
            'Test String', 'Long text content', '2023-12-25', '2023-12-25 15:30:45'
        )
    )";
    
    EXPECT_TRUE(manager.executeQuery(connectionId, insertQuery, result));
    EXPECT_TRUE(result["success"].get<bool>());
    
    // Select and verify data types
    std::string selectQuery = "SELECT * FROM test_data_types WHERE id = " + std::to_string(result["last_insert_id"].get<int64_t>());
    EXPECT_TRUE(manager.executeQuery(connectionId, selectQuery, result));
    EXPECT_TRUE(result["success"].get<bool>());
    
    auto rows = result["rows"];
    EXPECT_EQ(rows.size(), 1);
    
    auto row = rows[0];
    EXPECT_EQ(row["tiny_int_field"].get<int>(), 127);
    EXPECT_EQ(row["small_int_field"].get<int>(), 32767);
    EXPECT_EQ(row["int_field"].get<int>(), 2147483647);
    EXPECT_EQ(row["big_int_field"].get<int64_t>(), 9223372036854775807LL);
    EXPECT_FLOAT_EQ(row["float_field"].get<double>(), 3.14159f);
    EXPECT_DOUBLE_EQ(row["double_field"].get<double>(), 2.718281828);
    EXPECT_DOUBLE_EQ(row["decimal_field"].get<double>(), 123.45);
    EXPECT_EQ(row["bit_field"].get<bool>(), true);
    EXPECT_EQ(row["varchar_field"].get<std::string>(), "Test String");
    EXPECT_EQ(row["text_field"].get<std::string>(), "Long text content");
    EXPECT_EQ(row["date_field"].get<std::string>(), "2023-12-25");
    EXPECT_EQ(row["datetime_field"].get<std::string>(), "2023-12-25 15:30:45");
}

TEST_F(MySQLTest, ConnectionOptions) {
    // Test connection with SSL enabled
    json sslConfig = {
        {"host", "localhost"},
        {"port", 3306},
        {"database", "satox_test"},
        {"username", "satox_user"},
        {"password", "satox_password"},
        {"timeout", 30},
        {"enable_ssl", true}
    };
    
    std::string sslConnectionId = manager.connect(DatabaseType::MYSQL, sslConfig);
    if (!sslConnectionId.empty()) {
        EXPECT_TRUE(manager.isConnected());
        manager.disconnect(sslConnectionId);
    }
    
    // Test connection with custom timeouts
    json timeoutConfig = {
        {"host", "localhost"},
        {"port", 3306},
        {"database", "satox_test"},
        {"username", "satox_user"},
        {"password", "satox_password"},
        {"timeout", 60},
        {"read_timeout", 30},
        {"write_timeout", 30},
        {"enable_ssl", false}
    };
    
    std::string timeoutConnectionId = manager.connect(DatabaseType::MYSQL, timeoutConfig);
    EXPECT_FALSE(timeoutConnectionId.empty()) << "Failed to connect with custom timeouts: " << manager.getLastError();
    
    if (!timeoutConnectionId.empty()) {
        manager.disconnect(timeoutConnectionId);
    }
}

TEST_F(MySQLTest, ErrorHandling) {
    // Test invalid query
    std::string invalidQuery = "SELECT * FROM non_existent_table";
    json result;
    EXPECT_FALSE(manager.executeQuery(connectionId, invalidQuery, result));
    EXPECT_FALSE(result["success"].get<bool>());
    EXPECT_FALSE(manager.getLastError().empty());
    
    // Test invalid table creation
    std::string invalidCreateQuery = "CREATE TABLE test_users (invalid_column_type INVALID_TYPE)";
    EXPECT_FALSE(manager.executeQuery(connectionId, invalidCreateQuery, result));
    EXPECT_FALSE(result["success"].get<bool>());
    EXPECT_FALSE(manager.getLastError().empty());
    
    // Test invalid transaction
    std::vector<std::string> invalidTransaction = {
        "SELECT * FROM non_existent_table",
        "INSERT INTO test_users (name) VALUES ('test')"
    };
    
    EXPECT_FALSE(manager.executeTransaction(connectionId, invalidTransaction, result));
    EXPECT_FALSE(result["success"].get<bool>());
    EXPECT_FALSE(manager.getLastError().empty());
}

TEST_F(MySQLTest, ConcurrentAccess) {
    // Create table
    std::string createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS test_concurrent (
            id INT AUTO_INCREMENT PRIMARY KEY,
            value INT NOT NULL,
            updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
        )
    )";
    
    json result;
    EXPECT_TRUE(manager.executeQuery(connectionId, createTableQuery, result));
    
    // Insert initial data
    std::string insertQuery = "INSERT INTO test_concurrent (value) VALUES (0)";
    EXPECT_TRUE(manager.executeQuery(connectionId, insertQuery, result));
    int64_t recordId = result["last_insert_id"].get<int64_t>();
    
    // Simulate concurrent updates
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&, i]() {
            json threadResult;
            std::string updateQuery = "UPDATE test_concurrent SET value = value + 1 WHERE id = " + std::to_string(recordId);
            if (manager.executeQuery(connectionId, updateQuery, threadResult)) {
                successCount++;
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify final value
    std::string selectQuery = "SELECT * FROM test_concurrent WHERE id = " + std::to_string(recordId);
    EXPECT_TRUE(manager.executeQuery(connectionId, selectQuery, result));
    EXPECT_TRUE(result["success"].get<bool>());
    
    auto rows = result["rows"];
    EXPECT_EQ(rows.size(), 1);
    EXPECT_EQ(rows[0]["value"].get<int>(), 10);
    EXPECT_EQ(successCount.load(), 10);
}

TEST_F(MySQLTest, LargeDataSet) {
    // Create table for large dataset
    std::string createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS test_large_dataset (
            id INT AUTO_INCREMENT PRIMARY KEY,
            name VARCHAR(255) NOT NULL,
            email VARCHAR(255) NOT NULL,
            data TEXT,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    json result;
    EXPECT_TRUE(manager.executeQuery(connectionId, createTableQuery, result));
    
    // Insert large dataset
    std::string insertQuery = "INSERT INTO test_large_dataset (name, email, data) VALUES ";
    std::vector<std::string> values;
    
    for (int i = 1; i <= 1000; ++i) {
        values.push_back("('User" + std::to_string(i) + "', 'user" + std::to_string(i) + "@example.com', 'Data for user " + std::to_string(i) + "')");
    }
    
    insertQuery += values[0];
    for (size_t i = 1; i < values.size(); ++i) {
        insertQuery += ", " + values[i];
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    EXPECT_TRUE(manager.executeQuery(connectionId, insertQuery, result));
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_TRUE(result["success"].get<bool>());
    EXPECT_EQ(result["affected_rows"].get<int>(), 1000);
    
    // Query large dataset
    std::string selectQuery = "SELECT * FROM test_large_dataset WHERE id <= 100 ORDER BY id";
    start = std::chrono::high_resolution_clock::now();
    EXPECT_TRUE(manager.executeQuery(connectionId, selectQuery, result));
    end = std::chrono::high_resolution_clock::now();
    
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_TRUE(result["success"].get<bool>());
    
    auto rows = result["rows"];
    EXPECT_EQ(rows.size(), 100);
    
    // Verify first and last row
    EXPECT_EQ(rows[0]["name"].get<std::string>(), "User1");
    EXPECT_EQ(rows[99]["name"].get<std::string>(), "User100");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 