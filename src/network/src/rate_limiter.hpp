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

#include <chrono>
#include <mutex>
#include <unordered_map>
#include <string>
#include <memory>

namespace satox {

class RateLimiter {
public:
    static RateLimiter& getInstance();

    bool initialize();
    void shutdown();
    bool isRunning() const;

    // Rate limiting methods
    bool checkLimit(const std::string& method, const std::string& client_id);
    void updateLimit(const std::string& method, const std::string& client_id);
    void resetLimit(const std::string& method, const std::string& client_id);
    void resetAllLimits();

    // Configuration
    void setMethodLimit(const std::string& method, int max_requests, std::chrono::seconds window);
    void setDefaultLimit(int max_requests, std::chrono::seconds window);
    void setClientLimit(const std::string& client_id, int max_requests, std::chrono::seconds window);

private:
    RateLimiter();
    ~RateLimiter();

    // Prevent copying
    RateLimiter(const RateLimiter&) = delete;
    RateLimiter& operator=(const RateLimiter&) = delete;

    struct LimitConfig {
        int max_requests;
        std::chrono::seconds window;
    };

    struct RequestCount {
        int count;
        std::chrono::steady_clock::time_point window_start;
    };

    bool running_;
    std::mutex mutex_;
    std::unordered_map<std::string, LimitConfig> method_limits_;
    std::unordered_map<std::string, LimitConfig> client_limits_;
    LimitConfig default_limit_;
    std::unordered_map<std::string, std::unordered_map<std::string, RequestCount>> request_counts_;
};

} // namespace satox 