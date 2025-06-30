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

#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <functional>
#include <chrono>
#include <nlohmann/json.hpp>
#include "satox/core/logging_manager.hpp"

namespace satox::core {

class NetworkManager {
public:
    // Network types
    enum class NetworkType {
        MAINNET,
        TESTNET,
        REGTEST
    };

    // Connection states
    enum class ConnectionState {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        DISCONNECTING,
        ERROR
    };

    // Node types
    enum class NodeType {
        FULL,
        LIGHT,
        ARCHIVE
    };

    // Connection info structure
    struct ConnectionInfo {
        std::string address;
        uint16_t port;
        NetworkType networkType;
        NodeType nodeType;
        std::string version;
        std::string userAgent;
        std::chrono::system_clock::time_point lastSeen;
        std::chrono::system_clock::time_point lastPing;
        std::chrono::milliseconds latency;
        bool isInbound;
        bool isOutbound;
        nlohmann::json metadata;
    };

    // Network stats structure
    struct NetworkStats {
        size_t totalConnections;
        size_t activeConnections;
        size_t failedConnections;
        size_t totalBytesReceived;
        size_t totalBytesSent;
        std::chrono::milliseconds averageLatency;
        std::chrono::system_clock::time_point lastSync;
        std::chrono::system_clock::time_point lastBlock;
        size_t blockHeight;
        size_t peerCount;
        nlohmann::json additionalStats;
    };

    // Callback types
    using ConnectionCallback = std::function<void(const std::string&, ConnectionState)>;
    using MessageCallback = std::function<void(const std::string&, const std::string&, const nlohmann::json&)>;
    using ErrorCallback = std::function<void(const std::string&, const std::string&)>;

    // Singleton instance
    static NetworkManager& getInstance();

    // Initialization and shutdown
    bool initialize(const nlohmann::json& config);
    void shutdown();

    // Connection management
    bool connect(const std::string& address, uint16_t port);
    bool disconnect(const std::string& address);
    bool isConnected(const std::string& address) const;
    ConnectionState getConnectionState(const std::string& address) const;
    std::vector<ConnectionInfo> getConnections() const;
    ConnectionInfo getConnectionInfo(const std::string& address) const;

    // Message handling
    bool sendMessage(const std::string& address, const std::string& type, const nlohmann::json& data);
    bool broadcastMessage(const std::string& type, const nlohmann::json& data);
    bool subscribeToMessages(const std::string& type, MessageCallback callback);
    bool unsubscribeFromMessages(const std::string& type);

    // Network configuration
    bool setNetworkType(NetworkType type);
    NetworkType getNetworkType() const;
    bool setNodeType(NodeType type);
    NodeType getNodeType() const;
    bool setMaxConnections(size_t max);
    size_t getMaxConnections() const;
    bool setConnectionTimeout(std::chrono::milliseconds timeout);
    std::chrono::milliseconds getConnectionTimeout() const;

    // Network discovery
    std::vector<std::string> discoverNodes() const;
    bool addNode(const std::string& address, uint16_t port);
    bool removeNode(const std::string& address);
    bool isNodeKnown(const std::string& address) const;

    // Network statistics
    NetworkStats getStats() const;
    void resetStats();
    bool enableStats(bool enable);

    // Callback registration
    void registerConnectionCallback(ConnectionCallback callback);
    void registerErrorCallback(ErrorCallback callback);
    void unregisterConnectionCallback();
    void unregisterErrorCallback();

    // Error handling
    std::string getLastError() const;
    void clearLastError();

private:
    NetworkManager() = default;
    ~NetworkManager() = default;
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;

    // Helper methods
    bool validateConfig(const nlohmann::json& config) const;
    bool validateAddress(const std::string& address) const;
    bool validatePort(uint16_t port) const;
    bool validateMessage(const std::string& type, const nlohmann::json& data) const;
    bool checkConnectionLimit() const;
    bool checkNodeLimit() const;
    void updateConnectionState(const std::string& address, ConnectionState state);
    void updateConnectionInfo(const std::string& address, const ConnectionInfo& info);
    void updateNetworkStats(const std::string& address, size_t bytesReceived, size_t bytesSent);
    void notifyConnectionChange(const std::string& address, ConnectionState state);
    void notifyMessageReceived(const std::string& address, const std::string& type, const nlohmann::json& data);
    void notifyError(const std::string& address, const std::string& error);

    // Member variables
    mutable std::mutex mutex_;
    bool initialized_ = false;
    NetworkType networkType_ = NetworkType::MAINNET;
    NodeType nodeType_ = NodeType::FULL;
    size_t maxConnections_ = 100;
    std::chrono::milliseconds connectionTimeout_ = std::chrono::seconds(30);
    std::map<std::string, ConnectionInfo> connections_;
    std::map<std::string, std::vector<MessageCallback>> messageCallbacks_;
    std::vector<ConnectionCallback> connectionCallbacks_;
    std::vector<ErrorCallback> errorCallbacks_;
    NetworkStats stats_;
    bool statsEnabled_ = false;
    mutable std::string lastError_;
    std::unique_ptr<LoggingManager> logger_;
};

} // namespace satox::core 