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
#include <filesystem>
#include <fstream>
#include <thread>
#include <atomic>
#include <chrono>
#include <nlohmann/json.hpp>

namespace satox {
namespace test {

class AssetManagerTests : public ::testing::Test {
protected:
    void SetUp() override {
        config_.type = satox::asset::AssetType::TOKEN;
        config_.name = "test_asset_manager";
        config_.maxAssets = 1000;
        config_.timeout = 30;
        config_.enableLogging = true;
        config_.logPath = "logs/components/asset/";
        config_.additionalConfig = nlohmann::json::object();
        // Ensure clean state for each test
        auto& manager = satox::asset::AssetManager::getInstance();
        manager.shutdown();
    }

    void TearDown() override {
        // Clean up after each test
        auto& manager = satox::asset::AssetManager::getInstance();
        manager.shutdown();
    }

    satox::asset::AssetManager::AssetConfig config_;
};

TEST_F(AssetManagerTests, Initialization) {
    auto manager = &satox::asset::AssetManager::getInstance();
    EXPECT_TRUE(manager->initialize(config_));
    EXPECT_TRUE(manager->isInitialized());
}

TEST_F(AssetManagerTests, CreateAsset) {
    auto manager = &satox::asset::AssetManager::getInstance();
    EXPECT_TRUE(manager->initialize(config_));
    
    satox::asset::AssetManager::AssetCreationRequest request;
    request.owner_address = "SXxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    request.type = satox::asset::AssetManager::AssetType::SUB_ASSET;
    request.metadata.name = "TEST_ASSET";
    request.metadata.symbol = "TEST";
    request.metadata.description = "Test asset";
    request.metadata.decimals = 8;
    request.metadata.total_supply = 1000000;
    request.metadata.reissuable = true;
    request.initial_supply = 1000000;
    request.reissuable = true;
    request.restricted_addresses = {};
    
    EXPECT_TRUE(manager->createAsset(request));
}

TEST_F(AssetManagerTests, GetAssetMetadata) {
    auto manager = &satox::asset::AssetManager::getInstance();
    EXPECT_TRUE(manager->initialize(config_));
    
    satox::asset::AssetManager::AssetCreationRequest request;
    request.owner_address = "SXxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    request.type = satox::asset::AssetManager::AssetType::SUB_ASSET;
    request.metadata.name = "TEST_ASSET";
    request.metadata.symbol = "TEST";
    request.metadata.description = "Test asset";
    request.metadata.decimals = 8;
    request.metadata.total_supply = 1000000;
    request.metadata.reissuable = true;
    request.initial_supply = 1000000;
    request.reissuable = true;
    request.restricted_addresses = {};
    
    manager->createAsset(request);
    
    satox::asset::AssetManager::AssetMetadata metadata;
    EXPECT_TRUE(manager->getAssetMetadata("TEST_ASSET", metadata));
    EXPECT_EQ(metadata.name, "TEST_ASSET");
    EXPECT_EQ(metadata.symbol, "TEST");
}

TEST_F(AssetManagerTests, TransferAsset) {
    auto manager = &satox::asset::AssetManager::getInstance();
    EXPECT_TRUE(manager->initialize(config_));
    
    satox::asset::AssetManager::AssetCreationRequest create_request;
    create_request.owner_address = "SXxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    create_request.type = satox::asset::AssetManager::AssetType::SUB_ASSET;
    create_request.metadata.name = "TEST_ASSET";
    create_request.metadata.symbol = "TEST";
    create_request.metadata.description = "Test asset";
    create_request.metadata.decimals = 8;
    create_request.metadata.total_supply = 1000000;
    create_request.metadata.reissuable = true;
    create_request.initial_supply = 1000000;
    create_request.reissuable = true;
    create_request.restricted_addresses = {};
    
    manager->createAsset(create_request);
    
    satox::asset::AssetManager::AssetTransferRequest transfer_request;
    transfer_request.from_address = "SXxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    transfer_request.to_address = "SXyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy";
    transfer_request.asset_name = "TEST_ASSET";
    transfer_request.amount = 100;
    transfer_request.memo = "Test transfer";
    
    EXPECT_TRUE(manager->transferAsset(transfer_request));
}

TEST_F(AssetManagerTests, GetAssetStats) {
    auto manager = &satox::asset::AssetManager::getInstance();
    EXPECT_TRUE(manager->initialize(config_));
    
    auto stats = manager->getAssetStats();
    EXPECT_GE(stats.total_assets, 0);
    EXPECT_GE(stats.active_assets, 0);
}

TEST_F(AssetManagerTests, Shutdown) {
    auto manager = &satox::asset::AssetManager::getInstance();
    EXPECT_TRUE(manager->initialize(config_));
    manager->shutdown();
    EXPECT_FALSE(manager->isInitialized());
}

} // namespace test
} // namespace satox 