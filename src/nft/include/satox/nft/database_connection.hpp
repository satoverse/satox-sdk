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
#include <vector>
#include <map>
#include <functional>

namespace satox {
namespace nft {

class DatabaseConnection {
public:
    struct Config {
        std::string host;
        int port;
        std::string database;
        std::string username;
        std::string password;
        int max_connections = 10;
        int connection_timeout = 30;
        bool use_ssl = false;
    };

    struct QueryResult {
        bool success;
        std::string error;
        std::vector<std::map<std::string, std::string>> rows;
    };

    using QueryCallback = std::function<void(const QueryResult&)>;

    static std::shared_ptr<DatabaseConnection> create(const Config& config);
    virtual ~DatabaseConnection() = default;

    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
    virtual bool executeQuery(const std::string& query, QueryCallback callback) = 0;
    virtual bool executeTransaction(const std::vector<std::string>& queries, QueryCallback callback) = 0;
    virtual std::string getLastError() const = 0;
    virtual bool ping() = 0;
    virtual void setConfig(const Config& config) = 0;
    virtual Config getConfig() const = 0;

protected:
    DatabaseConnection() = default;
    DatabaseConnection(const DatabaseConnection&) = delete;
    DatabaseConnection& operator=(const DatabaseConnection&) = delete;
};

} // namespace nft
} // namespace satox 