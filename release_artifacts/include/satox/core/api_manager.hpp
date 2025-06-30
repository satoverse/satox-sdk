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
#include <mutex>
#include <optional>
#include <vector>
#include <unordered_map>
#include <functional>
#include <thread>
#include <atomic>
#include <queue>
#include <condition_variable>

namespace satox {
namespace core {

class APIManager {
public:
    struct Config {
        std::string host = "0.0.0.0";
        int port = 7777;  // Satoxcoin RPC port
        int max_connections = 100;
        int connection_timeout = 30;
        bool enable_ssl = true;
        std::string ssl_cert;
        std::string ssl_key;
        std::string ssl_ca;
        int max_request_size = 1024 * 1024;  // 1MB
        int max_websocket_message_size = 1024 * 1024;  // 1MB
        bool enable_rate_limiting = true;
        int rate_limit_requests = 100;  // requests per minute
        int rate_limit_window = 60;  // seconds
    };

    struct Request {
        std::string method;
        std::string path;
        std::unordered_map<std::string, std::string> headers;
        std::string body;
        std::string client_ip;
    };

    struct Response {
        int status_code;
        std::unordered_map<std::string, std::string> headers;
        std::string body;
    };

    struct WebSocketMessage {
        std::string type;  // "text", "binary", "ping", "pong", "close"
        std::string data;
        bool is_final;
    };

    using RequestHandler = std::function<Response(const Request&)>;
    using WebSocketHandler = std::function<void(const WebSocketMessage&)>;

    APIManager();
    ~APIManager();

    bool initialize(const Config& config);
    bool start();
    void stop();
    bool isRunning() const;

    // REST API endpoints
    bool registerEndpoint(const std::string& method, const std::string& path, RequestHandler handler);
    bool unregisterEndpoint(const std::string& method, const std::string& path);
    
    // WebSocket support
    bool registerWebSocketHandler(const std::string& path, WebSocketHandler handler);
    bool unregisterWebSocketHandler(const std::string& path);
    bool sendWebSocketMessage(const std::string& client_id, const WebSocketMessage& message);
    bool broadcastWebSocketMessage(const WebSocketMessage& message);

    // Rate limiting
    bool setRateLimit(int requests_per_minute);
    bool isRateLimited(const std::string& client_ip) const;

    // Error handling
    std::string getLastError() const;
    void clearError();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace core
} // namespace satox 