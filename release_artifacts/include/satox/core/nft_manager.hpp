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
#include <map>
#include <memory>
#include <mutex>
#include <functional>
#include <chrono>
#include <nlohmann/json.hpp>

namespace satox::core {

class NFTManager {
public:
    // NFT types
    enum class NFTType {
        IMAGE,
        VIDEO,
        AUDIO,
        DOCUMENT,
        COLLECTIBLE,
        GAME_ITEM,
        CUSTOM
    };

    // NFT states
    enum class NFTState {
        CREATED,
        MINTED,
        TRANSFERRED,
        BURNED,
        ERROR
    };

    // NFT metadata structure
    struct NFTMetadata {
        std::string name;
        std::string description;
        std::string creator;
        std::string owner;
        NFTType type;
        std::string uri;
        std::string hash;
        std::chrono::system_clock::time_point creationTime;
        std::chrono::system_clock::time_point lastModified;
        nlohmann::json attributes;
        nlohmann::json additionalData;
    };

    // NFT structure
    struct NFT {
        std::string id;
        std::string assetId;
        NFTMetadata metadata;
        NFTState state;
        std::string transactionId;
        std::chrono::system_clock::time_point mintTime;
        std::vector<std::string> history;
        nlohmann::json additionalData;
    };

    // NFT stats structure
    struct NFTStats {
        size_t totalNFTs;
        size_t mintedNFTs;
        size_t transferredNFTs;
        size_t burnedNFTs;
        size_t errorNFTs;
        std::chrono::milliseconds averageMintTime;
        std::chrono::milliseconds averageTransferTime;
        nlohmann::json typeDistribution;
        nlohmann::json additionalStats;
    };

    // Callback types
    using NFTCallback = std::function<void(const std::string&, NFTState)>;
    using NFTMetadataCallback = std::function<void(const std::string&, const NFTMetadata&)>;
    using NFTErrorCallback = std::function<void(const std::string&, const std::string&)>;

    // Singleton instance
    static NFTManager& getInstance();

    // Initialization and shutdown
    bool initialize(const nlohmann::json& config);
    void shutdown();

    // NFT creation and management
    bool createNFT(const std::string& assetId, const NFTMetadata& metadata);
    bool mintNFT(const std::string& nftId);
    bool transferNFT(const std::string& nftId, const std::string& newOwner);
    bool burnNFT(const std::string& nftId);
    bool updateNFTMetadata(const std::string& nftId, const NFTMetadata& metadata);

    // NFT querying
    bool isNFTExists(const std::string& nftId) const;
    NFT getNFT(const std::string& nftId) const;
    std::vector<NFT> getNFTsByAsset(const std::string& assetId) const;
    std::vector<NFT> getNFTsByOwner(const std::string& owner) const;
    std::vector<NFT> getNFTsByType(NFTType type) const;
    std::vector<NFT> getNFTsByState(NFTState state) const;

    // NFT metadata management
    bool validateMetadata(const NFTMetadata& metadata) const;
    bool updateNFTURI(const std::string& nftId, const std::string& uri);
    bool updateNFTHash(const std::string& nftId, const std::string& hash);
    bool addNFTAttribute(const std::string& nftId, const std::string& key, const nlohmann::json& value);
    bool removeNFTAttribute(const std::string& nftId, const std::string& key);

    // NFT history management
    std::vector<std::string> getNFTHistory(const std::string& nftId) const;
    bool addNFTHistoryEntry(const std::string& nftId, const std::string& entry);
    bool clearNFTHistory(const std::string& nftId);

    // NFT statistics
    NFTStats getStats() const;
    void resetStats();
    bool enableStats(bool enable);

    // Callback registration
    void registerNFTCallback(NFTCallback callback);
    void registerMetadataCallback(NFTMetadataCallback callback);
    void registerErrorCallback(NFTErrorCallback callback);
    void unregisterNFTCallback();
    void unregisterMetadataCallback();
    void unregisterErrorCallback();

    // Error handling
    std::string getLastError() const;
    void clearLastError();

private:
    NFTManager() = default;
    ~NFTManager() = default;
    NFTManager(const NFTManager&) = delete;
    NFTManager& operator=(const NFTManager&) = delete;

    // Helper methods
    bool validateConfig(const nlohmann::json& config) const;
    bool validateNFTId(const std::string& nftId) const;
    bool validateAssetId(const std::string& assetId) const;
    bool validateOwner(const std::string& owner) const;
    bool checkNFTLimit() const;
    void updateNFTState(const std::string& nftId, NFTState state);
    void updateNFTStats(const std::string& nftId, NFTState state);
    void notifyNFTChange(const std::string& nftId, NFTState state);
    void notifyMetadataChange(const std::string& nftId, const NFTMetadata& metadata);
    void notifyError(const std::string& nftId, const std::string& error);

    // Member variables
    mutable std::mutex mutex_;
    bool initialized_ = false;
    std::map<std::string, NFT> nfts_;
    std::map<std::string, std::vector<NFTCallback>> nftCallbacks_;
    std::map<std::string, std::vector<NFTMetadataCallback>> metadataCallbacks_;
    std::vector<NFTErrorCallback> errorCallbacks_;
    NFTStats stats_;
    bool statsEnabled_ = false;
    std::string lastError_;
};

} // namespace satox::core 