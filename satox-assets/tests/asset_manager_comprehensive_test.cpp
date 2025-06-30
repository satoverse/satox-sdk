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
#include <gmock/gmock.h>
#include "satox/assets/asset_manager.hpp"
#include "satox/assets/asset_types.hpp"
#include <nlohmann/json.hpp>
#include <thread>
#include <chrono>
#include <future>
#include <vector>
#include <memory>
#include <random>

using namespace satox::assets;
using namespace testing;
using json = nlohmann::json;

// Mock for blockchain interaction
class MockBlockchainInterface {
public:
    MOCK_METHOD(bool, validateAsset, (const std::string& asset_id), ());
    MOCK_METHOD(bool, validateTransfer, (const std::string& asset_id, const std::string& from, const std::string& to), ());
    MOCK_METHOD(bool, executeTransfer, (const std::string& asset_id, const std::string& from, const std::string& to, uint64_t amount), ());
    MOCK_METHOD(bool, executeMint, (const std::string& asset_id, uint64_t amount), ());
    MOCK_METHOD(bool, executeBurn, (const std::string& asset_id, uint64_t amount), ());
};

class AssetManagerComprehensiveTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test configuration
        config_ = {
            {"cache_size", 1000},
            {"storage_path", "/tmp/satox-assets"},
            {"enable_validation", true},
            {"max_assets_per_owner", 1000},
            {"max_metadata_size", 1024 * 1024}, // 1MB
            {"allowed_asset_types", {"TOKEN", "NFT", "COLLECTIBLE"}},
            {"restricted_asset_types", {"RESTRICTED_TOKEN"}}
        };

        // Initialize manager
        ASSERT_TRUE(manager.initialize(config_));
        
        // Create mock blockchain interface
        mock_blockchain_ = std::make_shared<MockBlockchainInterface>();
    }

    void TearDown() override {
        manager.shutdown();
    }

    AssetManager& manager = AssetManager::getInstance();
    json config_;
    std::shared_ptr<MockBlockchainInterface> mock_blockchain_;
    std::random_device rd_;
    std::mt19937 gen_{rd_()};
    std::uniform_int_distribution<> dis_{1, 1000000};

    std::string generateRandomAssetId() {
        return "ASSET_" + std::to_string(dis_(gen_));
    }

    std::string generateRandomAddress() {
        return "ADDR_" + std::to_string(dis_(gen_));
    }

    AssetMetadata createTestMetadata() {
        AssetMetadata metadata;
        metadata.name = "Test Asset";
        metadata.symbol = "TEST";
        metadata.type = AssetType::TOKEN;
        metadata.creator = generateRandomAddress();
        metadata.totalSupply = 1000000;
        metadata.metadata = {
            {"description", "Test asset description"},
            {"image", "ipfs://test-image-hash"},
            {"attributes", json::array()}
        };
        return metadata;
    }
};

// Basic Functionality Tests
TEST_F(AssetManagerComprehensiveTest, Initialization) {
    EXPECT_TRUE(manager.isInitialized());
    EXPECT_FALSE(manager.initialize(config_)); // Double initialization should fail
}

TEST_F(AssetManagerComprehensiveTest, Shutdown) {
    EXPECT_TRUE(manager.shutdown());
    EXPECT_FALSE(manager.isInitialized());
}

// Asset Creation Tests
TEST_F(AssetManagerComprehensiveTest, AssetCreation) {
    AssetMetadata metadata = createTestMetadata();
    std::string assetId;

    EXPECT_TRUE(manager.createAsset(metadata, assetId));
    EXPECT_FALSE(assetId.empty());

    Asset asset;
    EXPECT_TRUE(manager.getAsset(assetId, asset));
    EXPECT_EQ(asset.metadata.name, metadata.name);
    EXPECT_EQ(asset.metadata.symbol, metadata.symbol);
    EXPECT_EQ(asset.metadata.type, metadata.type);
    EXPECT_EQ(asset.metadata.creator, metadata.creator);
    EXPECT_EQ(asset.metadata.totalSupply, metadata.totalSupply);
}

TEST_F(AssetManagerComprehensiveTest, InvalidAssetCreation) {
    AssetMetadata metadata;
    std::string assetId;

    // Test missing required fields
    EXPECT_FALSE(manager.createAsset(metadata, assetId));

    // Test invalid asset type
    metadata = createTestMetadata();
    metadata.type = AssetType::UNKNOWN;
    EXPECT_FALSE(manager.createAsset(metadata, assetId));

    // Test invalid metadata size
    metadata = createTestMetadata();
    metadata.metadata = json::object();
    for (int i = 0; i < 1000000; ++i) {
        metadata.metadata["key" + std::to_string(i)] = "value" + std::to_string(i);
    }
    EXPECT_FALSE(manager.createAsset(metadata, assetId));
}

// Asset Update Tests
TEST_F(AssetManagerComprehensiveTest, AssetUpdate) {
    AssetMetadata metadata = createTestMetadata();
    std::string assetId;
    ASSERT_TRUE(manager.createAsset(metadata, assetId));

    // Update metadata
    metadata.name = "Updated Asset";
    metadata.metadata["description"] = "Updated description";
    EXPECT_TRUE(manager.updateAsset(assetId, metadata));

    Asset asset;
    EXPECT_TRUE(manager.getAsset(assetId, asset));
    EXPECT_EQ(asset.metadata.name, "Updated Asset");
    EXPECT_EQ(asset.metadata.metadata["description"], "Updated description");
}

// Asset Transfer Tests
TEST_F(AssetManagerComprehensiveTest, AssetTransfer) {
    AssetMetadata metadata = createTestMetadata();
    std::string assetId;
    ASSERT_TRUE(manager.createAsset(metadata, assetId));

    std::string from = metadata.creator;
    std::string to = generateRandomAddress();

    EXPECT_CALL(*mock_blockchain_, validateTransfer(assetId, from, to))
        .WillOnce(Return(true));
    EXPECT_CALL(*mock_blockchain_, executeTransfer(assetId, from, to, 100))
        .WillOnce(Return(true));

    EXPECT_TRUE(manager.transferAsset(assetId, from, to));
}

// Asset Lock/Unlock Tests
TEST_F(AssetManagerComprehensiveTest, AssetLockUnlock) {
    AssetMetadata metadata = createTestMetadata();
    std::string assetId;
    ASSERT_TRUE(manager.createAsset(metadata, assetId));

    EXPECT_TRUE(manager.lockAsset(assetId));
    
    // Try transfer while locked
    std::string from = metadata.creator;
    std::string to = generateRandomAddress();
    EXPECT_FALSE(manager.transferAsset(assetId, from, to));

    EXPECT_TRUE(manager.unlockAsset(assetId));
    
    // Transfer should work after unlock
    EXPECT_CALL(*mock_blockchain_, validateTransfer(assetId, from, to))
        .WillOnce(Return(true));
    EXPECT_CALL(*mock_blockchain_, executeTransfer(assetId, from, to, 100))
        .WillOnce(Return(true));
    EXPECT_TRUE(manager.transferAsset(assetId, from, to));
}

// Asset Mint/Burn Tests
TEST_F(AssetManagerComprehensiveTest, AssetMintBurn) {
    AssetMetadata metadata = createTestMetadata();
    std::string assetId;
    ASSERT_TRUE(manager.createAsset(metadata, assetId));

    EXPECT_CALL(*mock_blockchain_, executeMint(assetId, 1000))
        .WillOnce(Return(true));
    EXPECT_TRUE(manager.mintAsset(assetId, 1000));

    EXPECT_CALL(*mock_blockchain_, executeBurn(assetId, 500))
        .WillOnce(Return(true));
    EXPECT_TRUE(manager.burnAsset(assetId, 500));
}

// Asset History Tests
TEST_F(AssetManagerComprehensiveTest, AssetHistory) {
    AssetMetadata metadata = createTestMetadata();
    std::string assetId;
    ASSERT_TRUE(manager.createAsset(metadata, assetId));

    // Add history events
    json event1 = {
        {"type", "CREATION"},
        {"timestamp", std::chrono::system_clock::now().time_since_epoch().count()},
        {"details", "Asset created"}
    };
    EXPECT_TRUE(manager.addAssetHistory(assetId, event1));

    json event2 = {
        {"type", "TRANSFER"},
        {"timestamp", std::chrono::system_clock::now().time_since_epoch().count()},
        {"details", "Asset transferred"}
    };
    EXPECT_TRUE(manager.addAssetHistory(assetId, event2));

    // Get history
    std::vector<json> history;
    EXPECT_TRUE(manager.getAssetHistory(assetId, history));
    EXPECT_EQ(history.size(), 2);
    EXPECT_EQ(history[0]["type"], "CREATION");
    EXPECT_EQ(history[1]["type"], "TRANSFER");
}

// Asset Search Tests
TEST_F(AssetManagerComprehensiveTest, AssetSearch) {
    // Create multiple assets
    std::vector<std::string> assetIds;
    for (int i = 0; i < 5; ++i) {
        AssetMetadata metadata = createTestMetadata();
        metadata.name = "Test Asset " + std::to_string(i);
        std::string assetId;
        ASSERT_TRUE(manager.createAsset(metadata, assetId));
        assetIds.push_back(assetId);
    }

    // Search by name
    std::vector<Asset> results;
    EXPECT_TRUE(manager.searchAssets("Test Asset", results));
    EXPECT_EQ(results.size(), 5);

    // Search by specific name
    results.clear();
    EXPECT_TRUE(manager.searchAssets("Test Asset 1", results));
    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].metadata.name, "Test Asset 1");
}

// Performance Tests
TEST_F(AssetManagerComprehensiveTest, PerformanceTests) {
    // Test concurrent operations
    std::vector<std::future<bool>> futures;
    const int num_operations = 100;

    for (int i = 0; i < num_operations; ++i) {
        futures.push_back(std::async(std::launch::async, [&]() {
            AssetMetadata metadata = createTestMetadata();
            std::string assetId;
            return manager.createAsset(metadata, assetId);
        }));
    }

    for (auto& future : futures) {
        EXPECT_TRUE(future.get());
    }

    // Test search performance
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<Asset> results;
    for (int i = 0; i < 1000; ++i) {
        manager.searchAssets("Test", results);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Search operations should be fast (less than 1ms per operation)
    EXPECT_LT(duration.count() / 1000.0, 1.0);
}

// Error Handling Tests
TEST_F(AssetManagerComprehensiveTest, ErrorHandling) {
    // Test invalid asset ID
    Asset asset;
    EXPECT_FALSE(manager.getAsset("invalid_id", asset));

    // Test invalid transfer
    EXPECT_FALSE(manager.transferAsset("invalid_id", "from", "to"));

    // Test invalid mint/burn
    EXPECT_FALSE(manager.mintAsset("invalid_id", 1000));
    EXPECT_FALSE(manager.burnAsset("invalid_id", 1000));

    // Test invalid history
    std::vector<json> history;
    EXPECT_FALSE(manager.getAssetHistory("invalid_id", history));
}

// State Management Tests
TEST_F(AssetManagerComprehensiveTest, StateManagement) {
    // Test initialization state
    EXPECT_TRUE(manager.isInitialized());

    // Test shutdown
    manager.shutdown();
    EXPECT_FALSE(manager.isInitialized());

    // Test reinitialization
    EXPECT_TRUE(manager.initialize(config_));
    EXPECT_TRUE(manager.isInitialized());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 