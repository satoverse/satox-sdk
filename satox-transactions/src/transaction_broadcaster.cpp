#include "satox/transactions/transaction_broadcaster.hpp"
#include <chrono>
#include <algorithm>

namespace satox::transactions {

TransactionBroadcaster& TransactionBroadcaster::getInstance() {
    static TransactionBroadcaster instance;
    return instance;
}

bool TransactionBroadcaster::initialize(const nlohmann::json& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        lastError_ = "Broadcaster already initialized";
        return false;
    }

    if (!validateConfig(config)) {
        return false;
    }

    config_ = config;
    initialized_ = true;
    shouldStop_ = false;

    // Start worker thread
    workerThread_ = std::thread(&TransactionBroadcaster::broadcastWorker, this);

    return true;
}

void TransactionBroadcaster::shutdown() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!initialized_) {
            return;
        }
        shouldStop_ = true;
    }

    queueCondition_.notify_one();

    if (workerThread_.joinable()) {
        workerThread_.join();
    }

    std::lock_guard<std::mutex> lock(mutex_);
    initialized_ = false;
    callbacks_.clear();
    while (!broadcastQueue_.empty()) {
        broadcastQueue_.pop();
    }
}

TransactionBroadcaster::~TransactionBroadcaster() {
    shutdown();
}

TransactionBroadcaster::BroadcastResult TransactionBroadcaster::broadcastTransaction(
    const Transaction& transaction) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return {false, transaction.id, "Broadcaster not initialized"};
    }

    broadcastQueue_.push(transaction);
    queueCondition_.notify_one();

    return {true, transaction.id, ""};
}

std::vector<TransactionBroadcaster::BroadcastResult> 
TransactionBroadcaster::broadcastTransactions(
    const std::vector<Transaction>& transactions) {
    std::vector<BroadcastResult> results;
    results.reserve(transactions.size());

    for (const auto& transaction : transactions) {
        results.push_back(broadcastTransaction(transaction));
    }

    return results;
}

bool TransactionBroadcaster::registerCallback(BroadcastCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Broadcaster not initialized";
        return false;
    }

    callbacks_.push_back(callback);
    return true;
}

bool TransactionBroadcaster::unregisterCallback(BroadcastCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Broadcaster not initialized";
        return false;
    }

    // Note: std::function objects cannot be compared directly
    // In a real implementation, you would need to store callback IDs or use a different approach
    // For now, we'll just return false as this is a limitation of the current design
    lastError_ = "Callback unregistration not supported in current implementation";
    return false;
}

size_t TransactionBroadcaster::getQueueSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return broadcastQueue_.size();
}

std::string TransactionBroadcaster::getLastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastError_;
}

void TransactionBroadcaster::clearLastError() {
    std::lock_guard<std::mutex> lock(mutex_);
    lastError_.clear();
}

void TransactionBroadcaster::broadcastWorker() {
    while (true) {
        Transaction transaction;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            queueCondition_.wait(lock, [this] {
                return !broadcastQueue_.empty() || shouldStop_;
            });

            if (shouldStop_) {
                break;
            }

            transaction = broadcastQueue_.front();
            broadcastQueue_.pop();
        }

        // Simulate network broadcast
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        BroadcastResult result;
        result.success = true;
        result.transactionId = transaction.id;

        // Notify callbacks
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& callback : callbacks_) {
            callback(result);
        }
    }
}

bool TransactionBroadcaster::validateConfig(const nlohmann::json& config) {
    if (!config.is_object()) {
        lastError_ = "Invalid configuration format";
        return false;
    }

    // Add configuration validation as needed
    return true;
}

} // namespace satox::transactions 