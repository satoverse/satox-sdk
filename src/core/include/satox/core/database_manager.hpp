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
#include <nlohmann/json.hpp>
#include "types.hpp"

namespace satox {
namespace core {

enum class DatabaseType {
    SQLITE,
    POSTGRESQL,
    MYSQL,
    REDIS,
    MONGODB,
    ROCKSDB,
    MEMORY
};

class DatabaseManager {
public:
    struct DatabaseConfig {
        std::string host = "localhost";
        int port = 5432;
        std::string database = "satox";
        std::string username;
        std::string password;
        int max_connections = 10;
        int connection_timeout = 30;
        bool enable_ssl = true;
        std::string ssl_cert;
        std::string ssl_key;
        std::string ssl_ca;
    };

    struct QueryResult {
        bool success;
        std::string error;
        std::vector<std::unordered_map<std::string, std::string>> rows;
    };

    struct Transaction {
        bool success;
        std::string error;
        std::string transaction_id;
    };

    DatabaseManager();
    ~DatabaseManager();

    bool initialize(const DatabaseConfig& config);
    // Connection operations
    // bool connect();
    // void disconnect();
    // bool isConnected() const;
    std::string connect(DatabaseType type, const nlohmann::json& config);
    bool disconnect(const std::string& id);

    // Query operations
    // QueryResult executeQuery(const std::string& query);
    // QueryResult executePreparedQuery(const std::string& query, const std::vector<std::string>& params);
    bool executeQuery(const std::string& id, const std::string& query, nlohmann::json& result);
    bool executeTransaction(const std::string& id, const std::vector<std::string>& queries, nlohmann::json& result);
    
    // Transaction operations
    Transaction beginTransaction();
    bool commitTransaction(const std::string& transaction_id);
    bool rollbackTransaction(const std::string& transaction_id);

    // Connection pool operations
    bool setMaxConnections(int max_connections);
    int getActiveConnections() const;
    int getAvailableConnections() const;

    // Backup and restore
    bool backup(const std::string& backup_path);
    bool restore(const std::string& backup_path);

    // Error handling
    std::string getLastError() const;
    void clearError();

    void shutdown();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace core
} // namespace satox 