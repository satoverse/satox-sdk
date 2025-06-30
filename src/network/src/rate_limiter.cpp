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

#include "rate_limiter.hpp"
#include <spdlog/spdlog.h>

namespace satox {

RateLimiter& RateLimiter::getInstance() {
    static RateLimiter instance;
    return instance;
}

RateLimiter::RateLimiter() : running_(false) {
    // Set default limits
    default_limit_ = {100, std::chrono::seconds(60)}; // 100 requests per minute

    // Set method-specific limits
    method_limits_["getblockchaininfo"] = {10, std::chrono::seconds(60)};
    method_limits_["getmempoolinfo"] = {10, std::chrono::seconds(60)};
    method_limits_["getmininginfo"] = {10, std::chrono::seconds(60)};
    method_limits_["getnetworkinfo"] = {10, std::chrono::seconds(60)};
    method_limits_["getpeerinfo"] = {10, std::chrono::seconds(60)};
    method_limits_["getrawtransaction"] = {20, std::chrono::seconds(60)};
    method_limits_["sendrawtransaction"] = {5, std::chrono::seconds(60)};
    method_limits_["getblock"] = {20, std::chrono::seconds(60)};
    method_limits_["getblockhash"] = {20, std::chrono::seconds(60)};
    method_limits_["getblockcount"] = {20, std::chrono::seconds(60)};
    method_limits_["getdifficulty"] = {20, std::chrono::seconds(60)};
    method_limits_["getbalance"] = {20, std::chrono::seconds(60)};
    method_limits_["listunspent"] = {20, std::chrono::seconds(60)};
    method_limits_["getnewaddress"] = {10, std::chrono::seconds(60)};
    method_limits_["gettransaction"] = {20, std::chrono::seconds(60)};
    method_limits_["listtransactions"] = {20, std::chrono::seconds(60)};
    method_limits_["backupwallet"] = {1, std::chrono::seconds(3600)};
    method_limits_["importwallet"] = {1, std::chrono::seconds(3600)};
    method_limits_["dumpprivkey"] = {1, std::chrono::seconds(3600)};
    method_limits_["importprivkey"] = {1, std::chrono::seconds(3600)};
    method_limits_["getwalletinfo"] = {20, std::chrono::seconds(60)};
    method_limits_["getaddressesbyaccount"] = {20, std::chrono::seconds(60)};
    method_limits_["getaccount"] = {20, std::chrono::seconds(60)};
    method_limits_["getaccountaddress"] = {20, std::chrono::seconds(60)};
    method_limits_["getaddressesbylabel"] = {20, std::chrono::seconds(60)};
    method_limits_["getreceivedbylabel"] = {20, std::chrono::seconds(60)};
    method_limits_["listlabels"] = {20, std::chrono::seconds(60)};
    method_limits_["getaddressinfo"] = {20, std::chrono::seconds(60)};
    method_limits_["getblocktemplate"] = {10, std::chrono::seconds(60)};
    method_limits_["submitblock"] = {5, std::chrono::seconds(60)};
    method_limits_["getmempoolentry"] = {20, std::chrono::seconds(60)};
    method_limits_["gettxout"] = {20, std::chrono::seconds(60)};
    method_limits_["gettxoutsetinfo"] = {5, std::chrono::seconds(60)};
    method_limits_["verifychain"] = {1, std::chrono::seconds(3600)};
    method_limits_["getchaintips"] = {10, std::chrono::seconds(60)};
    method_limits_["getchaintxstats"] = {10, std::chrono::seconds(60)};
    method_limits_["getnettotals"] = {10, std::chrono::seconds(60)};
    method_limits_["getnetworkhashps"] = {10, std::chrono::seconds(60)};
    method_limits_["getmemoryinfo"] = {10, std::chrono::seconds(60)};
    method_limits_["getrpcinfo"] = {10, std::chrono::seconds(60)};
    method_limits_["help"] = {20, std::chrono::seconds(60)};
    method_limits_["stop"] = {1, std::chrono::seconds(3600)};
    method_limits_["uptime"] = {20, std::chrono::seconds(60)};

    // Set Satoxcoin-specific method limits
    method_limits_["issueasset"] = {5, std::chrono::seconds(3600)};
    method_limits_["reissueasset"] = {5, std::chrono::seconds(3600)};
    method_limits_["transferasset"] = {20, std::chrono::seconds(60)};
    method_limits_["listassets"] = {20, std::chrono::seconds(60)};
    method_limits_["getassetinfo"] = {20, std::chrono::seconds(60)};
    method_limits_["getassetallocation"] = {20, std::chrono::seconds(60)};
    method_limits_["getassethistory"] = {20, std::chrono::seconds(60)};
    method_limits_["getassetbalances"] = {20, std::chrono::seconds(60)};
    method_limits_["getassettransactions"] = {20, std::chrono::seconds(60)};
    method_limits_["getassetaddresses"] = {20, std::chrono::seconds(60)};
    method_limits_["getassetaddressbalances"] = {20, std::chrono::seconds(60)};
    method_limits_["getassetaddresstransactions"] = {20, std::chrono::seconds(60)};
    method_limits_["getassetaddresshistory"] = {20, std::chrono::seconds(60)};
    method_limits_["getassetaddressallocations"] = {20, std::chrono::seconds(60)};
    method_limits_["getassetaddressallocationhistory"] = {20, std::chrono::seconds(60)};
    method_limits_["getassetaddressallocationbalances"] = {20, std::chrono::seconds(60)};
    method_limits_["getassetaddressallocationtransactions"] = {20, std::chrono::seconds(60)};
    method_limits_["getipfshash"] = {20, std::chrono::seconds(60)};
    method_limits_["getipfsdata"] = {20, std::chrono::seconds(60)};
    method_limits_["getipfshistory"] = {20, std::chrono::seconds(60)};
    method_limits_["getipfsbalances"] = {20, std::chrono::seconds(60)};
    method_limits_["getipfstransactions"] = {20, std::chrono::seconds(60)};
    method_limits_["getipfsaddresses"] = {20, std::chrono::seconds(60)};
    method_limits_["getipfsaddressbalances"] = {20, std::chrono::seconds(60)};
    method_limits_["getipfsaddresstransactions"] = {20, std::chrono::seconds(60)};
    method_limits_["getipfsaddresshistory"] = {20, std::chrono::seconds(60)};
    method_limits_["getnftinfo"] = {20, std::chrono::seconds(60)};
    method_limits_["getnfthistory"] = {20, std::chrono::seconds(60)};
    method_limits_["getnftbalances"] = {20, std::chrono::seconds(60)};
    method_limits_["getnfttransactions"] = {20, std::chrono::seconds(60)};
    method_limits_["getnftaddresses"] = {20, std::chrono::seconds(60)};
    method_limits_["getnftaddressbalances"] = {20, std::chrono::seconds(60)};
    method_limits_["getnftaddresstransactions"] = {20, std::chrono::seconds(60)};
    method_limits_["getnftaddresshistory"] = {20, std::chrono::seconds(60)};
}

RateLimiter::~RateLimiter() {
    shutdown();
}

bool RateLimiter::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (running_) {
        spdlog::error("RateLimiter already running");
        return false;
    }

    running_ = true;
    spdlog::info("RateLimiter initialized successfully");
    return true;
}

void RateLimiter::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!running_) return;

    running_ = false;
    resetAllLimits();
    spdlog::info("RateLimiter shut down successfully");
}

bool RateLimiter::isRunning() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return running_;
}

bool RateLimiter::checkLimit(const std::string& method, const std::string& client_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!running_) {
        throw std::runtime_error("RateLimiter not running");
    }

    auto now = std::chrono::steady_clock::now();
    auto& client_counts = request_counts_[client_id];
    auto& count = client_counts[method];

    // Get the appropriate limit configuration
    const LimitConfig* limit = nullptr;
    if (auto it = method_limits_.find(method); it != method_limits_.end()) {
        limit = &it->second;
    } else if (auto it = client_limits_.find(client_id); it != client_limits_.end()) {
        limit = &it->second;
    } else {
        limit = &default_limit_;
    }

    // Check if the window has expired
    if (now - count.window_start > limit->window) {
        count.count = 0;
        count.window_start = now;
    }

    // Check if the limit has been exceeded
    return count.count < limit->max_requests;
}

void RateLimiter::updateLimit(const std::string& method, const std::string& client_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!running_) {
        throw std::runtime_error("RateLimiter not running");
    }

    auto now = std::chrono::steady_clock::now();
    auto& client_counts = request_counts_[client_id];
    auto& count = client_counts[method];

    // Get the appropriate limit configuration
    const LimitConfig* limit = nullptr;
    if (auto it = method_limits_.find(method); it != method_limits_.end()) {
        limit = &it->second;
    } else if (auto it = client_limits_.find(client_id); it != client_limits_.end()) {
        limit = &it->second;
    } else {
        limit = &default_limit_;
    }

    // Check if the window has expired
    if (now - count.window_start > limit->window) {
        count.count = 0;
        count.window_start = now;
    }

    // Increment the count
    count.count++;
}

void RateLimiter::resetLimit(const std::string& method, const std::string& client_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!running_) {
        throw std::runtime_error("RateLimiter not running");
    }

    auto& client_counts = request_counts_[client_id];
    client_counts.erase(method);
}

void RateLimiter::resetAllLimits() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!running_) {
        throw std::runtime_error("RateLimiter not running");
    }

    request_counts_.clear();
}

void RateLimiter::setMethodLimit(const std::string& method, int max_requests, std::chrono::seconds window) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!running_) {
        throw std::runtime_error("RateLimiter not running");
    }

    method_limits_[method] = {max_requests, window};
}

void RateLimiter::setDefaultLimit(int max_requests, std::chrono::seconds window) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!running_) {
        throw std::runtime_error("RateLimiter not running");
    }

    default_limit_ = {max_requests, window};
}

void RateLimiter::setClientLimit(const std::string& client_id, int max_requests, std::chrono::seconds window) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!running_) {
        throw std::runtime_error("RateLimiter not running");
    }

    client_limits_[client_id] = {max_requests, window};
}

} // namespace satox 