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
#include <unordered_map>
#include <functional>
#include <optional>
#include <chrono>
#include <cstdint>
#include <thread>
#include <atomic>
#include <nlohmann/json.hpp>
#include <queue>
#include <condition_variable>
#include "types.hpp"
#include <unordered_set>

namespace satox::core {

class TransactionManager {
public:
    struct Config {
        std::string data_dir;
        uint32_t max_tx_size = 1000000;  // 1MB
        uint32_t max_inputs = 1000;
        uint32_t max_outputs = 1000;
        uint32_t min_fee = 1000;  // 0.00001 SATOX
        uint32_t max_fee = 1000000;  // 0.01 SATOX
        uint32_t fee_rate = 100;  // satoshis per byte
        uint32_t mempool_size = 100000;  // Maximum number of transactions in mempool
        uint32_t mempool_expiry = 3600;  // 1 hour in seconds
        bool enable_fee_estimation = true;
        bool enable_utxo_cache = true;
        uint32_t utxo_cache_size = 1000000;  // Maximum number of UTXOs to cache
    };

    struct UTXO {
        std::string tx_hash;
        uint32_t output_index;
        uint64_t amount;
        std::string script_pubkey;
        uint32_t block_height;
        bool spent;
        std::string spending_tx_hash;
    };

    struct Input {
        std::string prev_tx_hash;
        uint32_t prev_tx_index;
        std::vector<uint8_t> script_sig;
        uint32_t sequence;
    };

    struct Output {
        uint64_t value;
        std::vector<uint8_t> script_pubkey;
    };

    enum class Priority {
        LOW,
        NORMAL,
        HIGH,
        CRITICAL
    };

    enum class TransactionStatus {
        PENDING,
        ACTIVE,
        COMPLETED,
        FAILED,
        CANCELLED
    };

    struct Transaction {
        std::string id;
        std::string type;
        nlohmann::json data;
        Priority priority;
        TransactionStatus status;
        nlohmann::json result;
        std::string error;
        std::chrono::system_clock::time_point createdAt;
        std::chrono::system_clock::time_point updatedAt;
        std::chrono::system_clock::time_point startedAt;
        std::chrono::system_clock::time_point completedAt;
        std::chrono::system_clock::time_point failedAt;
        std::chrono::system_clock::time_point cancelledAt;
        std::unordered_map<std::string, std::string> metadata;
    };

    static std::string TransactionStatusToString(TransactionStatus status);
    static TransactionStatus StringToTransactionStatus(const std::string& str);

    // nlohmann::json serialization for Transaction
    friend void to_json(nlohmann::json& j, const Transaction& t);
    friend void from_json(const nlohmann::json& j, Transaction& t);

    struct FeeEstimate {
        uint32_t fee_rate;
        uint32_t estimated_size;
        uint64_t estimated_fee;
        uint32_t confidence;
    };

    struct TransactionStats {
        uint32_t total_transactions;
        uint32_t pending_transactions;
        uint32_t confirmed_transactions;
        uint32_t rejected_transactions;
        uint64_t total_volume;
        uint64_t total_fees;
        std::time_t last_update;
    };

    struct Stats {
        uint32_t totalTransactions;
        uint32_t pendingTransactions;
        uint32_t activeTransactions;
        uint32_t completedTransactions;
        uint32_t failedTransactions;
        uint32_t cancelledTransactions;
        uint64_t totalVolume;
        uint64_t totalFees;
        uint32_t maxTransactions;
        uint32_t maxConcurrent;
        std::time_t lastUpdate;
    };

    struct PerformanceConfig {
        bool enable_batch_processing = true;
        uint32_t batch_size = 1000;
        bool enable_connection_pooling = true;
        uint32_t pool_size = 10;
        bool enable_caching = true;
        uint32_t cache_size = 100000;
        bool enable_async_processing = true;
        uint32_t worker_threads = 4;
    };

    struct ErrorRecoveryConfig {
        uint32_t max_retry_attempts = 3;
        uint32_t retry_delay_ms = 1000;
        uint32_t recovery_timeout_ms = 30000;
        bool enable_auto_recovery = true;
        uint32_t max_recovery_queue_size = 1000;
    };

    struct RecoveryOperation {
        enum class Type {
            TRANSACTION_VALIDATION,
            TRANSACTION_SIGNING,
            TRANSACTION_BROADCAST,
            UTXO_UPDATE,
            MEMPOOL_UPDATE,
            CACHE_UPDATE
        };

        Type type;
        std::string operation_id;
        std::chrono::system_clock::time_point timestamp;
        std::string error_message;
        uint32_t attempt_count;
        nlohmann::json context;
    };

    using TransactionCallback = std::function<void(const Transaction&)>;
    using ErrorCallback = std::function<void(const std::string&)>;

    // Singleton instance
    static TransactionManager& getInstance();

    // Constructor and destructor
    TransactionManager();
    ~TransactionManager();

    // Initialization and cleanup
    bool initialize(size_t maxTransactions, size_t maxConcurrent);
    bool shutdown();
    std::string createTransaction(const std::string& type, const nlohmann::json& data, Priority priority);

    // Transaction creation and management
    bool signTransaction(Transaction& transaction, const std::string& private_key);
    bool validateTransaction(const Transaction& transaction) const;
    bool submitTransaction(Transaction& transaction);
    bool broadcastTransaction(const std::string& tx_hash);
    std::optional<Transaction> getTransaction(const std::string& tx_hash) const;
    std::vector<Transaction> getMempool() const;
    bool removeFromMempool(const std::string& tx_hash);

    // UTXO management
    std::vector<UTXO> getUTXOs(const std::string& address) const;
    bool isUTXOSpent(const std::string& tx_hash, uint32_t output_index) const;
    bool updateUTXO(const UTXO& utxo);
    bool removeUTXO(const std::string& tx_hash, uint32_t output_index);

    // Fee management
    FeeEstimate estimateFee(const Transaction& transaction) const;
    uint64_t calculateFee(const Transaction& transaction) const;
    bool validateFee(const Transaction& transaction) const;

    // Transaction history
    std::vector<Transaction> getTransactionHistory(const std::string& address) const;
    std::vector<Transaction> getBlockTransactions(const std::string& block_hash) const;
    std::vector<Transaction> getAddressTransactions(const std::string& address) const;

    // Statistics
    TransactionStats getTransactionStats() const;

    // Callbacks
    void registerTransactionCallback(TransactionCallback callback);
    void registerErrorCallback(ErrorCallback callback);

    // Transaction monitoring
    void monitorTransaction(const std::string& transaction_id);
    void stopMonitoring();

    // Error handling
    std::string getLastError() const;
    void clearLastError();

    // Additional methods implemented in cpp file
    bool hasTransaction(const std::string& id) const;
    bool isTransactionActive(const std::string& id) const;
    bool isTransactionPending(const std::string& id) const;
    bool isTransactionCompleted(const std::string& id) const;
    bool isTransactionFailed(const std::string& id) const;
    bool isTransactionCancelled(const std::string& id) const;
    void clearCompletedTransactions(size_t maxAge);
    void clearAllTransactions();
    Stats getStats() const;
    bool isHealthy() const;

    // Performance optimization methods
    void setPerformanceConfig(const PerformanceConfig& config);
    void clearCache();
    size_t getCacheSize() const;
    size_t getConnectionPoolSize() const;
    size_t getBatchSize() const;
    void processBatch();
    void optimizeMemory();

    // Error recovery methods
    void setErrorRecoveryConfig(const ErrorRecoveryConfig& config);
    bool recoverFromError(const std::string& operation_id);
    std::vector<RecoveryOperation> getRecoveryHistory() const;
    void clearRecoveryHistory();
    bool isRecoveryInProgress() const;
    std::string getLastRecoveryError() const;

    bool startTransaction(const std::string& id);
    bool completeTransaction(const std::string& id, const nlohmann::json& result);
    bool failTransaction(const std::string& id, const std::string& error);
    bool cancelTransaction(const std::string& id);
    std::vector<Transaction> getTransactions(TransactionStatus status) const;
    std::vector<Transaction> getTransactions(const std::string& type) const;
    std::vector<Transaction> getPendingTransactions() const;
    std::vector<Transaction> getActiveTransactions() const;
    std::vector<Transaction> getCompletedTransactions() const;
    std::vector<Transaction> getFailedTransactions() const;

private:
    // Member variables
    bool initialized_ = false;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, Transaction> transactions_;
    std::vector<std::string> pendingTransactions_;
    std::unordered_set<std::string> activeTransactions_;
    std::vector<std::string> completedTransactions_;
    std::vector<std::string> failedTransactions_;
    size_t maxTransactions_ = 10000;
    size_t maxConcurrent_ = 100;
    std::vector<TransactionCallback> callbacks_;
    std::string lastError_;
    nlohmann::json stats_;

    // Helper methods
    std::string generateTransactionId();
    bool verifySignature(const Transaction& transaction);
    void updateTransactionStatus(const std::string& transactionId, Status status, const std::string& error = "");

    // Performance optimization members
    PerformanceConfig perf_config_;
    std::unordered_map<std::string, Transaction> transaction_cache_;
    std::unordered_map<std::string, UTXO> utxo_cache_;
    std::vector<Transaction> batch_queue_;
    std::vector<std::thread> worker_threads_;
    std::queue<std::function<void()>> task_queue_;
    std::mutex task_mutex_;
    std::condition_variable task_cv_;
    std::atomic<bool> workers_running_ = false;

    // Performance optimization methods
    void startWorkers();
    void stopWorkers();
    void workerFunction();
    void addTask(std::function<void()> task);
    void processTaskQueue();
    void updateCache(const Transaction& transaction);
    void updateCache(const UTXO& utxo);
    void cleanupCache();
    void cleanupConnections();
    void processBatchQueue();
    void optimizeMemoryUsage();

    // Error recovery members
    ErrorRecoveryConfig recovery_config_;
    std::vector<RecoveryOperation> recovery_history_;
    std::unordered_map<std::string, RecoveryOperation> active_recoveries_;
    mutable std::mutex recovery_mutex_;
    std::string last_recovery_error_;

    // Error recovery methods
    bool attemptRecovery(const RecoveryOperation& operation);
    void logRecoveryAttempt(const RecoveryOperation& operation, bool success);
    bool validateRecoveryContext(const RecoveryOperation& operation);
    void cleanupRecoveryOperation(const std::string& operation_id);
    bool isRecoveryAllowed(const RecoveryOperation& operation) const;
    std::string generateRecoveryId();
    void notifyError(const std::string& txHash, const std::string& error);
};

} // namespace satox::core 