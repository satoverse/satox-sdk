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
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
#include <chrono>
#include <unordered_map>
#include <functional>
#include <memory>
#include "network_interface.h"

namespace satox {
namespace core {

class MessageHandler {
public:
    MessageHandler();
    ~MessageHandler();

    // Message queue operations
    void enqueueMessage(const NetworkMessage& message);
    bool processNextMessage();
    void startProcessing();
    void stopProcessing();

    // Response handling
    std::string generateRequestId();
    std::future<NetworkMessage> waitForResponse(const std::string& requestId, 
                                              std::chrono::milliseconds timeout);
    void handleResponse(const NetworkMessage& response);

    // Callbacks
    using MessageCallback = std::function<void(const NetworkMessage&)>;
    void setMessageCallback(MessageCallback callback);
    void setErrorCallback(std::function<void(const std::string&)> callback);

private:
    // Message queue
    std::queue<NetworkMessage> messageQueue_;
    std::mutex queueMutex_;
    std::condition_variable queueCondition_;
    bool running_;

    // Response tracking
    struct PendingResponse {
        std::promise<NetworkMessage> promise;
        std::chrono::system_clock::time_point expiry;
    };
    std::unordered_map<std::string, PendingResponse> pendingResponses_;
    std::mutex responseMutex_;

    // Callbacks
    MessageCallback messageCallback_;
    std::function<void(const std::string&)> errorCallback_;

    // Helper methods
    void cleanupExpiredResponses();
    void processMessageQueue();
};

} // namespace core
} // namespace satox 