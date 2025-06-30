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

#include "satox/core/network_manager.hpp"
#include <algorithm>
#include <chrono>
#include <thread>
#include <filesystem>
#include "satox/core/logging_manager.hpp"

namespace satox::core {

NetworkManager& NetworkManager::getInstance() {
    static NetworkManager instance;
    return instance;
}

bool NetworkManager::initialize(const nlohmann::json& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        lastError_ = "Network manager already initialized";
        return false;
    }

    if (!validateConfig(config)) {
        return false;
    }

    // Initialize network settings from config
    if (config.contains("networkType")) {
        networkType_ = static_cast<NetworkType>(config["networkType"].get<int>());
    }

    if (config.contains("nodeType")) {
        nodeType_ = static_cast<NodeType>(config["nodeType"].get<int>());
    }

    if (config.contains("maxConnections")) {
        maxConnections_ = config["maxConnections"].get<size_t>();
    }

    if (config.contains("connectionTimeout")) {
        connectionTimeout_ = std::chrono::milliseconds(
            config["connectionTimeout"].get<int>());
    }

    if (config.contains("enableStats")) {
        statsEnabled_ = config["enableStats"].get<bool>();
    }

    satox::core::LoggingManager::LogConfig logConfig;
    logConfig.logDir = "logs/components/network";
    logConfig.logFile = "network.log";
    logConfig.maxFileSize = 10 * 1024 * 1024;
    logConfig.maxFiles = 5;
    logConfig.minLevel = satox::core::LoggingManager::Level::DEBUG;
    logConfig.consoleOutput = true;
    logConfig.fileOutput = true;
    logConfig.asyncLogging = true;
    logConfig.queueSize = 1000;
    logConfig.flushInterval = std::chrono::milliseconds(100);
    logConfig.includeTimestamp = true;
    logConfig.includeThreadId = true;
    logConfig.includeSource = true;
    logConfig.logFormat = "[{timestamp}] [{level}] [{thread}] [{source}] {message} {metadata}";
    std::filesystem::create_directories(logConfig.logDir);
    logger_ = std::make_unique<LoggingManager>();
    if (!logger_->initialize(logConfig)) {
        lastError_ = "Failed to initialize NetworkManager logger";
        return false;
    }

    initialized_ = true;
    return true;
}

void NetworkManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return;
    }

    // Disconnect all connections
    for (const auto& [address, info] : connections_) {
        disconnect(address);
    }

    connections_.clear();
    messageCallbacks_.clear();
    connectionCallbacks_.clear();
    errorCallbacks_.clear();
    initialized_ = false;
}

bool NetworkManager::connect(const std::string& address, uint16_t port) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Network manager not initialized";
        return false;
    }

    if (!validateAddress(address)) {
        return false;
    }

    if (!validatePort(port)) {
        return false;
    }

    if (isConnected(address)) {
        lastError_ = "Already connected to " + address;
        return false;
    }

    if (!checkConnectionLimit()) {
        return false;
    }

    updateConnectionState(address, ConnectionState::CONNECTING);

    // In a real implementation, this would establish the actual network connection
    // For now, we'll simulate a connection
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ConnectionInfo info;
    info.address = address;
    info.port = port;
    info.networkType = networkType_;
    info.nodeType = nodeType_;
    info.version = "1.0.0";
    info.userAgent = "SatoxSDK/1.0.0";
    info.lastSeen = std::chrono::system_clock::now();
    info.lastPing = std::chrono::system_clock::now();
    info.latency = std::chrono::milliseconds(0);
    info.isInbound = false;
    info.isOutbound = true;
    info.metadata = nlohmann::json::object();

    connections_[address] = info;
    updateConnectionState(address, ConnectionState::CONNECTED);

    if (statsEnabled_) {
        stats_.totalConnections++;
        stats_.activeConnections++;
    }

    return true;
}

bool NetworkManager::disconnect(const std::string& address) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Network manager not initialized";
        return false;
    }

    auto it = connections_.find(address);
    if (it == connections_.end()) {
        lastError_ = "Not connected to " + address;
        return false;
    }

    updateConnectionState(address, ConnectionState::DISCONNECTING);

    // In a real implementation, this would close the actual network connection
    // For now, we'll simulate a disconnection
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (statsEnabled_) {
        stats_.activeConnections--;
    }

    connections_.erase(it);
    updateConnectionState(address, ConnectionState::DISCONNECTED);

    return true;
}

bool NetworkManager::isConnected(const std::string& address) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = connections_.find(address);
    return it != connections_.end() &&
           it->second.lastSeen + connectionTimeout_ > std::chrono::system_clock::now();
}

NetworkManager::ConnectionState NetworkManager::getConnectionState(
    const std::string& address) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = connections_.find(address);
    if (it == connections_.end()) {
        return ConnectionState::DISCONNECTED;
    }
    return it->second.lastSeen + connectionTimeout_ > std::chrono::system_clock::now()
               ? ConnectionState::CONNECTED
               : ConnectionState::DISCONNECTED;
}

std::vector<NetworkManager::ConnectionInfo> NetworkManager::getConnections() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ConnectionInfo> result;
    result.reserve(connections_.size());
    for (const auto& [address, info] : connections_) {
        result.push_back(info);
    }
    return result;
}

NetworkManager::ConnectionInfo NetworkManager::getConnectionInfo(
    const std::string& address) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = connections_.find(address);
    if (it == connections_.end()) {
        return ConnectionInfo{};
    }
    return it->second;
}

bool NetworkManager::sendMessage(const std::string& address,
                               const std::string& type,
                               const nlohmann::json& data) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Network manager not initialized";
        return false;
    }

    if (!isConnected(address)) {
        lastError_ = "Not connected to " + address;
        return false;
    }

    if (!validateMessage(type, data)) {
        return false;
    }

    // In a real implementation, this would send the actual network message
    // For now, we'll simulate sending a message
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    if (statsEnabled_) {
        stats_.totalBytesSent += data.dump().size();
    }

    return true;
}

bool NetworkManager::broadcastMessage(const std::string& type,
                                    const nlohmann::json& data) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Network manager not initialized";
        return false;
    }

    if (!validateMessage(type, data)) {
        return false;
    }

    bool success = true;
    for (const auto& [address, info] : connections_) {
        if (!sendMessage(address, type, data)) {
            success = false;
        }
    }

    return success;
}

bool NetworkManager::subscribeToMessages(const std::string& type,
                                       MessageCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Network manager not initialized";
        return false;
    }

    messageCallbacks_[type].push_back(callback);
    return true;
}

bool NetworkManager::unsubscribeFromMessages(const std::string& type) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Network manager not initialized";
        return false;
    }

    return messageCallbacks_.erase(type) > 0;
}

bool NetworkManager::setNetworkType(NetworkType type) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Network manager not initialized";
        return false;
    }

    networkType_ = type;
    return true;
}

NetworkManager::NetworkType NetworkManager::getNetworkType() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return networkType_;
}

bool NetworkManager::setNodeType(NodeType type) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Network manager not initialized";
        return false;
    }

    nodeType_ = type;
    return true;
}

NetworkManager::NodeType NetworkManager::getNodeType() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return nodeType_;
}

bool NetworkManager::setMaxConnections(size_t max) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Network manager not initialized";
        return false;
    }

    if (max == 0) {
        lastError_ = "Maximum connections cannot be zero";
        return false;
    }

    maxConnections_ = max;
    return true;
}

size_t NetworkManager::getMaxConnections() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return maxConnections_;
}

bool NetworkManager::setConnectionTimeout(std::chrono::milliseconds timeout) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Network manager not initialized";
        return false;
    }

    if (timeout.count() <= 0) {
        lastError_ = "Connection timeout must be positive";
        return false;
    }

    connectionTimeout_ = timeout;
    return true;
}

std::chrono::milliseconds NetworkManager::getConnectionTimeout() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return connectionTimeout_;
}

std::vector<std::string> NetworkManager::discoverNodes() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Network manager not initialized";
        return {};
    }

    // In a real implementation, this would discover actual network nodes
    // For now, we'll return an empty list
    return {};
}

bool NetworkManager::addNode(const std::string& address, uint16_t port) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Network manager not initialized";
        return false;
    }

    if (!validateAddress(address)) {
        return false;
    }

    if (!validatePort(port)) {
        return false;
    }

    if (!checkNodeLimit()) {
        return false;
    }

    // In a real implementation, this would add the node to the known nodes list
    // For now, we'll just return true
    return true;
}

bool NetworkManager::removeNode(const std::string& address) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Network manager not initialized";
        return false;
    }

    if (!validateAddress(address)) {
        return false;
    }

    // In a real implementation, this would remove the node from the known nodes list
    // For now, we'll just return true
    return true;
}

bool NetworkManager::isNodeKnown(const std::string& address) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Network manager not initialized";
        return false;
    }

    if (!validateAddress(address)) {
        return false;
    }

    // In a real implementation, this would check if the node is in the known nodes list
    // For now, we'll just return false
    return false;
}

NetworkManager::NetworkStats NetworkManager::getStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

void NetworkManager::resetStats() {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_ = NetworkStats{};
}

bool NetworkManager::enableStats(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Network manager not initialized";
        return false;
    }

    statsEnabled_ = enable;
    return true;
}

void NetworkManager::registerConnectionCallback(ConnectionCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    connectionCallbacks_.push_back(callback);
}

void NetworkManager::registerErrorCallback(ErrorCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    errorCallbacks_.push_back(callback);
}

void NetworkManager::unregisterConnectionCallback() {
    std::lock_guard<std::mutex> lock(mutex_);
    connectionCallbacks_.clear();
}

void NetworkManager::unregisterErrorCallback() {
    std::lock_guard<std::mutex> lock(mutex_);
    errorCallbacks_.clear();
}

std::string NetworkManager::getLastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastError_;
}

void NetworkManager::clearLastError() {
    std::lock_guard<std::mutex> lock(mutex_);
    lastError_.clear();
}

bool NetworkManager::validateConfig(const nlohmann::json& config) const {
    if (config.contains("networkType") &&
        !config["networkType"].is_number_integer()) {
        lastError_ = "Invalid network type";
        return false;
    }

    if (config.contains("nodeType") &&
        !config["nodeType"].is_number_integer()) {
        lastError_ = "Invalid node type";
        return false;
    }

    if (config.contains("maxConnections") &&
        !config["maxConnections"].is_number_unsigned()) {
        lastError_ = "Invalid maximum connections";
        return false;
    }

    if (config.contains("connectionTimeout") &&
        !config["connectionTimeout"].is_number_integer()) {
        lastError_ = "Invalid connection timeout";
        return false;
    }

    if (config.contains("enableStats") &&
        !config["enableStats"].is_boolean()) {
        lastError_ = "Invalid enable stats";
        return false;
    }

    return true;
}

bool NetworkManager::validateAddress(const std::string& address) const {
    if (address.empty()) {
        lastError_ = "Address cannot be empty";
        return false;
    }

    // This is a simplified implementation. In a real system, you would
    // want to validate the address format (e.g., IP address, hostname).
    return true;
}

bool NetworkManager::validatePort(uint16_t port) const {
    if (port == 0) {
        lastError_ = "Port cannot be zero";
        return false;
    }

    return true;
}

bool NetworkManager::validateMessage(const std::string& type,
                                   const nlohmann::json& data) const {
    if (type.empty()) {
        lastError_ = "Message type cannot be empty";
        return false;
    }

    if (!data.is_object()) {
        lastError_ = "Message data must be an object";
        return false;
    }

    return true;
}

bool NetworkManager::checkConnectionLimit() const {
    if (connections_.size() >= maxConnections_) {
        lastError_ = "Maximum connections reached";
        return false;
    }

    return true;
}

bool NetworkManager::checkNodeLimit() const {
    // This is a simplified implementation. In a real system, you would
    // want to check against a maximum number of known nodes.
    return true;
}

void NetworkManager::updateConnectionState(const std::string& address,
                                        ConnectionState state) {
    auto it = connections_.find(address);
    if (it != connections_.end()) {
        notifyConnectionChange(address, state);
    }
}

void NetworkManager::updateConnectionInfo(const std::string& address,
                                        const ConnectionInfo& info) {
    auto it = connections_.find(address);
    if (it != connections_.end()) {
        it->second = info;
    }
}

void NetworkManager::updateNetworkStats(const std::string& address,
                                      size_t bytesReceived,
                                      size_t bytesSent) {
    if (!statsEnabled_) {
        return;
    }

    stats_.totalBytesReceived += bytesReceived;
    stats_.totalBytesSent += bytesSent;

    auto it = connections_.find(address);
    if (it != connections_.end()) {
        stats_.averageLatency = std::chrono::milliseconds(
            (stats_.averageLatency.count() * (stats_.activeConnections - 1) +
             it->second.latency.count()) /
            stats_.activeConnections);
    }
}

void NetworkManager::notifyConnectionChange(const std::string& address,
                                         ConnectionState state) {
    for (const auto& callback : connectionCallbacks_) {
        callback(address, state);
    }
}

void NetworkManager::notifyMessageReceived(const std::string& address,
                                         const std::string& type,
                                         const nlohmann::json& data) {
    auto it = messageCallbacks_.find(type);
    if (it != messageCallbacks_.end()) {
        for (const auto& callback : it->second) {
            callback(address, type, data);
        }
    }
}

void NetworkManager::notifyError(const std::string& address,
                               const std::string& error) {
    for (const auto& callback : errorCallbacks_) {
        callback(address, error);
    }
}

} // namespace satox::core 