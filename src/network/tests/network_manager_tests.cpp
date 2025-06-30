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

#include "satox/network/network_manager.hpp"
#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <future>

using namespace satox;

class NetworkManagerTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize network manager with test configuration
        NetworkManager::NetworkConfig config;
        config.host = "127.0.0.1";
        config.p2p_port = 60777;
        config.rpc_port = 7777;
        config.user_agent = "SatoxcoinSDK/1.0.0";
        config.max_connections = 100;
        config.max_outbound = 50;
        config.max_inbound = 50;
        config.allow_private_networks = true;

        ASSERT_TRUE(NetworkManager::getInstance().initialize(config));
    }

    void TearDown() override {
        NetworkManager::getInstance().shutdown();
    }
};

// Test initialization and shutdown
TEST_F(NetworkManagerTests, Initialization) {
    EXPECT_TRUE(NetworkManager::getInstance().isInitialized());
    EXPECT_EQ(NetworkManager::getInstance().getLastError(), "");
}

// Test P2P network operations
TEST_F(NetworkManagerTests, P2POperations) {
    // Start P2P network
    EXPECT_TRUE(NetworkManager::getInstance().startP2P());
    EXPECT_TRUE(NetworkManager::getInstance().isP2PRunning());

    // Connect to a peer
    EXPECT_TRUE(NetworkManager::getInstance().connectToPeer("127.0.0.1", 60778));

    // Get connected peers
    auto peers = NetworkManager::getInstance().getConnectedPeers();
    EXPECT_FALSE(peers.empty());

    // Send message to peer
    NetworkManager::Message message;
    message.type = NetworkManager::MessageType::VERSION;
    message.payload = {0x01, 0x02, 0x03};
    message.checksum = 0x12345678;
    EXPECT_TRUE(NetworkManager::getInstance().sendMessageToPeer("127.0.0.1", message));

    // Broadcast message
    EXPECT_TRUE(NetworkManager::getInstance().broadcastMessage(message));

    // Disconnect peer
    NetworkManager::getInstance().disconnectPeer("127.0.0.1");
    peers = NetworkManager::getInstance().getConnectedPeers();
    EXPECT_TRUE(peers.empty());

    // Stop P2P network
    NetworkManager::getInstance().stopP2P();
    EXPECT_FALSE(NetworkManager::getInstance().isP2PRunning());
}

// Test RPC operations
TEST_F(NetworkManagerTests, RPCOperations) {
    // Start RPC server
    EXPECT_TRUE(NetworkManager::getInstance().startRPC());
    EXPECT_TRUE(NetworkManager::getInstance().isRPCRunning());

    // Register RPC method
    bool method_called = false;
    EXPECT_TRUE(NetworkManager::getInstance().registerRPCMethod(
        "test_method",
        [&method_called](const NetworkManager::RPCRequest& req) -> NetworkManager::RPCResponse {
            method_called = true;
            NetworkManager::RPCResponse response;
            response.result = "{\"status\":\"success\"}";
            response.id = req.id;
            return response;
        }
    ));

    // Call RPC method
    NetworkManager::RPCRequest request;
    request.method = "test_method";
    request.params = {"param1", "param2"};
    request.id = "1";

    std::promise<void> promise;
    auto future = promise.get_future();

    EXPECT_TRUE(NetworkManager::getInstance().callRPC(
        "127.0.0.1",
        request,
        [&promise](const NetworkManager::RPCResponse& response) {
            EXPECT_EQ(response.id, "1");
            EXPECT_FALSE(response.result.empty());
            EXPECT_TRUE(response.error.empty());
            promise.set_value();
        }
    ));

    // Wait for response
    future.wait_for(std::chrono::seconds(5));
    EXPECT_TRUE(method_called);

    // Stop RPC server
    NetworkManager::getInstance().stopRPC();
    EXPECT_FALSE(NetworkManager::getInstance().isRPCRunning());
}

// Test message callbacks
TEST_F(NetworkManagerTests, MessageCallbacks) {
    // Start P2P network
    EXPECT_TRUE(NetworkManager::getInstance().startP2P());

    // Register message callback
    bool callback_called = false;
    NetworkManager::getInstance().registerMessageCallback(
        NetworkManager::MessageType::VERSION,
        [&callback_called](const NetworkManager::Message& message) {
            callback_called = true;
            EXPECT_EQ(message.type, NetworkManager::MessageType::VERSION);
            EXPECT_FALSE(message.payload.empty());
        }
    );

    // Send message
    NetworkManager::Message message;
    message.type = NetworkManager::MessageType::VERSION;
    message.payload = {0x01, 0x02, 0x03};
    message.checksum = 0x12345678;
    EXPECT_TRUE(NetworkManager::getInstance().broadcastMessage(message));

    // Wait for callback
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(callback_called);

    // Unregister callback
    NetworkManager::getInstance().unregisterMessageCallback(NetworkManager::MessageType::VERSION);

    // Stop P2P network
    NetworkManager::getInstance().stopP2P();
}

// Test peer callbacks
TEST_F(NetworkManagerTests, PeerCallbacks) {
    // Start P2P network
    EXPECT_TRUE(NetworkManager::getInstance().startP2P());

    // Register peer callback
    bool callback_called = false;
    NetworkManager::getInstance().registerPeerCallback(
        [&callback_called](const NetworkManager::Peer& peer) {
            callback_called = true;
            EXPECT_EQ(peer.address, "127.0.0.1");
            EXPECT_EQ(peer.port, 60778);
        }
    );

    // Connect to peer
    EXPECT_TRUE(NetworkManager::getInstance().connectToPeer("127.0.0.1", 60778));

    // Wait for callback
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(callback_called);

    // Unregister callback
    NetworkManager::getInstance().unregisterPeerCallback();

    // Stop P2P network
    NetworkManager::getInstance().stopP2P();
}

// Test error handling
TEST_F(NetworkManagerTests, ErrorHandling) {
    // Try to start P2P without initialization
    NetworkManager::getInstance().shutdown();
    EXPECT_FALSE(NetworkManager::getInstance().startP2P());
    EXPECT_FALSE(NetworkManager::getInstance().getLastError().empty());

    // Reinitialize
    NetworkManager::NetworkConfig config;
    config.host = "127.0.0.1";
    config.p2p_port = 60777;
    config.rpc_port = 7777;
    config.user_agent = "SatoxcoinSDK/1.0.0";
    config.max_connections = 100;
    config.max_outbound = 50;
    config.max_inbound = 50;
    config.allow_private_networks = true;

    EXPECT_TRUE(NetworkManager::getInstance().initialize(config));

    // Try to connect to invalid peer
    EXPECT_FALSE(NetworkManager::getInstance().connectToPeer("invalid_address", 60778));
    EXPECT_FALSE(NetworkManager::getInstance().getLastError().empty());
}

// Test concurrency
TEST_F(NetworkManagerTests, Concurrency) {
    // Start P2P network
    EXPECT_TRUE(NetworkManager::getInstance().startP2P());

    // Create multiple threads
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([i]() {
            // Connect to peer
            NetworkManager::getInstance().connectToPeer("127.0.0.1", 60778 + i);

            // Send message
            NetworkManager::Message message;
            message.type = NetworkManager::MessageType::VERSION;
            message.payload = {0x01, 0x02, 0x03};
            message.checksum = 0x12345678;
            NetworkManager::getInstance().sendMessageToPeer("127.0.0.1", message);
        });
    }

    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }

    // Stop P2P network
    NetworkManager::getInstance().stopP2P();
}

// Test configuration
TEST_F(NetworkManagerTests, Configuration) {
    NetworkManager::NetworkConfig config = NetworkManager::getInstance().getConfig();
    EXPECT_EQ(config.host, "127.0.0.1");
    EXPECT_EQ(config.p2p_port, 60777);
    EXPECT_EQ(config.rpc_port, 7777);
    EXPECT_EQ(config.user_agent, "SatoxcoinSDK/1.0.0");
    EXPECT_EQ(config.max_connections, 100);
    EXPECT_EQ(config.max_outbound, 50);
    EXPECT_EQ(config.max_inbound, 50);
    EXPECT_TRUE(config.allow_private_networks);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 