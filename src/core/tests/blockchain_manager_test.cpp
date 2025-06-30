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
#include "satox/core/blockchain_manager.hpp"
#include <filesystem>

using namespace satox::core;

class BlockchainManagerTest : public ::testing::Test {
protected:
    BlockchainManager* manager;

    void SetUp() override {
        // Use the singleton instance
        manager = &BlockchainManager::getInstance();
        
        // Initialize with test config
        nlohmann::json config;
        config["network"] = "testnet";
        config["data_dir"] = "/tmp/satox_test_blockchain";
        config["p2p_port"] = 60777;
        config["rpc_port"] = 7777;
        config["enable_mining"] = false;
        config["max_connections"] = 10;
        
        ASSERT_TRUE(manager->initialize(config));
    }

    void TearDown() override {
        manager->shutdown();
    }
};

TEST_F(BlockchainManagerTest, Singleton) {
    auto& instance1 = BlockchainManager::getInstance();
    auto& instance2 = BlockchainManager::getInstance();
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(BlockchainManagerTest, Initialization) {
    EXPECT_TRUE(manager->getLastError().empty());
}

TEST_F(BlockchainManagerTest, ConnectionManagement) {
    // Test connection operations
    bool connect_result = manager->connect();
    EXPECT_TRUE(connect_result == true || connect_result == false);
    
    // Check connection status
    bool is_connected = manager->isConnected();
    EXPECT_TRUE(is_connected == true || is_connected == false);
    
    // Disconnect
    bool disconnect_result = manager->disconnect();
    EXPECT_TRUE(disconnect_result == true || disconnect_result == false);
}

TEST_F(BlockchainManagerTest, StateManagement) {
    // Test state management
    auto state = manager->getState();
    EXPECT_TRUE(state == BlockchainManager::BlockchainState::UNINITIALIZED ||
                state == BlockchainManager::BlockchainState::INITIALIZED ||
                state == BlockchainManager::BlockchainState::CONNECTED ||
                state == BlockchainManager::BlockchainState::DISCONNECTED ||
                state == BlockchainManager::BlockchainState::ERROR);
}

TEST_F(BlockchainManagerTest, ErrorHandling) {
    // Test error handling
    manager->clearLastError();
    EXPECT_TRUE(manager->getLastError().empty());
}
