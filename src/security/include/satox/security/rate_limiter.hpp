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
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <memory>

namespace satox {
namespace security {

class RateLimiter {
public:
    RateLimiter();
    ~RateLimiter();

    // Initialize the rate limiter
    bool initialize();

    // Shutdown the rate limiter
    void shutdown();

    // Check if a request is allowed
    bool checkLimit(const std::string& key, int maxRequests, int timeWindow);

    // Reset the rate limit for a key
    void resetLimit(const std::string& key);

    // Get current request count for a key
    int getRequestCount(const std::string& key);

    // Get time remaining in the current window
    int getTimeRemaining(const std::string& key);

    // Set a custom time window for a key
    void setTimeWindow(const std::string& key, int timeWindow);

    // Set a custom max requests for a key
    void setMaxRequests(const std::string& key, int maxRequests);

    // Get window start time for a key
    std::chrono::system_clock::time_point getWindowStart(const std::string& key);

    // Get time window for a key
    int getTimeWindow(const std::string& key);

    // Get max requests for a key
    int getMaxRequests(const std::string& key);

    // Check if initialized
    bool isInitialized() const;

private:
    struct RateLimit {
        int requestCount;
        std::chrono::system_clock::time_point windowStart;
        std::chrono::seconds timeWindow;
        int maxRequests;
    };

    std::mutex mutex_;
    bool initialized_;
    std::unordered_map<std::string, RateLimit> limits_;
};

} // namespace security
} // namespace satox 