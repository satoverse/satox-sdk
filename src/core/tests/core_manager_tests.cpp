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
#include <fstream>
#include <nlohmann/json.hpp>
#include <filesystem>

namespace satox {
namespace core {
namespace tests {

class CoreManagerTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test configuration
        nlohmann::json config = {
            {"network", {
                {"type", "mainnet"},
                {"port", 7777}
            }},
            {"security", {
                {"encryption_level", "high"},
                {"key_storage_path", "test_keys"}
            }},
            {"database", {
                {"type", "sqlite"},
                {"connection_string", "test.db"}
            }},
            {"wallet", {
                {"storage_path", "test_wallets"},
                {"encryption_key", "test_key"}
            }},
            {"quantum", {
                {"algorithm", "kyber"},
                {"key_size", 1024}
            }}
        };

        // Write test configuration
        std::ofstream configFile("test_config.json");
        configFile << config.dump(4);
        configFile.close();
    }

    void TearDown() override {
        // Clean up test files
        std::filesystem::remove("test_config.json");
        std::filesystem::remove_all("test_keys");
        std::filesystem::remove_all("test_wallets");
        std::filesystem::remove("test.db");
    }
};

TEST_F(CoreManagerTests, SingletonInstance) {
    auto& instance1 = SatoxManager::getInstance();
    auto& instance2 = SatoxManager::getInstance();
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(CoreManagerTests, InitializationSuccess) {
    auto& manager = SatoxManager::getInstance();
    EXPECT_TRUE(manager.initialize("test_config.json"));
    EXPECT_TRUE(manager.hasComponent("network"));
    EXPECT_TRUE(manager.hasComponent("security"));
    EXPECT_TRUE(manager.hasComponent("database"));
    EXPECT_TRUE(manager.hasComponent("wallet"));
    EXPECT_TRUE(manager.hasComponent("quantum"));
}

TEST_F(CoreManagerTests, DoubleInitialization) {
    auto& manager = SatoxManager::getInstance();
    EXPECT_TRUE(manager.initialize("test_config.json"));
    EXPECT_FALSE(manager.initialize("test_config.json"));
    EXPECT_EQ(manager.getLastError(), "SDK already initialized");
}

TEST_F(CoreManagerTests, InvalidConfigPath) {
    auto& manager = SatoxManager::getInstance();
    EXPECT_FALSE(manager.initialize("nonexistent_config.json"));
    EXPECT_EQ(manager.getLastError(), "Configuration file does not exist: nonexistent_config.json");
}

TEST_F(CoreManagerTests, InvalidConfigFormat) {
    // Create invalid JSON
    std::ofstream configFile("invalid_config.json");
    configFile << "invalid json content";
    configFile.close();

    auto& manager = SatoxManager::getInstance();
    EXPECT_FALSE(manager.initialize("invalid_config.json"));
    EXPECT_TRUE(manager.getLastError().find("Failed to parse configuration file") != std::string::npos);

    std::filesystem::remove("invalid_config.json");
}

TEST_F(CoreManagerTests, MissingConfigSection) {
    // Create config with missing section
    nlohmann::json config = {
        {"network", {
            {"type", "mainnet"},
            {"port", 7777}
        }},
        {"security", {
            {"encryption_level", "high"},
            {"key_storage_path", "test_keys"}
        }}
        // Missing database, wallet, and quantum sections
    };

    std::ofstream configFile("incomplete_config.json");
    configFile << config.dump(4);
    configFile.close();

    auto& manager = SatoxManager::getInstance();
    EXPECT_FALSE(manager.initialize("incomplete_config.json"));
    EXPECT_TRUE(manager.getLastError().find("Missing required configuration section") != std::string::npos);

    std::filesystem::remove("incomplete_config.json");
}

TEST_F(CoreManagerTests, ComponentRegistration) {
    auto& manager = SatoxManager::getInstance();
    EXPECT_TRUE(manager.initialize("test_config.json"));

    // Test component retrieval
    auto network = manager.getComponent<NetworkComponent>("network");
    EXPECT_NE(network, nullptr);
    EXPECT_EQ(network->getType(), "mainnet");
    EXPECT_EQ(network->getPort(), 7777);

    auto security = manager.getComponent<SecurityComponent>("security");
    EXPECT_NE(security, nullptr);
    EXPECT_EQ(security->getEncryptionLevel(), "high");

    auto database = manager.getComponent<DatabaseComponent>("database");
    EXPECT_NE(database, nullptr);
    EXPECT_EQ(database->getType(), "sqlite");

    auto wallet = manager.getComponent<WalletComponent>("wallet");
    EXPECT_NE(wallet, nullptr);
    EXPECT_EQ(wallet->getStoragePath(), "test_wallets");

    auto quantum = manager.getComponent<QuantumComponent>("quantum");
    EXPECT_NE(quantum, nullptr);
    EXPECT_EQ(quantum->getAlgorithm(), "kyber");
    EXPECT_EQ(quantum->getKeySize(), 1024);
}

TEST_F(CoreManagerTests, ComponentNotFound) {
    auto& manager = SatoxManager::getInstance();
    EXPECT_TRUE(manager.initialize("test_config.json"));

    auto nonexistent = manager.getComponent<NetworkComponent>("nonexistent");
    EXPECT_EQ(nonexistent, nullptr);
    EXPECT_EQ(manager.getLastError(), "Component not found: nonexistent");
}

TEST_F(CoreManagerTests, Shutdown) {
    auto& manager = SatoxManager::getInstance();
    EXPECT_TRUE(manager.initialize("test_config.json"));

    // Verify components exist
    EXPECT_TRUE(manager.hasComponent("network"));
    EXPECT_TRUE(manager.hasComponent("security"));
    EXPECT_TRUE(manager.hasComponent("database"));
    EXPECT_TRUE(manager.hasComponent("wallet"));
    EXPECT_TRUE(manager.hasComponent("quantum"));

    // Shutdown
    manager.shutdown();

    // Verify components are removed
    EXPECT_FALSE(manager.hasComponent("network"));
    EXPECT_FALSE(manager.hasComponent("security"));
    EXPECT_FALSE(manager.hasComponent("database"));
    EXPECT_FALSE(manager.hasComponent("wallet"));
    EXPECT_FALSE(manager.hasComponent("quantum"));
}

TEST_F(CoreManagerTests, ErrorHandling) {
    auto& manager = SatoxManager::getInstance();
    
    // Test error setting
    manager.setError("Test error");
    EXPECT_EQ(manager.getLastError(), "Test error");

    // Test error clearing
    manager.clearLastError();
    EXPECT_TRUE(manager.getLastError().empty());
}

TEST_F(CoreManagerTests, DirectoryCreation) {
    auto& manager = SatoxManager::getInstance();
    EXPECT_TRUE(manager.initialize("test_config.json"));

    // Verify directories are created
    EXPECT_TRUE(std::filesystem::exists("test_keys"));
    EXPECT_TRUE(std::filesystem::exists("test_wallets"));
}

TEST_F(CoreManagerTests, ComponentInitializationOrder) {
    auto& manager = SatoxManager::getInstance();
    EXPECT_TRUE(manager.initialize("test_config.json"));

    // Components should be initialized in the correct order
    auto network = manager.getComponent<NetworkComponent>("network");
    auto security = manager.getComponent<SecurityComponent>("security");
    auto database = manager.getComponent<DatabaseComponent>("database");
    auto wallet = manager.getComponent<WalletComponent>("wallet");
    auto quantum = manager.getComponent<QuantumComponent>("quantum");

    EXPECT_TRUE(network->isInitialized());
    EXPECT_TRUE(security->isInitialized());
    EXPECT_TRUE(database->isInitialized());
    EXPECT_TRUE(wallet->isInitialized());
    EXPECT_TRUE(quantum->isInitialized());
}

} // namespace tests
} // namespace core
} // namespace satox 