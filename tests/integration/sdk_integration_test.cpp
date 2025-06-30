#include "satox/satox.hpp"
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

class SDKIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Configure logging for tests (removed global set_level due to segfault)
        
        // Basic SDK configuration
        config_ = {
            {"core", {
                {"name", "test_core"},
                {"enableLogging", true},
                {"logPath", "logs/components/core/"},
                {"data_dir", "/tmp/satox_test_data"},
                {"network", "testnet"},
                {"network_config", {
                    {"host", "localhost"},
                    {"p2p_port", 8333},
                    {"rpc_port", 8332},
                    {"max_connections", 10},
                    {"enable_ssl", false}
                }},
                {"sync_interval_ms", 1000},
                {"mining_threads", 1},
                {"timeout_ms", 30000},
                {"enable_mining", false},
                {"enable_sync", true}
            }},
            {"database", {
                {"name", "test_database"},
                {"type", "memory"},
                {"enableLogging", true},
                {"logPath", "logs/components/database/"}
            }},
            {"security", {
                {"name", "test_security"},
                {"enablePQC", true},
                {"enableInputValidation", true},
                {"enableRateLimiting", true},
                {"enableLogging", true},
                {"logPath", "logs/components/security/"}
            }},
            {"blockchain", {
                {"name", "test_blockchain"},
                {"network", "testnet"},
                {"enableLogging", true},
                {"logPath", "logs/components/blockchain/"}
            }},
            {"asset", {
                {"name", "test_asset"},
                {"enableLogging", true},
                {"logPath", "logs/components/asset/"}
            }},
            {"nft", {
                {"name", "test_nft"},
                {"enableLogging", true},
                {"logPath", "logs/components/nft/"}
            }},
            {"ipfs", {
                {"name", "test_ipfs"},
                {"enableLogging", true},
                {"logPath", "logs/components/ipfs/"}
            }},
            {"network", {
                {"name", "test_network"},
                {"enableLogging", true},
                {"logPath", "logs/components/network/"}
            }},
            {"wallet", {
                {"name", "test_wallet"},
                {"enableLogging", true},
                {"logPath", "logs/components/wallet/"}
            }}
        };
    }

    void TearDown() override {
        // Don't automatically shutdown - let individual tests handle it
        // This prevents segfaults during test cleanup
    }

    nlohmann::json config_;
};

TEST_F(SDKIntegrationTest, SDKInitialization) {
    auto& sdk = satox::SDK::getInstance();
    
    EXPECT_FALSE(sdk.isInitialized());
    
    bool result = sdk.initialize(config_);
    EXPECT_TRUE(result);
    EXPECT_TRUE(sdk.isInitialized());
    
    EXPECT_EQ(sdk.getVersion(), SATOX_SDK_VERSION);
}

TEST_F(SDKIntegrationTest, ComponentAccess) {
    auto& sdk = satox::SDK::getInstance();
    ASSERT_TRUE(sdk.initialize(config_));
    
    // Test that all component managers are accessible
    EXPECT_NO_THROW(sdk.getCoreManager());
    EXPECT_NO_THROW(sdk.getDatabaseManager());
    EXPECT_NO_THROW(sdk.getSecurityManager());
    EXPECT_NO_THROW(sdk.getBlockchainManager());
    EXPECT_NO_THROW(sdk.getAssetManager());
    EXPECT_NO_THROW(sdk.getNFTManager());
    EXPECT_NO_THROW(sdk.getIPFSManager());
    EXPECT_NO_THROW(sdk.getNetworkManager());
    EXPECT_NO_THROW(sdk.getWalletManager());
}

TEST_F(SDKIntegrationTest, ConvenienceFunctions) {
    auto& sdk = satox::SDK::getInstance();
    ASSERT_TRUE(sdk.initialize(config_));
    
    // Test convenience functions
    EXPECT_NO_THROW(satox::managers::core());
    EXPECT_NO_THROW(satox::managers::database());
    EXPECT_NO_THROW(satox::managers::security());
    EXPECT_NO_THROW(satox::managers::blockchain());
    EXPECT_NO_THROW(satox::managers::asset());
    EXPECT_NO_THROW(satox::managers::nft());
    EXPECT_NO_THROW(satox::managers::ipfs());
    EXPECT_NO_THROW(satox::managers::network());
    EXPECT_NO_THROW(satox::managers::wallet());
}

TEST_F(SDKIntegrationTest, HealthStatus) {
    auto& sdk = satox::SDK::getInstance();
    ASSERT_TRUE(sdk.initialize(config_));
    
    auto health = sdk.getHealthStatus();
    
    EXPECT_TRUE(health.contains("sdk"));
    EXPECT_TRUE(health["sdk"]["initialized"]);
    EXPECT_EQ(health["sdk"]["version"], SATOX_SDK_VERSION);
    
    EXPECT_TRUE(health.contains("components"));
    EXPECT_TRUE(health["components"].contains("core"));
    EXPECT_TRUE(health["components"].contains("database"));
    EXPECT_TRUE(health["components"].contains("security"));
    EXPECT_TRUE(health["components"].contains("blockchain"));
    EXPECT_TRUE(health["components"].contains("asset"));
    EXPECT_TRUE(health["components"].contains("nft"));
    EXPECT_TRUE(health["components"].contains("ipfs"));
    EXPECT_TRUE(health["components"].contains("network"));
    EXPECT_TRUE(health["components"].contains("wallet"));
}

TEST_F(SDKIntegrationTest, Statistics) {
    auto& sdk = satox::SDK::getInstance();
    ASSERT_TRUE(sdk.initialize(config_));
    
    auto stats = sdk.getStats();
    
    EXPECT_TRUE(stats.contains("sdk"));
    EXPECT_TRUE(stats["sdk"]["initialized"]);
    EXPECT_EQ(stats["sdk"]["version"], SATOX_SDK_VERSION);
    EXPECT_TRUE(stats["sdk"].contains("uptime_seconds"));
    
    EXPECT_TRUE(stats.contains("components"));
    EXPECT_TRUE(stats["components"].contains("core"));
    EXPECT_TRUE(stats["components"].contains("database"));
    EXPECT_TRUE(stats["components"].contains("security"));
    EXPECT_TRUE(stats["components"].contains("blockchain"));
    EXPECT_TRUE(stats["components"].contains("asset"));
    EXPECT_TRUE(stats["components"].contains("nft"));
    EXPECT_TRUE(stats["components"].contains("ipfs"));
    EXPECT_TRUE(stats["components"].contains("network"));
    EXPECT_TRUE(stats["components"].contains("wallet"));
}

TEST_F(SDKIntegrationTest, SDKShutdown) {
    auto& sdk = satox::SDK::getInstance();
    ASSERT_TRUE(sdk.initialize(config_));
    EXPECT_TRUE(sdk.isInitialized());
    
    sdk.shutdown();
    EXPECT_FALSE(sdk.isInitialized());
}

TEST_F(SDKIntegrationTest, ComponentInteraction) {
    auto& sdk = satox::SDK::getInstance();
    ASSERT_TRUE(sdk.initialize(config_));
    
    // Test basic component interactions
    auto& core = sdk.getCoreManager();
    auto& database = sdk.getDatabaseManager();
    auto& security = sdk.getSecurityManager();
    
    // Test that components are healthy
    EXPECT_TRUE(core.isHealthy());
    EXPECT_TRUE(database.isHealthy());
    EXPECT_TRUE(security.isHealthy());
    
    // Test that components can perform basic operations
    EXPECT_TRUE(core.isInitialized());
    EXPECT_TRUE(database.isInitialized());
    EXPECT_TRUE(security.isInitialized());
}

TEST_F(SDKIntegrationTest, ErrorHandling) {
    auto& sdk = satox::SDK::getInstance();
    
    // Test accessing managers before initialization
    EXPECT_THROW(sdk.getCoreManager(), std::runtime_error);
    EXPECT_THROW(sdk.getDatabaseManager(), std::runtime_error);
    EXPECT_THROW(sdk.getSecurityManager(), std::runtime_error);
    
    // Test double initialization
    ASSERT_TRUE(sdk.initialize(config_));
    EXPECT_TRUE(sdk.initialize(config_)); // Should return true for already initialized
    
    // Test shutdown
    sdk.shutdown();
    EXPECT_FALSE(sdk.isInitialized());
}

TEST_F(SDKIntegrationTest, GlobalMacros) {
    auto& sdk = satox::SDK::getInstance();
    ASSERT_TRUE(sdk.initialize(config_));
    
    // Test global macro access
    EXPECT_NO_THROW(SATOX_SDK);
    EXPECT_NO_THROW(SATOX_CORE);
    EXPECT_NO_THROW(SATOX_DATABASE);
    EXPECT_NO_THROW(SATOX_SECURITY);
    EXPECT_NO_THROW(SATOX_BLOCKCHAIN);
    EXPECT_NO_THROW(SATOX_ASSET);
    EXPECT_NO_THROW(SATOX_NFT);
    EXPECT_NO_THROW(SATOX_IPFS);
    EXPECT_NO_THROW(SATOX_NETWORK);
    EXPECT_NO_THROW(SATOX_WALLET);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    std::cerr << "[DEBUG] End of main() reached in sdk_integration_test" << std::endl;
    return result;
} 