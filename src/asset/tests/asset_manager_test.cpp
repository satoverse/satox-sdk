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
#include "satox/asset/asset_manager.hpp"
#include <thread>
#include <chrono>
#include <random>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <nlohmann/json.hpp>

using namespace satox::asset;

class AssetManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test configuration
        config.name = "test_asset_manager";
        config.type = AssetType::TOKEN;
        config.maxAssets = 1000;
        config.timeout = 30;
        config.enableLogging = true;
        config.logPath = "logs/components/asset/";
        config.additionalConfig = nlohmann::json::object();
        // Initialize manager
        ASSERT_TRUE(manager.initialize(config));
    }

    void TearDown() override {
        if (manager.isInitialized()) {
            manager.shutdown();
        }
    }

    AssetConfig config;
    AssetManager& manager = AssetManager::getInstance();
};

// Basic Functionality Tests
TEST_F(AssetManagerTest, Initialization) {
    EXPECT_TRUE(manager.isInitialized());
    EXPECT_EQ(manager.getState(), AssetState::INITIALIZED);
    EXPECT_FALSE(manager.initialize(config)); // Double initialization should fail
}

TEST_F(AssetManagerTest, Shutdown) {
    manager.shutdown();
    EXPECT_FALSE(manager.isInitialized());
    EXPECT_EQ(manager.getState(), AssetState::SHUTDOWN);
}

TEST_F(AssetManagerTest, HealthCheck) {
    EXPECT_TRUE(manager.healthCheck());
    auto health = manager.getHealthStatus();
    EXPECT_TRUE(health["initialized"].get<bool>());
    EXPECT_EQ(health["state"].get<int>(), static_cast<int>(AssetState::INITIALIZED));
}

// Asset Creation Tests
TEST_F(AssetManagerTest, CreateValidAsset) {
    AssetCreationRequest request;
    request.name = "TestAsset";
    request.symbol = "TEST";
    request.amount = 1000000;
    request.reissuable = true;
    request.owner_address = "test_owner";

    EXPECT_TRUE(manager.createAsset(request));
    
    // Verify asset was created
    AssetMetadata metadata;
    EXPECT_TRUE(manager.getAssetMetadata("TestAsset", metadata));
    EXPECT_EQ(metadata.name, "TestAsset");
    EXPECT_EQ(metadata.symbol, "TEST");
    EXPECT_EQ(metadata.total_supply, 1000000);
    EXPECT_EQ(metadata.issuer, "test_owner");
}

TEST_F(AssetManagerTest, CreateDuplicateAsset) {
    AssetCreationRequest request;
    request.name = "TestAsset";
    request.symbol = "TEST";
    request.amount = 1000000;
    request.reissuable = true;
    request.owner_address = "test_owner";

    EXPECT_TRUE(manager.createAsset(request));
    EXPECT_FALSE(manager.createAsset(request)); // Duplicate should fail
}

TEST_F(AssetManagerTest, CreateInvalidAsset) {
    AssetCreationRequest request;
    // Missing required fields
    request.owner_address = "test_owner";
    // No name or symbol

    EXPECT_FALSE(manager.createAsset(request));
    EXPECT_FALSE(manager.getLastError().empty());
}

// Asset Transfer Tests
TEST_F(AssetManagerTest, TransferValidAsset) {
    // Create asset first
    AssetCreationRequest request;
    request.name = "TestAsset";
    request.symbol = "TEST";
    request.amount = 1000000;
    request.reissuable = true;
    request.owner_address = "test_owner";

    ASSERT_TRUE(manager.createAsset(request));

    // Transfer asset
    AssetTransferRequest transfer_request;
    transfer_request.asset_name = "TestAsset";
    transfer_request.from_address = "test_owner";
    transfer_request.to_address = "test_recipient";
    transfer_request.amount = 500000;
    transfer_request.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    EXPECT_TRUE(manager.transferAsset(transfer_request));
    
    // Verify balances
    uint64_t sender_balance, recipient_balance;
    EXPECT_TRUE(manager.getAssetBalance("test_owner", "TestAsset", sender_balance));
    EXPECT_TRUE(manager.getAssetBalance("test_recipient", "TestAsset", recipient_balance));
    EXPECT_EQ(sender_balance, 500000);
    EXPECT_EQ(recipient_balance, 500000);
}

TEST_F(AssetManagerTest, TransferInvalidAmount) {
    // Create asset first
    AssetCreationRequest request;
    request.name = "TestAsset";
    request.symbol = "TEST";
    request.amount = 1000000;
    request.reissuable = true;
    request.owner_address = "test_owner";

    ASSERT_TRUE(manager.createAsset(request));

    // Try to transfer more than available
    AssetTransferRequest transfer_request;
    transfer_request.asset_name = "TestAsset";
    transfer_request.from_address = "test_owner";
    transfer_request.to_address = "test_recipient";
    transfer_request.amount = 2000000;
    transfer_request.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    EXPECT_FALSE(manager.transferAsset(transfer_request));
}

TEST_F(AssetManagerTest, TransferNonExistentAsset) {
    AssetTransferRequest transfer_request;
    transfer_request.asset_name = "NonExistentAsset";
    transfer_request.from_address = "test_owner";
    transfer_request.to_address = "test_recipient";
    transfer_request.amount = 1000;
    transfer_request.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    EXPECT_FALSE(manager.transferAsset(transfer_request));
}

// Asset Reissue Tests
TEST_F(AssetManagerTest, ReissueValidAsset) {
    // Create reissuable asset
    AssetCreationRequest request;
    request.name = "TestAsset";
    request.symbol = "TEST";
    request.amount = 1000000;
    request.reissuable = true;
    request.owner_address = "test_owner";

    ASSERT_TRUE(manager.createAsset(request));

    // Reissue asset
    AssetReissueRequest reissue_request;
    reissue_request.asset_name = "TestAsset";
    reissue_request.owner_address = "test_owner";
    reissue_request.amount = 500000;
    reissue_request.reissuable = true;
    reissue_request.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    EXPECT_TRUE(manager.reissueAsset(reissue_request));
    
    // Verify total supply increased
    AssetMetadata metadata;
    EXPECT_TRUE(manager.getAssetMetadata("TestAsset", metadata));
    EXPECT_EQ(metadata.total_supply, 1500000);
}

TEST_F(AssetManagerTest, ReissueNonReissuableAsset) {
    // Create non-reissuable asset
    AssetCreationRequest request;
    request.name = "TestAsset";
    request.symbol = "TEST";
    request.amount = 1000000;
    request.reissuable = false;
    request.owner_address = "test_owner";

    ASSERT_TRUE(manager.createAsset(request));

    // Try to reissue
    AssetReissueRequest reissue_request;
    reissue_request.asset_name = "TestAsset";
    reissue_request.owner_address = "test_owner";
    reissue_request.amount = 500000;
    reissue_request.reissuable = true;
    reissue_request.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    EXPECT_FALSE(manager.reissueAsset(reissue_request));
}

// Asset Burn Tests
TEST_F(AssetManagerTest, BurnValidAsset) {
    // Create asset
    AssetCreationRequest request;
    request.name = "TestAsset";
    request.symbol = "TEST";
    request.amount = 1000000;
    request.reissuable = true;
    request.owner_address = "test_owner";

    ASSERT_TRUE(manager.createAsset(request));

    // Burn asset
    AssetBurnRequest burn_request;
    burn_request.asset_name = "TestAsset";
    burn_request.owner_address = "test_owner";
    burn_request.amount = 500000;
    burn_request.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    EXPECT_TRUE(manager.burnAsset(burn_request));
    
    // Verify total supply decreased
    AssetMetadata metadata;
    EXPECT_TRUE(manager.getAssetMetadata("TestAsset", metadata));
    EXPECT_EQ(metadata.total_supply, 500000);
}

TEST_F(AssetManagerTest, BurnInsufficientBalance) {
    // Create asset
    AssetCreationRequest request;
    request.name = "TestAsset";
    request.symbol = "TEST";
    request.amount = 1000000;
    request.reissuable = true;
    request.owner_address = "test_owner";

    ASSERT_TRUE(manager.createAsset(request));

    // Try to burn more than available
    AssetBurnRequest burn_request;
    burn_request.asset_name = "TestAsset";
    burn_request.owner_address = "test_owner";
    burn_request.amount = 2000000;
    burn_request.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    EXPECT_FALSE(manager.burnAsset(burn_request));
}

// Asset Freeze/Unfreeze Tests
TEST_F(AssetManagerTest, FreezeAndUnfreezeAsset) {
    // Create asset
    AssetCreationRequest request;
    request.name = "TestAsset";
    request.symbol = "TEST";
    request.amount = 1000000;
    request.reissuable = true;
    request.owner_address = "test_owner";

    ASSERT_TRUE(manager.createAsset(request));

    // Freeze asset
    EXPECT_TRUE(manager.freezeAsset("TestAsset", "test_owner"));
    
    AssetStatus status;
    EXPECT_TRUE(manager.getAssetStatus("TestAsset", status));
    EXPECT_EQ(status, AssetStatus::FROZEN);

    // Try to transfer frozen asset
    AssetTransferRequest transfer_request;
    transfer_request.asset_name = "TestAsset";
    transfer_request.from_address = "test_owner";
    transfer_request.to_address = "test_recipient";
    transfer_request.amount = 1000;
    transfer_request.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    EXPECT_FALSE(manager.transferAsset(transfer_request));

    // Unfreeze asset
    EXPECT_TRUE(manager.unfreezeAsset("TestAsset", "test_owner"));
    
    EXPECT_TRUE(manager.getAssetStatus("TestAsset", status));
    EXPECT_EQ(status, AssetStatus::ACTIVE);

    // Transfer should work now
    EXPECT_TRUE(manager.transferAsset(transfer_request));
}

// Asset Validation Tests
TEST_F(AssetManagerTest, ValidateValidAsset) {
    AssetMetadata metadata;
    metadata.name = "TestAsset";
    metadata.symbol = "TEST";
    metadata.total_supply = 1000000;
    metadata.decimals = 8;
    metadata.reissuable = true;
    metadata.issuer = "test_owner";

    AssetVerificationResult result;
    EXPECT_TRUE(manager.validateAssetMetadata(metadata, result));
    EXPECT_TRUE(result.valid);
}

TEST_F(AssetManagerTest, ValidateInvalidAsset) {
    AssetMetadata metadata;
    // Missing required fields
    metadata.decimals = 8;

    AssetVerificationResult result;
    EXPECT_FALSE(manager.validateAssetMetadata(metadata, result));
    EXPECT_FALSE(result.valid);
    EXPECT_FALSE(result.error_message.empty());
}

// Asset Query Tests
TEST_F(AssetManagerTest, GetAssetBalance) {
    // Create asset
    AssetCreationRequest request;
    request.name = "TestAsset";
    request.symbol = "TEST";
    request.amount = 1000000;
    request.reissuable = true;
    request.owner_address = "test_owner";

    ASSERT_TRUE(manager.createAsset(request));

    // Check balance
    uint64_t balance;
    EXPECT_TRUE(manager.getAssetBalance("test_owner", "TestAsset", balance));
    EXPECT_EQ(balance, 1000000);

    // Check non-existent address
    EXPECT_TRUE(manager.getAssetBalance("non_existent", "TestAsset", balance));
    EXPECT_EQ(balance, 0);
}

TEST_F(AssetManagerTest, GetAssetOwners) {
    // Create asset
    AssetCreationRequest request;
    request.name = "TestAsset";
    request.symbol = "TEST";
    request.amount = 1000000;
    request.reissuable = true;
    request.owner_address = "test_owner";

    ASSERT_TRUE(manager.createAsset(request));

    // Transfer some to another address
    AssetTransferRequest transfer_request;
    transfer_request.asset_name = "TestAsset";
    transfer_request.from_address = "test_owner";
    transfer_request.to_address = "test_recipient";
    transfer_request.amount = 500000;
    transfer_request.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    ASSERT_TRUE(manager.transferAsset(transfer_request));

    // Get owners
    std::vector<std::string> owners;
    EXPECT_TRUE(manager.getAssetOwners("TestAsset", owners));
    EXPECT_EQ(owners.size(), 2);
    EXPECT_TRUE(std::find(owners.begin(), owners.end(), "test_owner") != owners.end());
    EXPECT_TRUE(std::find(owners.begin(), owners.end(), "test_recipient") != owners.end());
}

TEST_F(AssetManagerTest, ListAssets) {
    // Create multiple assets
    AssetCreationRequest request1;
    request1.name = "Asset1";
    request1.symbol = "A1";
    request1.amount = 1000000;
    request1.reissuable = true;
    request1.owner_address = "test_owner";

    AssetCreationRequest request2;
    request2.name = "Asset2";
    request2.symbol = "A2";
    request2.amount = 2000000;
    request2.reissuable = false;
    request2.owner_address = "test_owner";

    ASSERT_TRUE(manager.createAsset(request1));
    ASSERT_TRUE(manager.createAsset(request2));

    // List assets
    auto assets = manager.listAssets();
    EXPECT_EQ(assets.size(), 2);
}

// Statistics Tests
TEST_F(AssetManagerTest, Statistics) {
    // Enable statistics
    EXPECT_TRUE(manager.enableStats(true));

    // Create and transfer asset
    AssetCreationRequest request;
    request.name = "TestAsset";
    request.symbol = "TEST";
    request.amount = 1000000;
    request.reissuable = true;
    request.owner_address = "test_owner";

    ASSERT_TRUE(manager.createAsset(request));

    AssetTransferRequest transfer_request;
    transfer_request.asset_name = "TestAsset";
    transfer_request.from_address = "test_owner";
    transfer_request.to_address = "test_recipient";
    transfer_request.amount = 500000;
    transfer_request.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    ASSERT_TRUE(manager.transferAsset(transfer_request));

    // Check statistics
    auto stats = manager.getStats();
    EXPECT_GT(stats.totalOperations, 0);
    EXPECT_GT(stats.successfulOperations, 0);
}

// Configuration Tests
TEST_F(AssetManagerTest, UpdateConfig) {
    AssetConfig new_config = config;
    new_config.maxAssets = 2000;
    new_config.timeout = 60;

    EXPECT_TRUE(manager.updateConfig(new_config));
    
    auto current_config = manager.getConfig();
    EXPECT_EQ(current_config.maxAssets, 2000);
    EXPECT_EQ(current_config.timeout, 60);
}

TEST_F(AssetManagerTest, ValidateConfig) {
    AssetConfig invalid_config;
    invalid_config.name = ""; // Invalid: empty name
    invalid_config.maxAssets = 0; // Invalid: zero max assets

    EXPECT_FALSE(manager.validateConfig(invalid_config));
}

// Error Handling Tests
TEST_F(AssetManagerTest, ErrorHandling) {
    // Try to create asset without initialization
    manager.shutdown();
    
    AssetCreationRequest request;
    request.name = "TestAsset";
    request.symbol = "TEST";
    request.amount = 1000000;
    request.reissuable = true;
    request.owner_address = "test_owner";

    EXPECT_FALSE(manager.createAsset(request));
    EXPECT_FALSE(manager.getLastError().empty());
    
    // Clear error
    manager.clearLastError();
    EXPECT_TRUE(manager.getLastError().empty());
}

// Callback Tests
TEST_F(AssetManagerTest, Callbacks) {
    bool asset_callback_called = false;
    bool error_callback_called = false;

    manager.registerAssetCallback([&](const std::string& event, bool success) {
        asset_callback_called = true;
    });

    manager.registerErrorCallback([&](const std::string& operation, const std::string& error) {
        error_callback_called = true;
    });

    // Create asset to trigger callback
    AssetCreationRequest request;
    request.name = "TestAsset";
    request.symbol = "TEST";
    request.amount = 1000000;
    request.reissuable = true;
    request.owner_address = "test_owner";

    manager.createAsset(request);

    // Callbacks should be called
    EXPECT_TRUE(asset_callback_called);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 