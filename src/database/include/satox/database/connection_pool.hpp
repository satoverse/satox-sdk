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
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <memory>
#include <functional>
#include <nlohmann/json.hpp>

namespace satox {
namespace database {

class ConnectionPool {
public:
    struct ConnectionConfig {
        std::string host;
        int port;
        std::string database;
        std::string username;
        std::string password;
        bool use_ssl;
        int max_connections;
        int min_connections;
        std::chrono::seconds connection_timeout;
        std::chrono::seconds idle_timeout;
        std::chrono::seconds max_lifetime;
    };

    struct ConnectionStats {
        size_t total_connections;
        size_t active_connections;
        size_t idle_connections;
        size_t waiting_requests;
        std::chrono::milliseconds avg_wait_time;
        size_t connection_errors;
        size_t timeout_errors;
    };

    static ConnectionPool& getInstance();

    bool initialize(const ConnectionConfig& config);
    void shutdown();

    // Connection management
    std::shared_ptr<void> acquireConnection();
    void releaseConnection(std::shared_ptr<void> connection);
    bool testConnection(std::shared_ptr<void> connection);

    // Pool management
    void setMaxConnections(size_t max);
    void setMinConnections(size_t min);
    void setConnectionTimeout(std::chrono::seconds timeout);
    void setIdleTimeout(std::chrono::seconds timeout);
    void setMaxLifetime(std::chrono::seconds lifetime);

    // Statistics
    ConnectionStats getStats() const;
    void resetStats();

    // Health check
    bool isHealthy() const;
    std::string getLastError() const;

private:
    ConnectionPool() = default;
    ~ConnectionPool();
    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;

    // Connection lifecycle
    std::shared_ptr<void> createConnection();
    void destroyConnection(std::shared_ptr<void> connection);
    void cleanupIdleConnections();
    void maintainPool();

    // Internal state
    struct Connection {
        std::shared_ptr<void> handle;
        std::chrono::steady_clock::time_point created_at;
        std::chrono::steady_clock::time_point last_used;
        bool in_use;
    };

    ConnectionConfig config_;
    std::vector<Connection> connections_;
    std::queue<size_t> available_connections_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> running_;
    std::atomic<size_t> total_connections_;
    std::atomic<size_t> active_connections_;
    std::atomic<size_t> waiting_requests_;
    std::atomic<size_t> connection_errors_;
    std::atomic<size_t> timeout_errors_;
    std::chrono::steady_clock::time_point start_time_;
    std::string last_error_;
    std::thread maintenance_thread_;
};

} // namespace database
} // namespace satox 