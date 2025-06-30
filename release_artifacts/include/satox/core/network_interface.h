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
#include <functional>
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <future>
#include <chrono>
#include <random>

namespace satox {
namespace core {

struct NetworkMessage {
    enum class Type {
        BLOCK,
        TRANSACTION,
        SYNC_REQUEST,
        SYNC_RESPONSE,
        PING,
        PONG,
        ERROR
    };

    Type type;
    std::string payload;
    std::string sender;
    uint64_t timestamp;
    std::string requestId;  // For correlating requests and responses
};

struct NetworkResponse {
    bool success;
    std::string data;
    std::string error;
    uint64_t timestamp;
};

class NetworkInterface {
public:
    NetworkInterface();
    ~NetworkInterface();

    // Prevent copying
    NetworkInterface(const NetworkInterface&) = delete;
    NetworkInterface& operator=(const NetworkInterface&) = delete;

    // Allow moving
    NetworkInterface(NetworkInterface&&) noexcept = default;
    NetworkInterface& operator=(NetworkInterface&&) noexcept = default;

    // Connection management
    bool connect(const std::string& address, uint16_t port);
    void disconnect();
    bool isConnected() const;

    // Message handling
    void sendMessage(const NetworkMessage& message);
    void setMessageCallback(std::function<void(const NetworkMessage&)> callback);

    // Response handling
    NetworkResponse waitForResponse(const std::string& requestId, 
                                  std::chrono::milliseconds timeout = std::chrono::seconds(30));
    void handleResponse(const NetworkMessage& response);

    // Network operations
    std::vector<std::string> getPeers() const;
    void addPeer(const std::string& address, uint16_t port);
    void removePeer(const std::string& address);
    void broadcastMessage(const NetworkMessage& message);

    // Blockchain-specific operations
    std::string getLatestBlock();
    std::string getBlockByHash(const std::string& hash);
    std::string getTransactionByHash(const std::string& hash);
    std::string getBalance(const std::string& address);
    std::string sendTransaction(const std::string& transaction);

    // For compatibility with old code
    std::vector<NetworkMessage> getMessages() { return {}; }

    // Error handling
    void setErrorCallback(std::function<void(const std::string&)> callback);

private:
    void messageLoop();
    void handleIncomingMessage(const NetworkMessage& message);
    void processMessageQueue();
    void reconnect();
    std::string generateRequestId();

    struct Peer {
        std::string address;
        uint16_t port;
        bool connected;
        std::chrono::system_clock::time_point lastSeen;
    };

    struct PendingResponse {
        std::promise<NetworkResponse> promise;
        std::chrono::system_clock::time_point expiry;
    };

    std::string address_;
    uint16_t port_;
    std::atomic<bool> connected_;
    std::atomic<bool> running_;
    std::thread messageThread_;
    std::mutex peersMutex_;
    std::mutex queueMutex_;
    std::mutex responseMutex_;
    std::condition_variable queueCondition_;
    std::queue<NetworkMessage> messageQueue_;
    std::vector<Peer> peers_;
    std::function<void(const NetworkMessage&)> messageCallback_;
    std::unordered_map<std::string, PendingResponse> pendingResponses_;
    std::random_device rd_;
    std::mt19937 gen_;
    std::function<void(const std::string&)> errorCallback_;
};

} // namespace core
} // namespace satox 