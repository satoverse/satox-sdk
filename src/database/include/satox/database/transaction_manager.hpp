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
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>
#include <functional>
#include <nlohmann/json.hpp>
#include "satox/database/connection_pool.hpp"

namespace satox {
namespace database {

class TransactionManager {
public:
    struct TransactionConfig {
        std::chrono::seconds timeout;
        bool auto_commit;
        bool read_only;
        std::string isolation_level;
    };

    struct TransactionStats {
        size_t active_transactions;
        size_t committed_transactions;
        size_t rolled_back_transactions;
        size_t failed_transactions;
        std::chrono::milliseconds avg_transaction_time;
        size_t deadlocks;
        size_t timeouts;
    };

    static TransactionManager& getInstance();

    bool initialize(const TransactionConfig& config);
    void shutdown();

    // Transaction management
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    bool isInTransaction() const;

    // Transaction operations
    bool executeQuery(const std::string& query, const nlohmann::json& params = nlohmann::json::object());
    bool executeBatch(const std::vector<std::string>& queries, const std::vector<nlohmann::json>& params);
    nlohmann::json executeQueryWithResult(const std::string& query, const nlohmann::json& params = nlohmann::json::object());

    // Savepoint management
    bool createSavepoint(const std::string& name);
    bool rollbackToSavepoint(const std::string& name);
    bool releaseSavepoint(const std::string& name);

    // Transaction configuration
    void setTimeout(std::chrono::seconds timeout);
    void setAutoCommit(bool auto_commit);
    void setReadOnly(bool read_only);
    void setIsolationLevel(const std::string& level);

    // Statistics and monitoring
    TransactionStats getStats() const;
    void resetStats();
    bool isHealthy() const;
    std::string getLastError() const;

private:
    TransactionManager() = default;
    ~TransactionManager();
    TransactionManager(const TransactionManager&) = delete;
    TransactionManager& operator=(const TransactionManager&) = delete;

    // Internal transaction state
    struct Transaction {
        std::shared_ptr<void> connection;
        std::chrono::steady_clock::time_point start_time;
        std::string isolation_level;
        bool read_only;
        std::vector<std::string> savepoints;
    };

    // Helper methods
    bool validateTransaction() const;
    bool checkTimeout() const;
    void updateStats(bool success, bool committed);
    void handleDeadlock();
    void handleTimeout();

    TransactionConfig config_;
    std::unique_ptr<Transaction> current_transaction_;
    mutable std::mutex mutex_;
    std::atomic<bool> running_;
    std::atomic<size_t> active_transactions_;
    std::atomic<size_t> committed_transactions_;
    std::atomic<size_t> rolled_back_transactions_;
    std::atomic<size_t> failed_transactions_;
    std::atomic<size_t> deadlocks_;
    std::atomic<size_t> timeouts_;
    std::string last_error_;
};

} // namespace database
} // namespace satox 