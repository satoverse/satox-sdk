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
#include "satox/assets/asset_manager.hpp"
#include "satox/assets/asset_types.hpp"
#include <nlohmann/json.hpp>
#include <thread>
#include <vector>

using namespace satox::assets;

class AssetManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager.shutdown();  // Ensure clean state
        EXPECT_TRUE(manager.initialize(config_));
        config_ = {
            {"cache_size", 1000},
            {"storage_path", "/tmp/satox-assets"}
        };
        manager.shutdown();  // Ensure clean state
        EXPECT_TRUE(manager.initialize(config_));
    }

    void TearDown() override {
        manager.shutdown();
    }

    AssetManager& manager = AssetManager::getInstance();
    nlohmann::json config_;
};

TEST_F(AssetManagerTest, SingletonInstance) {
    auto& instance1 = AssetManager::getInstance();
    auto& instance2 = AssetManager::getInstance();
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(AssetManagerTest, Initialization) {
    manager.shutdown();  // Ensure clean state
    EXPECT_TRUE(manager.initialize(config_));
}

TEST_F(AssetManagerTest, DoubleInitialization) {
    EXPECT_FALSE(manager.initialize(config_));
    EXPECT_EQ(manager.getLastError(), "AssetManager already initialized");
}

TEST_F(AssetManagerTest, AssetCreation) {
    AssetMetadata metadata;
    metadata.name = "Test Asset";
    metadata.symbol = "TEST";
    metadata.type = AssetType::TOKEN;
    metadata.creator = "test_user";
    metadata.totalSupply = 1000000;

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

TEST_F(AssetManagerTest, AssetUpdate) {
    AssetMetadata metadata;
    metadata.name = "Test Asset";
    metadata.symbol = "TEST";
    metadata.creator = "test_creator";
    metadata.owner = "test_owner";
    metadata.totalSupply = 1000;

    std::string assetId;
    EXPECT_TRUE(manager.createAsset(metadata, assetId));

    metadata.name = "Updated Asset";
    EXPECT_TRUE(manager.updateAsset(assetId, metadata));

    Asset asset;
    EXPECT_TRUE(manager.getAsset(assetId, asset));
    EXPECT_EQ(asset.metadata.name, "Updated Asset");
}

TEST_F(AssetManagerTest, AssetDeletion) {
    AssetMetadata metadata;
    metadata.name = "Test Asset";
    metadata.symbol = "TEST";
    metadata.creator = "test_creator";
    metadata.owner = "test_owner";
    metadata.totalSupply = 1000;

    std::string assetId;
    EXPECT_TRUE(manager.createAsset(metadata, assetId));
    EXPECT_TRUE(manager.deleteAsset(assetId));

    Asset asset;
    EXPECT_FALSE(manager.getAsset(assetId, asset));
}

TEST_F(AssetManagerTest, AssetTransfer) {
    AssetMetadata metadata;
    metadata.name = "Test Asset";
    metadata.symbol = "TEST";
    metadata.creator = "test_creator";
    metadata.owner = "test_owner";
    metadata.totalSupply = 1000;

    std::string assetId;
    EXPECT_TRUE(manager.createAsset(metadata, assetId));

    EXPECT_TRUE(manager.transferAsset(assetId, "test_owner", "new_owner"));

    Asset asset;
    EXPECT_TRUE(manager.getAsset(assetId, asset));
    EXPECT_EQ(asset.metadata.owner, "new_owner");
}

TEST_F(AssetManagerTest, AssetLocking) {
    AssetMetadata metadata;
    metadata.name = "Test Asset";
    metadata.symbol = "TEST";
    metadata.creator = "test_creator";
    metadata.owner = "test_owner";
    metadata.totalSupply = 1000;

    std::string assetId;
    EXPECT_TRUE(manager.createAsset(metadata, assetId));

    EXPECT_TRUE(manager.lockAsset(assetId));
    EXPECT_FALSE(manager.transferAsset(assetId, "test_owner", "new_owner"));
    EXPECT_TRUE(manager.unlockAsset(assetId));
    EXPECT_TRUE(manager.transferAsset(assetId, "test_owner", "new_owner"));
}

TEST_F(AssetManagerTest, AssetMinting) {
    AssetMetadata metadata;
    metadata.name = "Test Asset";
    metadata.symbol = "TEST";
    metadata.creator = "test_creator";
    metadata.owner = "test_owner";
    metadata.totalSupply = 1000;

    std::string assetId;
    EXPECT_TRUE(manager.createAsset(metadata, assetId));

    EXPECT_TRUE(manager.mintAsset(assetId, 500));
    Asset asset;
    EXPECT_TRUE(manager.getAsset(assetId, asset));
    EXPECT_EQ(asset.metadata.totalSupply, 1500);
}

TEST_F(AssetManagerTest, AssetBurning) {
    AssetMetadata metadata;
    metadata.name = "Test Asset";
    metadata.symbol = "TEST";
    metadata.creator = "test_creator";
    metadata.owner = "test_owner";
    metadata.totalSupply = 1000;

    std::string assetId;
    EXPECT_TRUE(manager.createAsset(metadata, assetId));

    EXPECT_TRUE(manager.burnAsset(assetId, 500));
    Asset asset;
    EXPECT_TRUE(manager.getAsset(assetId, asset));
    EXPECT_EQ(asset.metadata.totalSupply, 500);
}

TEST_F(AssetManagerTest, AssetSearch) {
    AssetMetadata metadata1;
    metadata1.name = "Test Asset 1";
    metadata1.symbol = "TEST1";
    metadata1.creator = "test_creator";
    metadata1.owner = "test_owner";
    metadata1.totalSupply = 1000;

    AssetMetadata metadata2;
    metadata2.name = "Test Asset 2";
    metadata2.symbol = "TEST2";
    metadata2.creator = "test_creator";
    metadata2.owner = "test_owner";
    metadata2.totalSupply = 2000;

    std::string assetId1, assetId2;
    EXPECT_TRUE(manager.createAsset(metadata1, assetId1));
    EXPECT_TRUE(manager.createAsset(metadata2, assetId2));

    std::vector<Asset> results;
    EXPECT_TRUE(manager.searchAssets("Test", results));
    EXPECT_EQ(results.size(), 2);
}

TEST_F(AssetManagerTest, AssetPermissions) {
    AssetMetadata metadata;
    metadata.name = "Test Asset";
    metadata.symbol = "TEST";
    metadata.creator = "test_creator";
    metadata.owner = "test_owner";
    metadata.totalSupply = 1000;

    std::string assetId;
    EXPECT_TRUE(manager.createAsset(metadata, assetId));

    nlohmann::json permissions = {
        {"transfer", {"test_owner", "admin"}},
        {"mint", {"admin"}},
        {"burn", {"admin"}}
    };

    EXPECT_TRUE(manager.setAssetPermissions(assetId, permissions));

    nlohmann::json retrievedPermissions;
    EXPECT_TRUE(manager.getAssetPermissions(assetId, retrievedPermissions));
    EXPECT_EQ(retrievedPermissions, permissions);
}

TEST_F(AssetManagerTest, ErrorHandling) {
    AssetMetadata metadata;
    metadata.name = "Test Asset";
    metadata.symbol = "TEST";
    metadata.creator = "test_creator";
    metadata.owner = "test_owner";
    metadata.totalSupply = 1000;

    std::string assetId;
    EXPECT_TRUE(manager.createAsset(metadata, assetId));

    // Test invalid asset ID
    Asset asset;
    EXPECT_FALSE(manager.getAsset("nonexistent", asset));
    EXPECT_FALSE(manager.getLastError().empty());

    // Test invalid transfer
    EXPECT_FALSE(manager.transferAsset("nonexistent", "from", "to"));
    EXPECT_FALSE(manager.getLastError().empty());
}

TEST_F(AssetManagerTest, ConcurrentAccess) {
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&]() {
            AssetMetadata metadata;
            metadata.name = "Test Asset";
            metadata.symbol = "TEST";
            metadata.creator = "test_creator";
            metadata.owner = "test_owner";
            metadata.totalSupply = 1000;

            std::string assetId;
            EXPECT_TRUE(manager.createAsset(metadata, assetId));
            Asset asset;
            EXPECT_TRUE(manager.getAsset(assetId, asset));
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 