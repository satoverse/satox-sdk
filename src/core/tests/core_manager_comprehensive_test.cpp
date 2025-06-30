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
#include "satox/core/core_manager.hpp"
#include <thread>
#include <chrono>
#include <random>
#include <fstream>
#include <filesystem>

using namespace satox::core;

class CoreManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = &CoreManager::getInstance();
        manager->shutdown(); // Ensure clean state
    }

    void TearDown() override {
        manager->shutdown();
    }

    CoreManager* manager;
};

// Initialization Tests
TEST_F(CoreManagerTest, Initialization) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    EXPECT_FALSE(manager->initialize(nlohmann::json::object())); // Should fail on second init
    EXPECT_TRUE(manager->isInitialized());
}

// Lifecycle Tests
TEST_F(CoreManagerTest, Lifecycle) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    
    // Start
    EXPECT_TRUE(manager->start());
    EXPECT_TRUE(manager->isRunning());
    
    // Pause
    EXPECT_TRUE(manager->pause());
    EXPECT_FALSE(manager->isRunning());
    
    // Resume
    EXPECT_TRUE(manager->resume());
    EXPECT_TRUE(manager->isRunning());
    
    // Stop
    EXPECT_TRUE(manager->stop());
    EXPECT_FALSE(manager->isRunning());
}

// Configuration Tests
TEST_F(CoreManagerTest, Configuration) {
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

// Manager Integration Tests
TEST_F(CoreManagerTest, ManagerIntegration) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    EXPECT_TRUE(manager->start());
    
    // Test Logging Manager
    auto& loggingManager = LoggingManager::getInstance();
    EXPECT_TRUE(loggingManager.isInitialized());
    EXPECT_TRUE(loggingManager.isRunning());
    
    // Test Config Manager
    auto& configManager = ConfigManager::getInstance();
    EXPECT_TRUE(configManager.isInitialized());
    EXPECT_TRUE(configManager.isRunning());
    
    // Test Cache Manager
    auto& cacheManager = CacheManager::getInstance();
    EXPECT_TRUE(cacheManager.isInitialized());
    EXPECT_TRUE(cacheManager.isRunning());
    
    // Test Event Manager
    auto& eventManager = EventManager::getInstance();
    EXPECT_TRUE(eventManager.isInitialized());
    EXPECT_TRUE(eventManager.isRunning());
    
    // Test Plugin Manager
    auto& pluginManager = PluginManager::getInstance();
    EXPECT_TRUE(pluginManager.isInitialized());
    EXPECT_TRUE(pluginManager.isRunning());
    
    // Test Network Manager
    auto& networkManager = NetworkManager::getInstance();
    EXPECT_TRUE(networkManager.isInitialized());
    EXPECT_TRUE(networkManager.isRunning());
    
    // Test Blockchain Manager
    auto& blockchainManager = BlockchainManager::getInstance();
    EXPECT_TRUE(blockchainManager.isInitialized());
    EXPECT_TRUE(blockchainManager.isRunning());
    
    // Test Transaction Manager
    auto& transactionManager = TransactionManager::getInstance();
    EXPECT_TRUE(transactionManager.isInitialized());
    EXPECT_TRUE(transactionManager.isRunning());
    
    // Test Asset Manager
    auto& assetManager = AssetManager::getInstance();
    EXPECT_TRUE(assetManager.isInitialized());
    EXPECT_TRUE(assetManager.isRunning());
    
    // Test Wallet Manager
    auto& walletManager = WalletManager::getInstance();
    EXPECT_TRUE(walletManager.isInitialized());
    EXPECT_TRUE(walletManager.isRunning());
    
    // Test IPFS Manager
    auto& ipfsManager = IPFSManager::getInstance();
    EXPECT_TRUE(ipfsManager.isInitialized());
    EXPECT_TRUE(ipfsManager.isRunning());
    
    // Test Database Manager
    auto& databaseManager = DatabaseManager::getInstance();
    EXPECT_TRUE(databaseManager.isInitialized());
    EXPECT_TRUE(databaseManager.isRunning());
    
    // Test Security Manager
    auto& securityManager = SecurityManager::getInstance();
    EXPECT_TRUE(securityManager.isInitialized());
    EXPECT_TRUE(securityManager.isRunning());
    
    // Test NFT Manager
    auto& nftManager = NFTManager::getInstance();
    EXPECT_TRUE(nftManager.isInitialized());
    EXPECT_TRUE(nftManager.isRunning());
    
    EXPECT_TRUE(manager->stop());
}

// Event Handling Tests
TEST_F(CoreManagerTest, EventHandling) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    EXPECT_TRUE(manager->start());
    
    auto& eventManager = EventManager::getInstance();
    
    // Test system event
    Event systemEvent;
    systemEvent.type = EventType::SYSTEM;
    systemEvent.name = "test_system_event";
    systemEvent.source = "test";
    systemEvent.priority = EventPriority::NORMAL;
    systemEvent.data = {{"key", "value"}};
    
    EXPECT_TRUE(eventManager.publishEvent(systemEvent));
    
    // Test network event
    Event networkEvent;
    networkEvent.type = EventType::NETWORK;
    networkEvent.name = "test_network_event";
    networkEvent.source = "test";
    networkEvent.priority = EventPriority::NORMAL;
    networkEvent.data = {{"key", "value"}};
    
    EXPECT_TRUE(eventManager.publishEvent(networkEvent));
    
    // Test blockchain event
    Event blockchainEvent;
    blockchainEvent.type = EventType::BLOCKCHAIN;
    blockchainEvent.name = "test_blockchain_event";
    blockchainEvent.source = "test";
    blockchainEvent.priority = EventPriority::NORMAL;
    blockchainEvent.data = {{"key", "value"}};
    
    EXPECT_TRUE(eventManager.publishEvent(blockchainEvent));
    
    EXPECT_TRUE(manager->stop());
}

// Statistics Tests
TEST_F(CoreManagerTest, Statistics) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    EXPECT_TRUE(manager->start());
    
    // Enable statistics
    manager->enableStats(true);
    
    // Get info
    auto info = manager->getInfo();
    EXPECT_TRUE(info.initialized);
    EXPECT_TRUE(info.running);
    EXPECT_FALSE(info.paused);
    EXPECT_FALSE(info.managers.empty());
    
    // Get stats
    auto stats = manager->getStats();
    EXPECT_GE(stats.uptime, 0);
    EXPECT_FALSE(stats.managers.empty());
    
    // Reset stats
    manager->resetStats();
    stats = manager->getStats();
    EXPECT_EQ(stats.uptime, 0);
    
    EXPECT_TRUE(manager->stop());
}

// Callback Tests
TEST_F(CoreManagerTest, Callbacks) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    
    // Register callback
    bool callbackCalled = false;
    manager->registerCallback([&](const CoreInfo& info) {
        callbackCalled = true;
    });
    
    // Start manager
    EXPECT_TRUE(manager->start());
    EXPECT_TRUE(callbackCalled);
    
    // Unregister callback
    manager->unregisterCallback();
    callbackCalled = false;
    EXPECT_TRUE(manager->stop());
    EXPECT_FALSE(callbackCalled);
}

// Error Handling Tests
TEST_F(CoreManagerTest, ErrorHandling) {
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
}

// Concurrency Tests
TEST_F(CoreManagerTest, Concurrency) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    EXPECT_TRUE(manager->start());
    
    const int numThreads = 10;
    const int numOperationsPerThread = 100;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([this, i, numOperationsPerThread]() {
            for (int j = 0; j < numOperationsPerThread; ++j) {
                // Perform operations
                auto info = manager->getInfo();
                auto stats = manager->getStats();
                
                // Publish events
                auto& eventManager = EventManager::getInstance();
                Event event;
                event.type = EventType::SYSTEM;
                event.name = "test_event_" + std::to_string(i) + "_" + std::to_string(j);
                event.source = "test";
                event.priority = EventPriority::NORMAL;
                event.data = {{"key", "value"}};
                eventManager.publishEvent(event);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_TRUE(manager->stop());
}

// Edge Cases Tests
TEST_F(CoreManagerTest, EdgeCases) {
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
TEST_F(CoreManagerTest, Cleanup) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    EXPECT_TRUE(manager->start());
    
    // Perform operations
    auto info = manager->getInfo();
    auto stats = manager->getStats();
    
    // Shutdown
    EXPECT_TRUE(manager->shutdown());
    EXPECT_FALSE(manager->isInitialized());
    EXPECT_FALSE(manager->isRunning());
}

// Stress Tests
TEST_F(CoreManagerTest, StressTest) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    EXPECT_TRUE(manager->start());
    
    const int numOperations = 1000;
    for (int i = 0; i < numOperations; ++i) {
        // Perform operations
        auto info = manager->getInfo();
        auto stats = manager->getStats();
        
        // Publish events
        auto& eventManager = EventManager::getInstance();
        Event event;
        event.type = EventType::SYSTEM;
        event.name = "test_event_" + std::to_string(i);
        event.source = "test";
        event.priority = EventPriority::NORMAL;
        event.data = {{"key", "value"}};
        eventManager.publishEvent(event);
        
        // Update configuration
        nlohmann::json config = {
            {"test_key", "test_value_" + std::to_string(i)}
        };
        manager->update(config);
    }
    
    EXPECT_TRUE(manager->stop());
} 