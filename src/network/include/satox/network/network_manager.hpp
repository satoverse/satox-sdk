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
#include <memory>
#include <vector>
#include <functional>
#include <chrono>
#include <mutex>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "letsencrypt_manager.hpp"

namespace satox::network {

class NetworkManager {
public:
    // Singleton instance
    static NetworkManager& getInstance();

    // Configuration structures
    struct SecurityConfig {
        bool enable_encryption;
        std::string ssl_cert_path;
        std::string ssl_key_path;
        std::string ssl_ca_path;
        bool verify_hostname;
        uint32_t min_protocol_version;
        std::vector<std::string> allowed_ciphers;
    };

    struct NetworkConfig {
        std::string host;
        uint16_t p2p_port;
        uint16_t rpc_port;
        uint32_t max_connections;
        bool enable_ssl;
        std::string ssl_cert_path;
        std::string ssl_key_path;
        LetsEncryptManager::Config letsencrypt_config;
        SecurityConfig security;
    };

    struct NetworkStats {
        uint32_t active_connections;
        uint32_t total_connections;
        uint32_t bytes_sent;
        uint32_t bytes_received;
        uint32_t errors;
        uint64_t uptime_seconds;
    };

    struct Peer {
        std::string address;
        uint16_t port;
        int64_t last_seen;
        bool inbound;
        bool whitelisted;
        uint32_t bytes_sent;
        uint32_t bytes_received;
    };

    struct RPCRequest {
        std::string method;
        nlohmann::json params;
        std::string id;
    };

    struct RPCResponse {
        nlohmann::json result;
        std::string error;
        std::string id;
    };

    enum class MessageType {
        P2P_MESSAGE,
        RPC_REQUEST,
        RPC_RESPONSE,
        HEARTBEAT,
        PEER_DISCOVERY
    };

    // Callback types
    using RPCCallback = std::function<void(const RPCResponse&)>;
    using MessageCallback = std::function<void(MessageType, const std::string&, const std::vector<uint8_t>&)>;
    using PeerCallback = std::function<void(const std::string&, bool)>;
    using ErrorCallback = std::function<void(const std::string&)>;
    using CertificateUpdateCallback = LetsEncryptManager::CertificateUpdateCallback;

    // Initialization and shutdown
    bool initialize(const NetworkConfig& config = NetworkConfig{});
    bool shutdown();
    bool isInitialized() const;
    bool isHealthy() const;

    // Core functionality
    bool isRunning() const;
    bool isP2PRunning() const;
    bool isRPCRunning() const;

    // P2P network operations
    bool startP2P();
    void stopP2P();
    bool connectToPeer(const std::string& address, uint16_t port);
    void disconnectPeer(const std::string& address);
    bool isPeerConnected(const std::string& address) const;
    std::vector<Peer> getConnectedPeers() const;
    int getPeerCount() const;

    // RPC operations
    bool startRPC();
    void stopRPC();
    bool registerRPCMethod(const std::string& method, std::function<RPCResponse(const RPCRequest&)> handler);
    bool callRPC(const std::string& address, const RPCRequest& request, RPCCallback callback);

    // Message handling
    bool sendP2PMessage(const std::string& peer_address, const std::vector<uint8_t>& message);
    bool sendRPCMessage(const std::string& client_address, const nlohmann::json& message);
    void registerMessageCallback(MessageType type, MessageCallback callback);
    void unregisterMessageCallback(MessageType type);

    // Callback registration
    void registerPeerCallback(PeerCallback callback);
    void unregisterPeerCallback();
    void registerErrorCallback(ErrorCallback callback);
    void unregisterErrorCallback();

    // Security and SSL
    bool enableEncryption(const SecurityConfig& config);
    bool enableSSL(const SecurityConfig& config);
    bool disableSSL();
    LetsEncryptManager::Certificate getCurrentCertificate() const;
    bool renewCertificate();
    void registerCertificateUpdateCallback(CertificateUpdateCallback callback);

    // Statistics and monitoring
    NetworkStats getNetworkStats() const;
    bool enableStats(bool enable);
    bool resetStats();

    // Error handling
    std::string getLastError() const;
    bool clearLastError();

    // Health check
    bool healthCheck() const;

private:
    NetworkManager();
    ~NetworkManager() = default;
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;

    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace satox::network 