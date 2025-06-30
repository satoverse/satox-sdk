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
#include <chrono>
#include <thread>
#include <vector>
#include <random>
#include <memory>
#include "asset_manager.hpp"

using namespace satox::asset;
using namespace std::chrono;

class AssetManagerPerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = &AssetManager::getInstance();
        ASSERT_TRUE(manager->initialize("test_network"));
    }

    void TearDown() override {
        manager->shutdown();
    }

    AssetManager* manager;
};

// Asset Creation Performance Tests
TEST_F(AssetManagerPerformanceTest, AssetCreationPerformance) {
    const int num_assets = 1000;
    std::vector<Asset> created_assets;

    auto start = high_resolution_clock::now();
    for (int i = 0; i < num_assets; ++i) {
        Asset asset;
        asset.name = "Test Asset " + std::to_string(i);
        asset.symbol = "TEST" + std::to_string(i);
        asset.owner = "test_owner";
        asset.total_supply = 1000000;
        asset.decimals = 8;
        asset.status = "active";
        asset.metadata["description"] = "Test asset " + std::to_string(i);
        asset.created_at = system_clock::now();
        asset.updated_at = system_clock::now();

        ASSERT_TRUE(manager->createAsset(asset));
        created_assets.push_back(asset);
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    
    double assets_per_second = (num_assets * 1000.0) / duration.count();
    EXPECT_GE(assets_per_second, 100.0); // Expect at least 100 assets per second
}

// Asset Transfer Performance Tests
TEST_F(AssetManagerPerformanceTest, AssetTransferPerformance) {
    const int num_transfers = 1000;
    std::vector<Asset> assets;

    // Create assets first
    for (int i = 0; i < num_transfers; ++i) {
        Asset asset;
        asset.name = "Test Asset " + std::to_string(i);
        asset.symbol = "TEST" + std::to_string(i);
        asset.owner = "test_owner";
        asset.total_supply = 1000000;
        asset.decimals = 8;
        asset.status = "active";
        asset.created_at = system_clock::now();
        asset.updated_at = system_clock::now();

        ASSERT_TRUE(manager->createAsset(asset));
        assets.push_back(asset);
    }

    // Test transfers
    auto start = high_resolution_clock::now();
    for (const auto& asset : assets) {
        ASSERT_TRUE(manager->transferAsset(asset.id, "test_owner", "test_recipient", 1000));
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    
    double transfers_per_second = (num_transfers * 1000.0) / duration.count();
    EXPECT_GE(transfers_per_second, 100.0); // Expect at least 100 transfers per second
}

// Asset Validation Performance Tests
TEST_F(AssetManagerPerformanceTest, AssetValidationPerformance) {
    const int num_assets = 1000;
    std::vector<Asset> assets;

    // Create assets first
    for (int i = 0; i < num_assets; ++i) {
        Asset asset;
        asset.name = "Test Asset " + std::to_string(i);
        asset.symbol = "TEST" + std::to_string(i);
        asset.owner = "test_owner";
        asset.total_supply = 1000000;
        asset.decimals = 8;
        asset.status = "active";
        asset.created_at = system_clock::now();
        asset.updated_at = system_clock::now();

        ASSERT_TRUE(manager->createAsset(asset));
        assets.push_back(asset);
    }

    // Test validation
    auto start = high_resolution_clock::now();
    for (const auto& asset : assets) {
        EXPECT_TRUE(manager->validateAsset(asset));
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    
    double validations_per_second = (num_assets * 1000.0) / duration.count();
    EXPECT_GE(validations_per_second, 200.0); // Expect at least 200 validations per second
}

// Asset Metadata Performance Tests
TEST_F(AssetManagerPerformanceTest, AssetMetadataPerformance) {
    const int num_assets = 1000;
    std::vector<Asset> assets;

    // Create assets with metadata
    auto start = high_resolution_clock::now();
    for (int i = 0; i < num_assets; ++i) {
        Asset asset;
        asset.name = "Test Asset " + std::to_string(i);
        asset.symbol = "TEST" + std::to_string(i);
        asset.owner = "test_owner";
        asset.total_supply = 1000000;
        asset.decimals = 8;
        asset.status = "active";
        
        // Add multiple metadata fields
        for (int j = 0; j < 10; ++j) {
            asset.metadata["field" + std::to_string(j)] = "value" + std::to_string(j);
        }
        
        asset.created_at = system_clock::now();
        asset.updated_at = system_clock::now();

        ASSERT_TRUE(manager->createAsset(asset));
        assets.push_back(asset);
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    
    double assets_per_second = (num_assets * 1000.0) / duration.count();
    EXPECT_GE(assets_per_second, 50.0); // Expect at least 50 assets per second with metadata
}

// Asset Permissions Performance Tests
TEST_F(AssetManagerPerformanceTest, AssetPermissionsPerformance) {
    const int num_assets = 1000;
    std::vector<Asset> assets;

    // Create assets first
    for (int i = 0; i < num_assets; ++i) {
        Asset asset;
        asset.name = "Test Asset " + std::to_string(i);
        asset.symbol = "TEST" + std::to_string(i);
        asset.owner = "test_owner";
        asset.total_supply = 1000000;
        asset.decimals = 8;
        asset.status = "active";
        asset.created_at = system_clock::now();
        asset.updated_at = system_clock::now();

        ASSERT_TRUE(manager->createAsset(asset));
        assets.push_back(asset);
    }

    // Test permission operations
    auto start = high_resolution_clock::now();
    for (const auto& asset : assets) {
        std::unordered_map<std::string, std::vector<std::string>> permissions = {
            {"transfer", {"test_owner", "admin"}},
            {"mint", {"admin"}},
            {"burn", {"admin"}}
        };
        EXPECT_TRUE(manager->setAssetPermissions(asset.id, permissions));
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    
    double operations_per_second = (num_assets * 1000.0) / duration.count();
    EXPECT_GE(operations_per_second, 100.0); // Expect at least 100 permission operations per second
}

// Asset Search Performance Tests
TEST_F(AssetManagerPerformanceTest, AssetSearchPerformance) {
    const int num_assets = 1000;
    std::vector<Asset> assets;

    // Create assets with searchable content
    for (int i = 0; i < num_assets; ++i) {
        Asset asset;
        asset.name = "Test Asset " + std::to_string(i);
        asset.symbol = "TEST" + std::to_string(i);
        asset.owner = "test_owner";
        asset.total_supply = 1000000;
        asset.decimals = 8;
        asset.status = "active";
        asset.metadata["description"] = "Test asset " + std::to_string(i);
        asset.created_at = system_clock::now();
        asset.updated_at = system_clock::now();

        ASSERT_TRUE(manager->createAsset(asset));
        assets.push_back(asset);
    }

    // Test search operations
    auto start = high_resolution_clock::now();
    std::vector<Asset> results;
    EXPECT_TRUE(manager->searchAssets("Test", results));
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    
    EXPECT_GE(results.size(), num_assets * 0.9); // Expect at least 90% of assets to be found
    EXPECT_LE(duration.count(), 1000); // Expect search to complete in less than 1 second
}

// Parallel Processing Performance Tests
TEST_F(AssetManagerPerformanceTest, ParallelProcessingPerformance) {
    const int num_threads = 8;
    const int assets_per_thread = 100;
    std::atomic<int> success_count{0};
    std::vector<std::thread> threads;

    auto worker = [&]() {
        for (int i = 0; i < assets_per_thread; ++i) {
            Asset asset;
            asset.name = "Test Asset " + std::to_string(i);
            asset.symbol = "TEST" + std::to_string(i);
            asset.owner = "test_owner";
            asset.total_supply = 1000000;
            asset.decimals = 8;
            asset.status = "active";
            asset.created_at = system_clock::now();
            asset.updated_at = system_clock::now();

            if (manager->createAsset(asset) &&
                manager->validateAsset(asset) &&
                manager->transferAsset(asset.id, "test_owner", "test_recipient", 1000)) {
                success_count++;
            }
        }
    };

    auto start = high_resolution_clock::now();
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker);
    }
    for (auto& thread : threads) {
        thread.join();
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    
    double operations_per_second = (success_count * 1000.0) / duration.count();
    EXPECT_GE(operations_per_second, 100.0); // Expect at least 100 operations per second
    EXPECT_GE(success_count, num_threads * assets_per_thread * 0.9); // 90% success rate
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 