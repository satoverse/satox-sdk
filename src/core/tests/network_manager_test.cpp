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
#include "satox/core/network_manager.hpp"
#include <filesystem>

using namespace satox::core;

class NetworkManagerTest : public ::testing::Test {
protected:
    NetworkManager* manager;

    void SetUp() override {
        // Use the singleton instance
        manager = &NetworkManager::getInstance();
        
        // Initialize with test config
        nlohmann::json config;
        config["network_type"] = "testnet";
        config["node_type"] = "light";
        config["max_connections"] = 50;
        config["connection_timeout_ms"] = 5000;
        
        ASSERT_TRUE(manager->initialize(config));
    }

    void TearDown() override {
        manager->shutdown();
    }
};

TEST_F(NetworkManagerTest, Singleton) {
    auto& instance1 = NetworkManager::getInstance();
    auto& instance2 = NetworkManager::getInstance();
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(NetworkManagerTest, Initialization) {
    EXPECT_TRUE(manager->getLastError().empty());
}

TEST_F(NetworkManagerTest, ErrorHandling) {
    // Test error handling
    manager->clearLastError();
    EXPECT_TRUE(manager->getLastError().empty());
}
