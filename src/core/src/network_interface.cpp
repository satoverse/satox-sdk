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

#include "satox/core/network_interface.h"
#include <stdexcept>
#include <chrono>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <nlohmann/json.hpp>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <spdlog/spdlog.h>

namespace satox {
namespace core {

NetworkInterface::NetworkInterface()
    : port_(0)
    , connected_(false)
    , running_(false)
    , gen_(rd_()) {
}

NetworkInterface::~NetworkInterface() {
    disconnect();
}

std::string NetworkInterface::generateRequestId() {
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << gen_();
    return ss.str();
    }

void NetworkInterface::enqueueMessage(const NetworkMessage& message) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    messageQueue_.push(message);
    queueCondition_.notify_one();
}

bool NetworkInterface::processNextMessage() {
    std::unique_lock<std::mutex> lock(queueMutex_);
    if (messageQueue_.empty()) {
        return false;
    }

    NetworkMessage message = messageQueue_.front();
    messageQueue_.pop();
    lock.unlock();

    handleIncomingMessage(message);
    return true;
}

void NetworkInterface::handleIncomingMessage(const NetworkMessage& message) {
    if (!running_) {
        spdlog::warn("Received message while NetworkInterface is not running");
        return;
    }

    try {
        spdlog::debug("Handling incoming message of type: {}", 
            static_cast<int>(message.type));
            
        if (message.type == NetworkMessage::Type::SYNC_RESPONSE) {
            handleResponse(message);
        } else if (messageCallback_) {
            messageCallback_(message);
        } else {
            spdlog::warn("Received message but no callback registered");
        }
    } catch (const std::exception& e) {
        spdlog::error("Error handling message: {}", e.what());
        if (errorCallback_) {
            errorCallback_(e.what());
        }
    }
}

void NetworkInterface::handleResponse(const NetworkMessage& response) {
    if (!running_) {
        spdlog::warn("Received response while NetworkInterface is not running");
        return;
    }

    if (response.requestId.empty()) {
        spdlog::warn("Received response with empty request ID");
        return;
    }

    try {
        std::lock_guard<std::mutex> lock(responseMutex_);
        auto it = pendingResponses_.find(response.requestId);
        if (it != pendingResponses_.end()) {
            spdlog::debug("Found pending response for request ID: {}", response.requestId);
            it->second.promise.set_value(response);
            pendingResponses_.erase(it);
        } else {
            spdlog::warn("Received response for unknown request ID: {}", response.requestId);
        }
    } catch (const std::exception& e) {
        spdlog::error("Error handling response: {}", e.what());
        if (errorCallback_) {
            errorCallback_(e.what());
        }
    }
}

std::future<NetworkMessage> NetworkInterface::waitForResponse(
    const std::string& requestId, 
    std::chrono::milliseconds timeout) {
    
    if (requestId.empty()) {
        spdlog::error("Cannot wait for response with empty request ID");
        std::promise<NetworkMessage> promise;
        NetworkMessage errorMsg;
        errorMsg.type = NetworkMessage::Type::ERROR;
        errorMsg.payload = "Invalid request ID";
        promise.set_value(errorMsg);
        return promise.get_future();
    }

    if (timeout.count() <= 0) {
        spdlog::warn("Invalid timeout value: {}ms, using default 30s", timeout.count());
        timeout = std::chrono::seconds(30);
    }

    std::future<NetworkMessage> future;
    {
        std::lock_guard<std::mutex> lock(responseMutex_);
        auto it = pendingResponses_.find(requestId);
        if (it == pendingResponses_.end()) {
            spdlog::debug("Creating new pending response for request ID: {}", requestId);
            PendingResponse pending;
            pending.expiry = std::chrono::system_clock::now() + timeout;
            future = pending.promise.get_future();
            pendingResponses_[requestId] = std::move(pending);
        } else {
            spdlog::warn("Request ID already exists: {}", requestId);
            future = it->second.promise.get_future();
        }
    }
    return future;
    }

void NetworkInterface::cleanupExpiredResponses() {
    auto now = std::chrono::system_clock::now();
    std::lock_guard<std::mutex> lock(responseMutex_);
    
    for (auto it = pendingResponses_.begin(); it != pendingResponses_.end();) {
        if (it->second.expiry < now) {
            NetworkMessage timeoutResponse;
            timeoutResponse.type = NetworkMessage::Type::ERROR;
            timeoutResponse.payload = "Response timeout";
            it->second.promise.set_value(timeoutResponse);
            it = pendingResponses_.erase(it);
        } else {
            ++it;
        }
    }
}

void NetworkInterface::setMessageCallback(MessageCallback callback) {
    messageCallback_ = std::move(callback);
}

void NetworkInterface::setErrorCallback(std::function<void(const std::string&)> callback) {
    errorCallback_ = std::move(callback);
}

bool NetworkInterface::connect(const std::string& address, uint16_t port) {
    address_ = address;
    port_ = port;
    running_ = true;
    messageThread_ = std::thread(&NetworkInterface::messageLoop, this);
    return true;
}

void NetworkInterface::disconnect() {
    running_ = false;
    if (messageThread_.joinable()) {
        messageThread_.join();
    }
    connected_ = false;
    
    // Reject all pending responses
    std::lock_guard<std::mutex> lock(responseMutex_);
    for (auto& [requestId, pending] : pendingResponses_) {
        NetworkResponse response;
        response.success = false;
        response.error = "Connection closed";
        pending.promise.set_value(response);
    }
    pendingResponses_.clear();
}

bool NetworkInterface::isConnected() const {
    return connected_;
}

void NetworkInterface::sendMessage(const NetworkMessage& message) {
    if (!running_) {
        spdlog::error("Cannot send message: NetworkInterface is not running");
        if (errorCallback_) {
            errorCallback_("NetworkInterface is not running");
        }
        return;
    }

    // Validate message
    if (message.type == NetworkMessage::Type::SYNC_REQUEST && message.requestId.empty()) {
        spdlog::error("SYNC_REQUEST message must have a request ID");
        if (errorCallback_) {
            errorCallback_("SYNC_REQUEST message must have a request ID");
    }
        return;
    }

    try {
    std::lock_guard<std::mutex> lock(queueMutex_);
    messageQueue_.push(message);
    queueCondition_.notify_one();
        spdlog::debug("Message queued for sending, type: {}, requestId: {}", 
            static_cast<int>(message.type), message.requestId);
    } catch (const std::exception& e) {
        spdlog::error("Failed to queue message: {}", e.what());
        if (errorCallback_) {
            errorCallback_(std::string("Failed to queue message: ") + e.what());
}
    }
}

void NetworkInterface::processMessageQueue() {
    if (!running_) {
        return;
    }

    try {
        std::unique_lock<std::mutex> lock(queueMutex_);
        if (messageQueue_.empty()) {
            return;
        }

        // Process messages in batches to avoid holding the lock too long
        const size_t MAX_BATCH_SIZE = 10;
        size_t processed = 0;
        
        while (!messageQueue_.empty() && processed < MAX_BATCH_SIZE) {
            NetworkMessage message = messageQueue_.front();
            messageQueue_.pop();
            lock.unlock();

            try {
                spdlog::debug("Processing message from queue, type: {}", 
                    static_cast<int>(message.type));
                handleIncomingMessage(message);
            } catch (const std::exception& e) {
                spdlog::error("Error processing message: {}", e.what());
                if (errorCallback_) {
                    errorCallback_(e.what());
}
            }

            lock.lock();
            processed++;
        }
    } catch (const std::exception& e) {
        spdlog::error("Error in message queue processing: {}", e.what());
        if (errorCallback_) {
            errorCallback_(e.what());
        }
    }
}

void NetworkInterface::messageLoop() {
    while (running_) {
            processMessageQueue();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void NetworkInterface::reconnect() {
    if (!connected_) {
        // Implementation of reconnection logic
        connected_ = true;
        }
    }

// Blockchain-specific operations
std::string NetworkInterface::getLatestBlock() {
    NetworkMessage msg;
    msg.type = NetworkMessage::Type::SYNC_REQUEST;
    msg.requestId = generateRequestId();
    msg.payload = "getLatestBlock";
    msg.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    
    sendMessage(msg);
    auto response = waitForResponse(msg.requestId);
    return response.get().success ? response.get().data : "{}";
}

std::string NetworkInterface::getBlockByHash(const std::string& hash) {
    NetworkMessage msg;
    msg.type = NetworkMessage::Type::SYNC_REQUEST;
    msg.requestId = generateRequestId();
    msg.payload = "getBlockByHash:" + hash;
    msg.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    
    sendMessage(msg);
    auto response = waitForResponse(msg.requestId);
    return response.get().success ? response.get().data : "{}";
}

std::string NetworkInterface::getTransactionByHash(const std::string& hash) {
    NetworkMessage msg;
    msg.type = NetworkMessage::Type::SYNC_REQUEST;
    msg.requestId = generateRequestId();
    msg.payload = "getTransactionByHash:" + hash;
    msg.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    
    sendMessage(msg);
    auto response = waitForResponse(msg.requestId);
    return response.get().success ? response.get().data : "{}";
}

std::string NetworkInterface::getBalance(const std::string& address) {
    NetworkMessage msg;
    msg.type = NetworkMessage::Type::SYNC_REQUEST;
    msg.requestId = generateRequestId();
    msg.payload = "getBalance:" + address;
    msg.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    
    sendMessage(msg);
    auto response = waitForResponse(msg.requestId);
    return response.get().success ? response.get().data : "{}";
    }

std::string NetworkInterface::sendTransaction(const std::string& transaction) {
    NetworkMessage msg;
    msg.type = NetworkMessage::Type::TRANSACTION;
    msg.requestId = generateRequestId();
    msg.payload = transaction;
    msg.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    
    sendMessage(msg);
    auto response = waitForResponse(msg.requestId);
    return response.get().success ? response.get().data : "{}";
}

} // namespace core
} // namespace satox 