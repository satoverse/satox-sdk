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

#include "satox/nft/nft_storage.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <nlohmann/json.hpp>

namespace satox {
namespace nft {

NFTStorage& NFTStorage::getInstance() {
    static NFTStorage instance;
    return instance;
}

bool NFTStorage::initialize(const std::string& storagePath) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) {
        return true;
    }

    if (!validateStoragePath(storagePath)) {
        lastError_ = {1, "Invalid storage path"};
        return false;
    }

    if (!createStorageDirectory(storagePath)) {
        lastError_ = {2, "Failed to create storage directory"};
        return false;
    }

    storagePath_ = storagePath;
    initialized_ = true;
    return true;
}

void NFTStorage::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    clearCache();
    initialized_ = false;
}

bool NFTStorage::storeNFT(const NFTManager::NFT& nft) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = {3, "Storage not initialized"};
        return false;
    }

    try {
        nlohmann::json nftJson = {
            {"id", nft.id},
            {"contractAddress", nft.contractAddress},
            {"owner", nft.owner},
            {"creator", nft.creator},
            {"tokenURI", nft.tokenURI},
            {"createdAt", nft.createdAt},
            {"isTransferable", nft.isTransferable},
            {"isBurnable", nft.isBurnable},
            {"royaltyRecipient", nft.royaltyRecipient},
            {"royaltyBasisPoints", nft.royaltyBasisPoints}
        };

        std::string filePath = getNFTFilePath(nft.id);
        if (!writeToFile(filePath, nftJson.dump())) {
            return false;
        }

        if (cacheEnabled_) {
            nftCache_[nft.id] = nft;
            if (nftCache_.size() > maxCacheSize_) {
                nftCache_.erase(nftCache_.begin());
            }
        }

        return updateIndex(nft.id, nft);
    } catch (const std::exception& e) {
        lastError_ = {4, std::string("Failed to store NFT: ") + e.what()};
        return false;
    }
}

bool NFTStorage::storeNFTMetadata(const std::string& nftId, const NFTManager::NFTMetadata& metadata) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = {3, "Storage not initialized"};
        return false;
    }

    try {
        nlohmann::json metadataJson = {
            {"name", metadata.name},
            {"description", metadata.description},
            {"image", metadata.image},
            {"attributes", metadata.attributes}
        };

        std::string filePath = getMetadataFilePath(nftId);
        if (!writeToFile(filePath, metadataJson.dump())) {
            return false;
        }

        if (cacheEnabled_) {
            metadataCache_[nftId] = metadata;
            if (metadataCache_.size() > maxCacheSize_) {
                metadataCache_.erase(metadataCache_.begin());
            }
        }

        return true;
    } catch (const std::exception& e) {
        lastError_ = {5, std::string("Failed to store NFT metadata: ") + e.what()};
        return false;
    }
}

bool NFTStorage::storeNFTBatch(const std::vector<NFTManager::NFT>& nfts) {
    bool success = true;
    for (const auto& nft : nfts) {
        if (!storeNFT(nft)) {
            success = false;
        }
    }
    return success;
}

bool NFTStorage::storeNFTMetadataBatch(const std::map<std::string, NFTManager::NFTMetadata>& metadataMap) {
    bool success = true;
    for (const auto& [nftId, metadata] : metadataMap) {
        if (!storeNFTMetadata(nftId, metadata)) {
            success = false;
        }
    }
    return success;
}

std::optional<NFTManager::NFT> NFTStorage::getNFT(const std::string& nftId) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = {3, "Storage not initialized"};
        return std::nullopt;
    }

    // Check cache first
    if (cacheEnabled_) {
        auto it = nftCache_.find(nftId);
        if (it != nftCache_.end()) {
            return it->second;
        }
    }

    try {
        std::string filePath = getNFTFilePath(nftId);
        auto data = readFromFile(filePath);
        if (!data) {
            return std::nullopt;
        }

        nlohmann::json nftJson = nlohmann::json::parse(*data);
        NFTManager::NFT nft;
        nft.id = nftJson["id"];
        nft.contractAddress = nftJson["contractAddress"];
        nft.owner = nftJson["owner"];
        nft.creator = nftJson["creator"];
        nft.tokenURI = nftJson["tokenURI"];
        nft.createdAt = nftJson["createdAt"];
        nft.isTransferable = nftJson["isTransferable"];
        nft.isBurnable = nftJson["isBurnable"];
        nft.royaltyRecipient = nftJson["royaltyRecipient"];
        nft.royaltyBasisPoints = nftJson["royaltyBasisPoints"];

        if (cacheEnabled_) {
            nftCache_[nftId] = nft;
            if (nftCache_.size() > maxCacheSize_) {
                nftCache_.erase(nftCache_.begin());
            }
        }

        return nft;
    } catch (const std::exception& e) {
        lastError_ = {6, std::string("Failed to get NFT: ") + e.what()};
        return std::nullopt;
    }
}

std::optional<NFTManager::NFTMetadata> NFTStorage::getNFTMetadata(const std::string& nftId) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = {3, "Storage not initialized"};
        return std::nullopt;
    }

    // Check cache first
    if (cacheEnabled_) {
        auto it = metadataCache_.find(nftId);
        if (it != metadataCache_.end()) {
            return it->second;
        }
    }

    try {
        std::string filePath = getMetadataFilePath(nftId);
        auto data = readFromFile(filePath);
        if (!data) {
            return std::nullopt;
        }

        nlohmann::json metadataJson = nlohmann::json::parse(*data);
        NFTManager::NFTMetadata metadata;
        metadata.name = metadataJson["name"];
        metadata.description = metadataJson["description"];
        metadata.image = metadataJson["image"];
        metadata.attributes = metadataJson["attributes"].get<std::vector<std::string>>();

        if (cacheEnabled_) {
            metadataCache_[nftId] = metadata;
            if (metadataCache_.size() > maxCacheSize_) {
                metadataCache_.erase(metadataCache_.begin());
            }
        }

        return metadata;
    } catch (const std::exception& e) {
        lastError_ = {7, std::string("Failed to get NFT metadata: ") + e.what()};
        return std::nullopt;
    }
}

std::vector<NFTManager::NFT> NFTStorage::getNFTsByOwner(const std::string& owner) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<NFTManager::NFT> nfts;
    
    auto it = ownerIndex_.find(owner);
    if (it != ownerIndex_.end()) {
        for (const auto& nftId : it->second) {
            if (auto nft = getNFT(nftId)) {
                nfts.push_back(*nft);
            }
        }
    }
    
    return nfts;
}

std::vector<NFTManager::NFT> NFTStorage::getNFTsByContract(const std::string& contractAddress) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<NFTManager::NFT> nfts;
    
    auto it = contractIndex_.find(contractAddress);
    if (it != contractIndex_.end()) {
        for (const auto& nftId : it->second) {
            if (auto nft = getNFT(nftId)) {
                nfts.push_back(*nft);
            }
        }
    }
    
    return nfts;
}

std::vector<NFTManager::NFT> NFTStorage::getNFTsByCreator(const std::string& creator) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<NFTManager::NFT> nfts;
    
    auto it = creatorIndex_.find(creator);
    if (it != creatorIndex_.end()) {
        for (const auto& nftId : it->second) {
            if (auto nft = getNFT(nftId)) {
                nfts.push_back(*nft);
            }
        }
    }
    
    return nfts;
}

bool NFTStorage::updateNFT(const NFTManager::NFT& nft) {
    return storeNFT(nft); // Reuse storeNFT as it handles updates
}

bool NFTStorage::updateNFTMetadata(const std::string& nftId, const NFTManager::NFTMetadata& metadata) {
    return storeNFTMetadata(nftId, metadata); // Reuse storeNFTMetadata as it handles updates
}

bool NFTStorage::updateNFTBatch(const std::vector<NFTManager::NFT>& nfts) {
    return storeNFTBatch(nfts); // Reuse storeNFTBatch as it handles updates
}

bool NFTStorage::updateNFTMetadataBatch(const std::map<std::string, NFTManager::NFTMetadata>& metadataMap) {
    return storeNFTMetadataBatch(metadataMap); // Reuse storeNFTMetadataBatch as it handles updates
}

bool NFTStorage::deleteNFT(const std::string& nftId) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = {3, "Storage not initialized"};
        return false;
    }

    try {
        std::string filePath = getNFTFilePath(nftId);
        if (!deleteFile(filePath)) {
            return false;
        }

        if (cacheEnabled_) {
            nftCache_.erase(nftId);
        }

        return removeFromIndex(nftId);
    } catch (const std::exception& e) {
        lastError_ = {8, std::string("Failed to delete NFT: ") + e.what()};
        return false;
    }
}

bool NFTStorage::deleteNFTMetadata(const std::string& nftId) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = {3, "Storage not initialized"};
        return false;
    }

    try {
        std::string filePath = getMetadataFilePath(nftId);
        if (!deleteFile(filePath)) {
            return false;
        }

        if (cacheEnabled_) {
            metadataCache_.erase(nftId);
        }

        return true;
    } catch (const std::exception& e) {
        lastError_ = {9, std::string("Failed to delete NFT metadata: ") + e.what()};
        return false;
    }
}

bool NFTStorage::deleteNFTBatch(const std::vector<std::string>& nftIds) {
    bool success = true;
    for (const auto& nftId : nftIds) {
        if (!deleteNFT(nftId)) {
            success = false;
        }
    }
    return success;
}

bool NFTStorage::deleteNFTMetadataBatch(const std::vector<std::string>& nftIds) {
    bool success = true;
    for (const auto& nftId : nftIds) {
        if (!deleteNFTMetadata(nftId)) {
            success = false;
        }
    }
    return success;
}

std::vector<NFTManager::NFT> NFTStorage::searchNFTs(const std::string& query) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<NFTManager::NFT> results;

    // Search in owner index
    for (const auto& [owner, nftIds] : ownerIndex_) {
        if (owner.find(query) != std::string::npos) {
            for (const auto& nftId : nftIds) {
                if (auto nft = getNFT(nftId)) {
                    results.push_back(*nft);
                }
            }
        }
    }

    // Search in contract index
    for (const auto& [contract, nftIds] : contractIndex_) {
        if (contract.find(query) != std::string::npos) {
            for (const auto& nftId : nftIds) {
                if (auto nft = getNFT(nftId)) {
                    results.push_back(*nft);
                }
            }
        }
    }

    // Remove duplicates
    std::sort(results.begin(), results.end(), [](const NFTManager::NFT& a, const NFTManager::NFT& b) {
        return a.id < b.id;
    });
    results.erase(std::unique(results.begin(), results.end(), [](const NFTManager::NFT& a, const NFTManager::NFT& b) {
        return a.id == b.id;
    }), results.end());

    return results;
}

std::vector<NFTManager::NFT> NFTStorage::searchNFTsByMetadata(const nlohmann::json& metadataQuery) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<NFTManager::NFT> results;

    // Search through all NFTs
    for (const auto& [nftId, _] : nftCache_) {
        if (auto metadata = getNFTMetadata(nftId)) {
            bool matches = true;
            for (const auto& [key, value] : metadataQuery.items()) {
                if (!metadata->attributes.empty() && 
                    std::find(metadata->attributes.begin(), metadata->attributes.end(), value.get<std::string>()) == metadata->attributes.end()) {
                    matches = false;
                    break;
                }
            }
            if (matches) {
                if (auto nft = getNFT(nftId)) {
                    results.push_back(*nft);
                }
            }
        }
    }

    return results;
}

std::vector<NFTManager::NFT> NFTStorage::searchNFTsByAttributes(const std::map<std::string, std::string>& attributes) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<NFTManager::NFT> results;

    // Search through all NFTs
    for (const auto& [nftId, _] : nftCache_) {
        if (auto metadata = getNFTMetadata(nftId)) {
            bool matches = true;
            for (const auto& [key, value] : attributes) {
                if (std::find(metadata->attributes.begin(), metadata->attributes.end(), value) == metadata->attributes.end()) {
                    matches = false;
                    break;
                }
            }
            if (matches) {
                if (auto nft = getNFT(nftId)) {
                    results.push_back(*nft);
                }
            }
        }
    }

    return results;
}

void NFTStorage::enableCache(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    cacheEnabled_ = enable;
    if (!enable) {
        clearCache();
    }
}

void NFTStorage::clearCache() {
    std::lock_guard<std::mutex> lock(mutex_);
    nftCache_.clear();
    metadataCache_.clear();
}

void NFTStorage::setCacheSize(size_t maxSize) {
    std::lock_guard<std::mutex> lock(mutex_);
    maxCacheSize_ = maxSize;
}

size_t NFTStorage::getCacheSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return maxCacheSize_;
}

bool NFTStorage::createBackup(const std::string& backupPath) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = {3, "Storage not initialized"};
        return false;
    }

    try {
        std::filesystem::create_directories(backupPath);
        
        // Backup NFT files
        for (const auto& [nftId, _] : nftCache_) {
            std::string sourcePath = getNFTFilePath(nftId);
            std::string destPath = backupPath + "/" + std::filesystem::path(sourcePath).filename().string();
            std::filesystem::copy_file(sourcePath, destPath, std::filesystem::copy_options::overwrite_existing);
        }

        // Backup metadata files
        for (const auto& [nftId, _] : metadataCache_) {
            std::string sourcePath = getMetadataFilePath(nftId);
            std::string destPath = backupPath + "/" + std::filesystem::path(sourcePath).filename().string();
            std::filesystem::copy_file(sourcePath, destPath, std::filesystem::copy_options::overwrite_existing);
        }

        return true;
    } catch (const std::exception& e) {
        lastError_ = {10, std::string("Failed to create backup: ") + e.what()};
        return false;
    }
}

bool NFTStorage::restoreFromBackup(const std::string& backupPath) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = {3, "Storage not initialized"};
        return false;
    }

    try {
        // Clear existing data
        clearCache();
        for (const auto& entry : std::filesystem::directory_iterator(storagePath_)) {
            std::filesystem::remove(entry.path());
        }

        // Restore from backup
        for (const auto& entry : std::filesystem::directory_iterator(backupPath)) {
            std::string destPath = storagePath_ + "/" + entry.path().filename().string();
            std::filesystem::copy_file(entry.path(), destPath, std::filesystem::copy_options::overwrite_existing);
        }

        return true;
    } catch (const std::exception& e) {
        lastError_ = {11, std::string("Failed to restore from backup: ") + e.what()};
        return false;
    }
}

NFTStorage::Error NFTStorage::getLastError() const {
    return lastError_;
}

void NFTStorage::clearLastError() {
    lastError_ = {0, ""};
}

// Private helper methods
bool NFTStorage::validateStoragePath(const std::string& path) {
    try {
        return std::filesystem::exists(path) || std::filesystem::create_directories(path);
    } catch (const std::exception&) {
        return false;
    }
}

bool NFTStorage::createStorageDirectory(const std::string& path) {
    try {
        return std::filesystem::create_directories(path);
    } catch (const std::exception&) {
        return false;
    }
}

bool NFTStorage::writeToFile(const std::string& path, const std::string& data) {
    try {
        std::ofstream file(path);
        if (!file.is_open()) {
            lastError_ = {12, "Failed to open file for writing"};
            return false;
        }
        file << data;
        return true;
    } catch (const std::exception& e) {
        lastError_ = {13, std::string("Failed to write to file: ") + e.what()};
        return false;
    }
}

std::optional<std::string> NFTStorage::readFromFile(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            return std::nullopt;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

bool NFTStorage::deleteFile(const std::string& path) {
    try {
        return std::filesystem::remove(path);
    } catch (const std::exception&) {
        return false;
    }
}

bool NFTStorage::fileExists(const std::string& path) {
    try {
        return std::filesystem::exists(path);
    } catch (const std::exception&) {
        return false;
    }
}

std::string NFTStorage::getNFTFilePath(const std::string& nftId) {
    return storagePath_ + "/nft_" + nftId + ".json";
}

std::string NFTStorage::getMetadataFilePath(const std::string& nftId) {
    return storagePath_ + "/metadata_" + nftId + ".json";
}

bool NFTStorage::updateIndex(const std::string& nftId, const NFTManager::NFT& nft) {
    try {
        // Update owner index
        ownerIndex_[nft.owner].push_back(nftId);
        
        // Update contract index
        contractIndex_[nft.contractAddress].push_back(nftId);
        
        // Update creator index
        creatorIndex_[nft.creator].push_back(nftId);
        
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool NFTStorage::removeFromIndex(const std::string& nftId) {
    try {
        // Remove from owner index
        for (auto& [owner, nftIds] : ownerIndex_) {
            nftIds.erase(std::remove(nftIds.begin(), nftIds.end(), nftId), nftIds.end());
            if (nftIds.empty()) {
                ownerIndex_.erase(owner);
            }
        }
        
        // Remove from contract index
        for (auto& [contract, nftIds] : contractIndex_) {
            nftIds.erase(std::remove(nftIds.begin(), nftIds.end(), nftId), nftIds.end());
            if (nftIds.empty()) {
                contractIndex_.erase(contract);
            }
        }
        
        // Remove from creator index
        for (auto& [creator, nftIds] : creatorIndex_) {
            nftIds.erase(std::remove(nftIds.begin(), nftIds.end(), nftId), nftIds.end());
            if (nftIds.empty()) {
                creatorIndex_.erase(creator);
            }
        }
        
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

} // namespace nft
} // namespace satox 