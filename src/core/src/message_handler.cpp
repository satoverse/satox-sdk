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

#include "satox/core/message_handler.hpp"
#include <stdexcept>
#include <chrono>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <spdlog/spdlog.h>

namespace satox {
namespace core {

MessageHandler::MessageHandler() : gen_(rd_()) {
}

MessageHandler::~MessageHandler() {
    cleanupExpiredResponses();
}

std::string MessageHandler::generateRequestId() {
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << gen_();
    return ss.str();
}

void MessageHandler::handleMessage(const NetworkMessage& message) {
    try {
        if (message.type == NetworkMessage::Type::SYNC_RESPONSE) {
            handleResponse(message);
        } else if (messageCallback_) {
            messageCallback_(message);
        }
    } catch (const std::exception& e) {
        spdlog::error("Error handling message: {}", e.what());
        if (errorCallback_) {
            errorCallback_(e.what());
        }
    }
}

void MessageHandler::handleResponse(const NetworkMessage& response) {
    if (response.requestId.empty()) {
        return;
    }

    std::lock_guard<std::mutex> lock(responseMutex_);
    auto it = pendingResponses_.find(response.requestId);
    if (it != pendingResponses_.end()) {
        NetworkResponse resp;
        resp.success = response.type != NetworkMessage::Type::ERROR;
        resp.data = response.payload;
        resp.timestamp = response.timestamp;
        if (!resp.success) {
            resp.error = response.payload;
        }
        it->second.promise.set_value(resp);
        pendingResponses_.erase(it);
    }
}

NetworkResponse MessageHandler::waitForResponse(const std::string& requestId, 
                                             std::chrono::milliseconds timeout) {
    std::future<NetworkResponse> future;
    {
        std::lock_guard<std::mutex> lock(responseMutex_);
        auto it = pendingResponses_.find(requestId);
        if (it == pendingResponses_.end()) {
            NetworkResponse response;
            response.success = false;
            response.error = "No pending response found for request ID: " + requestId;
            return response;
        }
        future = it->second.promise.get_future();
    }

    if (future.wait_for(timeout) == std::future_status::timeout) {
        std::lock_guard<std::mutex> lock(responseMutex_);
        pendingResponses_.erase(requestId);
        NetworkResponse response;
        response.success = false;
        response.error = "Response timeout";
        return response;
    }

    return future.get();
}

void MessageHandler::addPendingResponse(const std::string& requestId) {
    std::lock_guard<std::mutex> lock(responseMutex_);
    PendingResponse pending;
    pending.expiry = std::chrono::system_clock::now() + std::chrono::seconds(30);
    pendingResponses_[requestId] = std::move(pending);
}

void MessageHandler::cleanupExpiredResponses() {
    auto now = std::chrono::system_clock::now();
    std::lock_guard<std::mutex> lock(responseMutex_);
    
    for (auto it = pendingResponses_.begin(); it != pendingResponses_.end();) {
        if (it->second.expiry < now) {
            NetworkMessage response;
            response.type = NetworkMessage::Type::ERROR;
            response.payload = "Response timeout";
            response.timestamp = now;
            it->second.promise.set_value(response);
            it = pendingResponses_.erase(it);
        } else {
            ++it;
        }
    }
}

void MessageHandler::setMessageCallback(std::function<void(const NetworkMessage&)> callback) {
    messageCallback_ = std::move(callback);
}

void MessageHandler::setErrorCallback(std::function<void(const std::string&)> callback) {
    errorCallback_ = std::move(callback);
}

} // namespace core
} // namespace satox 