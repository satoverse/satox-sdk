/**
 * @file firebase_example.cpp
 * @brief C++ example usage of Firebase integration with Satox SDK
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

#include "satox/core/database_manager.hpp"
#include "satox/core/cloud/firebase_config.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

using json = nlohmann::json;
using satox::core::DatabaseManager;
using satox::core::DatabaseType;
using satox::core::cloud::FirebaseConfig;

int main() {
    std::cout << "=== Satox SDK Firebase Integration Example (C++) ===" << std::endl;
    
    try {
        // Initialize database manager
        DatabaseManager db_manager;
        
        // Load Firebase configuration
        std::ifstream config_file("config/cloud/firebase_config.json");
        if (!config_file.is_open()) {
            std::cerr << "Error: Could not open firebase_config.json" << std::endl;
            return 1;
        }
        
        json config_json;
        config_file >> config_json;
        config_file.close();
        
        // Connect to Firebase
        std::string connection_id = db_manager.connect(DatabaseType::FIREBASE, config_json);
        if (connection_id.empty()) {
            std::cerr << "Error: Failed to connect to Firebase" << std::endl;
            std::cerr << "Last error: " << db_manager.getLastError() << std::endl;
            return 1;
        }
        
        std::cout << "Successfully connected to Firebase with ID: " << connection_id << std::endl;
        
        // Example: Create a collection
        json create_result;
        std::string create_collection_query = R"(
            CREATE COLLECTION IF NOT EXISTS users
        )";
        
        if (db_manager.executeQuery(connection_id, create_collection_query, create_result)) {
            std::cout << "Collection created successfully" << std::endl;
        } else {
            std::cout << "Error creating collection: " << create_result["error"] << std::endl;
        }
        
        // Example: Insert data
        json insert_result;
        std::string insert_query = R"(
            INSERT INTO users (username, email, created_at) 
            VALUES ('john_doe', 'john@example.com', NOW())
        )";
        
        if (db_manager.executeQuery(connection_id, insert_query, insert_result)) {
            std::cout << "Data inserted successfully" << std::endl;
        } else {
            std::cout << "Error inserting data: " << insert_result["error"] << std::endl;
        }
        
        // Example: Query data
        json query_result;
        std::string select_query = "SELECT * FROM users WHERE username = 'john_doe'";
        
        if (db_manager.executeQuery(connection_id, select_query, query_result)) {
            std::cout << "Query executed successfully" << std::endl;
            if (query_result.contains("rows") && query_result["rows"].is_array()) {
                std::cout << "Found " << query_result["rows"].size() << " users:" << std::endl;
                for (const auto& row : query_result["rows"]) {
                    std::cout << "  - Username: " << row["username"]
                              << ", Email: " << row["email"] << std::endl;
                }
            }
        } else {
            std::cout << "Error querying data: " << query_result["error"] << std::endl;
        }
        
        // Example: Update data
        json update_result;
        std::string update_query = R"(
            UPDATE users SET email = 'john.updated@example.com' WHERE username = 'john_doe'
        )";
        
        if (db_manager.executeQuery(connection_id, update_query, update_result)) {
            std::cout << "Data updated successfully" << std::endl;
        } else {
            std::cout << "Error updating data: " << update_result["error"] << std::endl;
        }
        
        // Example: Real-time features
        std::cout << "\n=== Real-time Features ===" << std::endl;
        std::cout << "Note: Real-time features require WebSocket support" << std::endl;
        std::cout << "This would typically involve:" << std::endl;
        std::cout << "1. Setting up WebSocket connection" << std::endl;
        std::cout << "2. Subscribing to collection changes" << std::endl;
        std::cout << "3. Handling real-time events" << std::endl;
        
        // Example: Authentication
        std::cout << "\n=== Authentication Features ===" << std::endl;
        std::cout << "Firebase provides built-in authentication:" << std::endl;
        std::cout << "1. Email/Password authentication" << std::endl;
        std::cout << "2. Google Sign-In" << std::endl;
        std::cout << "3. Facebook Sign-In" << std::endl;
        std::cout << "4. Anonymous authentication" << std::endl;
        
        // Disconnect
        if (db_manager.disconnect(connection_id)) {
            std::cout << "Successfully disconnected from Firebase" << std::endl;
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
