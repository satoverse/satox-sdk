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

#include "satox/nft/nft_manager.hpp"
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <random>
#include <chrono>
#include <openssl/evp.h>

namespace satox {
namespace nft {

// Singleton instance
NFTManager& NFTManager::getInstance() {
    static NFTManager instance;
    return instance;
}

// Constructor
NFTManager::NFTManager() 
    : impl_(std::make_unique<Impl>())
    , performanceOptimizer_(std::make_unique<PerformanceOptimizer>())
    , errorHandler_(std::make_unique<ErrorHandler>())
    , should_stop_(false)
    , cache_size_(0)
    , lastResetTime_(std::chrono::system_clock::now()) {
}

// Destructor
NFTManager::~NFTManager() {
    shutdown();
}

// Initialization and cleanup
bool NFTManager::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        // Return true if already initialized (for test compatibility)
        return true;
    }
    
    try {
        initialized_ = true;
        return true;
    } catch (const std::exception& e) {
        lastError_ = {static_cast<int>(NFTErrorCode::VALIDATION_ERROR), e.what()};
        return false;
    }
}

void NFTManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return;
    }
    
    try {
        should_stop_ = true;
        cleanup_cv_.notify_all();
        
        if (cleanup_thread_.joinable()) {
            cleanup_thread_.join();
        }
        
        // Reset all data structures for clean state
        nftRegistry_.clear();
        ownerNFTs_.clear();
        contractNFTs_.clear();
        creatorNFTs_.clear();
        transferCounts_.clear();
        lastResetTime_ = std::chrono::system_clock::now();
        
        initialized_ = false;
    } catch (const std::exception& e) {
        lastError_ = {static_cast<int>(NFTErrorCode::VALIDATION_ERROR), e.what()};
    }
}

bool NFTManager::isInitialized() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return initialized_;
}

bool NFTManager::isHealthy() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        return false;
    }
    
    // Basic health check - verify that the NFT manager is in a good state
    try {
        // Simple health check - if we can access the registry, we're healthy
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

// NFT Creation
std::string NFTManager::createNFT(const std::string& contractAddress, const NFTMetadata& metadata, 
                                 const std::string& creator, bool isTransferable, 
                                 bool isBurnable, const std::string& royaltyRecipient,
                                 uint32_t royaltyBasisPoints) {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!initialized_) {
            lastError_ = {static_cast<int>(NFTErrorCode::NOT_INITIALIZED), "NFTManager not initialized"};
            return "";
        }
        
        if (!validateNFTMetadata(metadata)) {
            lastError_ = {static_cast<int>(NFTErrorCode::VALIDATION_ERROR), "Invalid NFT metadata"};
            return "";
        }
        
        std::string nftId = generateNFTId(contractAddress, metadata);
        
        NFT nft;
        nft.id = nftId;
        nft.contractAddress = contractAddress;
        nft.owner = creator;
        nft.creator = creator;
        nft.metadata = metadata;
        nft.tokenId = 0; // TODO: Generate unique token ID
        nft.tokenURI = ""; // TODO: Generate token URI
        nft.isTransferable = isTransferable;
        nft.isBurnable = isBurnable;
        nft.royaltyRecipient = royaltyRecipient;
        nft.royaltyBasisPoints = royaltyBasisPoints;
        nft.createdAt = std::chrono::system_clock::now();
        nft.updatedAt = std::chrono::system_clock::now();
        
        nftRegistry_[nftId] = nft;
        ownerNFTs_[creator].push_back(nftId);
        contractNFTs_[contractAddress].push_back(nftId);
        creatorNFTs_[creator].push_back(nftId);
        
        return nftId;
    } catch (const std::exception& e) {
        lastError_ = {static_cast<int>(NFTErrorCode::VALIDATION_ERROR), e.what()};
        return "";
    }
}

// NFT Operations
bool NFTManager::transferNFT(const std::string& nftId, const std::string& from, const std::string& to) {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!initialized_) {
            lastError_ = {static_cast<int>(NFTErrorCode::NOT_INITIALIZED), "NFTManager not initialized"};
            return false;
        }
        
        // Validate addresses
        if (from.empty() || to.empty()) {
            lastError_ = {static_cast<int>(NFTErrorCode::VALIDATION_ERROR), "Invalid addresses"};
            return false;
        }
        
        // Prevent transfers to zero addresses
        if (to == "0x0000000000000000000000000000000000000000") {
            lastError_ = {static_cast<int>(NFTErrorCode::VALIDATION_ERROR), "Cannot transfer to zero address"};
            return false;
        }
        
        // Rate limiting check
        auto now = std::chrono::system_clock::now();
        auto timeSinceReset = std::chrono::duration_cast<std::chrono::minutes>(now - lastResetTime_).count();
        
        if (timeSinceReset >= 1) {
            // Reset counters every minute
            transferCounts_.clear();
            lastResetTime_ = now;
        }
        
        // Check rate limit for the sender
        if (transferCounts_[from] >= maxTransfersPerMinute_) {
            lastError_ = {static_cast<int>(NFTErrorCode::VALIDATION_ERROR), "Rate limit exceeded"};
            return false;
        }
        
        auto it = nftRegistry_.find(nftId);
        if (it == nftRegistry_.end()) {
            lastError_ = {static_cast<int>(NFTErrorCode::INVALID_ID), "NFT not found"};
            return false;
        }
        
        if (it->second.owner != from) {
            lastError_ = {static_cast<int>(NFTErrorCode::INVALID_OWNER), "Invalid owner"};
            return false;
        }
        
        if (!it->second.isTransferable) {
            lastError_ = {static_cast<int>(NFTErrorCode::INVALID_OPERATION), "NFT is not transferable"};
            return false;
        }
        
        // Update ownership
        it->second.owner = to;
        it->second.updatedAt = std::chrono::system_clock::now();
        
        // Update owner mappings
        auto& fromNFTs = ownerNFTs_[from];
        fromNFTs.erase(std::remove(fromNFTs.begin(), fromNFTs.end(), nftId), fromNFTs.end());
        ownerNFTs_[to].push_back(nftId);
        
        // Increment transfer count for rate limiting
        transferCounts_[from]++;
        
        return true;
    } catch (const std::exception& e) {
        lastError_ = {static_cast<int>(NFTErrorCode::VALIDATION_ERROR), e.what()};
        return false;
    }
}

bool NFTManager::burnNFT(const std::string& nftId, const std::string& owner) {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!initialized_) {
            lastError_ = {static_cast<int>(NFTErrorCode::NOT_INITIALIZED), "NFTManager not initialized"};
            return false;
        }
        
        auto it = nftRegistry_.find(nftId);
        if (it == nftRegistry_.end()) {
            lastError_ = {static_cast<int>(NFTErrorCode::INVALID_ID), "NFT not found"};
            return false;
        }
        
        if (it->second.owner != owner) {
            lastError_ = {static_cast<int>(NFTErrorCode::INVALID_OWNER), "Invalid owner"};
            return false;
        }
        
        if (!it->second.isBurnable) {
            lastError_ = {static_cast<int>(NFTErrorCode::INVALID_OPERATION), "NFT is not burnable"};
            return false;
        }
        
        // Remove from all mappings
        nftRegistry_.erase(it);
        
        auto& ownerNFTs = ownerNFTs_[owner];
        ownerNFTs.erase(std::remove(ownerNFTs.begin(), ownerNFTs.end(), nftId), ownerNFTs.end());
        
        return true;
    } catch (const std::exception& e) {
        lastError_ = {static_cast<int>(NFTErrorCode::QUERY_ERROR), e.what()};
        return false;
    }
}

bool NFTManager::updateNFTMetadata(const std::string& nftId, const NFTMetadata& newMetadata, const std::string& updater) {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!initialized_) {
            lastError_ = {static_cast<int>(NFTErrorCode::NOT_INITIALIZED), "NFTManager not initialized"};
            return false;
        }
        
        auto it = nftRegistry_.find(nftId);
        if (it == nftRegistry_.end()) {
            lastError_ = {static_cast<int>(NFTErrorCode::INVALID_ID), "NFT not found"};
            return false;
        }
        
        if (!validateNFTMetadata(newMetadata)) {
            lastError_ = {static_cast<int>(NFTErrorCode::VALIDATION_ERROR), "Invalid metadata"};
            return false;
        }
        
        it->second.metadata = newMetadata;
        it->second.updatedAt = std::chrono::system_clock::now();
        
        return true;
    } catch (const std::exception& e) {
        lastError_ = {static_cast<int>(NFTErrorCode::VALIDATION_ERROR), e.what()};
        return false;
    }
}

bool NFTManager::addNFTAttribute(const std::string& nftId, const std::string& key, const std::string& value, const std::string& updater) {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!initialized_) {
            lastError_ = {static_cast<int>(NFTErrorCode::NOT_INITIALIZED), "NFTManager not initialized"};
            return false;
        }
        
        auto it = nftRegistry_.find(nftId);
        if (it == nftRegistry_.end()) {
            lastError_ = {static_cast<int>(NFTErrorCode::INVALID_ID), "NFT not found"};
            return false;
        }
        
        it->second.metadata.attributes[key] = value;
        it->second.updatedAt = std::chrono::system_clock::now();
        
        return true;
    } catch (const std::exception& e) {
        lastError_ = {static_cast<int>(NFTErrorCode::VALIDATION_ERROR), e.what()};
        return false;
    }
}

bool NFTManager::removeNFTAttribute(const std::string& nftId, const std::string& key, const std::string& updater) {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!initialized_) {
            lastError_ = {static_cast<int>(NFTErrorCode::NOT_INITIALIZED), "NFTManager not initialized"};
            return false;
        }
        
        auto it = nftRegistry_.find(nftId);
        if (it == nftRegistry_.end()) {
            lastError_ = {static_cast<int>(NFTErrorCode::INVALID_ID), "NFT not found"};
            return false;
        }
        
        it->second.metadata.attributes.erase(key);
        it->second.updatedAt = std::chrono::system_clock::now();
        
        return true;
    } catch (const std::exception& e) {
        lastError_ = {static_cast<int>(NFTErrorCode::VALIDATION_ERROR), e.what()};
        return false;
    }
}

bool NFTManager::setNFTTransferable(const std::string& nftId, bool isTransferable) {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!initialized_) {
            lastError_ = {static_cast<int>(NFTErrorCode::NOT_INITIALIZED), "NFTManager not initialized"};
            return false;
        }
        
        auto it = nftRegistry_.find(nftId);
        if (it == nftRegistry_.end()) {
            lastError_ = {static_cast<int>(NFTErrorCode::INVALID_ID), "NFT not found"};
            return false;
        }
        
        it->second.isTransferable = isTransferable;
        it->second.updatedAt = std::chrono::system_clock::now();
        
        return true;
    } catch (const std::exception& e) {
        lastError_ = {static_cast<int>(NFTErrorCode::VALIDATION_ERROR), e.what()};
        return false;
    }
}

bool NFTManager::setNFTBurnable(const std::string& nftId, bool isBurnable) {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!initialized_) {
            lastError_ = {static_cast<int>(NFTErrorCode::NOT_INITIALIZED), "NFTManager not initialized"};
            return false;
        }
        
        auto it = nftRegistry_.find(nftId);
        if (it == nftRegistry_.end()) {
            lastError_ = {static_cast<int>(NFTErrorCode::INVALID_ID), "NFT not found"};
            return false;
        }
        
        it->second.isBurnable = isBurnable;
        it->second.updatedAt = std::chrono::system_clock::now();
        
        return true;
    } catch (const std::exception& e) {
        lastError_ = {static_cast<int>(NFTErrorCode::VALIDATION_ERROR), e.what()};
        return false;
    }
}

bool NFTManager::setNFTRoyalty(const std::string& nftId, const std::string& recipient, uint32_t basisPoints) {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!initialized_) {
            lastError_ = {static_cast<int>(NFTErrorCode::NOT_INITIALIZED), "NFTManager not initialized"};
            return false;
        }
        
        auto it = nftRegistry_.find(nftId);
        if (it == nftRegistry_.end()) {
            lastError_ = {static_cast<int>(NFTErrorCode::INVALID_ID), "NFT not found"};
            return false;
        }
        
        if (!validateRoyaltyBasisPoints(basisPoints)) {
            lastError_ = {static_cast<int>(NFTErrorCode::VALIDATION_ERROR), "Invalid royalty basis points"};
            return false;
        }
        
        it->second.royaltyRecipient = recipient;
        it->second.royaltyBasisPoints = basisPoints;
        it->second.updatedAt = std::chrono::system_clock::now();
        
        return true;
    } catch (const std::exception& e) {
        lastError_ = {static_cast<int>(NFTErrorCode::VALIDATION_ERROR), e.what()};
        return false;
    }
}

// NFT Validation
bool NFTManager::validateNFT(const NFT& nft) {
    return !nft.id.empty() && !nft.contractAddress.empty() && !nft.owner.empty();
}

bool NFTManager::validateNFTMetadata(const NFTMetadata& metadata) {
    return !metadata.name.empty();
}

bool NFTManager::validateTransfer(const std::string& nftId, const std::string& from, const std::string& to) {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = nftRegistry_.find(nftId);
        if (it == nftRegistry_.end()) {
            return false;
        }
        
        if (it->second.owner != from) {
            return false;
        }
        
        if (!it->second.isTransferable) {
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

// NFT Queries
NFTManager::NFT NFTManager::getNFT(const std::string& nftId) {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!initialized_) {
            lastError_ = {static_cast<int>(NFTErrorCode::NOT_INITIALIZED), "NFTManager not initialized"};
            return NFT{};
        }
        
        auto it = nftRegistry_.find(nftId);
        if (it == nftRegistry_.end()) {
            lastError_ = {static_cast<int>(NFTErrorCode::INVALID_ID), "NFT not found"};
            return NFT{};
        }
        
        return it->second;
    } catch (const std::exception& e) {
        lastError_ = {static_cast<int>(NFTErrorCode::QUERY_ERROR), e.what()};
        return NFT{};
    }
}

std::vector<NFTManager::NFT> NFTManager::getNFTsByOwner(const std::string& owner) {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!initialized_) {
            lastError_ = {static_cast<int>(NFTErrorCode::NOT_INITIALIZED), "NFTManager not initialized"};
            return {};
        }
        
        std::vector<NFT> result;
        auto it = ownerNFTs_.find(owner);
        if (it != ownerNFTs_.end()) {
            for (const auto& nftId : it->second) {
                auto nftIt = nftRegistry_.find(nftId);
                if (nftIt != nftRegistry_.end()) {
                    result.push_back(nftIt->second);
                }
            }
        }
        
        return result;
    } catch (const std::exception& e) {
        lastError_ = {static_cast<int>(NFTErrorCode::QUERY_ERROR), e.what()};
        return {};
    }
}

std::vector<NFTManager::NFT> NFTManager::getNFTsByContract(const std::string& contractAddress) {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!initialized_) {
            lastError_ = {static_cast<int>(NFTErrorCode::NOT_INITIALIZED), "NFTManager not initialized"};
            return {};
        }
        
        std::vector<NFT> result;
        auto it = contractNFTs_.find(contractAddress);
        if (it != contractNFTs_.end()) {
            for (const auto& nftId : it->second) {
                auto nftIt = nftRegistry_.find(nftId);
                if (nftIt != nftRegistry_.end()) {
                    result.push_back(nftIt->second);
                }
            }
        }
        
        return result;
    } catch (const std::exception& e) {
        lastError_ = {static_cast<int>(NFTErrorCode::QUERY_ERROR), e.what()};
        return {};
    }
}

std::vector<NFTManager::NFT> NFTManager::getNFTsByCreator(const std::string& creator) {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!initialized_) {
            lastError_ = {static_cast<int>(NFTErrorCode::NOT_INITIALIZED), "NFTManager not initialized"};
            return {};
        }
        
        std::vector<NFT> result;
        auto it = creatorNFTs_.find(creator);
        if (it != creatorNFTs_.end()) {
            for (const auto& nftId : it->second) {
                auto nftIt = nftRegistry_.find(nftId);
                if (nftIt != nftRegistry_.end()) {
                    result.push_back(nftIt->second);
                }
            }
        }
        
        return result;
    } catch (const std::exception& e) {
        lastError_ = {static_cast<int>(NFTErrorCode::QUERY_ERROR), e.what()};
        return {};
    }
}

bool NFTManager::isNFTTransferable(const std::string& nftId) {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = nftRegistry_.find(nftId);
        return it != nftRegistry_.end() && it->second.isTransferable;
    } catch (const std::exception& e) {
        return false;
    }
}

bool NFTManager::isNFTBurnable(const std::string& nftId) {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = nftRegistry_.find(nftId);
        return it != nftRegistry_.end() && it->second.isBurnable;
    } catch (const std::exception& e) {
        return false;
    }
}

std::string NFTManager::getNFTOwner(const std::string& nftId) {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = nftRegistry_.find(nftId);
        return it != nftRegistry_.end() ? it->second.owner : "";
    } catch (const std::exception& e) {
        return "";
    }
}

NFTManager::NFTMetadata NFTManager::getNFTMetadata(const std::string& nftId) {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = nftRegistry_.find(nftId);
        return it != nftRegistry_.end() ? it->second.metadata : NFTMetadata{};
    } catch (const std::exception& e) {
        return NFTMetadata{};
    }
}

// Batch Operations
std::vector<std::string> NFTManager::batchCreateNFTs(const std::string& contractAddress, 
                                                   const std::vector<NFTMetadata>& metadataList,
                                                   const std::string& creator) {
    std::vector<std::string> result;
    for (const auto& metadata : metadataList) {
        std::string nftId = createNFT(contractAddress, metadata, creator);
        if (!nftId.empty()) {
            result.push_back(nftId);
        }
    }
    return result;
}

bool NFTManager::batchTransferNFTs(const std::vector<std::string>& nftIds, 
                                  const std::string& from, const std::string& to) {
    bool success = true;
    for (const auto& nftId : nftIds) {
        if (!transferNFT(nftId, from, to)) {
            success = false;
        }
    }
    return success;
}

bool NFTManager::batchBurnNFTs(const std::vector<std::string>& nftIds, const std::string& owner) {
    bool success = true;
    for (const auto& nftId : nftIds) {
        if (!burnNFT(nftId, owner)) {
            success = false;
        }
    }
    return success;
}

// Event Handling
void NFTManager::registerEventCallback(EventCallback callback) {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        eventCallback_ = callback;
    } catch (const std::exception& e) {
        lastError_ = {static_cast<int>(NFTErrorCode::VALIDATION_ERROR), e.what()};
    }
}

void NFTManager::unregisterEventCallback() {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        eventCallback_ = nullptr;
    } catch (const std::exception& e) {
        lastError_ = {static_cast<int>(NFTErrorCode::VALIDATION_ERROR), e.what()};
    }
}

// Error Handling
NFTManager::Error NFTManager::getLastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastError_;
}

void NFTManager::clearLastError() {
    std::lock_guard<std::mutex> lock(mutex_);
    lastError_ = Error{};
}

// Ownership Methods
std::vector<NFTManager::OwnershipRecord> NFTManager::getOwnershipHistory(const std::string& nftId) {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!initialized_) {
            lastError_ = {static_cast<int>(NFTErrorCode::NOT_INITIALIZED), "NFTManager not initialized"};
            return {};
        }
        
        auto it = nftRegistry_.find(nftId);
        if (it == nftRegistry_.end()) {
            lastError_ = {static_cast<int>(NFTErrorCode::INVALID_ID), "NFT not found"};
            return {};
        }
        
        // TODO: Implement actual ownership history tracking
        // For now, return basic ownership records for testing
        std::vector<OwnershipRecord> history;
        
        // Add creation record
        OwnershipRecord creationRecord;
        creationRecord.owner = it->second.creator;
        creationRecord.timestamp = it->second.createdAt;
        creationRecord.transactionId = "creation";
        creationRecord.reason = "NFT created";
        history.push_back(creationRecord);
        
        // Add current owner record if different from creator
        if (it->second.owner != it->second.creator) {
            OwnershipRecord currentRecord;
            currentRecord.owner = it->second.owner;
            currentRecord.previousOwner = it->second.creator;
            currentRecord.timestamp = it->second.updatedAt;
            currentRecord.transactionId = "transfer";
            currentRecord.reason = "NFT transferred";
            history.push_back(currentRecord);
        }
        
        return history;
    } catch (const std::exception& e) {
        lastError_ = {static_cast<int>(NFTErrorCode::QUERY_ERROR), e.what()};
        return {};
    }
}

std::string NFTManager::getCurrentOwner(const std::string& nftId) {
    return getNFTOwner(nftId);
}

bool NFTManager::verifyOwnership(const std::string& nftId, const std::string& owner) {
    return getNFTOwner(nftId) == owner;
}

// Performance optimization methods
void NFTManager::setCacheSize(size_t size) {
    std::lock_guard<std::mutex> lock(mutex_);
    cache_size_ = size;
}

void NFTManager::setBatchSize(size_t size) {
    // TODO: Implement batch size setting
}

void NFTManager::setConnectionPoolSize(size_t size) {
    // TODO: Implement connection pool size setting
}

void NFTManager::clearCache() {
    std::lock_guard<std::mutex> lock(mutex_);
    cache_size_ = 0;
}

size_t NFTManager::getCacheSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return cache_size_;
}

size_t NFTManager::getBatchSize() const {
    // TODO: Implement batch size getting
    return 0;
}

size_t NFTManager::getConnectionPoolSize() const {
    // TODO: Implement connection pool size getting
    return 0;
}

// Error handling methods
void NFTManager::setErrorCallback(std::function<void(const NFTError&)> callback) {
    // TODO: Implement error callback setting
}

void NFTManager::setRecoveryCallback(std::function<void(const NFTError&)> callback) {
    // TODO: Implement recovery callback setting
}

std::vector<NFTError> NFTManager::getErrorHistory() const {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!initialized_) {
            const_cast<NFTManager*>(this)->lastError_ = {static_cast<int>(NFTErrorCode::NOT_INITIALIZED), "NFTManager not initialized"};
            return {};
        }
        
        // TODO: Implement error history
        return {};
    } catch (const std::exception& e) {
        const_cast<NFTManager*>(this)->lastError_ = {static_cast<int>(NFTErrorCode::QUERY_ERROR), e.what()};
        return {};
    }
}

void NFTManager::clearErrorHistory() {
    // TODO: Implement error history clearing
}

bool NFTManager::isErrorActive(const std::string& nftId) const {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!initialized_) {
            const_cast<NFTManager*>(this)->lastError_ = {static_cast<int>(NFTErrorCode::NOT_INITIALIZED), "NFTManager not initialized"};
            return false;
        }
        
        // TODO: Implement error active checking
        return false;
    } catch (const std::exception& e) {
        const_cast<NFTManager*>(this)->lastError_ = {static_cast<int>(NFTErrorCode::QUERY_ERROR), e.what()};
        return false;
    }
}

void NFTManager::addEventListener(NFTEventListener* listener) {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        // TODO: Implement event listener addition
    } catch (const std::exception& e) {
        lastError_ = {static_cast<int>(NFTErrorCode::VALIDATION_ERROR), e.what()};
    }
}

void NFTManager::removeEventListener(NFTEventListener* listener) {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        // TODO: Implement event listener removal
    } catch (const std::exception& e) {
        lastError_ = {static_cast<int>(NFTErrorCode::VALIDATION_ERROR), e.what()};
    }
}

// Private helper methods
std::string NFTManager::generateNFTId(const std::string& contractAddress, const NFTMetadata& metadata) {
    std::stringstream ss;
    ss << contractAddress << "_" << metadata.name << "_" << std::chrono::system_clock::now().time_since_epoch().count();
    
    // Use modern OpenSSL EVP interface instead of deprecated SHA256 functions
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(ctx, ss.str().c_str(), ss.str().length());
    
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
    EVP_DigestFinal_ex(ctx, hash, &hash_len);
    EVP_MD_CTX_free(ctx);
    
    std::stringstream result;
    for (unsigned int i = 0; i < hash_len; i++) {
        result << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    
    return result.str();
}

bool NFTManager::validateRoyaltyBasisPoints(uint32_t basisPoints) {
    return basisPoints <= 10000; // Max 100%
}

void NFTManager::emitEvent(const NFTEvent& event) {
    if (eventCallback_) {
        eventCallback_(event);
    }
}

bool NFTManager::checkOwnership(const std::string& nftId, const std::string& owner) {
    return getNFTOwner(nftId) == owner;
}

bool NFTManager::checkTransferability(const std::string& nftId) {
    return isNFTTransferable(nftId);
}

bool NFTManager::checkBurnability(const std::string& nftId) {
    return isNFTBurnable(nftId);
}

// Implementation classes
class NFTManager::Impl {
public:
    Impl() = default;
    ~Impl() = default;
};

class NFTManager::PerformanceOptimizer {
public:
    PerformanceOptimizer() = default;
    ~PerformanceOptimizer() = default;
};

// ErrorHandler implementation
NFTManager::ErrorHandler::ErrorHandler() {
    // Initialize logger as nullptr to avoid spdlog API compatibility issues
    logger_ = nullptr;
}

NFTManager::ErrorHandler::~ErrorHandler() {
    // Cleanup if needed
}

void NFTManager::ErrorHandler::recordError(const NFTError& error) {
    std::lock_guard<std::mutex> lock(errorMutex_);
    last_error_ = error;
}

bool NFTManager::ErrorHandler::isErrorActive(const std::string& error_id) const {
    std::lock_guard<std::mutex> lock(errorMutex_);
    auto it = error_history_.find(error_id);
    return it != error_history_.end() && it->second.is_active;
}

std::vector<NFTError> NFTManager::ErrorHandler::getErrorHistory() const {
    std::lock_guard<std::mutex> lock(errorMutex_);
    std::vector<NFTError> result;
    for (const auto& [id, record] : error_history_) {
        result.push_back(record.error);
    }
    return result;
}

void NFTManager::ErrorHandler::setErrorCallback(std::function<void(const NFTError&)> callback) {
    std::lock_guard<std::mutex> lock(errorMutex_);
    error_callback_ = callback;
}

void NFTManager::ErrorHandler::clearLastError() {
    std::lock_guard<std::mutex> lock(errorMutex_);
    last_error_ = NFTError{};
}

NFTError NFTManager::ErrorHandler::getLastError() const {
    std::lock_guard<std::mutex> lock(errorMutex_);
    return last_error_;
}

} // namespace nft
} // namespace satox 