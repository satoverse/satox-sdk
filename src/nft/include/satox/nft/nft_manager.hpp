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
#include <map>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <functional>
#include <chrono>
#include <spdlog/spdlog.h>
#include <atomic>
#include <thread>
#include <condition_variable>

namespace satox {
namespace nft {

enum class NFTErrorCode {
    NONE = 0,
    INVALID_METADATA,
    INVALID_OWNER,
    TRANSFER_FAILED,
    BURN_FAILED,
    DUPLICATE_ID,
    INVALID_ID,
    INVALID_AMOUNT,
    INSUFFICIENT_BALANCE,
    INVALID_OPERATION,
    STORAGE_ERROR,
    NETWORK_ERROR,
    VALIDATION_ERROR,
    UNKNOWN_ERROR,
    NOT_INITIALIZED,
    CACHE_ERROR,
    CONFIG_ERROR,
    QUERY_ERROR,
    CLEAR_ERROR
};

struct NFTError {
    NFTErrorCode code;
    std::string message;
    std::string details;

    NFTError(NFTErrorCode c = NFTErrorCode::NONE, 
             const std::string& msg = "", 
             const std::string& det = "")
        : code(c), message(msg), details(det) {}
};

class NFTManager {
public:
    static NFTManager& getInstance();

    // Prevent copying
    NFTManager(const NFTManager&) = delete;
    NFTManager& operator=(const NFTManager&) = delete;

    // Initialization and cleanup
    bool initialize();
    void shutdown();
    bool isInitialized() const;
    bool isHealthy() const;

    // NFT Creation and Management
    struct NFTMetadata {
        std::string name;
        std::string description;
        std::string image;
        std::string externalUrl;
        std::map<std::string, std::string> attributes;
        nlohmann::json additionalData;
    };

    struct NFT {
        std::string id;
        std::string contractAddress;
        std::string owner;
        std::string creator;
        NFTMetadata metadata;
        uint64_t tokenId;
        std::string tokenURI;
        bool isTransferable;
        bool isBurnable;
        std::string royaltyRecipient;
        uint32_t royaltyBasisPoints;
        std::chrono::system_clock::time_point createdAt;
        std::chrono::system_clock::time_point updatedAt;
    };

    struct OwnershipRecord {
        std::string owner;
        std::string previousOwner;
        std::chrono::system_clock::time_point timestamp;
        std::string transactionId;
        std::string reason;
    };

    struct CacheEntry {
        NFT nft;
        std::chrono::system_clock::time_point expiry;
    };

    enum class NFTEventType {
        CREATED,
        TRANSFERRED,
        BURNED,
        METADATA_UPDATED,
        TRANSFERABLE_CHANGED,
        BURNABLE_CHANGED,
        ROYALTY_CHANGED,
        ATTRIBUTE_ADDED,
        ATTRIBUTE_REMOVED
    };

    // NFT Creation
    std::string createNFT(const std::string& contractAddress, const NFTMetadata& metadata, 
                         const std::string& creator, bool isTransferable = true, 
                         bool isBurnable = true, const std::string& royaltyRecipient = "",
                         uint32_t royaltyBasisPoints = 0);
    
    // NFT Operations
    bool transferNFT(const std::string& nftId, const std::string& from, const std::string& to);
    bool burnNFT(const std::string& nftId, const std::string& owner);
    bool updateNFTMetadata(const std::string& nftId, const NFTMetadata& newMetadata, const std::string& updater);
    bool addNFTAttribute(const std::string& nftId, const std::string& key, const std::string& value, const std::string& updater);
    bool removeNFTAttribute(const std::string& nftId, const std::string& key, const std::string& updater);
    bool setNFTTransferable(const std::string& nftId, bool isTransferable);
    bool setNFTBurnable(const std::string& nftId, bool isBurnable);
    bool setNFTRoyalty(const std::string& nftId, const std::string& recipient, uint32_t basisPoints);

    // NFT Validation
    bool validateNFT(const NFT& nft);
    bool validateNFTMetadata(const NFTMetadata& metadata);
    bool validateTransfer(const std::string& nftId, const std::string& from, const std::string& to);

    // NFT Queries
    NFT getNFT(const std::string& nftId);
    std::vector<NFT> getNFTsByOwner(const std::string& owner);
    std::vector<NFT> getNFTsByContract(const std::string& contractAddress);
    std::vector<NFT> getNFTsByCreator(const std::string& creator);
    bool isNFTTransferable(const std::string& nftId);
    bool isNFTBurnable(const std::string& nftId);
    std::string getNFTOwner(const std::string& nftId);
    NFTMetadata getNFTMetadata(const std::string& nftId);

    // Batch Operations
    std::vector<std::string> batchCreateNFTs(const std::string& contractAddress, 
                                           const std::vector<NFTMetadata>& metadataList,
                                           const std::string& creator);
    bool batchTransferNFTs(const std::vector<std::string>& nftIds, 
                          const std::string& from, const std::string& to);
    bool batchBurnNFTs(const std::vector<std::string>& nftIds, const std::string& owner);

    // Event Handling
    struct NFTEvent {
        NFTEventType type;
        std::string nftId;
        std::string contractAddress;
        std::string from;
        std::string to;
        uint64_t timestamp;
        nlohmann::json additionalData;
    };

    using EventCallback = std::function<void(const NFTEvent&)>;
    void registerEventCallback(EventCallback callback);
    void unregisterEventCallback();

    // Error Handling
    struct Error {
        int code;
        std::string message;
        Error(int c = 0, const std::string& msg = "") : code(c), message(msg) {}
    };
    Error getLastError() const;
    void clearLastError();

    // Ownership Methods
    std::vector<OwnershipRecord> getOwnershipHistory(const std::string& nftId);
    std::string getCurrentOwner(const std::string& nftId);
    bool verifyOwnership(const std::string& nftId, const std::string& owner);

    // Performance optimization methods
    void setCacheSize(size_t size);
    void setBatchSize(size_t size);
    void setConnectionPoolSize(size_t size);
    void clearCache();
    size_t getCacheSize() const;
    size_t getBatchSize() const;
    size_t getConnectionPoolSize() const;

    // Error handling methods
    void setErrorCallback(std::function<void(const NFTError&)> callback);
    void setRecoveryCallback(std::function<void(const NFTError&)> callback);
    std::vector<NFTError> getErrorHistory() const;
    void clearErrorHistory();
    bool isErrorActive(const std::string& nftId) const;

    class NFTEventListener {
    public:
        virtual ~NFTEventListener() = default;
        virtual void onNFTEvent(NFTEventType type, const NFT& nft) = 0;
    };

    void addEventListener(NFTEventListener* listener);
    void removeEventListener(NFTEventListener* listener);

private:
    NFTManager();
    ~NFTManager();

    // Internal helper methods
    std::string generateNFTId(const std::string& contractAddress, const NFTMetadata& metadata);
    bool validateRoyaltyBasisPoints(uint32_t basisPoints);
    void emitEvent(const NFTEvent& event);
    bool checkOwnership(const std::string& nftId, const std::string& owner);
    bool checkTransferability(const std::string& nftId);
    bool checkBurnability(const std::string& nftId);

    // Member variables
    class PerformanceOptimizer;
    class ErrorHandler;
    class Impl;
    std::unique_ptr<Impl> impl_;
    std::unique_ptr<PerformanceOptimizer> performanceOptimizer_;
    std::unique_ptr<ErrorHandler> errorHandler_;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, NFT> nftRegistry_;
    std::unordered_map<std::string, std::vector<std::string>> ownerNFTs_;
    std::unordered_map<std::string, std::vector<std::string>> contractNFTs_;
    std::unordered_map<std::string, std::vector<std::string>> creatorNFTs_;
    EventCallback eventCallback_;
    Error lastError_;
    bool initialized_ = false;
    std::atomic<bool> should_stop_;
    std::thread cleanup_thread_;
    std::condition_variable cleanup_cv_;
    mutable std::mutex cleanup_mutex_;
    size_t cache_size_;
    const size_t max_cache_size_ = 1000;
    
    // Rate limiting
    std::unordered_map<std::string, int> transferCounts_;
    std::chrono::system_clock::time_point lastResetTime_;
    const int maxTransfersPerMinute_ = 10;
};

class NFTManager::ErrorHandler {
public:
    struct ErrorRecord {
        NFTError error;
        std::chrono::system_clock::time_point timestamp;
        bool is_active;

        ErrorRecord(const NFTError& err = NFTError(),
                   const std::chrono::system_clock::time_point& ts = std::chrono::system_clock::now(),
                   bool active = true)
            : error(err), timestamp(ts), is_active(active) {}
    };

    ErrorHandler();
    ~ErrorHandler();

    void recordError(const NFTError& error);
    bool isErrorActive(const std::string& error_id) const;
    std::vector<NFTError> getErrorHistory() const;
    void setErrorCallback(std::function<void(const NFTError&)> callback);
    void clearLastError();
    NFTError getLastError() const;

private:
    std::unordered_map<std::string, ErrorRecord> error_history_;
    std::function<void(const NFTError&)> error_callback_;
    mutable std::mutex errorMutex_;
    std::shared_ptr<spdlog::logger> logger_;
    NFTError last_error_;
};

} // namespace nft
} // namespace satox 