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

#include "satox/core/nft_manager.hpp"
#include "satox/core/logging_manager.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace satox::core {

NFTManager& NFTManager::getInstance() {
    static NFTManager instance;
    return instance;
}

bool NFTManager::initialize(const nlohmann::json& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        lastError_ = "NFT Manager already initialized";
        return false;
    }

    if (!validateConfig(config)) {
        lastError_ = "Invalid configuration";
        return false;
    }

    initialized_ = true;
    statsEnabled_ = config.value("enableStats", false);
    return true;
}

void NFTManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return;
    }

    nfts_.clear();
    nftCallbacks_.clear();
    metadataCallbacks_.clear();
    errorCallbacks_.clear();
    initialized_ = false;
}

bool NFTManager::createNFT(const std::string& assetId, const NFTMetadata& metadata) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "NFT Manager not initialized";
        return false;
    }

    if (!validateAssetId(assetId)) {
        lastError_ = "Invalid asset ID";
        return false;
    }

    if (!validateMetadata(metadata)) {
        lastError_ = "Invalid metadata";
        return false;
    }

    if (!checkNFTLimit()) {
        lastError_ = "NFT limit reached";
        return false;
    }

    std::string nftId = "nft_" + assetId + "_" + std::to_string(nfts_.size());
    NFTManager::NFT nft{
        nftId,
        assetId,
        metadata,
        NFTState::CREATED,
        "",
        std::chrono::system_clock::now(),
        {},
        nlohmann::json::object()
    };

    nfts_[nftId] = nft;
    updateNFTStats(nftId, NFTState::CREATED);
    notifyNFTChange(nftId, NFTState::CREATED);
    return true;
}

bool NFTManager::mintNFT(const std::string& nftId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "NFT Manager not initialized";
        return false;
    }

    auto it = nfts_.find(nftId);
    if (it == nfts_.end()) {
        lastError_ = "NFT not found";
        return false;
    }

    if (it->second.state != NFTState::CREATED) {
        lastError_ = "Invalid NFT state for minting";
        return false;
    }

    it->second.state = NFTState::MINTED;
    it->second.mintTime = std::chrono::system_clock::now();
    it->second.history.push_back("Minted at " + std::to_string(
        std::chrono::system_clock::to_time_t(it->second.mintTime)));

    updateNFTStats(nftId, NFTState::MINTED);
    notifyNFTChange(nftId, NFTState::MINTED);
    return true;
}

bool NFTManager::transferNFT(const std::string& nftId, const std::string& newOwner) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "NFT Manager not initialized";
        return false;
    }

    auto it = nfts_.find(nftId);
    if (it == nfts_.end()) {
        lastError_ = "NFT not found";
        return false;
    }

    if (it->second.state != NFTState::MINTED) {
        lastError_ = "Invalid NFT state for transfer";
        return false;
    }

    if (!validateOwner(newOwner)) {
        lastError_ = "Invalid new owner";
        return false;
    }

    it->second.metadata.owner = newOwner;
    it->second.state = NFTState::TRANSFERRED;
    it->second.history.push_back("Transferred to " + newOwner + " at " + 
        std::to_string(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())));

    updateNFTStats(nftId, NFTState::TRANSFERRED);
    notifyNFTChange(nftId, NFTState::TRANSFERRED);
    notifyMetadataChange(nftId, it->second.metadata);
    return true;
}

bool NFTManager::burnNFT(const std::string& nftId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "NFT Manager not initialized";
        return false;
    }

    auto it = nfts_.find(nftId);
    if (it == nfts_.end()) {
        lastError_ = "NFT not found";
        return false;
    }

    if (it->second.state == NFTState::BURNED) {
        lastError_ = "NFT already burned";
        return false;
    }

    it->second.state = NFTState::BURNED;
    it->second.history.push_back("Burned at " + 
        std::to_string(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())));

    updateNFTStats(nftId, NFTState::BURNED);
    notifyNFTChange(nftId, NFTState::BURNED);
    return true;
}

bool NFTManager::updateNFTMetadata(const std::string& nftId, const NFTMetadata& metadata) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "NFTManager::NFT Manager not initialized";
        return false;
    }

    auto it = nfts_.find(nftId);
    if (it == nfts_.end()) {
        lastError_ = "NFTManager::NFT not found";
        return false;
    }

    if (!validateMetadata(metadata)) {
        lastError_ = "Invalid metadata";
        return false;
    }

    it->second.metadata = metadata;
    it->second.metadata.lastModified = std::chrono::system_clock::now();
    it->second.history.push_back("Metadata updated at " + 
        std::to_string(std::chrono::system_clock::to_time_t(it->second.metadata.lastModified)));

    notifyMetadataChange(nftId, metadata);
    return true;
}

bool NFTManager::isNFTExists(const std::string& nftId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return nfts_.find(nftId) != nfts_.end();
}

NFTManager::NFT NFTManager::getNFT(const std::string& nftId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = nfts_.find(nftId);
    if (it == nfts_.end()) {
        return NFT{};
    }
    return it->second;
}

std::vector<NFTManager::NFT> NFTManager::getNFTsByAsset(const std::string& assetId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<NFT> result;
    for (const auto& [id, nft] : nfts_) {
        if (nft.assetId == assetId) {
            result.push_back(nft);
        }
    }
    
    return result;
}

std::vector<NFTManager::NFT> NFTManager::getNFTsByOwner(const std::string& owner) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<NFT> result;
    for (const auto& [id, nft] : nfts_) {
        if (nft.metadata.owner == owner) {
            result.push_back(nft);
        }
    }
    
    return result;
}

std::vector<NFTManager::NFT> NFTManager::getNFTsByType(NFTType type) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<NFT> result;
    for (const auto& [id, nft] : nfts_) {
        if (nft.metadata.type == type) {
            result.push_back(nft);
        }
    }
    
    return result;
}

std::vector<NFTManager::NFT> NFTManager::getNFTsByState(NFTState state) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<NFT> result;
    for (const auto& [id, nft] : nfts_) {
        if (nft.state == state) {
            result.push_back(nft);
        }
    }
    
    return result;
}

bool NFTManager::validateMetadata(const NFTMetadata& metadata) const {
    if (metadata.name.empty() || metadata.creator.empty() || metadata.owner.empty()) {
        return false;
    }
    return true;
}

bool NFTManager::updateNFTURI(const std::string& nftId, const std::string& uri) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = nfts_.find(nftId);
    if (it == nfts_.end()) {
        lastError_ = "NFTManager::NFT not found";
        return false;
    }

    it->second.metadata.uri = uri;
    it->second.metadata.lastModified = std::chrono::system_clock::now();
    notifyMetadataChange(nftId, it->second.metadata);
    return true;
}

bool NFTManager::updateNFTHash(const std::string& nftId, const std::string& hash) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = nfts_.find(nftId);
    if (it == nfts_.end()) {
        lastError_ = "NFTManager::NFT not found";
        return false;
    }

    it->second.metadata.hash = hash;
    it->second.metadata.lastModified = std::chrono::system_clock::now();
    notifyMetadataChange(nftId, it->second.metadata);
    return true;
}

bool NFTManager::addNFTAttribute(const std::string& nftId, const std::string& key, const nlohmann::json& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = nfts_.find(nftId);
    if (it == nfts_.end()) {
        lastError_ = "NFTManager::NFT not found";
        return false;
    }

    it->second.metadata.attributes[key] = value;
    it->second.metadata.lastModified = std::chrono::system_clock::now();
    notifyMetadataChange(nftId, it->second.metadata);
    return true;
}

bool NFTManager::removeNFTAttribute(const std::string& nftId, const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = nfts_.find(nftId);
    if (it == nfts_.end()) {
        lastError_ = "NFTManager::NFT not found";
        return false;
    }

    it->second.metadata.attributes.erase(key);
    it->second.metadata.lastModified = std::chrono::system_clock::now();
    notifyMetadataChange(nftId, it->second.metadata);
    return true;
}

std::vector<std::string> NFTManager::getNFTHistory(const std::string& nftId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = nfts_.find(nftId);
    if (it == nfts_.end()) {
        return {};
    }
    return it->second.history;
}

bool NFTManager::addNFTHistoryEntry(const std::string& nftId, const std::string& entry) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = nfts_.find(nftId);
    if (it == nfts_.end()) {
        lastError_ = "NFTManager::NFT not found";
        return false;
    }

    it->second.history.push_back(entry);
    return true;
}

bool NFTManager::clearNFTHistory(const std::string& nftId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = nfts_.find(nftId);
    if (it == nfts_.end()) {
        lastError_ = "NFTManager::NFT not found";
        return false;
    }

    it->second.history.clear();
    return true;
}

NFTManager::NFTStats NFTManager::getStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

void NFTManager::resetStats() {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_ = NFTStats{};
}

bool NFTManager::enableStats(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    statsEnabled_ = enable;
    return true;
}

void NFTManager::registerNFTCallback(NFTCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    nftCallbacks_["all"].push_back(callback);
}

void NFTManager::registerMetadataCallback(NFTMetadataCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    metadataCallbacks_["all"].push_back(callback);
}

void NFTManager::registerErrorCallback(NFTErrorCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    errorCallbacks_.push_back(callback);
}

void NFTManager::unregisterNFTCallback() {
    std::lock_guard<std::mutex> lock(mutex_);
    nftCallbacks_.clear();
}

void NFTManager::unregisterMetadataCallback() {
    std::lock_guard<std::mutex> lock(mutex_);
    metadataCallbacks_.clear();
}

void NFTManager::unregisterErrorCallback() {
    std::lock_guard<std::mutex> lock(mutex_);
    errorCallbacks_.clear();
}

std::string NFTManager::getLastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastError_;
}

void NFTManager::clearLastError() {
    std::lock_guard<std::mutex> lock(mutex_);
    lastError_.clear();
}

// Private helper methods
bool NFTManager::validateConfig(const nlohmann::json& config) const {
    return true; // Add validation logic as needed
}

bool NFTManager::validateNFTId(const std::string& nftId) const {
    return !nftId.empty() && nftId.length() <= 64;
}

bool NFTManager::validateAssetId(const std::string& assetId) const {
    return !assetId.empty() && assetId.length() <= 64;
}

bool NFTManager::validateOwner(const std::string& owner) const {
    return !owner.empty() && owner.length() <= 64;
}

bool NFTManager::checkNFTLimit() const {
    return nfts_.size() < 1000000; // Add configurable limit
}

void NFTManager::updateNFTState(const std::string& nftId, NFTState state) {
    auto it = nfts_.find(nftId);
    if (it != nfts_.end()) {
        it->second.state = state;
    }
}

void NFTManager::updateNFTStats(const std::string& nftId, NFTState state) {
    if (!statsEnabled_) {
        return;
    }

    stats_.totalNFTs = nfts_.size();
    stats_.mintedNFTs = std::count_if(nfts_.begin(), nfts_.end(),
        [](const auto& pair) { return pair.second.state == NFTState::MINTED; });
    stats_.transferredNFTs = std::count_if(nfts_.begin(), nfts_.end(),
        [](const auto& pair) { return pair.second.state == NFTState::TRANSFERRED; });
    stats_.burnedNFTs = std::count_if(nfts_.begin(), nfts_.end(),
        [](const auto& pair) { return pair.second.state == NFTState::BURNED; });
    stats_.errorNFTs = std::count_if(nfts_.begin(), nfts_.end(),
        [](const auto& pair) { return pair.second.state == NFTState::ERROR; });

    // Update type distribution
    stats_.typeDistribution = nlohmann::json::object();
    for (const auto& [id, nft] : nfts_) {
        std::string typeStr = std::to_string(static_cast<int>(nft.metadata.type));
        stats_.typeDistribution[typeStr] = stats_.typeDistribution.value(typeStr, 0) + 1;
    }
}

void NFTManager::notifyNFTChange(const std::string& nftId, NFTState state) {
    for (const auto& callback : nftCallbacks_["all"]) {
        try {
            callback(nftId, state);
        } catch (const std::exception& e) {
            notifyError(nftId, e.what());
        }
    }
}

void NFTManager::notifyMetadataChange(const std::string& nftId, const NFTMetadata& metadata) {
    for (const auto& callback : metadataCallbacks_["all"]) {
        try {
            callback(nftId, metadata);
        } catch (const std::exception& e) {
            notifyError(nftId, e.what());
        }
    }
}

void NFTManager::notifyError(const std::string& nftId, const std::string& error) {
    for (const auto& callback : errorCallbacks_) {
        try {
            callback(nftId, error);
        } catch (...) {
            // Ignore errors in error callbacks
        }
    }
}

} // namespace satox::core 