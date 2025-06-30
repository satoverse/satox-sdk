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

#include "satox/core/blockchain_manager.hpp"
#include "satox/core/logging_manager.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <chrono>
#include <thread>

namespace satox::core {

BlockchainManager::BlockchainManager() : initialized_(false) {
    // Logging is not safe here; do not log in constructor
}

BlockchainManager::~BlockchainManager() {
    // Do not log in destructor; logger may be destroyed
    if (initialized_) {
        shutdown();
    }
}

BlockchainManager& BlockchainManager::getInstance() {
    static BlockchainManager instance;
    return instance;
}

bool BlockchainManager::initialize(const nlohmann::json& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        lastError_ = "Blockchain Manager already initialized";
        return false;
    }

    if (!validateConfig(config)) {
        lastError_ = "Invalid configuration";
        return false;
    }

    // Initialize logger
    logger_ = std::make_unique<LoggingManager>();
    LoggingManager::LogConfig logConfig;
    logConfig.logDir = "logs/components/blockchain";
    logConfig.logFile = "blockchain.log";
    logConfig.maxFileSize = 10 * 1024 * 1024;
    logConfig.maxFiles = 5;
    logConfig.minLevel = LoggingManager::Level::DEBUG;
    logConfig.consoleOutput = true;
    logConfig.fileOutput = true;
    logConfig.asyncLogging = true;
    logConfig.queueSize = 1000;
    logConfig.flushInterval = std::chrono::milliseconds(100);
    logConfig.includeTimestamp = true;
    logConfig.includeThreadId = true;
    logConfig.includeSource = true;
    logConfig.logFormat = "[{timestamp}] [{level}] [{thread}] [{source}] {message} {metadata}";
    std::filesystem::create_directories(logConfig.logDir);
    if (!logger_->initialize(logConfig)) {
        lastError_ = "Failed to initialize BlockchainManager logger";
        logger_.reset();
        return false;
    }
    if (logger_) logger_->debug("BlockchainManager initialized");

    initialized_ = true;
    statsEnabled_ = config.value("enableStats", false);
    return true;
}

bool BlockchainManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return true;
    }

    state_ = BlockchainState::DISCONNECTED;
    blocks_.clear();
    transactions_.clear();
    blockCallbacks_.clear();
    transactionCallbacks_.clear();
    stateCallbacks_.clear();
    errorCallbacks_.clear();
    initialized_ = false;
    lastError_.clear();
    if (logger_) {
        logger_->info("BlockchainManager shutdown");
        logger_->shutdown();
        logger_.reset();
    }
    return true;
}

bool BlockchainManager::connect() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Blockchain Manager not initialized";
        return false;
    }

    if (state_ == BlockchainManager::BlockchainState::CONNECTED) {
        lastError_ = "Already connected";
        return false;
    }

    updateState(BlockchainManager::BlockchainState::CONNECTING);
    // Implement actual connection logic here
    updateState(BlockchainManager::BlockchainState::CONNECTED);
    return true;
}

bool BlockchainManager::disconnect() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Blockchain Manager not initialized";
        return false;
    }

    if (state_ == BlockchainManager::BlockchainState::DISCONNECTED) {
        return true;
    }

    // Implement actual disconnection logic here
    updateState(BlockchainManager::BlockchainState::DISCONNECTED);
    return true;
}

bool BlockchainManager::isConnected() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_ == BlockchainManager::BlockchainState::CONNECTED || state_ == BlockchainManager::BlockchainState::SYNCED;
}

BlockchainManager::BlockchainState BlockchainManager::getState() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_;
}

// bool BlockchainManager::getBlock(const std::string& blockHash, BlockchainManager::Block& block) const {
//     std::lock_guard<std::mutex> lock(mutex_);
//     
//     auto it = blocks_.find(blockHash);
//     if (it == blocks_.end()) {
//         return false;
//     }
// 
//     block = it->second;
//     return true;
// }

bool BlockchainManager::getBlockByHeight(uint64_t height, BlockchainManager::Block& block) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& [hash, b] : blocks_) {
        if (b.height == height) {
            block = b;
            return true;
        }
    }
    return false;
}

// bool BlockchainManager::getLatestBlock(BlockchainManager::Block& block) const {
//     std::lock_guard<std::mutex> lock(mutex_);
//     
//     if (blocks_.empty()) {
//         return false;
//     }
// 
//     uint64_t maxHeight = 0;
//     for (const auto& [hash, b] : blocks_) {
//         if (b.height > maxHeight) {
//             maxHeight = b.height;
//             block = b;
//         }
//     }
//     return true;
// }

std::vector<BlockchainManager::Block> BlockchainManager::getBlocks(uint64_t startHeight, uint64_t endHeight) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<BlockchainManager::Block> result;
    
    for (const auto& [hash, block] : blocks_) {
        if (block.height >= startHeight && block.height <= endHeight) {
            result.push_back(block);
        }
    }
    
    std::sort(result.begin(), result.end(),
        [](const BlockchainManager::Block& a, const BlockchainManager::Block& b) { return a.height < b.height; });
    
    return result;
}

// bool BlockchainManager::getTransaction(const std::string& txHash, BlockchainManager::Transaction& transaction) const {
//     std::lock_guard<std::mutex> lock(mutex_);
//     
//     auto it = transactions_.find(txHash);
//     if (it == transactions_.end()) {
//         return false;
//     }
// 
//     transaction = it->second;
//     return true;
// }

// bool BlockchainManager::broadcastTransaction(const BlockchainManager::Transaction& transaction) {
//     std::lock_guard<std::mutex> lock(mutex_);
//     
//     if (!initialized_) {
//         lastError_ = "Blockchain Manager not initialized";
//         return false;
//     }
// 
//     if (!validateTransaction(transaction)) {
//         lastError_ = "Invalid transaction";
//         return false;
//     }
// 
//     // Implement actual transaction broadcasting logic here
//     transactions_[transaction.hash] = transaction;
//     updateStats(transaction);
//     notifyTransactionChange(transaction);
//     return true;
// }

// bool BlockchainManager::validateTransaction(const BlockchainManager::Transaction& transaction) const {
//     if (transaction.hash.empty() || transaction.inputs.empty() || transaction.outputs.empty()) {
//         return false;
//     }
//     return true;
// }

// std::vector<BlockchainManager::Transaction> BlockchainManager::getTransactionsByBlock(const std::string& blockHash) const {
//     std::lock_guard<std::mutex> lock(mutex_);
//     std::vector<BlockchainManager::Transaction> result;
//     
//     auto blockIt = blocks_.find(blockHash);
//     if (blockIt == blocks_.end()) {
//         return result;
//     }
// 
//     for (const auto& txHash : blockIt->second.transactions) {
//         auto txIt = transactions_.find(txHash);
//         if (txIt != transactions_.end()) {
//             result.push_back(txIt->second);
//         }
//     }
//     
//     return result;
// }

BlockchainManager::BlockchainInfo BlockchainManager::getInfo() const {
    std::lock_guard<std::mutex> lock(mutex_);
    BlockchainManager::BlockchainInfo info;
    
    info.network = "mainnet";
    
    // Calculate current height directly instead of calling getCurrentHeight()
    uint64_t maxHeight = 0;
    for (const auto& [hash, block] : blocks_) {
        if (block.height > maxHeight) {
            maxHeight = block.height;
        }
    }
    info.height = maxHeight;
    
    // Calculate best block hash directly instead of calling getBestBlockHash()
    uint64_t bestHeight = 0;
    std::string bestHash;
    for (const auto& [hash, block] : blocks_) {
        if (block.height > bestHeight) {
            bestHeight = block.height;
            bestHash = hash;
        }
    }
    info.bestBlockHash = bestHash;
    
    // Get difficulty and network hash rate directly
    info.difficulty = getDifficultyInternal();
    info.networkHashrate = getNetworkHashRateInternal();
    info.mempoolSize = 0; // Implement actual mempool size
    info.mempoolTransactions = 0; // Implement actual mempool transaction count
    info.lastBlockTime = std::chrono::system_clock::now(); // Implement actual last block time
    info.additionalInfo = nlohmann::json::object();
    
    return info;
}

uint64_t BlockchainManager::getCurrentHeight() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return getCurrentHeightInternal();
}

uint64_t BlockchainManager::getCurrentHeightInternal() const {
    uint64_t maxHeight = 0;
    
    for (const auto& [hash, block] : blocks_) {
        if (block.height > maxHeight) {
            maxHeight = block.height;
        }
    }
    
    return maxHeight;
}

std::string BlockchainManager::getBestBlockHash() const {
    std::lock_guard<std::mutex> lock(mutex_);
    uint64_t maxHeight = 0;
    std::string bestHash;
    
    for (const auto& [hash, block] : blocks_) {
        if (block.height > maxHeight) {
            maxHeight = block.height;
            bestHash = hash;
        }
    }
    
    return bestHash;
}

uint64_t BlockchainManager::getDifficulty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (blocks_.empty()) {
        return 0;
    }
    
    // Calculate difficulty based on the last 2016 blocks (similar to Bitcoin)
    const uint64_t difficultyAdjustmentInterval = 2016;
    const uint64_t targetTimeSpan = 14 * 24 * 60 * 60; // 14 days in seconds
    
    uint64_t currentHeight = getCurrentHeight();
    if (currentHeight < difficultyAdjustmentInterval) {
        return 1; // Default difficulty for early blocks
    }
    
    // Get the block from 2016 blocks ago
    uint64_t oldHeight = currentHeight - difficultyAdjustmentInterval;
    auto oldBlockIt = std::find_if(blocks_.begin(), blocks_.end(),
        [oldHeight](const auto& pair) { return pair.second.height == oldHeight; });
    
    if (oldBlockIt == blocks_.end()) {
        return 1; // Fallback difficulty
    }
    
    // Get current block
    auto currentBlockIt = std::find_if(blocks_.begin(), blocks_.end(),
        [currentHeight](const auto& pair) { return pair.second.height == currentHeight; });
    
    if (currentBlockIt == blocks_.end()) {
        return 1; // Fallback difficulty
    }
    
    // Calculate time span
    auto timeSpan = currentBlockIt->second.timestamp - oldBlockIt->second.timestamp;
    if (timeSpan == 0) {
        timeSpan = 1; // Avoid division by zero
    }
    
    // Calculate difficulty adjustment based on bits field
    // The bits field encodes the target difficulty
    uint32_t oldBits = oldBlockIt->second.bits;
    if (oldBits == 0) {
        oldBits = 0x1d00ffff; // Default difficulty target
    }
    
    // Convert bits to target
    uint64_t oldTarget = (oldBits & 0xffffff) << (8 * ((oldBits >> 24) - 3));
    
    // Calculate new target based on time span
    uint64_t newTarget = (oldTarget * targetTimeSpan) / timeSpan;
    
    // Limit target adjustment to 4x increase or 1/4 decrease
    if (newTarget > oldTarget * 4) {
        newTarget = oldTarget * 4;
    } else if (newTarget < oldTarget / 4) {
        newTarget = oldTarget / 4;
    }
    
    // Convert target back to difficulty (difficulty = target_max / target)
    const uint64_t targetMax = 0x00000000ffff0000ULL;
    uint64_t difficulty = targetMax / newTarget;
    
    return difficulty;
}

uint64_t BlockchainManager::getNetworkHashRate() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    uint64_t difficulty = getDifficulty();
    if (difficulty == 0) {
        return 0;
    }
    
    // Calculate network hash rate based on difficulty
    // For KAWPOW algorithm (used by Satoxcoin)
    const uint64_t targetTime = 60; // 60 seconds target block time
    const uint64_t hashRate = (difficulty * (1ULL << 32)) / targetTime;
    
    return hashRate;
}

BlockchainManager::Stats BlockchainManager::getStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

void BlockchainManager::resetStats() {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_ = BlockchainManager::Stats{};
}

bool BlockchainManager::enableStats(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    statsEnabled_ = enable;
    return true;
}

void BlockchainManager::registerBlockCallback(BlockCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    blockCallbacks_["all"].push_back(callback);
}

void BlockchainManager::registerTransactionCallback(TransactionCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    transactionCallbacks_["all"].push_back(callback);
}

void BlockchainManager::registerStateCallback(StateCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    stateCallbacks_.push_back(callback);
}

void BlockchainManager::registerErrorCallback(ErrorCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    errorCallbacks_.push_back(callback);
}

void BlockchainManager::unregisterBlockCallback() {
    std::lock_guard<std::mutex> lock(mutex_);
    blockCallbacks_.clear();
}

void BlockchainManager::unregisterTransactionCallback() {
    std::lock_guard<std::mutex> lock(mutex_);
    transactionCallbacks_.clear();
}

void BlockchainManager::unregisterStateCallback() {
    std::lock_guard<std::mutex> lock(mutex_);
    stateCallbacks_.clear();
}

void BlockchainManager::unregisterErrorCallback() {
    std::lock_guard<std::mutex> lock(mutex_);
    errorCallbacks_.clear();
}

std::string BlockchainManager::getLastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastError_;
}

void BlockchainManager::clearLastError() {
    std::lock_guard<std::mutex> lock(mutex_);
    lastError_.clear();
}

// Private helper methods
bool BlockchainManager::validateConfig(const nlohmann::json& config) const {
    return true; // Add validation logic as needed
}

bool BlockchainManager::validateBlock(const BlockchainManager::Block& block) const {
    if (block.hash.empty() || block.previousHash.empty() || block.merkleRoot.empty()) {
        return false;
    }
    return true;
}

void BlockchainManager::updateState(BlockchainManager::BlockchainState state) {
    state_ = state;
    notifyStateChange(state);
}

void BlockchainManager::updateStats(const BlockchainManager::Block& block) {
    if (!statsEnabled_) {
        return;
    }

    stats_.totalBlocks++;
    stats_.totalTransactions += block.transactions.size();
}

void BlockchainManager::updateStats(const BlockchainManager::Transaction& transaction) {
    if (!statsEnabled_) {
        return;
    }

    stats_.totalTransactions++;
}

void BlockchainManager::notifyBlockChange(const BlockchainManager::Block& block) {
    for (const auto& callback : blockCallbacks_["all"]) {
        try {
            callback(block);
        } catch (const std::exception& e) {
            notifyError(e.what());
        }
    }
}

void BlockchainManager::notifyTransactionChange(const BlockchainManager::Transaction& transaction) {
    for (const auto& callback : transactionCallbacks_["all"]) {
        try {
            callback(transaction);
        } catch (const std::exception& e) {
            notifyError(e.what());
        }
    }
}

void BlockchainManager::notifyStateChange(BlockchainManager::BlockchainState state) {
    for (const auto& callback : stateCallbacks_) {
        try {
            callback(state);
        } catch (const std::exception& e) {
            notifyError(e.what());
        }
    }
}

void BlockchainManager::notifyError(const std::string& error) {
    for (const auto& callback : errorCallbacks_) {
        try {
            callback(error);
        } catch (...) {
            // Ignore errors in error callbacks
        }
    }
}

uint64_t BlockchainManager::getDifficultyInternal() const {
    if (blocks_.empty()) {
        return 0;
    }
    
    // Calculate difficulty based on the last 2016 blocks (similar to Bitcoin)
    const uint64_t difficultyAdjustmentInterval = 2016;
    const uint64_t targetTimeSpan = 14 * 24 * 60 * 60; // 14 days in seconds
    
    // Calculate current height directly without taking a lock
    uint64_t currentHeight = 0;
    for (const auto& [hash, block] : blocks_) {
        if (block.height > currentHeight) {
            currentHeight = block.height;
        }
    }
    
    if (currentHeight < difficultyAdjustmentInterval) {
        return 1; // Default difficulty for early blocks
    }
    
    // Get the block from 2016 blocks ago
    uint64_t oldHeight = currentHeight - difficultyAdjustmentInterval;
    auto oldBlockIt = std::find_if(blocks_.begin(), blocks_.end(),
        [oldHeight](const auto& pair) { return pair.second.height == oldHeight; });
    
    if (oldBlockIt == blocks_.end()) {
        return 1; // Fallback difficulty
    }
    
    // Get current block
    auto currentBlockIt = std::find_if(blocks_.begin(), blocks_.end(),
        [currentHeight](const auto& pair) { return pair.second.height == currentHeight; });
    
    if (currentBlockIt == blocks_.end()) {
        return 1; // Fallback difficulty
    }
    
    // Calculate time span
    auto timeSpan = currentBlockIt->second.timestamp - oldBlockIt->second.timestamp;
    if (timeSpan == 0) {
        timeSpan = 1; // Avoid division by zero
    }
    
    // Calculate difficulty adjustment based on bits field
    // The bits field encodes the target difficulty
    uint32_t oldBits = oldBlockIt->second.bits;
    if (oldBits == 0) {
        oldBits = 0x1d00ffff; // Default difficulty target
    }
    
    // Convert bits to target
    uint64_t oldTarget = (oldBits & 0xffffff) << (8 * ((oldBits >> 24) - 3));
    
    // Calculate new target based on time span
    uint64_t newTarget = (oldTarget * targetTimeSpan) / timeSpan;
    
    // Limit target adjustment to 4x increase or 1/4 decrease
    if (newTarget > oldTarget * 4) {
        newTarget = oldTarget * 4;
    } else if (newTarget < oldTarget / 4) {
        newTarget = oldTarget / 4;
    }
    
    // Convert target back to difficulty (difficulty = target_max / target)
    const uint64_t targetMax = 0x00000000ffff0000ULL;
    uint64_t difficulty = targetMax / newTarget;
    
    return difficulty;
}

uint64_t BlockchainManager::getNetworkHashRateInternal() const {
    uint64_t difficulty = getDifficultyInternal();
    if (difficulty == 0) {
        return 0;
    }
    
    // Calculate network hash rate based on difficulty
    // For KAWPOW algorithm (used by Satoxcoin)
    const uint64_t targetTime = 60; // 60 seconds target block time
    const uint64_t hashRate = (difficulty * (1ULL << 32)) / targetTime;
    
    return hashRate;
}

} // namespace satox::core
 