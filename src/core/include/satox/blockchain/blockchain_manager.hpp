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
#include <memory>
#include <mutex>
#include <unordered_map>
#include <functional>
#include <optional>
#include <chrono>
#include <list>
#include <shared_mutex>
#include <boost/pool/pool.hpp>
#include <boost/pool/object_pool.hpp>
#include <queue>
#include <condition_variable>
#include <thread>

namespace satox {
namespace blockchain {

struct BlockchainConfig {
    std::string network;  // "mainnet", "testnet", "regtest"
    std::string data_dir;
    std::string rpc_endpoint;
    std::string rpc_username;
    std::string rpc_password;
    uint32_t timeout_ms;
    bool enable_mining;
    uint32_t mining_threads;
    bool enable_sync;
    uint32_t sync_interval_ms;
};

struct BlockInfo {
    std::string hash;
    std::string previous_hash;
    uint32_t height;
    uint32_t timestamp;
    uint32_t difficulty;
    std::string miner;
    uint64_t reward;
    std::vector<std::string> transactions;
    std::unordered_map<std::string, std::string> metadata;
};

struct TransactionInfo {
    std::string txid;
    std::string block_hash;
    uint32_t block_height;
    uint32_t timestamp;
    std::string from_address;
    std::string to_address;
    uint64_t amount;
    uint64_t fee;
    std::string status;  // "pending", "confirmed", "failed"
    std::unordered_map<std::string, std::string> metadata;
};

struct MiningInfo {
    uint32_t height;
    uint32_t difficulty;
    std::string target;
    uint64_t reward;
    std::chrono::system_clock::time_point timestamp;
    std::unordered_map<std::string, std::string> metadata;
};

struct BlockchainStatistics {
    uint32_t current_height;
    uint32_t total_blocks;
    uint32_t total_transactions;
    uint64_t total_supply;
    uint32_t difficulty;
    std::chrono::system_clock::time_point last_sync_time;
    std::unordered_map<std::string, std::string> metadata;
};

// Performance optimization structures
struct CacheConfig {
    size_t max_block_cache_size = 1000;
    size_t max_tx_cache_size = 10000;
    size_t max_balance_cache_size = 100000;
    std::chrono::seconds block_cache_ttl{300};  // 5 minutes
    std::chrono::seconds tx_cache_ttl{60};      // 1 minute
    std::chrono::seconds balance_cache_ttl{30}; // 30 seconds
};

struct ConnectionPoolConfig {
    size_t max_connections = 100;
    size_t min_connections = 10;
    std::chrono::seconds connection_timeout{30};
    std::chrono::seconds idle_timeout{300};
    size_t max_retries = 3;
};

struct BatchConfig {
    size_t max_batch_size = 1000;
    std::chrono::milliseconds batch_timeout{100};
    size_t max_retries = 3;
};

// Error recovery structures
struct ErrorRecoveryConfig {
    size_t max_retry_attempts = 3;
    std::chrono::seconds retry_delay{5};
    std::chrono::seconds recovery_timeout{30};
    bool enable_auto_recovery = true;
    size_t max_recovery_queue_size = 1000;
};

struct RecoveryOperation {
    enum class Type {
        BLOCK_VALIDATION,
        TRANSACTION_VALIDATION,
        NETWORK_CONNECTION,
        STATE_SYNC,
        MINING_OPERATION
    };

    Type type;
    std::string operation_id;
    std::chrono::system_clock::time_point timestamp;
    std::string error_message;
    size_t attempt_count;
    std::unordered_map<std::string, std::string> context;
};

template<typename T>
class LRUCache {
public:
    explicit LRUCache(size_t capacity) : capacity_(capacity) {}

    void put(const std::string& key, const T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            list_.erase(it->second.second);
        } else if (cache_.size() >= capacity_) {
            cache_.erase(list_.back());
            list_.pop_back();
        }
        list_.push_front(key);
        cache_[key] = {value, list_.begin()};
    }

    std::optional<T> get(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = cache_.find(key);
        if (it == cache_.end()) {
            return std::nullopt;
        }
        list_.splice(list_.begin(), list_, it->second.second);
        return it->second.first;
    }

    std::optional<T> get(const std::string& key) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = cache_.find(key);
        if (it == cache_.end()) {
            return std::nullopt;
        }
        return it->second.first;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        cache_.clear();
        list_.clear();
    }

private:
    size_t capacity_;
    std::list<std::string> list_;
    std::unordered_map<std::string, std::pair<T, std::list<std::string>::iterator>> cache_;
    mutable std::mutex mutex_;
};

class BlockchainManager {
public:
    using BlockCallback = std::function<void(const std::string&, const BlockInfo&)>;
    using TransactionCallback = std::function<void(const std::string&, const TransactionInfo&)>;
    using MiningCallback = std::function<void(const std::string&, const MiningInfo&)>;
    using ErrorCallback = std::function<void(const std::string&)>;
    using ProgressCallback = std::function<void(const std::string&, uint32_t)>;

    BlockchainManager();
    ~BlockchainManager();

    // Initialization and shutdown
    bool initialize(const BlockchainConfig& config);
    void shutdown();
    std::string getLastError() const;

    // Block management
    std::optional<BlockInfo> getBlockInfo(const std::string& block_hash) const;
    std::optional<BlockInfo> getBlockByHeight(uint32_t height) const;
    std::vector<std::string> getBlockHashes(uint32_t start_height, uint32_t end_height) const;
    bool validateBlock(const std::string& block_hash) const;
    bool broadcastBlock(const std::string& block_hash);

    // Transaction management
    std::optional<TransactionInfo> getTransactionInfo(const std::string& txid) const;
    std::vector<std::string> getBlockTransactions(const std::string& block_hash) const;
    std::vector<std::string> getAddressTransactions(const std::string& address) const;
    bool validateTransaction(const std::string& txid) const;
    bool broadcastTransaction(const std::string& txid);
    std::vector<std::string> getMempoolTransactions() const;

    // Mining operations
    bool startMining(const std::string& address);
    bool stopMining();
    bool isMining() const;
    std::optional<MiningInfo> getMiningInfo() const;
    bool setMiningThreads(uint32_t threads);
    uint32_t getMiningThreads() const;

    // Synchronization
    bool startSync();
    bool stopSync();
    bool isSyncing() const;
    uint32_t getCurrentHeight() const;
    bool syncToHeight(uint32_t height);
    bool verifyChain() const;

    // State management
    uint64_t getBalance(const std::string& address) const;
    uint64_t getTotalSupply() const;
    uint32_t getDifficulty() const;
    std::optional<BlockchainStatistics> getStatistics() const;

    // Callback registration
    void registerBlockCallback(BlockCallback callback);
    void registerTransactionCallback(TransactionCallback callback);
    void registerMiningCallback(MiningCallback callback);
    void registerErrorCallback(ErrorCallback callback);
    void registerProgressCallback(ProgressCallback callback);

    // Performance optimization methods
    void setCacheConfig(const CacheConfig& config);
    void setConnectionPoolConfig(const ConnectionPoolConfig& config);
    void setBatchConfig(const BatchConfig& config);
    void clearCache();
    size_t getCacheSize() const;
    size_t getConnectionPoolSize() const;
    size_t getBatchSize() const;

    // Error recovery methods
    void setErrorRecoveryConfig(const ErrorRecoveryConfig& config);
    bool recoverFromError(const std::string& operation_id);
    std::vector<RecoveryOperation> getRecoveryHistory() const;
    void clearRecoveryHistory();
    bool isRecoveryInProgress() const;
    std::string getLastRecoveryError() const;

private:
    // Helper methods
    bool validateConfig(const BlockchainConfig& config) const;
    bool validateBlockHash(const std::string& block_hash) const;
    bool validateTransactionId(const std::string& txid) const;
    bool validateAddress(const std::string& address) const;
    void updateBlockchainState(const std::string& block_hash);
    bool persistBlock(const std::string& block_hash, const BlockInfo& block);
    bool loadBlock(const std::string& block_hash, BlockInfo& block);
    void notifyBlockUpdate(const std::string& block_hash, const BlockInfo& block);
    void notifyTransactionUpdate(const std::string& txid, const TransactionInfo& tx);
    void notifyMiningUpdate(const std::string& block_hash, const MiningInfo& mining_info);
    void notifyError(const std::string& error);
    void notifyProgress(const std::string& block_hash, uint32_t progress);

    // Member variables
    BlockchainConfig config_;
    std::unordered_map<std::string, BlockInfo> blocks_;
    std::unordered_map<std::string, TransactionInfo> transactions_;
    std::unordered_map<std::string, std::vector<std::string>> address_transactions_;
    std::vector<BlockCallback> blockCallbacks_;
    std::vector<TransactionCallback> transactionCallbacks_;
    std::vector<MiningCallback> miningCallbacks_;
    std::vector<ErrorCallback> errorCallbacks_;
    std::vector<ProgressCallback> progressCallbacks_;
    mutable std::mutex mutex_;
    std::string lastError_;
    bool is_mining_;
    bool is_syncing_;
    uint32_t current_height_;
    uint32_t mining_threads_;
    std::string mining_address_;

    // Performance optimization members
    CacheConfig cache_config_;
    ConnectionPoolConfig pool_config_;
    BatchConfig batch_config_;

    // Caches
    LRUCache<BlockInfo> block_cache_;
    LRUCache<TransactionInfo> tx_cache_;
    LRUCache<uint64_t> balance_cache_;

    // Connection pools
    // boost::object_pool<boost::asio::ip::tcp::socket> socket_pool_;
    // boost::object_pool<boost::asio::ssl::context> ssl_pool_;

    // Batch processing
    std::queue<BlockInfo> block_batch_;
    std::queue<TransactionInfo> tx_batch_;
    std::mutex batch_mutex_;
    std::condition_variable batch_cv_;
    std::thread batch_processor_;

    // Helper methods for performance optimization
    void processBatch();
    void optimizeMemory();
    void cleanupConnections();
    void updateCache();

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
    std::string generateRecoveryId() const;
};

} // namespace blockchain
} // namespace satox 