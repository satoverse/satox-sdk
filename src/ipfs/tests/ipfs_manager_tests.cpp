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

#include "ipfs_manager.hpp"
#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <thread>

namespace satox {
namespace test {

class IPFSManagerTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test configuration
        config_ = {
            {"ipfs", {
                {"data_dir", "test_ipfs_data"}
            }}
        };

        // Clean up any existing test data
        if (std::filesystem::exists("test_ipfs_data")) {
            std::filesystem::remove_all("test_ipfs_data");
        }
    }

    void TearDown() override {
        // Clean up test data
        if (std::filesystem::exists("test_ipfs_data")) {
            std::filesystem::remove_all("test_ipfs_data");
        }
    }

    nlohmann::json config_;
};

TEST_F(IPFSManagerTests, InitializationSuccess) {
    auto& manager = IPFSManager::getInstance();
    EXPECT_TRUE(manager.initialize(config_));
    EXPECT_TRUE(manager.isRunning());
}

TEST_F(IPFSManagerTests, DoubleInitialization) {
    auto& manager = IPFSManager::getInstance();
    EXPECT_TRUE(manager.initialize(config_));
    EXPECT_FALSE(manager.initialize(config_));
}

TEST_F(IPFSManagerTests, InvalidConfig) {
    auto& manager = IPFSManager::getInstance();
    nlohmann::json invalid_config;
    EXPECT_FALSE(manager.initialize(invalid_config));
}

TEST_F(IPFSManagerTests, Shutdown) {
    auto& manager = IPFSManager::getInstance();
    EXPECT_TRUE(manager.initialize(config_));
    manager.shutdown();
    EXPECT_FALSE(manager.isRunning());
}

TEST_F(IPFSManagerTests, AddAndGetData) {
    auto& manager = IPFSManager::getInstance();
    EXPECT_TRUE(manager.initialize(config_));

    // Test data
    std::vector<uint8_t> test_data = {'t', 'e', 's', 't'};
    
    // Add data
    std::string hash = manager.addData(test_data);
    EXPECT_FALSE(hash.empty());

    // Get data
    std::vector<uint8_t> retrieved_data = manager.getData(hash);
    EXPECT_EQ(retrieved_data, test_data);
}

TEST_F(IPFSManagerTests, GetNonExistentData) {
    auto& manager = IPFSManager::getInstance();
    EXPECT_TRUE(manager.initialize(config_));

    EXPECT_THROW(manager.getData("nonexistent"), std::runtime_error);
}

TEST_F(IPFSManagerTests, PinAndUnpinHash) {
    auto& manager = IPFSManager::getInstance();
    EXPECT_TRUE(manager.initialize(config_));

    // Add test data
    std::vector<uint8_t> test_data = {'t', 'e', 's', 't'};
    std::string hash = manager.addData(test_data);

    // Pin hash
    EXPECT_TRUE(manager.pinHash(hash));

    // Verify hash is pinned
    auto pinned_hashes = manager.listPinnedHashes();
    EXPECT_EQ(pinned_hashes.size(), 1);
    EXPECT_EQ(pinned_hashes[0], hash);

    // Unpin hash
    EXPECT_TRUE(manager.unpinHash(hash));

    // Verify hash is not pinned
    pinned_hashes = manager.listPinnedHashes();
    EXPECT_TRUE(pinned_hashes.empty());
}

TEST_F(IPFSManagerTests, PinNonExistentHash) {
    auto& manager = IPFSManager::getInstance();
    EXPECT_TRUE(manager.initialize(config_));

    EXPECT_THROW(manager.pinHash("nonexistent"), std::runtime_error);
}

TEST_F(IPFSManagerTests, UnpinNonExistentHash) {
    auto& manager = IPFSManager::getInstance();
    EXPECT_TRUE(manager.initialize(config_));

    EXPECT_THROW(manager.unpinHash("nonexistent"), std::runtime_error);
}

TEST_F(IPFSManagerTests, GetIPFSInfo) {
    auto& manager = IPFSManager::getInstance();
    EXPECT_TRUE(manager.initialize(config_));

    // Add test data
    std::vector<uint8_t> test_data = {'t', 'e', 's', 't'};
    std::string hash = manager.addData(test_data);

    // Get info
    auto info = manager.getIPFSInfo(hash);
    EXPECT_EQ(info["hash"], hash);
    EXPECT_EQ(info["size"], test_data.size());
    EXPECT_FALSE(info["owner"].empty());
    EXPECT_TRUE(info.contains("creation_time"));
    EXPECT_FALSE(info["pinned"]);
}

TEST_F(IPFSManagerTests, GetIPFSHistory) {
    auto& manager = IPFSManager::getInstance();
    EXPECT_TRUE(manager.initialize(config_));

    // Add test data
    std::vector<uint8_t> test_data = {'t', 'e', 's', 't'};
    std::string hash = manager.addData(test_data);

    // Get history
    auto history = manager.getIPFSHistory(hash);
    EXPECT_TRUE(history.empty()); // Initially empty

    // Pin and unpin to create history
    manager.pinHash(hash);
    manager.unpinHash(hash);

    // Get updated history
    history = manager.getIPFSHistory(hash);
    EXPECT_FALSE(history.empty());
}

TEST_F(IPFSManagerTests, ConcurrentOperations) {
    auto& manager = IPFSManager::getInstance();
    EXPECT_TRUE(manager.initialize(config_));

    // Create multiple threads performing different operations
    std::vector<std::thread> threads;
    std::vector<std::string> hashes;

    // Add data in multiple threads
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&manager, &hashes, i]() {
            std::vector<uint8_t> data = {'t', 'e', 's', 't', static_cast<uint8_t>(i)};
            hashes.push_back(manager.addData(data));
        });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Verify all hashes were created
    EXPECT_EQ(hashes.size(), 10);

    // Perform concurrent pin/unpin operations
    threads.clear();
    for (const auto& hash : hashes) {
        threads.emplace_back([&manager, hash]() {
            manager.pinHash(hash);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            manager.unpinHash(hash);
        });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Verify no hashes are pinned
    EXPECT_TRUE(manager.listPinnedHashes().empty());
}

TEST_F(IPFSManagerTests, LargeDataHandling) {
    auto& manager = IPFSManager::getInstance();
    EXPECT_TRUE(manager.initialize(config_));

    // Create large test data (1MB)
    std::vector<uint8_t> large_data(1024 * 1024, 'x');
    
    // Add large data
    std::string hash = manager.addData(large_data);
    EXPECT_FALSE(hash.empty());

    // Get and verify large data
    std::vector<uint8_t> retrieved_data = manager.getData(hash);
    EXPECT_EQ(retrieved_data.size(), large_data.size());
    EXPECT_EQ(retrieved_data, large_data);
}

TEST_F(IPFSManagerTests, InvalidOperationsWhenNotRunning) {
    auto& manager = IPFSManager::getInstance();
    
    // Try operations without initialization
    EXPECT_THROW(manager.addData({'t', 'e', 's', 't'}), std::runtime_error);
    EXPECT_THROW(manager.getData("test"), std::runtime_error);
    EXPECT_THROW(manager.pinHash("test"), std::runtime_error);
    EXPECT_THROW(manager.unpinHash("test"), std::runtime_error);
    EXPECT_THROW(manager.getIPFSInfo("test"), std::runtime_error);
    EXPECT_THROW(manager.getIPFSHistory("test"), std::runtime_error);
}

} // namespace test
} // namespace satox 