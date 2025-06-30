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
#include "satox/core/security_manager.hpp"
#include <filesystem>

using namespace satox::core;

class SecurityManagerTest : public ::testing::Test {
protected:
    std::unique_ptr<SecurityManager> manager;
    SecurityManager::SecurityConfig config;

    void SetUp() override {
        // Create a new instance
        manager = std::make_unique<SecurityManager>();
        
        // Configure for testing
        config.network = "test_network";
        config.key_length = 32;
        config.sessionTimeout = 3600;
        
        // Initialize with config
        ASSERT_TRUE(manager->initialize(config));
    }

    void TearDown() override {
        if (manager) {
            manager->shutdown();
        }
    }
};

TEST_F(SecurityManagerTest, Initialization) {
    EXPECT_TRUE(manager->getLastError().empty());
}

TEST_F(SecurityManagerTest, Configuration) {
    // Test configuration
    auto current_config = manager->getConfig();
    EXPECT_EQ(current_config.network, "test_network");
    EXPECT_EQ(current_config.key_length, 32);
    EXPECT_EQ(current_config.sessionTimeout, 3600);
}

TEST_F(SecurityManagerTest, ErrorHandling) {
    // Test error handling
    manager->setLastError("Test error");
    EXPECT_EQ(manager->getLastError(), "Test error");
    
    // Clear error
    manager->setLastError("");
    EXPECT_TRUE(manager->getLastError().empty());
}

TEST_F(SecurityManagerTest, SecurityLevel) {
    // Test security level management
    auto initial_level = manager->getSecurityLevel();
    EXPECT_TRUE(initial_level == SecurityManager::SecurityLevel::LOW ||
                initial_level == SecurityManager::SecurityLevel::MEDIUM ||
                initial_level == SecurityManager::SecurityLevel::HIGH);
    
    // Set security level
    manager->setSecurityLevel(SecurityManager::SecurityLevel::HIGH);
    EXPECT_EQ(manager->getSecurityLevel(), SecurityManager::SecurityLevel::HIGH);
}

TEST_F(SecurityManagerTest, Statistics) {
    // Test statistics
    auto stats = manager->getStats();
    EXPECT_GE(stats.totalEncryptions, 0);
    EXPECT_GE(stats.totalDecryptions, 0);
    EXPECT_GE(stats.totalAuths, 0);
    EXPECT_GE(stats.successfulLogins, 0);
    EXPECT_GE(stats.failedAttempts, 0);
    
    // Reset stats
    manager->resetStats();
    auto reset_stats = manager->getStats();
    EXPECT_EQ(reset_stats.totalEncryptions, 0);
    EXPECT_EQ(reset_stats.totalDecryptions, 0);
}
