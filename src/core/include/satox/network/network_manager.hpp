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
#include <mutex>
#include <queue>
#include <thread>
#include <atomic>
#include <unordered_map>

namespace satox {
namespace network {

class NetworkManager {
public:
    using MessageCallback = std::function<void(const std::string&, const std::vector<uint8_t>&)>;
    using PeerCallback = std::function<void(const std::string&, bool)>;

    NetworkManager(const std::string& type, int port);
    ~NetworkManager();

    bool initialize();
    void shutdown();
    std::string getLastError() const;

    // Network operations
    bool startListening();
    bool stopListening();
    bool connect(const std::string& address, int port);
    bool disconnect(const std::string& address);
    bool broadcast(const std::vector<uint8_t>& message);
    bool send(const std::string& address, const std::vector<uint8_t>& message);

    // Callback registration
    void registerMessageCallback(MessageCallback callback);
    void registerPeerCallback(PeerCallback callback);

    // Peer management
    std::vector<std::string> getConnectedPeers() const;
    bool isPeerConnected(const std::string& address) const;
    int getPeerCount() const;

private:
    struct Peer {
        std::string address;
        int port;
        std::atomic<bool> connected{false};
        std::thread receiveThread;
        std::queue<std::vector<uint8_t>> messageQueue;
        std::mutex queueMutex;
    };

    bool initializeNetwork();
    void cleanupNetwork();
    void handleIncomingConnection();
    void handlePeerMessages(std::shared_ptr<Peer> peer);
    void processMessageQueue(std::shared_ptr<Peer> peer);

    std::string type_;
    int port_;
    std::atomic<bool> running_{false};
    std::string lastError_;
    mutable std::mutex mutex_;

    std::unordered_map<std::string, std::shared_ptr<Peer>> peers_;
    std::vector<MessageCallback> messageCallbacks_;
    std::vector<PeerCallback> peerCallbacks_;
    std::thread listenerThread_;
};

} // namespace network
} // namespace satox 