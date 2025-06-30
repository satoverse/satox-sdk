/*
 * MIT License
 * Copyright (c) 2025 Satoxcoin Core Developer
 */
#include <gtest/gtest.h>
#include "satox/rpc_proxy/rpc_proxy_manager.hpp"

using namespace satox::rpc_proxy;

class RpcProxyManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        config.endpoint = "http://localhost:8545";
        config.username = "user";
        config.password = "pass";
        config.timeout_ms = 1000;
        config.enableLogging = false;
        config.logPath = "logs/components/rpc_proxy/";
    }
    RpcProxyConfig config;
};

TEST_F(RpcProxyManagerTest, SingletonInstance) {
    auto& proxy1 = RpcProxyManager::getInstance();
    auto& proxy2 = RpcProxyManager::getInstance();
    EXPECT_EQ(&proxy1, &proxy2);
}

TEST_F(RpcProxyManagerTest, InitializationAndShutdown) {
    auto& proxy = RpcProxyManager::getInstance();
    EXPECT_TRUE(proxy.initialize(config));
    EXPECT_TRUE(proxy.isInitialized());
    proxy.shutdown();
    EXPECT_FALSE(proxy.isInitialized());
}

TEST_F(RpcProxyManagerTest, SendRpcRequest) {
    auto& proxy = RpcProxyManager::getInstance();
    proxy.initialize(config);
    nlohmann::json req = {{"method", "ping"}, {"id", "1"}};
    nlohmann::json resp;
    EXPECT_TRUE(proxy.sendRpcRequest(req, resp));
    EXPECT_EQ(resp["result"], "ok");
    proxy.shutdown();
} 