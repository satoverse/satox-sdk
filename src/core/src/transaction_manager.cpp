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

#include "satox/core/transaction_manager.hpp"
#include <algorithm>
#include <chrono>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <random>

namespace satox::core {

TransactionManager::TransactionManager() : initialized_(false) {
    spdlog::debug("TransactionManager constructor called");
}

TransactionManager::~TransactionManager() {
    spdlog::debug("TransactionManager destructor called");
    if (initialized_) {
        shutdown();
    }
}

TransactionManager& TransactionManager::getInstance() {
    static TransactionManager instance;
    return instance;
}

bool TransactionManager::initialize(size_t maxTransactions, size_t maxConcurrent) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        lastError_ = "TransactionManager already initialized";
        return false;
    }
    
    maxTransactions_ = maxTransactions;
    maxConcurrent_ = maxConcurrent;
    initialized_ = true;
    
    spdlog::info("TransactionManager initialized with max transactions: {}, max concurrent: {}", 
                 maxTransactions, maxConcurrent);
    return true;
}

bool TransactionManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return false;
    }
    
    // Wait for all active transactions to complete
    while (!activeTransactions_.empty()) {
        spdlog::info("Waiting for {} active transactions to complete", activeTransactions_.size());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    transactions_.clear();
    activeTransactions_.clear();
    completedTransactions_.clear();
    failedTransactions_.clear();
    initialized_ = false;
    
    spdlog::info("TransactionManager shutdown complete");
    return true;
}

std::string TransactionManager::createTransaction(const std::string& type,
                                                const nlohmann::json& data,
                                                Priority priority) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "TransactionManager not initialized";
        return "";
    }
    
    if (transactions_.size() >= maxTransactions_) {
        lastError_ = "Maximum number of transactions reached";
        return "";
    }
    
    std::string id = generateTransactionId();
    Transaction transaction;
    transaction.id = id;
    transaction.type = type;
    transaction.data = data;
    transaction.priority = priority;
    transaction.status = TransactionStatus::PENDING;
    transaction.createdAt = std::chrono::system_clock::now();
    transaction.updatedAt = transaction.createdAt;
    
    transactions_[id] = transaction;
    pendingTransactions_.push_back(id);
    
    // Sort pending transactions by priority
    std::sort(pendingTransactions_.begin(), pendingTransactions_.end(),
              [this](const std::string& a, const std::string& b) {
                  return transactions_[a].priority > transactions_[b].priority;
              });
    
    spdlog::debug("Transaction created: {} (type: {}, priority: {})", 
                  id, type, static_cast<int>(priority));
    return id;
}

bool TransactionManager::startTransaction(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "TransactionManager not initialized";
        return false;
    }
    
    auto it = transactions_.find(id);
    if (it == transactions_.end()) {
        lastError_ = "Transaction not found: " + id;
        return false;
    }
    
    Transaction& transaction = it->second;
    if (transaction.status != TransactionStatus::PENDING) {
        lastError_ = "Transaction is not pending: " + id;
        return false;
    }
    
    if (activeTransactions_.size() >= maxConcurrent_) {
        lastError_ = "Maximum concurrent transactions reached";
        return false;
    }
    
    // Remove from pending
    pendingTransactions_.erase(
        std::remove(pendingTransactions_.begin(), pendingTransactions_.end(), id),
        pendingTransactions_.end());
    
    // Add to active
    transaction.status = TransactionStatus::ACTIVE;
    transaction.startedAt = std::chrono::system_clock::now();
    transaction.updatedAt = transaction.startedAt;
    activeTransactions_.insert(id);
    
    spdlog::debug("Transaction started: {}", id);
    return true;
}

bool TransactionManager::completeTransaction(const std::string& id,
                                           const nlohmann::json& result) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "TransactionManager not initialized";
        return false;
    }
    
    auto it = transactions_.find(id);
    if (it == transactions_.end()) {
        lastError_ = "Transaction not found: " + id;
        return false;
    }
    
    Transaction& transaction = it->second;
    if (transaction.status != TransactionStatus::ACTIVE) {
        lastError_ = "Transaction is not active: " + id;
        return false;
    }
    
    // Remove from active
    activeTransactions_.erase(id);
    
    // Update transaction
    transaction.status = TransactionStatus::COMPLETED;
    transaction.result = result;
    transaction.completedAt = std::chrono::system_clock::now();
    transaction.updatedAt = transaction.completedAt;
    
    // Add to completed
    completedTransactions_.push_back(id);
    
    spdlog::debug("Transaction completed: {}", id);
    return true;
}

bool TransactionManager::failTransaction(const std::string& id,
                                       const std::string& error) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "TransactionManager not initialized";
        return false;
    }
    
    auto it = transactions_.find(id);
    if (it == transactions_.end()) {
        lastError_ = "Transaction not found: " + id;
        return false;
    }
    
    Transaction& transaction = it->second;
    if (transaction.status != TransactionStatus::ACTIVE) {
        lastError_ = "Transaction is not active: " + id;
        return false;
    }
    
    // Remove from active
    activeTransactions_.erase(id);
    
    // Update transaction
    transaction.status = TransactionStatus::FAILED;
    transaction.error = error;
    transaction.failedAt = std::chrono::system_clock::now();
    transaction.updatedAt = transaction.failedAt;
    
    // Add to failed
    failedTransactions_.push_back(id);
    
    spdlog::debug("Transaction failed: {} - {}", id, error);
    return true;
}

bool TransactionManager::cancelTransaction(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "TransactionManager not initialized";
        return false;
    }
    
    auto it = transactions_.find(id);
    if (it == transactions_.end()) {
        lastError_ = "Transaction not found: " + id;
        return false;
    }
    
    Transaction& transaction = it->second;
    if (transaction.status != TransactionStatus::PENDING) {
        lastError_ = "Transaction is not pending: " + id;
        return false;
    }
    
    // Remove from pending
    pendingTransactions_.erase(
        std::remove(pendingTransactions_.begin(), pendingTransactions_.end(), id),
        pendingTransactions_.end());
    
    // Update transaction
    transaction.status = TransactionStatus::CANCELLED;
    transaction.cancelledAt = std::chrono::system_clock::now();
    transaction.updatedAt = transaction.cancelledAt;
    
    spdlog::debug("Transaction cancelled: {}", id);
    return true;
}

std::optional<TransactionManager::Transaction> TransactionManager::getTransaction(const std::string& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = transactions_.find(id);
    if (it == transactions_.end()) {
        return std::nullopt;
    }
    
    return it->second;
}

std::vector<TransactionManager::Transaction> TransactionManager::getTransactions(TransactionStatus status) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<Transaction> result;
    for (const auto& [id, transaction] : transactions_) {
        if (transaction.status == status) {
            result.push_back(transaction);
        }
    }
    
    return result;
}

std::vector<TransactionManager::Transaction> TransactionManager::getTransactions(const std::string& type) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<Transaction> result;
    for (const auto& [id, transaction] : transactions_) {
        if (transaction.type == type) {
            result.push_back(transaction);
        }
    }
    
    return result;
}

std::vector<TransactionManager::Transaction> TransactionManager::getPendingTransactions() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<Transaction> result;
    for (const auto& id : pendingTransactions_) {
        auto it = transactions_.find(id);
        if (it != transactions_.end()) {
            result.push_back(it->second);
        }
    }
    
    return result;
}

std::vector<TransactionManager::Transaction> TransactionManager::getActiveTransactions() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<Transaction> result;
    for (const auto& id : activeTransactions_) {
        auto it = transactions_.find(id);
        if (it != transactions_.end()) {
            result.push_back(it->second);
        }
    }
    
    return result;
}

std::vector<TransactionManager::Transaction> TransactionManager::getCompletedTransactions() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<Transaction> result;
    for (const auto& id : completedTransactions_) {
        auto it = transactions_.find(id);
        if (it != transactions_.end()) {
            result.push_back(it->second);
        }
    }
    
    return result;
}

std::vector<TransactionManager::Transaction> TransactionManager::getFailedTransactions() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<Transaction> result;
    for (const auto& id : failedTransactions_) {
        auto it = transactions_.find(id);
        if (it != transactions_.end()) {
            result.push_back(it->second);
        }
    }
    
    return result;
}

bool TransactionManager::hasTransaction(const std::string& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return transactions_.find(id) != transactions_.end();
}

bool TransactionManager::isTransactionActive(const std::string& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return activeTransactions_.find(id) != activeTransactions_.end();
}

bool TransactionManager::isTransactionPending(const std::string& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return std::find(pendingTransactions_.begin(), pendingTransactions_.end(), id) != pendingTransactions_.end();
}

bool TransactionManager::isTransactionCompleted(const std::string& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return std::find(completedTransactions_.begin(), completedTransactions_.end(), id) != completedTransactions_.end();
}

bool TransactionManager::isTransactionFailed(const std::string& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return std::find(failedTransactions_.begin(), failedTransactions_.end(), id) != failedTransactions_.end();
}

bool TransactionManager::isTransactionCancelled(const std::string& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = transactions_.find(id);
    return it != transactions_.end() && it->second.status == TransactionStatus::CANCELLED;
}

void TransactionManager::clearCompletedTransactions(size_t maxAge) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::system_clock::now();
    auto cutoff = now - std::chrono::hours(maxAge);
    
    // Remove old completed transactions
    completedTransactions_.erase(
        std::remove_if(completedTransactions_.begin(), completedTransactions_.end(),
                      [this, cutoff](const std::string& id) {
                          auto it = transactions_.find(id);
                          if (it != transactions_.end() && it->second.completedAt < cutoff) {
                              transactions_.erase(it);
                              return true;
                          }
                          return false;
                      }),
        completedTransactions_.end());
    
    // Remove old failed transactions
    failedTransactions_.erase(
        std::remove_if(failedTransactions_.begin(), failedTransactions_.end(),
                      [this, cutoff](const std::string& id) {
                          auto it = transactions_.find(id);
                          if (it != transactions_.end() && it->second.failedAt < cutoff) {
                              transactions_.erase(it);
                              return true;
                          }
                          return false;
                      }),
        failedTransactions_.end());
    
    spdlog::debug("Cleared transactions older than {} hours", maxAge);
}

void TransactionManager::clearAllTransactions() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    transactions_.clear();
    pendingTransactions_.clear();
    activeTransactions_.clear();
    completedTransactions_.clear();
    failedTransactions_.clear();
    
    spdlog::info("All transactions cleared");
}

TransactionManager::Stats TransactionManager::getStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    Stats stats;
    stats.totalTransactions = transactions_.size();
    stats.pendingTransactions = pendingTransactions_.size();
    stats.activeTransactions = activeTransactions_.size();
    stats.completedTransactions = completedTransactions_.size();
    stats.failedTransactions = failedTransactions_.size();
    stats.maxTransactions = maxTransactions_;
    stats.maxConcurrent = maxConcurrent_;
    
    return stats;
}

std::string TransactionManager::getLastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastError_;
}

void TransactionManager::clearLastError() {
    std::lock_guard<std::mutex> lock(mutex_);
    lastError_.clear();
}

bool TransactionManager::isHealthy() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return initialized_ && lastError_.empty();
}

std::string TransactionManager::generateTransactionId() {
    static std::atomic<uint64_t> counter{0};
    return "txn_" + std::to_string(++counter);
}

std::string TransactionManager::TransactionStatusToString(TransactionStatus status) {
    switch (status) {
        case TransactionStatus::PENDING: return "pending";
        case TransactionStatus::ACTIVE: return "active";
        case TransactionStatus::COMPLETED: return "completed";
        case TransactionStatus::FAILED: return "failed";
        case TransactionStatus::CANCELLED: return "cancelled";
        default: return "unknown";
    }
}

TransactionManager::TransactionStatus TransactionManager::StringToTransactionStatus(const std::string& str) {
    if (str == "pending") return TransactionStatus::PENDING;
    if (str == "active") return TransactionStatus::ACTIVE;
    if (str == "completed") return TransactionStatus::COMPLETED;
    if (str == "failed") return TransactionStatus::FAILED;
    if (str == "cancelled") return TransactionStatus::CANCELLED;
    return TransactionStatus::FAILED;
}

void to_json(nlohmann::json& j, const TransactionManager::Transaction& t) {
    j = nlohmann::json{
        {"id", t.id},
        {"type", t.type},
        {"data", t.data},
        {"priority", t.priority},
        {"status", TransactionManager::TransactionStatusToString(t.status)},
        {"result", t.result},
        {"error", t.error},
        {"createdAt", t.createdAt.time_since_epoch().count()},
        {"updatedAt", t.updatedAt.time_since_epoch().count()},
        {"startedAt", t.startedAt.time_since_epoch().count()},
        {"completedAt", t.completedAt.time_since_epoch().count()},
        {"failedAt", t.failedAt.time_since_epoch().count()},
        {"cancelledAt", t.cancelledAt.time_since_epoch().count()},
        {"metadata", t.metadata}
    };
}

void from_json(const nlohmann::json& j, TransactionManager::Transaction& t) {
    j.at("id").get_to(t.id);
    j.at("type").get_to(t.type);
    j.at("data").get_to(t.data);
    t.priority = j.at("priority").get<TransactionManager::Priority>();
    t.status = TransactionManager::StringToTransactionStatus(j.at("status").get<std::string>());
    j.at("result").get_to(t.result);
    j.at("error").get_to(t.error);
    t.createdAt = std::chrono::system_clock::time_point(std::chrono::system_clock::duration(j.at("createdAt").get<int64_t>()));
    t.updatedAt = std::chrono::system_clock::time_point(std::chrono::system_clock::duration(j.at("updatedAt").get<int64_t>()));
    t.startedAt = std::chrono::system_clock::time_point(std::chrono::system_clock::duration(j.at("startedAt").get<int64_t>()));
    t.completedAt = std::chrono::system_clock::time_point(std::chrono::system_clock::duration(j.at("completedAt").get<int64_t>()));
    t.failedAt = std::chrono::system_clock::time_point(std::chrono::system_clock::duration(j.at("failedAt").get<int64_t>()));
    t.cancelledAt = std::chrono::system_clock::time_point(std::chrono::system_clock::duration(j.at("cancelledAt").get<int64_t>()));
    j.at("metadata").get_to(t.metadata);
}

} // namespace satox::core 