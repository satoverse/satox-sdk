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
#include <thread>
#include <chrono>

using namespace satox::core;

class NetworkManagerComprehensiveTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test config
        config = {
            {"networkType", static_cast<int>(NetworkManager::NetworkType::MAINNET)},
            {"nodeType", static_cast<int>(NetworkManager::NodeType::FULL)},
            {"maxConnections", 10},
            {"connectionTimeout", 5000},
            {"enableStats", true}
        };
    }

    void TearDown() override {
        auto& manager = NetworkManager::getInstance();
        manager.shutdown();
    }

    nlohmann::json config;
};

// Helper function to create a test connection info
NetworkManager::ConnectionInfo createTestConnectionInfo(
    const std::string& address,
    uint16_t port) {
    NetworkManager::ConnectionInfo info;
    info.address = address;
    info.port = port;
    info.networkType = NetworkManager::NetworkType::MAINNET;
    info.nodeType = NetworkManager::NodeType::FULL;
    info.version = "1.0.0";
    info.userAgent = "SatoxSDK/1.0.0";
    info.lastSeen = std::chrono::system_clock::now();
    info.lastPing = std::chrono::system_clock::now();
    info.latency = std::chrono::milliseconds(0);
    info.isInbound = false;
    info.isOutbound = true;
    info.metadata = nlohmann::json::object();
    return info;
}

// Helper function to create a test message
nlohmann::json createTestMessage() {
    return {
        {"type", "test"},
        {"data", {
            {"key1", "value1"},
            {"key2", 42},
            {"key3", true}
        }}
    };
}

// Initialization Tests
TEST_F(NetworkManagerComprehensiveTest, InitializeWithValidConfig) {
    auto& manager = NetworkManager::getInstance();
    EXPECT_TRUE(manager.initialize(config));
    EXPECT_EQ(manager.getNetworkType(), NetworkManager::NetworkType::MAINNET);
    EXPECT_EQ(manager.getNodeType(), NetworkManager::NodeType::FULL);
    EXPECT_EQ(manager.getMaxConnections(), 10);
    EXPECT_EQ(manager.getConnectionTimeout(), std::chrono::milliseconds(5000));
}

TEST_F(NetworkManagerComprehensiveTest, InitializeWithInvalidConfig) {
    auto& manager = NetworkManager::getInstance();
    config["networkType"] = "invalid";
    EXPECT_FALSE(manager.initialize(config));
    EXPECT_FALSE(manager.getLastError().empty());
}

TEST_F(NetworkManagerComprehensiveTest, InitializeWithZeroMaxConnections) {
    auto& manager = NetworkManager::getInstance();
    config["maxConnections"] = 0;
    EXPECT_FALSE(manager.initialize(config));
    EXPECT_FALSE(manager.getLastError().empty());
}

TEST_F(NetworkManagerComprehensiveTest, InitializeWithZeroTimeout) {
    auto& manager = NetworkManager::getInstance();
    config["connectionTimeout"] = 0;
    EXPECT_FALSE(manager.initialize(config));
    EXPECT_FALSE(manager.getLastError().empty());
}

// Connection Tests
TEST_F(NetworkManagerComprehensiveTest, ConnectToValidAddress) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_TRUE(manager.connect("127.0.0.1", 7777));
    EXPECT_TRUE(manager.isConnected("127.0.0.1"));
    EXPECT_EQ(manager.getConnectionState("127.0.0.1"),
              NetworkManager::ConnectionState::CONNECTED);
}

TEST_F(NetworkManagerComprehensiveTest, ConnectToInvalidAddress) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_FALSE(manager.connect("", 7777));
    EXPECT_FALSE(manager.isConnected(""));
    EXPECT_FALSE(manager.getLastError().empty());
}

TEST_F(NetworkManagerComprehensiveTest, ConnectToInvalidPort) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_FALSE(manager.connect("127.0.0.1", 0));
    EXPECT_FALSE(manager.isConnected("127.0.0.1"));
    EXPECT_FALSE(manager.getLastError().empty());
}

TEST_F(NetworkManagerComprehensiveTest, ConnectToSameAddress) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    ASSERT_TRUE(manager.connect("127.0.0.1", 7777));
    EXPECT_FALSE(manager.connect("127.0.0.1", 7777));
    EXPECT_FALSE(manager.getLastError().empty());
}

TEST_F(NetworkManagerComprehensiveTest, ConnectToMaxConnections) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    for (int i = 0; i < 10; ++i) {
        std::string address = "127.0.0." + std::to_string(i + 1);
        EXPECT_TRUE(manager.connect(address, 7777));
    }

    EXPECT_FALSE(manager.connect("127.0.0.11", 7777));
    EXPECT_FALSE(manager.getLastError().empty());
}

// Disconnection Tests
TEST_F(NetworkManagerComprehensiveTest, DisconnectFromConnectedAddress) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    ASSERT_TRUE(manager.connect("127.0.0.1", 7777));
    EXPECT_TRUE(manager.disconnect("127.0.0.1"));
    EXPECT_FALSE(manager.isConnected("127.0.0.1"));
    EXPECT_EQ(manager.getConnectionState("127.0.0.1"),
              NetworkManager::ConnectionState::DISCONNECTED);
}

TEST_F(NetworkManagerComprehensiveTest, DisconnectFromNonexistentAddress) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_FALSE(manager.disconnect("127.0.0.1"));
    EXPECT_FALSE(manager.getLastError().empty());
}

// Message Tests
TEST_F(NetworkManagerComprehensiveTest, SendMessageToConnectedAddress) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    ASSERT_TRUE(manager.connect("127.0.0.1", 7777));
    auto message = createTestMessage();
    EXPECT_TRUE(manager.sendMessage("127.0.0.1", "test", message));
}

TEST_F(NetworkManagerComprehensiveTest, SendMessageToNonexistentAddress) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    auto message = createTestMessage();
    EXPECT_FALSE(manager.sendMessage("127.0.0.1", "test", message));
    EXPECT_FALSE(manager.getLastError().empty());
}

TEST_F(NetworkManagerComprehensiveTest, SendInvalidMessage) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    ASSERT_TRUE(manager.connect("127.0.0.1", 7777));
    EXPECT_FALSE(manager.sendMessage("127.0.0.1", "", nlohmann::json::array()));
    EXPECT_FALSE(manager.getLastError().empty());
}

TEST_F(NetworkManagerComprehensiveTest, BroadcastMessage) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    ASSERT_TRUE(manager.connect("127.0.0.1", 7777));
    ASSERT_TRUE(manager.connect("127.0.0.2", 7777));
    auto message = createTestMessage();
    EXPECT_TRUE(manager.broadcastMessage("test", message));
}

// Subscription Tests
TEST_F(NetworkManagerComprehensiveTest, SubscribeToMessages) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    bool callbackCalled = false;
    auto callback = [&](const std::string& address,
                       const std::string& type,
                       const nlohmann::json& data) {
        callbackCalled = true;
    };

    EXPECT_TRUE(manager.subscribeToMessages("test", callback));
}

TEST_F(NetworkManagerComprehensiveTest, UnsubscribeFromMessages) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    auto callback = [](const std::string& address,
                      const std::string& type,
                      const nlohmann::json& data) {};

    ASSERT_TRUE(manager.subscribeToMessages("test", callback));
    EXPECT_TRUE(manager.unsubscribeFromMessages("test"));
}

// Network Type Tests
TEST_F(NetworkManagerComprehensiveTest, SetNetworkType) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_TRUE(manager.setNetworkType(NetworkManager::NetworkType::TESTNET));
    EXPECT_EQ(manager.getNetworkType(), NetworkManager::NetworkType::TESTNET);
}

// Node Type Tests
TEST_F(NetworkManagerComprehensiveTest, SetNodeType) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_TRUE(manager.setNodeType(NetworkManager::NodeType::LIGHT));
    EXPECT_EQ(manager.getNodeType(), NetworkManager::NodeType::LIGHT);
}

// Connection Limit Tests
TEST_F(NetworkManagerComprehensiveTest, SetMaxConnections) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_TRUE(manager.setMaxConnections(20));
    EXPECT_EQ(manager.getMaxConnections(), 20);
}

TEST_F(NetworkManagerComprehensiveTest, SetZeroMaxConnections) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_FALSE(manager.setMaxConnections(0));
    EXPECT_FALSE(manager.getLastError().empty());
}

// Connection Timeout Tests
TEST_F(NetworkManagerComprehensiveTest, SetConnectionTimeout) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_TRUE(manager.setConnectionTimeout(std::chrono::seconds(10)));
    EXPECT_EQ(manager.getConnectionTimeout(), std::chrono::seconds(10));
}

TEST_F(NetworkManagerComprehensiveTest, SetZeroConnectionTimeout) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_FALSE(manager.setConnectionTimeout(std::chrono::milliseconds(0)));
    EXPECT_FALSE(manager.getLastError().empty());
}

// Node Discovery Tests
TEST_F(NetworkManagerComprehensiveTest, DiscoverNodes) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    auto nodes = manager.discoverNodes();
    EXPECT_TRUE(nodes.empty());
}

TEST_F(NetworkManagerComprehensiveTest, AddNode) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_TRUE(manager.addNode("127.0.0.1", 7777));
}

TEST_F(NetworkManagerComprehensiveTest, AddInvalidNode) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_FALSE(manager.addNode("", 7777));
    EXPECT_FALSE(manager.getLastError().empty());
}

TEST_F(NetworkManagerComprehensiveTest, RemoveNode) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    ASSERT_TRUE(manager.addNode("127.0.0.1", 7777));
    EXPECT_TRUE(manager.removeNode("127.0.0.1"));
}

TEST_F(NetworkManagerComprehensiveTest, RemoveNonexistentNode) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_TRUE(manager.removeNode("127.0.0.1"));
}

TEST_F(NetworkManagerComprehensiveTest, IsNodeKnown) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_FALSE(manager.isNodeKnown("127.0.0.1"));
}

// Statistics Tests
TEST_F(NetworkManagerComprehensiveTest, GetStats) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    ASSERT_TRUE(manager.connect("127.0.0.1", 7777));
    auto stats = manager.getStats();
    EXPECT_EQ(stats.totalConnections, 1);
    EXPECT_EQ(stats.activeConnections, 1);
    EXPECT_EQ(stats.failedConnections, 0);
}

TEST_F(NetworkManagerComprehensiveTest, ResetStats) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    ASSERT_TRUE(manager.connect("127.0.0.1", 7777));
    manager.resetStats();
    auto stats = manager.getStats();
    EXPECT_EQ(stats.totalConnections, 0);
    EXPECT_EQ(stats.activeConnections, 0);
    EXPECT_EQ(stats.failedConnections, 0);
}

TEST_F(NetworkManagerComprehensiveTest, EnableStats) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_TRUE(manager.enableStats(false));
    auto stats = manager.getStats();
    EXPECT_EQ(stats.totalConnections, 0);
    EXPECT_EQ(stats.activeConnections, 0);
    EXPECT_EQ(stats.failedConnections, 0);
}

// Callback Tests
TEST_F(NetworkManagerComprehensiveTest, ConnectionCallback) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    bool callbackCalled = false;
    NetworkManager::ConnectionState callbackState;
    std::string callbackAddress;

    auto callback = [&](const std::string& address,
                       NetworkManager::ConnectionState state) {
        callbackCalled = true;
        callbackAddress = address;
        callbackState = state;
    };

    manager.registerConnectionCallback(callback);
    ASSERT_TRUE(manager.connect("127.0.0.1", 7777));

    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(callbackAddress, "127.0.0.1");
    EXPECT_EQ(callbackState, NetworkManager::ConnectionState::CONNECTED);
}

TEST_F(NetworkManagerComprehensiveTest, ErrorCallback) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    bool callbackCalled = false;
    std::string callbackAddress;
    std::string callbackError;

    auto callback = [&](const std::string& address,
                       const std::string& error) {
        callbackCalled = true;
        callbackAddress = address;
        callbackError = error;
    };

    manager.registerErrorCallback(callback);
    EXPECT_FALSE(manager.connect("", 7777));

    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(callbackAddress, "");
    EXPECT_FALSE(callbackError.empty());
}

// Error Handling Tests
TEST_F(NetworkManagerComprehensiveTest, GetLastError) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_FALSE(manager.connect("", 7777));
    EXPECT_FALSE(manager.getLastError().empty());
}

TEST_F(NetworkManagerComprehensiveTest, ClearLastError) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_FALSE(manager.connect("", 7777));
    EXPECT_FALSE(manager.getLastError().empty());

    manager.clearLastError();
    EXPECT_TRUE(manager.getLastError().empty());
}

// Concurrency Tests
TEST_F(NetworkManagerComprehensiveTest, ConcurrentConnections) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&, i]() {
            std::string address = "127.0.0." + std::to_string(i + 1);
            manager.connect(address, 7777);
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    for (int i = 0; i < 5; ++i) {
        std::string address = "127.0.0." + std::to_string(i + 1);
        EXPECT_TRUE(manager.isConnected(address));
    }
}

// Edge Cases Tests
TEST_F(NetworkManagerComprehensiveTest, EmptyAddress) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_FALSE(manager.connect("", 7777));
    EXPECT_FALSE(manager.getLastError().empty());
}

TEST_F(NetworkManagerComprehensiveTest, LongAddress) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::string longAddress(256, 'a');
    EXPECT_FALSE(manager.connect(longAddress, 7777));
    EXPECT_FALSE(manager.getLastError().empty());
}

TEST_F(NetworkManagerComprehensiveTest, SpecialCharactersInAddress) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_FALSE(manager.connect("127.0.0.1@test", 7777));
    EXPECT_FALSE(manager.getLastError().empty());
}

// Cleanup Tests
TEST_F(NetworkManagerComprehensiveTest, ShutdownWithConnections) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    ASSERT_TRUE(manager.connect("127.0.0.1", 7777));
    manager.shutdown();
    EXPECT_FALSE(manager.isConnected("127.0.0.1"));
}

TEST_F(NetworkManagerComprehensiveTest, ShutdownWithoutInitialization) {
    auto& manager = NetworkManager::getInstance();
    manager.shutdown();
    EXPECT_FALSE(manager.isConnected("127.0.0.1"));
}

// Stress Tests
TEST_F(NetworkManagerComprehensiveTest, ManyConnections) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    for (int i = 0; i < 10; ++i) {
        std::string address = "127.0.0." + std::to_string(i + 1);
        ASSERT_TRUE(manager.connect(address, 7777));
    }

    auto stats = manager.getStats();
    EXPECT_EQ(stats.totalConnections, 10);
    EXPECT_EQ(stats.activeConnections, 10);
}

TEST_F(NetworkManagerComprehensiveTest, RapidConnections) {
    auto& manager = NetworkManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    for (int i = 0; i < 10; ++i) {
        std::string address = "127.0.0." + std::to_string(i + 1);
        ASSERT_TRUE(manager.connect(address, 7777));
        ASSERT_TRUE(manager.disconnect(address));
    }

    auto stats = manager.getStats();
    EXPECT_EQ(stats.totalConnections, 10);
    EXPECT_EQ(stats.activeConnections, 0);
} 