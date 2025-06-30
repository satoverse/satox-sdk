#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include <functional>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <nlohmann/json.hpp>
#include "satox/transactions/transaction_manager.hpp"

namespace satox::transactions {

using Transaction = TransactionManager::Transaction;

class TransactionBroadcaster {
public:
    // Broadcast result structure
    struct BroadcastResult {
        bool success;
        std::string transactionId;
        std::string error;
    };

    // Broadcast callback type
    using BroadcastCallback = std::function<void(const BroadcastResult&)>;

    // Get singleton instance
    static TransactionBroadcaster& getInstance();

    // Initialize the broadcaster
    bool initialize(const nlohmann::json& config);

    // Shutdown the broadcaster
    void shutdown();

    // Broadcast a transaction
    BroadcastResult broadcastTransaction(const Transaction& transaction);

    // Broadcast multiple transactions
    std::vector<BroadcastResult> broadcastTransactions(const std::vector<Transaction>& transactions);

    // Register broadcast callback
    bool registerCallback(BroadcastCallback callback);

    // Unregister broadcast callback
    bool unregisterCallback(BroadcastCallback callback);

    // Get broadcast queue size
    size_t getQueueSize() const;

    // Get last error message
    std::string getLastError() const;

    // Clear last error
    void clearLastError();

private:
    // Private constructor for singleton
    TransactionBroadcaster() = default;
    ~TransactionBroadcaster();

    // Prevent copying
    TransactionBroadcaster(const TransactionBroadcaster&) = delete;
    TransactionBroadcaster& operator=(const TransactionBroadcaster&) = delete;

    // Broadcast worker thread function
    void broadcastWorker();

    // Validate configuration
    bool validateConfig(const nlohmann::json& config);

    // Member variables
    bool initialized_ = false;
    std::string lastError_;
    nlohmann::json config_;
    std::queue<Transaction> broadcastQueue_;
    std::vector<BroadcastCallback> callbacks_;
    mutable std::mutex mutex_;
    std::condition_variable queueCondition_;
    std::thread workerThread_;
    std::atomic<bool> shouldStop_{false};
};

} // namespace satox::transactions 