#include <gtest/gtest.h>
#include "satox/core/core_manager.hpp"

using namespace satox::core;

class CoreManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<CoreManager>();
        config.data_dir = "/tmp/satox-test";
        config.network = "mainnet";
        config.enable_mining = false;
        config.enable_sync = true;
        config.sync_interval_ms = 1000;
        config.mining_threads = 1;
        config.rpc_endpoint = "localhost:7777";
        config.rpc_username = "test";
        config.rpc_password = "test";
        config.timeout_ms = 30000;
    }

    void TearDown() override {
        manager.reset();
    }

    std::unique_ptr<CoreManager> manager;
    CoreConfig config;
};

TEST_F(CoreManagerTest, Initialization) {
    EXPECT_TRUE(manager->initialize(config));
    EXPECT_TRUE(manager->isRunning());
}

TEST_F(CoreManagerTest, Shutdown) {
    EXPECT_TRUE(manager->initialize(config));
    manager->shutdown();
    EXPECT_FALSE(manager->isRunning());
}

TEST_F(CoreManagerTest, StartStop) {
    EXPECT_TRUE(manager->initialize(config));
    EXPECT_TRUE(manager->start());
    EXPECT_TRUE(manager->isRunning());
    EXPECT_TRUE(manager->stop());
    EXPECT_FALSE(manager->isRunning());
}

TEST_F(CoreManagerTest, DoubleInitialization) {
    EXPECT_TRUE(manager->initialize(config));
    EXPECT_FALSE(manager->initialize(config));
    EXPECT_EQ(manager->getLastError(), "CoreManager already initialized");
}

TEST_F(CoreManagerTest, StartWithoutInitialization) {
    EXPECT_FALSE(manager->start());
    EXPECT_EQ(manager->getLastError(), "CoreManager not initialized");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 