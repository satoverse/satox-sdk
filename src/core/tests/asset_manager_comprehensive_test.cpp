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
#include <chrono>
#include <thread>
#include <vector>
#include <random>
#include <memory>
#include <future>
#include "satox/core/asset_manager.hpp"

using namespace satox::core;

class AssetManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = &AssetManager::getInstance();
        manager->shutdown(); // Ensure clean state
        manager->initialize(nlohmann::json::object());
    }

    void TearDown() override {
        manager->shutdown();
    }

    AssetManager* manager;
};

// Initialization Tests
TEST_F(AssetManagerTest, Initialization) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    EXPECT_FALSE(manager->initialize(nlohmann::json::object())); // Should fail on second init
}

// Asset Creation Tests
TEST_F(AssetManagerTest, CreateAsset) {
    AssetMetadata metadata{
        "Test Asset",
        "TEST",
        "Test Issuer",
        "Test Owner",
        AssetType::TOKEN,
        "https://example.com/asset",
        "0x123",
        std::chrono::system_clock::now(),
        nlohmann::json::object()
    };

    EXPECT_TRUE(manager->createAsset("asset1", metadata));
    EXPECT_TRUE(manager->isAssetExists("asset1"));
    EXPECT_FALSE(manager->createAsset("asset1", metadata)); // Should fail for duplicate ID
}

// Asset State Management Tests
TEST_F(AssetManagerTest, AssetStateManagement) {
    AssetMetadata metadata{
        "Test Asset",
        "TEST",
        "Test Issuer",
        "Test Owner",
        AssetType::TOKEN,
        "https://example.com/asset",
        "0x123",
        std::chrono::system_clock::now(),
        nlohmann::json::object()
    };

    EXPECT_TRUE(manager->createAsset("asset1", metadata));
    EXPECT_TRUE(manager->issueAsset("asset1"));
    EXPECT_TRUE(manager->transferAsset("asset1", "New Owner"));
    EXPECT_TRUE(manager->burnAsset("asset1"));
}

// Asset Metadata Tests
TEST_F(AssetManagerTest, AssetMetadata) {
    AssetMetadata metadata{
        "Test Asset",
        "TEST",
        "Test Issuer",
        "Test Owner",
        AssetType::TOKEN,
        "https://example.com/asset",
        "0x123",
        std::chrono::system_clock::now(),
        nlohmann::json::object()
    };

    EXPECT_TRUE(manager->createAsset("asset1", metadata));
    
    // Update metadata
    metadata.name = "Updated Asset";
    EXPECT_TRUE(manager->updateAssetMetadata("asset1", metadata));
    
    // Update URI
    EXPECT_TRUE(manager->updateAssetURI("asset1", "https://example.com/updated"));
    
    // Update hash
    EXPECT_TRUE(manager->updateAssetHash("asset1", "0x456"));
    
    // Add attribute
    EXPECT_TRUE(manager->addAssetAttribute("asset1", "color", "red"));
    
    // Remove attribute
    EXPECT_TRUE(manager->removeAssetAttribute("asset1", "color"));
}

// Asset Query Tests
TEST_F(AssetManagerTest, AssetQueries) {
    AssetMetadata metadata1{
        "Token 1",
        "TKN1",
        "Issuer 1",
        "Owner 1",
        AssetType::TOKEN,
        "https://example.com/token1",
        "0x123",
        std::chrono::system_clock::now(),
        nlohmann::json::object()
    };

    AssetMetadata metadata2{
        "NFT 1",
        "NFT1",
        "Issuer 2",
        "Owner 2",
        AssetType::NFT,
        "https://example.com/nft1",
        "0x456",
        std::chrono::system_clock::now(),
        nlohmann::json::object()
    };

    EXPECT_TRUE(manager->createAsset("token1", metadata1));
    EXPECT_TRUE(manager->createAsset("nft1", metadata2));

    // Query by type
    auto tokens = manager->getAssetsByType(AssetType::TOKEN);
    EXPECT_EQ(tokens.size(), 1);
    EXPECT_EQ(tokens[0].metadata.symbol, "TKN1");

    // Query by owner
    auto owner1Assets = manager->getAssetsByOwner("Owner 1");
    EXPECT_EQ(owner1Assets.size(), 1);
    EXPECT_EQ(owner1Assets[0].metadata.symbol, "TKN1");

    // Query by state
    EXPECT_TRUE(manager->issueAsset("token1"));
    auto issuedAssets = manager->getAssetsByState(AssetState::ISSUED);
    EXPECT_EQ(issuedAssets.size(), 1);
    EXPECT_EQ(issuedAssets[0].metadata.symbol, "TKN1");
}

// Asset History Tests
TEST_F(AssetManagerTest, AssetHistory) {
    AssetMetadata metadata{
        "Test Asset",
        "TEST",
        "Test Issuer",
        "Test Owner",
        AssetType::TOKEN,
        "https://example.com/asset",
        "0x123",
        std::chrono::system_clock::now(),
        nlohmann::json::object()
    };

    EXPECT_TRUE(manager->createAsset("asset1", metadata));
    EXPECT_TRUE(manager->issueAsset("asset1"));
    EXPECT_TRUE(manager->transferAsset("asset1", "New Owner"));
    EXPECT_TRUE(manager->burnAsset("asset1"));

    auto history = manager->getAssetHistory("asset1");
    EXPECT_GE(history.size(), 4); // Should have at least 4 entries

    EXPECT_TRUE(manager->addAssetHistoryEntry("asset1", "Custom event"));
    EXPECT_TRUE(manager->clearAssetHistory("asset1"));
    EXPECT_TRUE(manager->getAssetHistory("asset1").empty());
}

// Statistics Tests
TEST_F(AssetManagerTest, Statistics) {
    AssetMetadata metadata{
        "Test Asset",
        "TEST",
        "Test Issuer",
        "Test Owner",
        AssetType::TOKEN,
        "https://example.com/asset",
        "0x123",
        std::chrono::system_clock::now(),
        nlohmann::json::object()
    };

    EXPECT_TRUE(manager->enableStats(true));
    EXPECT_TRUE(manager->createAsset("asset1", metadata));
    EXPECT_TRUE(manager->issueAsset("asset1"));
    EXPECT_TRUE(manager->transferAsset("asset1", "New Owner"));
    EXPECT_TRUE(manager->burnAsset("asset1"));

    auto stats = manager->getStats();
    EXPECT_EQ(stats.totalAssets, 1);
    EXPECT_EQ(stats.issuedAssets, 1);
    EXPECT_EQ(stats.transferredAssets, 1);
    EXPECT_EQ(stats.burnedAssets, 1);

    manager->resetStats();
    stats = manager->getStats();
    EXPECT_EQ(stats.totalAssets, 0);
    EXPECT_EQ(stats.issuedAssets, 0);
    EXPECT_EQ(stats.transferredAssets, 0);
    EXPECT_EQ(stats.burnedAssets, 0);
}

// Callback Tests
TEST_F(AssetManagerTest, Callbacks) {
    bool assetCallbackCalled = false;
    bool metadataCallbackCalled = false;
    bool errorCallbackCalled = false;

    manager->registerAssetCallback(
        [&](const std::string& id, AssetState state) {
            assetCallbackCalled = true;
        }
    );

    manager->registerMetadataCallback(
        [&](const std::string& id, const AssetMetadata& metadata) {
            metadataCallbackCalled = true;
        }
    );

    manager->registerErrorCallback(
        [&](const std::string& id, const std::string& error) {
            errorCallbackCalled = true;
        }
    );

    AssetMetadata metadata{
        "Test Asset",
        "TEST",
        "Test Issuer",
        "Test Owner",
        AssetType::TOKEN,
        "https://example.com/asset",
        "0x123",
        std::chrono::system_clock::now(),
        nlohmann::json::object()
    };

    EXPECT_TRUE(manager->createAsset("asset1", metadata));
    EXPECT_TRUE(assetCallbackCalled);
    EXPECT_TRUE(metadataCallbackCalled);

    // Test error callback
    EXPECT_FALSE(manager->createAsset("", metadata));
    EXPECT_TRUE(errorCallbackCalled);
}

// Error Handling Tests
TEST_F(AssetManagerTest, ErrorHandling) {
    AssetMetadata metadata{
        "Test Asset",
        "TEST",
        "Test Issuer",
        "Test Owner",
        AssetType::TOKEN,
        "https://example.com/asset",
        "0x123",
        std::chrono::system_clock::now(),
        nlohmann::json::object()
    };

    // Test invalid asset ID
    EXPECT_FALSE(manager->createAsset("", metadata));
    EXPECT_FALSE(manager->getLastError().empty());

    // Test non-existent asset
    EXPECT_FALSE(manager->issueAsset("nonexistent"));
    EXPECT_FALSE(manager->getLastError().empty());

    manager->clearLastError();
    EXPECT_TRUE(manager->getLastError().empty());
}

// Concurrency Tests
TEST_F(AssetManagerTest, Concurrency) {
    const int numThreads = 10;
    const int numAssetsPerThread = 100;
    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([this, i, numAssetsPerThread]() {
            for (int j = 0; j < numAssetsPerThread; ++j) {
                std::string id = "asset_" + std::to_string(i) + "_" + std::to_string(j);
                AssetMetadata metadata{
                    "Test Asset " + id,
                    "TEST",
                    "Test Issuer",
                    "Test Owner",
                    AssetType::TOKEN,
                    "https://example.com/asset",
                    "0x123",
                    std::chrono::system_clock::now(),
                    nlohmann::json::object()
                };

                manager->createAsset(id, metadata);
                manager->issueAsset(id);
                manager->transferAsset(id, "New Owner");
                manager->burnAsset(id);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    auto stats = manager->getStats();
    EXPECT_EQ(stats.totalAssets, numThreads * numAssetsPerThread);
    EXPECT_EQ(stats.burnedAssets, numThreads * numAssetsPerThread);
}

// Edge Cases Tests
TEST_F(AssetManagerTest, EdgeCases) {
    // Test empty metadata
    AssetMetadata emptyMetadata;
    EXPECT_FALSE(manager->createAsset("asset1", emptyMetadata));

    // Test maximum length IDs
    std::string longId(64, 'a');
    AssetMetadata metadata{
        "Test Asset",
        "TEST",
        "Test Issuer",
        "Test Owner",
        AssetType::TOKEN,
        "https://example.com/asset",
        "0x123",
        std::chrono::system_clock::now(),
        nlohmann::json::object()
    };
    EXPECT_TRUE(manager->createAsset(longId, metadata));

    // Test special characters in metadata
    metadata.name = "Test Asset !@#$%^&*()";
    metadata.symbol = "TEST!@#";
    EXPECT_TRUE(manager->createAsset("asset2", metadata));
}

// Cleanup Tests
TEST_F(AssetManagerTest, Cleanup) {
    AssetMetadata metadata{
        "Test Asset",
        "TEST",
        "Test Issuer",
        "Test Owner",
        AssetType::TOKEN,
        "https://example.com/asset",
        "0x123",
        std::chrono::system_clock::now(),
        nlohmann::json::object()
    };

    EXPECT_TRUE(manager->createAsset("asset1", metadata));
    manager->shutdown();
    EXPECT_FALSE(manager->isAssetExists("asset1"));
}

// Stress Tests
TEST_F(AssetManagerTest, StressTest) {
    const int numAssets = 1000;
    std::vector<std::string> assetIds;

    // Create many assets
    for (int i = 0; i < numAssets; ++i) {
        std::string id = "asset_" + std::to_string(i);
        AssetMetadata metadata{
            "Test Asset " + std::to_string(i),
            "TEST" + std::to_string(i),
            "Test Issuer",
            "Test Owner",
            AssetType::TOKEN,
            "https://example.com/asset",
            "0x123",
            std::chrono::system_clock::now(),
            nlohmann::json::object()
        };

        EXPECT_TRUE(manager->createAsset(id, metadata));
        assetIds.push_back(id);
    }

    // Perform operations on all assets
    for (const auto& id : assetIds) {
        EXPECT_TRUE(manager->issueAsset(id));
        EXPECT_TRUE(manager->transferAsset(id, "New Owner"));
        EXPECT_TRUE(manager->burnAsset(id));
    }

    auto stats = manager->getStats();
    EXPECT_EQ(stats.totalAssets, numAssets);
    EXPECT_EQ(stats.burnedAssets, numAssets);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 