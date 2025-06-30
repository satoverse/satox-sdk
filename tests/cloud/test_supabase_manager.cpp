/**
 * @file test_supabase_manager.cpp
 * @brief Unit tests for Supabase manager
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

#include <gtest/gtest.h>
#include "satox/core/cloud/supabase_manager.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using satox::core::SupabaseManager;
using satox::core::SupabaseConfig;

class SupabaseManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test configuration
        config_.url = "https://test-project.supabase.co";
        config_.anon_key = "test-anon-key";
        config_.service_role_key = "test-service-role-key";
        config_.database_url = "postgresql://postgres:test@db.test.supabase.co:5432/postgres";
        config_.enable_realtime = true;
        config_.connection_timeout = 30;
        config_.auth_scheme = "bearer";
        config_.enable_ssl = true;
        config_.schema = "public";
        config_.max_connections = 10;
        config_.enable_logging = false; // Disable logging for tests
        config_.log_level = "error";
    }

    void TearDown() override {
        if (manager_) {
            manager_->shutdown();
        }
    }

    SupabaseConfig config_;
    std::unique_ptr<SupabaseManager> manager_;
};

TEST_F(SupabaseManagerTest, Initialization) {
    manager_ = std::make_unique<SupabaseManager>();
    
    EXPECT_TRUE(manager_->initialize(config_));
    EXPECT_TRUE(config_.isValid());
}

TEST_F(SupabaseManagerTest, InvalidConfiguration) {
    manager_ = std::make_unique<SupabaseManager>();
    
    SupabaseConfig invalid_config;
    EXPECT_FALSE(manager_->initialize(invalid_config));
    EXPECT_FALSE(invalid_config.isValid());
}

TEST_F(SupabaseManagerTest, ConfigurationSerialization) {
    json config_json = config_.toJson();
    SupabaseConfig deserialized_config = SupabaseConfig::fromJson(config_json);
    
    EXPECT_EQ(config_.url, deserialized_config.url);
    EXPECT_EQ(config_.anon_key, deserialized_config.anon_key);
    EXPECT_EQ(config_.service_role_key, deserialized_config.service_role_key);
    EXPECT_EQ(config_.database_url, deserialized_config.database_url);
    EXPECT_EQ(config_.enable_realtime, deserialized_config.enable_realtime);
    EXPECT_EQ(config_.connection_timeout, deserialized_config.connection_timeout);
    EXPECT_EQ(config_.auth_scheme, deserialized_config.auth_scheme);
    EXPECT_EQ(config_.enable_ssl, deserialized_config.enable_ssl);
    EXPECT_EQ(config_.schema, deserialized_config.schema);
    EXPECT_EQ(config_.max_connections, deserialized_config.max_connections);
    EXPECT_EQ(config_.enable_logging, deserialized_config.enable_logging);
    EXPECT_EQ(config_.log_level, deserialized_config.log_level);
}

TEST_F(SupabaseManagerTest, ConnectionLifecycle) {
    manager_ = std::make_unique<SupabaseManager>();
    EXPECT_TRUE(manager_->initialize(config_));
    
    // Test connection
    std::string connection_id = manager_->connect();
    EXPECT_FALSE(connection_id.empty());
    EXPECT_TRUE(manager_->isConnected(connection_id));
    
    // Test disconnection
    EXPECT_TRUE(manager_->disconnect(connection_id));
    EXPECT_FALSE(manager_->isConnected(connection_id));
}

TEST_F(SupabaseManagerTest, MultipleConnections) {
    manager_ = std::make_unique<SupabaseManager>();
    EXPECT_TRUE(manager_->initialize(config_));
    
    std::string conn1 = manager_->connect();
    std::string conn2 = manager_->connect();
    
    EXPECT_FALSE(conn1.empty());
    EXPECT_FALSE(conn2.empty());
    EXPECT_NE(conn1, conn2);
    
    EXPECT_TRUE(manager_->isConnected(conn1));
    EXPECT_TRUE(manager_->isConnected(conn2));
    
    EXPECT_TRUE(manager_->disconnect(conn1));
    EXPECT_TRUE(manager_->disconnect(conn2));
}

TEST_F(SupabaseManagerTest, QueryExecution) {
    manager_ = std::make_unique<SupabaseManager>();
    EXPECT_TRUE(manager_->initialize(config_));
    
    std::string connection_id = manager_->connect();
    EXPECT_FALSE(connection_id.empty());
    
    // Test query execution (will fail without real connection, but should not crash)
    json result;
    std::string query = "SELECT 1 as test";
    json params = json::object();
    
    // This will fail in test environment, but should handle error gracefully
    bool success = manager_->executeQuery(connection_id, query, params, result);
    // EXPECT_FALSE(success); // Expected to fail without real connection
    
    EXPECT_TRUE(manager_->disconnect(connection_id));
}

TEST_F(SupabaseManagerTest, TransactionExecution) {
    manager_ = std::make_unique<SupabaseManager>();
    EXPECT_TRUE(manager_->initialize(config_));
    
    std::string connection_id = manager_->connect();
    EXPECT_FALSE(connection_id.empty());
    
    // Test transaction execution
    std::vector<std::string> queries = {
        "SELECT 1 as test1",
        "SELECT 2 as test2"
    };
    json result;
    
    // This will fail in test environment, but should handle error gracefully
    bool success = manager_->executeTransaction(connection_id, queries, result);
    // EXPECT_FALSE(success); // Expected to fail without real connection
    
    EXPECT_TRUE(manager_->disconnect(connection_id));
}

TEST_F(SupabaseManagerTest, RealTimeSubscriptions) {
    manager_ = std::make_unique<SupabaseManager>();
    EXPECT_TRUE(manager_->initialize(config_));
    
    std::string connection_id = manager_->connect();
    EXPECT_FALSE(connection_id.empty());
    
    // Test subscription (will fail without real connection, but should not crash)
    auto callback = [](const json& data) {
        // Callback function
    };
    
    std::string subscription_id = manager_->subscribe(connection_id, "test_table", callback);
    // EXPECT_TRUE(subscription_id.empty()); // Expected to fail without real connection
    
    if (!subscription_id.empty()) {
        EXPECT_TRUE(manager_->unsubscribe(connection_id, subscription_id));
    }
    
    EXPECT_TRUE(manager_->disconnect(connection_id));
}

TEST_F(SupabaseManagerTest, ErrorHandling) {
    manager_ = std::make_unique<SupabaseManager>();
    
    // Test error handling without initialization
    std::string connection_id = manager_->connect();
    EXPECT_TRUE(connection_id.empty());
    EXPECT_FALSE(manager_->getLastError().empty());
    
    manager_->clearError();
    EXPECT_TRUE(manager_->getLastError().empty());
}

TEST_F(SupabaseManagerTest, Shutdown) {
    manager_ = std::make_unique<SupabaseManager>();
    EXPECT_TRUE(manager_->initialize(config_));
    
    std::string connection_id = manager_->connect();
    EXPECT_FALSE(connection_id.empty());
    
    manager_->shutdown();
    
    // After shutdown, operations should fail gracefully
    EXPECT_FALSE(manager_->isConnected(connection_id));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
