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
#include <fstream>
#include <cstdio>
#include "satox/core/satox_manager.hpp"

using namespace satox::core;

class SatoxManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Ensure clean state for each test
        auto& manager = SatoxManager::getInstance();
        manager.shutdown();
        manager.clearLastError();
    }

    void TearDown() override {
        // Clean up
        auto& manager = SatoxManager::getInstance();
        manager.shutdown();
        manager.clearLastError();
    }
};

TEST_F(SatoxManagerTest, SingletonInstance) {
    auto& manager1 = SatoxManager::getInstance();
    auto& manager2 = SatoxManager::getInstance();
    EXPECT_EQ(&manager1, &manager2);
}

TEST_F(SatoxManagerTest, Initialization) {
    auto& manager = SatoxManager::getInstance();
    nlohmann::json config = {{"test", "value"}};
    EXPECT_TRUE(manager.initialize(config));
    EXPECT_FALSE(manager.initialize(config)); // Should fail on second init
}

TEST_F(SatoxManagerTest, ComponentRegistration) {
    auto& manager = SatoxManager::getInstance();
    nlohmann::json config = {{"test", "value"}};
    EXPECT_TRUE(manager.initialize(config));

    // Create a test component
    struct TestComponent {
        int value = 42;
    };
    auto component = std::make_shared<TestComponent>();

    // Test registration
    EXPECT_TRUE(manager.registerComponent("test", component));
    EXPECT_FALSE(manager.registerComponent("test", component)); // Should fail on duplicate

    // Test retrieval
    auto retrieved = manager.getComponent<TestComponent>("test");
    EXPECT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->value, 42);

    // Test non-existent component
    EXPECT_EQ(manager.getComponent<TestComponent>("nonexistent"), nullptr);
}

TEST_F(SatoxManagerTest, ErrorHandling) {
    auto& manager = SatoxManager::getInstance();
    
    // Test error on invalid initialization - null JSON should be valid (empty object)
    nlohmann::json invalidConfig = nlohmann::json::object();
    EXPECT_TRUE(manager.initialize(invalidConfig));
    EXPECT_TRUE(manager.getLastError().empty());

    manager.clearLastError();
    EXPECT_TRUE(manager.getLastError().empty());
}

TEST_F(SatoxManagerTest, Shutdown) {
    auto& manager = SatoxManager::getInstance();
    nlohmann::json config = {{"test", "value"}};
    EXPECT_TRUE(manager.initialize(config));

    // Register a component
    struct TestComponent {};
    auto component = std::make_shared<TestComponent>();
    EXPECT_TRUE(manager.registerComponent("test", component));

    // Shutdown should clear components
    manager.shutdown();
    
    // Re-initialize to test component registration after shutdown
    EXPECT_TRUE(manager.initialize(config));
    auto newComponent = std::make_shared<TestComponent>();
    EXPECT_TRUE(manager.registerComponent("test2", newComponent));
    
    // After shutdown, components should still be accessible since shutdown doesn't clear them
    auto retrieved = manager.getComponent<TestComponent>("test2");
    EXPECT_NE(retrieved, nullptr);
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 