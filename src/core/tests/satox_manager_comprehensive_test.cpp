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
#include "satox/core/satox_manager.hpp"
#include <thread>
#include <chrono>
#include <random>
#include <fstream>
#include <filesystem>

using namespace satox::core;

class SatoxManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = &SatoxManager::getInstance();
        manager->shutdown(); // Ensure clean state
    }

    void TearDown() override {
        manager->shutdown();
    }

    SatoxManager* manager;
};

// Initialization Tests
TEST_F(SatoxManagerTest, Initialization) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    EXPECT_FALSE(manager->initialize(nlohmann::json::object())); // Should fail on second init
    EXPECT_TRUE(manager->isInitialized());
}

// Lifecycle Tests
TEST_F(SatoxManagerTest, Lifecycle) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    
    // Start
    EXPECT_TRUE(manager->start());
    EXPECT_TRUE(manager->isRunning());
    
    // Pause
    EXPECT_TRUE(manager->pause());
    EXPECT_TRUE(manager->isPaused());
    
    // Resume
    EXPECT_TRUE(manager->resume());
    EXPECT_FALSE(manager->isPaused());
    
    // Stop
    EXPECT_TRUE(manager->stop());
    EXPECT_FALSE(manager->isRunning());
}

// Configuration Tests
TEST_F(SatoxManagerTest, Configuration) {
    nlohmann::json config = {
        {"logging", {
            {"level", "info"},
            {"file", "test.log"}
        }},
        {"network", {
            {"port", 7777},
            {"host", "localhost"}
        }},
        {"blockchain", {
            {"network", "mainnet"},
            {"rpc_port", 7777}
        }}
    };
    
    EXPECT_TRUE(manager->initialize(config));
    EXPECT_TRUE(manager->start());
    
    // Update configuration
    config["logging"]["level"] = "debug";
    EXPECT_TRUE(manager->update(config));
    
    EXPECT_TRUE(manager->stop());
}

// Manager Access Tests
TEST_F(SatoxManagerTest, ManagerAccess) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    EXPECT_TRUE(manager->start());
    
    // Test Core Manager
    auto& coreManager = manager->getCoreManager();
    EXPECT_TRUE(coreManager.isInitialized());
    EXPECT_TRUE(coreManager.isRunning());
    
    // Test Network Manager
    auto& networkManager = manager->getNetworkManager();
    EXPECT_TRUE(networkManager.isInitialized());
    EXPECT_TRUE(networkManager.isRunning());
    
    // Test Blockchain Manager
    auto& blockchainManager = manager->getBlockchainManager();
    EXPECT_TRUE(blockchainManager.isInitialized());
    EXPECT_TRUE(blockchainManager.isRunning());
    
    // Test Transaction Manager
    auto& transactionManager = manager->getTransactionManager();
    EXPECT_TRUE(transactionManager.isInitialized());
    EXPECT_TRUE(transactionManager.isRunning());
    
    // Test Asset Manager
    auto& assetManager = manager->getAssetManager();
    EXPECT_TRUE(assetManager.isInitialized());
    EXPECT_TRUE(assetManager.isRunning());
    
    // Test Wallet Manager
    auto& walletManager = manager->getWalletManager();
    EXPECT_TRUE(walletManager.isInitialized());
    EXPECT_TRUE(walletManager.isRunning());
    
    // Test IPFS Manager
    auto& ipfsManager = manager->getIPFSManager();
    EXPECT_TRUE(ipfsManager.isInitialized());
    EXPECT_TRUE(ipfsManager.isRunning());
    
    // Test Database Manager
    auto& databaseManager = manager->getDatabaseManager();
    EXPECT_TRUE(databaseManager.isInitialized());
    EXPECT_TRUE(databaseManager.isRunning());
    
    // Test Security Manager
    auto& securityManager = manager->getSecurityManager();
    EXPECT_TRUE(securityManager.isInitialized());
    EXPECT_TRUE(securityManager.isRunning());
    
    // Test NFT Manager
    auto& nftManager = manager->getNFTManager();
    EXPECT_TRUE(nftManager.isInitialized());
    EXPECT_TRUE(nftManager.isRunning());
    
    // Test Plugin Manager
    auto& pluginManager = manager->getPluginManager();
    EXPECT_TRUE(pluginManager.isInitialized());
    EXPECT_TRUE(pluginManager.isRunning());
    
    // Test Event Manager
    auto& eventManager = manager->getEventManager();
    EXPECT_TRUE(eventManager.isInitialized());
    EXPECT_TRUE(eventManager.isRunning());
    
    // Test Cache Manager
    auto& cacheManager = manager->getCacheManager();
    EXPECT_TRUE(cacheManager.isInitialized());
    EXPECT_TRUE(cacheManager.isRunning());
    
    // Test Config Manager
    auto& configManager = manager->getConfigManager();
    EXPECT_TRUE(configManager.isInitialized());
    EXPECT_TRUE(configManager.isRunning());
    
    // Test Logging Manager
    auto& loggingManager = manager->getLoggingManager();
    EXPECT_TRUE(loggingManager.isInitialized());
    EXPECT_TRUE(loggingManager.isRunning());
    
    EXPECT_TRUE(manager->stop());
}

// System Information Tests
TEST_F(SatoxManagerTest, SystemInformation) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    EXPECT_TRUE(manager->start());
    
    // Get system info
    auto info = manager->getSystemInfo();
    EXPECT_TRUE(info.initialized);
    EXPECT_TRUE(info.running);
    EXPECT_FALSE(info.paused);
    EXPECT_FALSE(info.version.empty());
    EXPECT_FALSE(info.build.empty());
    EXPECT_FALSE(info.managerStatus.empty());
    
    // Check manager status
    EXPECT_TRUE(info.managerStatus["CoreManager"]);
    EXPECT_TRUE(info.managerStatus["NetworkManager"]);
    EXPECT_TRUE(info.managerStatus["BlockchainManager"]);
    EXPECT_TRUE(info.managerStatus["TransactionManager"]);
    EXPECT_TRUE(info.managerStatus["AssetManager"]);
    EXPECT_TRUE(info.managerStatus["WalletManager"]);
    EXPECT_TRUE(info.managerStatus["IPFSManager"]);
    EXPECT_TRUE(info.managerStatus["DatabaseManager"]);
    EXPECT_TRUE(info.managerStatus["SecurityManager"]);
    EXPECT_TRUE(info.managerStatus["NFTManager"]);
    EXPECT_TRUE(info.managerStatus["PluginManager"]);
    EXPECT_TRUE(info.managerStatus["EventManager"]);
    EXPECT_TRUE(info.managerStatus["CacheManager"]);
    EXPECT_TRUE(info.managerStatus["ConfigManager"]);
    EXPECT_TRUE(info.managerStatus["LoggingManager"]);
    
    EXPECT_TRUE(manager->stop());
}

// Statistics Tests
TEST_F(SatoxManagerTest, Statistics) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    EXPECT_TRUE(manager->start());
    
    // Enable statistics
    EXPECT_TRUE(manager->enableStats(true));
    
    // Get stats
    auto stats = manager->getSystemStats();
    EXPECT_GE(stats.uptime, 0);
    EXPECT_FALSE(stats.managerStats.empty());
    
    // Reset stats
    EXPECT_TRUE(manager->resetStats());
    stats = manager->getSystemStats();
    EXPECT_EQ(stats.uptime, 0);
    
    EXPECT_TRUE(manager->stop());
}

// Callback Tests
TEST_F(SatoxManagerTest, Callbacks) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    
    // Register callbacks
    bool stateCallbackCalled = false;
    bool errorCallbackCalled = false;
    bool statsCallbackCalled = false;
    
    EXPECT_TRUE(manager->registerStateCallback([&](const SatoxManager::SystemInfo& info) {
        stateCallbackCalled = true;
    }));
    
    EXPECT_TRUE(manager->registerErrorCallback([&](const std::string& error) {
        errorCallbackCalled = true;
    }));
    
    EXPECT_TRUE(manager->registerStatsCallback([&](const SatoxManager::SystemStats& stats) {
        statsCallbackCalled = true;
    }));
    
    // Start manager
    EXPECT_TRUE(manager->start());
    EXPECT_TRUE(stateCallbackCalled);
    
    // Enable stats
    EXPECT_TRUE(manager->enableStats(true));
    EXPECT_TRUE(statsCallbackCalled);
    
    // Unregister callbacks
    EXPECT_TRUE(manager->unregisterStateCallback());
    EXPECT_TRUE(manager->unregisterErrorCallback());
    EXPECT_TRUE(manager->unregisterStatsCallback());
    
    stateCallbackCalled = false;
    errorCallbackCalled = false;
    statsCallbackCalled = false;
    
    EXPECT_TRUE(manager->stop());
    EXPECT_FALSE(stateCallbackCalled);
    EXPECT_FALSE(errorCallbackCalled);
    EXPECT_FALSE(statsCallbackCalled);
}

// Error Handling Tests
TEST_F(SatoxManagerTest, ErrorHandling) {
    // Test invalid initialization
    EXPECT_FALSE(manager->start());
    EXPECT_FALSE(manager->getLastError().empty());
    
    // Test invalid update
    EXPECT_FALSE(manager->update(nlohmann::json::object()));
    EXPECT_FALSE(manager->getLastError().empty());
    
    // Test invalid pause/resume
    EXPECT_FALSE(manager->pause());
    EXPECT_FALSE(manager->getLastError().empty());
    EXPECT_FALSE(manager->resume());
    EXPECT_FALSE(manager->getLastError().empty());
    
    // Test clear error
    EXPECT_TRUE(manager->clearLastError());
    EXPECT_TRUE(manager->getLastError().empty());
}

// Concurrency Tests
TEST_F(SatoxManagerTest, Concurrency) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    EXPECT_TRUE(manager->start());
    
    const int numThreads = 10;
    const int numOperationsPerThread = 100;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([this, i, numOperationsPerThread]() {
            for (int j = 0; j < numOperationsPerThread; ++j) {
                // Perform operations
                auto info = manager->getSystemInfo();
                auto stats = manager->getSystemStats();
                
                // Update configuration
                nlohmann::json config = {
                    {"test_key", "test_value_" + std::to_string(i) + "_" + std::to_string(j)}
                };
                manager->update(config);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_TRUE(manager->stop());
}

// Edge Cases Tests
TEST_F(SatoxManagerTest, EdgeCases) {
    // Test empty configuration
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    
    // Test invalid configuration
    nlohmann::json invalidConfig = {
        {"invalid_key", "invalid_value"}
    };
    EXPECT_TRUE(manager->initialize(invalidConfig));
    
    // Test multiple start/stop
    EXPECT_TRUE(manager->start());
    EXPECT_TRUE(manager->start()); // Should be idempotent
    EXPECT_TRUE(manager->stop());
    EXPECT_TRUE(manager->stop()); // Should be idempotent
    
    // Test pause/resume without start
    EXPECT_FALSE(manager->pause());
    EXPECT_FALSE(manager->resume());
}

// Cleanup Tests
TEST_F(SatoxManagerTest, Cleanup) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    EXPECT_TRUE(manager->start());
    
    // Perform operations
    auto info = manager->getSystemInfo();
    auto stats = manager->getSystemStats();
    
    // Shutdown
    EXPECT_TRUE(manager->shutdown());
    EXPECT_FALSE(manager->isInitialized());
    EXPECT_FALSE(manager->isRunning());
}

// Stress Tests
TEST_F(SatoxManagerTest, StressTest) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    EXPECT_TRUE(manager->start());
    
    const int numOperations = 1000;
    for (int i = 0; i < numOperations; ++i) {
        // Perform operations
        auto info = manager->getSystemInfo();
        auto stats = manager->getSystemStats();
        
        // Update configuration
        nlohmann::json config = {
            {"test_key", "test_value_" + std::to_string(i)}
        };
        manager->update(config);
    }
    
    EXPECT_TRUE(manager->stop());
} 