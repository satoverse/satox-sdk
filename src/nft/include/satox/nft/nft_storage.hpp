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

#include "satox/nft/nft_manager.hpp"
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <map>
#include <unordered_map>
#include <functional>
#include <nlohmann/json.hpp>

namespace satox {
namespace nft {

class NFTStorage {
public:
    static NFTStorage& getInstance();

    // Prevent copying
    NFTStorage(const NFTStorage&) = delete;
    NFTStorage& operator=(const NFTStorage&) = delete;

    // Initialization and cleanup
    bool initialize(const std::string& storagePath);
    void shutdown();

    // Storage Operations
    bool storeNFT(const NFTManager::NFT& nft);
    bool storeNFTMetadata(const std::string& nftId, const NFTManager::NFTMetadata& metadata);
    bool storeNFTBatch(const std::vector<NFTManager::NFT>& nfts);
    bool storeNFTMetadataBatch(const std::map<std::string, NFTManager::NFTMetadata>& metadataMap);

    // Retrieval Operations
    std::optional<NFTManager::NFT> getNFT(const std::string& nftId);
    std::optional<NFTManager::NFTMetadata> getNFTMetadata(const std::string& nftId);
    std::vector<NFTManager::NFT> getNFTsByOwner(const std::string& owner);
    std::vector<NFTManager::NFT> getNFTsByContract(const std::string& contractAddress);
    std::vector<NFTManager::NFT> getNFTsByCreator(const std::string& creator);

    // Update Operations
    bool updateNFT(const NFTManager::NFT& nft);
    bool updateNFTMetadata(const std::string& nftId, const NFTManager::NFTMetadata& metadata);
    bool updateNFTBatch(const std::vector<NFTManager::NFT>& nfts);
    bool updateNFTMetadataBatch(const std::map<std::string, NFTManager::NFTMetadata>& metadataMap);

    // Delete Operations
    bool deleteNFT(const std::string& nftId);
    bool deleteNFTMetadata(const std::string& nftId);
    bool deleteNFTBatch(const std::vector<std::string>& nftIds);
    bool deleteNFTMetadataBatch(const std::vector<std::string>& nftIds);

    // Search Operations
    std::vector<NFTManager::NFT> searchNFTs(const std::string& query);
    std::vector<NFTManager::NFT> searchNFTsByMetadata(const nlohmann::json& metadataQuery);
    std::vector<NFTManager::NFT> searchNFTsByAttributes(const std::map<std::string, std::string>& attributes);

    // Cache Operations
    void enableCache(bool enable);
    void clearCache();
    void setCacheSize(size_t maxSize);
    size_t getCacheSize() const;

    // Backup and Restore
    bool createBackup(const std::string& backupPath);
    bool restoreFromBackup(const std::string& backupPath);

    // Error Handling
    struct Error {
        int code;
        std::string message;
    };
    Error getLastError() const;
    void clearLastError();

private:
    NFTStorage() = default;
    ~NFTStorage() = default;

    // Internal helper methods
    bool validateStoragePath(const std::string& path);
    bool createStorageDirectory(const std::string& path);
    bool writeToFile(const std::string& path, const std::string& data);
    std::optional<std::string> readFromFile(const std::string& path);
    bool deleteFile(const std::string& path);
    bool fileExists(const std::string& path);
    std::string getNFTFilePath(const std::string& nftId);
    std::string getMetadataFilePath(const std::string& nftId);
    bool updateIndex(const std::string& nftId, const NFTManager::NFT& nft);
    bool removeFromIndex(const std::string& nftId);

    // Member variables
    std::mutex mutex_;
    std::string storagePath_;
    std::unordered_map<std::string, NFTManager::NFT> nftCache_;
    std::unordered_map<std::string, NFTManager::NFTMetadata> metadataCache_;
    std::unordered_map<std::string, std::vector<std::string>> ownerIndex_;
    std::unordered_map<std::string, std::vector<std::string>> contractIndex_;
    std::unordered_map<std::string, std::vector<std::string>> creatorIndex_;
    Error lastError_;
    bool initialized_ = false;
    bool cacheEnabled_ = true;
    size_t maxCacheSize_ = 1000;
};

} // namespace nft
} // namespace satox 