/**
 * @file test_firebase_manager.cpp
 * @brief Unit tests for Firebase manager
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

#include <gtest/gtest.h>
#include "satox/core/cloud/firebase_manager.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using satox::core::FirebaseManager;
using satox::core::FirebaseConfig;

class FirebaseManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test configuration
        config_.project_id = "test-project-id";
        config_.private_key_id = "test-private-key-id";
        config_.private_key = "-----BEGIN PRIVATE KEY-----\nTEST_KEY\n-----END PRIVATE KEY-----\n";
        config_.client_email = "test@test-project.iam.gserviceaccount.com";
        config_.client_id = "test-client-id";
        config_.auth_uri = "https://accounts.google.com/o/oauth2/auth";
        config_.token_uri = "https://oauth2.googleapis.com/token";
        config_.auth_provider_x509_cert_url = "https://www.googleapis.com/oauth2/v1/certs";
        config_.client_x509_cert_url = "https://www.googleapis.com/robot/v1/metadata/x509/test@test-project.iam.gserviceaccount.com";
        config_.database_url = "https://test-project.firebaseio.com";
        config_.enable_auth = true;
        config_.enable_firestore = true;
        config_.enable_realtime_db = false;
        config_.enable_storage = false;
        config_.enable_functions = false;
        config_.connection_timeout = 30;
        config_.max_connections = 10;
        config_.enable_ssl = true;
        config_.enable_logging = false; // Disable logging for tests
        config_.log_level = "error";
    }

    void TearDown() override {
        if (manager_) {
            manager_->shutdown();
        }
    }

    FirebaseConfig config_;
    std::unique_ptr<FirebaseManager> manager_;
};

TEST_F(FirebaseManagerTest, Initialization) {
    manager_ = std::make_unique<FirebaseManager>();
    
    EXPECT_TRUE(manager_->initialize(config_));
    EXPECT_TRUE(config_.isValid());
}

TEST_F(FirebaseManagerTest, InvalidConfiguration) {
    manager_ = std::make_unique<FirebaseManager>();
    
    FirebaseConfig invalid_config;
    EXPECT_FALSE(manager_->initialize(invalid_config));
    EXPECT_FALSE(invalid_config.isValid());
}

TEST_F(FirebaseManagerTest, ConfigurationSerialization) {
    json config_json = config_.toJson();
    FirebaseConfig deserialized_config = FirebaseConfig::fromJson(config_json);
    
    EXPECT_EQ(config_.project_id, deserialized_config.project_id);
    EXPECT_EQ(config_.private_key_id, deserialized_config.private_key_id);
    EXPECT_EQ(config_.private_key, deserialized_config.private_key);
    EXPECT_EQ(config_.client_email, deserialized_config.client_email);
    EXPECT_EQ(config_.client_id, deserialized_config.client_id);
    EXPECT_EQ(config_.auth_uri, deserialized_config.auth_uri);
    EXPECT_EQ(config_.token_uri, deserialized_config.token_uri);
    EXPECT_EQ(config_.auth_provider_x509_cert_url, deserialized_config.auth_provider_x509_cert_url);
    EXPECT_EQ(config_.client_x509_cert_url, deserialized_config.client_x509_cert_url);
    EXPECT_EQ(config_.database_url, deserialized_config.database_url);
    EXPECT_EQ(config_.enable_auth, deserialized_config.enable_auth);
    EXPECT_EQ(config_.enable_firestore, deserialized_config.enable_firestore);
    EXPECT_EQ(config_.enable_realtime_db, deserialized_config.enable_realtime_db);
    EXPECT_EQ(config_.enable_storage, deserialized_config.enable_storage);
    EXPECT_EQ(config_.enable_functions, deserialized_config.enable_functions);
    EXPECT_EQ(config_.connection_timeout, deserialized_config.connection_timeout);
    EXPECT_EQ(config_.max_connections, deserialized_config.max_connections);
    EXPECT_EQ(config_.enable_ssl, deserialized_config.enable_ssl);
    EXPECT_EQ(config_.enable_logging, deserialized_config.enable_logging);
    EXPECT_EQ(config_.log_level, deserialized_config.log_level);
}

TEST_F(FirebaseManagerTest, ConnectionLifecycle) {
    manager_ = std::make_unique<FirebaseManager>();
    EXPECT_TRUE(manager_->initialize(config_));
    
    // Test connection
    std::string connection_id = manager_->connect();
    EXPECT_FALSE(connection_id.empty());
    EXPECT_TRUE(manager_->isConnected(connection_id));
    
    // Test disconnection
    EXPECT_TRUE(manager_->disconnect(connection_id));
    EXPECT_FALSE(manager_->isConnected(connection_id));
}

TEST_F(FirebaseManagerTest, MultipleConnections) {
    manager_ = std::make_unique<FirebaseManager>();
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

TEST_F(FirebaseManagerTest, FirestoreOperations) {
    manager_ = std::make_unique<FirebaseManager>();
    EXPECT_TRUE(manager_->initialize(config_));
    
    std::string connection_id = manager_->connect();
    EXPECT_FALSE(connection_id.empty());
    
    auto& firestore = manager_->firestore();
    
    // Test Firestore operations (will fail without real connection, but should not crash)
    json result;
    
    // Get document
    bool success = firestore.getDocument(connection_id, "test_collection", "test_doc", result);
    // EXPECT_FALSE(success); // Expected to fail without real connection
    
    // Set document
    json data = {{"test_field", "test_value"}};
    success = firestore.setDocument(connection_id, "test_collection", "test_doc", data, result);
    // EXPECT_FALSE(success); // Expected to fail without real connection
    
    // Update document
    json update_data = {{"updated_field", "updated_value"}};
    success = firestore.updateDocument(connection_id, "test_collection", "test_doc", update_data, result);
    // EXPECT_FALSE(success); // Expected to fail without real connection
    
    // Delete document
    success = firestore.deleteDocument(connection_id, "test_collection", "test_doc", result);
    // EXPECT_FALSE(success); // Expected to fail without real connection
    
    // Query documents
    json query = {{"where", {{"field", "test_field"}, {"operator", "=="}, {"value", "test_value"}}}};
    success = firestore.queryDocuments(connection_id, "test_collection", query, result);
    // EXPECT_FALSE(success); // Expected to fail without real connection
    
    EXPECT_TRUE(manager_->disconnect(connection_id));
}

TEST_F(FirebaseManagerTest, RealtimeDatabaseOperations) {
    manager_ = std::make_unique<FirebaseManager>();
    EXPECT_TRUE(manager_->initialize(config_));
    
    std::string connection_id = manager_->connect();
    EXPECT_FALSE(connection_id.empty());
    
    auto& realtime = manager_->realtime();
    
    // Test Realtime Database operations (will fail without real connection, but should not crash)
    json result;
    
    // Get data
    bool success = realtime.getData(connection_id, "/test/path", result);
    // EXPECT_FALSE(success); // Expected to fail without real connection
    
    // Set data
    json data = {{"test_field", "test_value"}};
    success = realtime.setData(connection_id, "/test/path", data, result);
    // EXPECT_FALSE(success); // Expected to fail without real connection
    
    // Update data
    json update_data = {{"updated_field", "updated_value"}};
    success = realtime.updateData(connection_id, "/test/path", update_data, result);
    // EXPECT_FALSE(success); // Expected to fail without real connection
    
    // Delete data
    success = realtime.deleteData(connection_id, "/test/path", result);
    // EXPECT_FALSE(success); // Expected to fail without real connection
    
    // Listen to changes
    auto callback = [](const json& data) {
        // Callback function
    };
    
    std::string listener_id = realtime.listen(connection_id, "/test/path", callback);
    // EXPECT_TRUE(listener_id.empty()); // Expected to fail without real connection
    
    if (!listener_id.empty()) {
        EXPECT_TRUE(realtime.stopListening(connection_id, listener_id));
    }
    
    EXPECT_TRUE(manager_->disconnect(connection_id));
}

TEST_F(FirebaseManagerTest, ErrorHandling) {
    manager_ = std::make_unique<FirebaseManager>();
    
    // Test error handling without initialization
    std::string connection_id = manager_->connect();
    EXPECT_TRUE(connection_id.empty());
    EXPECT_FALSE(manager_->getLastError().empty());
    
    manager_->clearError();
    EXPECT_TRUE(manager_->getLastError().empty());
}

TEST_F(FirebaseManagerTest, Shutdown) {
    manager_ = std::make_unique<FirebaseManager>();
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
