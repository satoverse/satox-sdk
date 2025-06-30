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

#include "satox/security/rate_limiter.hpp"

namespace satox {
namespace security {

RateLimiter::RateLimiter() : initialized_(false) {}

RateLimiter::~RateLimiter() {
    shutdown();
}

bool RateLimiter::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) {
        return true;
    }

    // Initialize rate limiter
    initialized_ = true;
    return true;
}

void RateLimiter::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        return;
    }

    // Cleanup
    limits_.clear();
    initialized_ = false;
}

bool RateLimiter::checkLimit(const std::string& key, int maxRequests, int timeWindow) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        return false;
    }

    auto now = std::chrono::system_clock::now();
    auto& limit = limits_[key];

    // Initialize limit if it doesn't exist
    if (limit.windowStart == std::chrono::system_clock::time_point()) {
        limit.requestCount = 1;
        limit.windowStart = now;
        limit.timeWindow = std::chrono::seconds(timeWindow);
        limit.maxRequests = maxRequests;
        return true;
    }

    // Check if the time window has expired
    if (now - limit.windowStart > limit.timeWindow) {
        limit.requestCount = 1;
        limit.windowStart = now;
        return true;
    }

    // Check if we're under the limit
    if (limit.requestCount < limit.maxRequests) {
        limit.requestCount++;
        return true;
    }

    return false;
}

void RateLimiter::resetLimit(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        return;
    }
    limits_.erase(key);
}

int RateLimiter::getRequestCount(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        return 0;
    }

    auto it = limits_.find(key);
    if (it == limits_.end()) {
        return 0;
    }

    return it->second.requestCount;
}

int RateLimiter::getTimeRemaining(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        return 0;
    }

    auto it = limits_.find(key);
    if (it == limits_.end()) {
        return 0;
    }

    auto now = std::chrono::system_clock::now();
    auto elapsed = now - it->second.windowStart;
    auto remaining = it->second.timeWindow - elapsed;
    
    return std::chrono::duration_cast<std::chrono::seconds>(remaining).count();
}

void RateLimiter::setTimeWindow(const std::string& key, int timeWindow) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        return;
    }

    auto& limit = limits_[key];
    limit.timeWindow = std::chrono::seconds(timeWindow);
}

void RateLimiter::setMaxRequests(const std::string& key, int maxRequests) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        return;
    }

    auto& limit = limits_[key];
    limit.maxRequests = maxRequests;
}

std::chrono::system_clock::time_point RateLimiter::getWindowStart(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        return std::chrono::system_clock::time_point();
    }

    auto it = limits_.find(key);
    if (it == limits_.end()) {
        return std::chrono::system_clock::time_point();
    }

    return it->second.windowStart;
}

int RateLimiter::getTimeWindow(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        return 0;
    }

    auto it = limits_.find(key);
    if (it == limits_.end()) {
        return 0;
    }

    return std::chrono::duration_cast<std::chrono::seconds>(it->second.timeWindow).count();
}

int RateLimiter::getMaxRequests(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        return 0;
    }

    auto it = limits_.find(key);
    if (it == limits_.end()) {
        return 0;
    }

    return it->second.maxRequests;
}

bool RateLimiter::isInitialized() const {
    return initialized_;
}

} // namespace security
} // namespace satox 