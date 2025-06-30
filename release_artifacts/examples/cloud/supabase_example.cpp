/**
 * @file supabase_example.cpp
 * @brief C++ example usage of Supabase integration with Satox SDK
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

#include "satox/core/database_manager.hpp"
#include "satox/core/cloud/supabase_config.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

using json = nlohmann::json;
using satox::core::DatabaseManager;
using satox::core::DatabaseType;
using satox::core::cloud::SupabaseConfig;

int main() {
    std::cout << "=== Satox SDK Supabase Integration Example (C++) ===" << std::endl;
    
    try {
        // Initialize database manager
        DatabaseManager db_manager;
        
        // Load Supabase configuration
        std::ifstream config_file("config/cloud/supabase_config.json");
        if (!config_file.is_open()) {
            std::cerr << "Error: Could not open supabase_config.json" << std::endl;
            return 1;
        }
        
        json config_json;
        config_file >> config_json;
        config_file.close();
        
        // Connect to Supabase
        std::string connection_id = db_manager.connect(DatabaseType::SUPABASE, config_json);
        if (connection_id.empty()) {
            std::cerr << "Error: Failed to connect to Supabase" << std::endl;
            std::cerr << "Last error: " << db_manager.getLastError() << std::endl;
            return 1;
        }
        
        std::cout << "Successfully connected to Supabase with ID: " << connection_id << std::endl;
        
        // Example: Create a table
        json create_result;
        std::string create_table_query = R"(
            CREATE TABLE IF NOT EXISTS users (
                id SERIAL PRIMARY KEY,
                username VARCHAR(255) UNIQUE NOT NULL,
                email VARCHAR(255) UNIQUE NOT NULL,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        )";
        
        if (db_manager.executeQuery(connection_id, create_table_query, create_result)) {
            std::cout << "Table created successfully" << std::endl;
        } else {
            std::cout << "Error creating table: " << create_result["error"] << std::endl;
        }
        
        // Example: Insert data
        json insert_result;
        std::string insert_query = R"(
            INSERT INTO users (username, email) 
            VALUES ('john_doe', 'john@example.com')
            ON CONFLICT (username) DO NOTHING
        )";
        
        if (db_manager.executeQuery(connection_id, insert_query, insert_result)) {
            std::cout << "Data inserted successfully" << std::endl;
        } else {
            std::cout << "Error inserting data: " << insert_result["error"] << std::endl;
        }
        
        // Example: Query data
        json query_result;
        std::string select_query = "SELECT * FROM users";
        
        if (db_manager.executeQuery(connection_id, select_query, query_result)) {
            std::cout << "Query executed successfully" << std::endl;
            if (query_result.contains("rows") && query_result["rows"].is_array()) {
                std::cout << "Found " << query_result["rows"].size() << " users:" << std::endl;
                for (const auto& row : query_result["rows"]) {
                    std::cout << "  - ID: " << row["id"] 
                              << ", Username: " << row["username"]
                              << ", Email: " << row["email"] << std::endl;
                }
            }
        } else {
            std::cout << "Error querying data: " << query_result["error"] << std::endl;
        }
        
        // Example: Transaction
        json transaction_result;
        std::vector<std::string> transaction_queries = {
            "INSERT INTO users (username, email) VALUES ('jane_doe', 'jane@example.com')",
            "UPDATE users SET email = 'jane.updated@example.com' WHERE username = 'jane_doe'"
        };
        
        if (db_manager.executeTransaction(connection_id, transaction_queries, transaction_result)) {
            std::cout << "Transaction executed successfully" << std::endl;
        } else {
            std::cout << "Error executing transaction: " << transaction_result["error"] << std::endl;
        }
        
        // Example: Real-time subscription (if supported)
        std::cout << "\n=== Real-time Features ===" << std::endl;
        std::cout << "Note: Real-time subscriptions require WebSocket support" << std::endl;
        std::cout << "This would typically involve:" << std::endl;
        std::cout << "1. Setting up WebSocket connection" << std::endl;
        std::cout << "2. Subscribing to table changes" << std::endl;
        std::cout << "3. Handling real-time events" << std::endl;
        
        // Disconnect
        if (db_manager.disconnect(connection_id)) {
            std::cout << "Successfully disconnected from Supabase" << std::endl;
        } else {
            std::cout << "Error disconnecting: " << db_manager.getLastError() << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Example completed successfully ===" << std::endl;
    return 0;
}
