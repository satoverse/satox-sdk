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

#include "satox/core/asset_manager.hpp"
#include "satox/core/logging_manager.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace satox::core {

AssetManager& AssetManager::getInstance() {
    static AssetManager instance;
    return instance;
}

bool AssetManager::initialize(const nlohmann::json& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        lastError_ = "Asset Manager already initialized";
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

void AssetManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return;
    }

    assets_.clear();
    assetCallbacks_.clear();
    metadataCallbacks_.clear();
    errorCallbacks_.clear();
    initialized_ = false;
}

bool AssetManager::createAsset(const std::string& id, const AssetMetadata& metadata) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Asset Manager not initialized";
        return false;
    }

    if (!validateAssetId(id)) {
        lastError_ = "Invalid asset ID";
        return false;
    }

    if (!validateMetadata(metadata)) {
        lastError_ = "Invalid metadata";
        return false;
    }

    if (!checkAssetLimit()) {
        lastError_ = "Asset limit reached";
        return false;
    }

    AssetManager::Asset asset{
        id,
        metadata,
        AssetState::CREATED,
        "",
        std::chrono::system_clock::now(),
        {},
        nlohmann::json::object()
    };

    assets_[id] = asset;
    updateAssetStats(id, AssetState::CREATED);
    notifyAssetChange(id, AssetState::CREATED);
    return true;
}

bool AssetManager::issueAsset(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Asset Manager not initialized";
        return false;
    }

    auto it = assets_.find(id);
    if (it == assets_.end()) {
        lastError_ = "Asset not found";
        return false;
    }

    if (it->second.state != AssetState::CREATED) {
        lastError_ = "Invalid asset state for issuing";
        return false;
    }

    it->second.state = AssetState::ISSUED;
    it->second.issueTime = std::chrono::system_clock::now();
    it->second.history.push_back("Issued at " + std::to_string(
        std::chrono::system_clock::to_time_t(it->second.issueTime)));

    updateAssetStats(id, AssetState::ISSUED);
    notifyAssetChange(id, AssetState::ISSUED);
    return true;
}

bool AssetManager::transferAsset(const std::string& id, const std::string& newOwner) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Asset Manager not initialized";
        return false;
    }

    auto it = assets_.find(id);
    if (it == assets_.end()) {
        lastError_ = "Asset not found";
        return false;
    }

    if (it->second.state != AssetState::ISSUED) {
        lastError_ = "Invalid asset state for transfer";
        return false;
    }

    if (!validateOwner(newOwner)) {
        lastError_ = "Invalid new owner";
        return false;
    }

    it->second.metadata.owner = newOwner;
    it->second.state = AssetState::TRANSFERRED;
    it->second.history.push_back("Transferred to " + newOwner + " at " + 
        std::to_string(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())));

    updateAssetStats(id, AssetState::TRANSFERRED);
    notifyAssetChange(id, AssetState::TRANSFERRED);
    notifyMetadataChange(id, it->second.metadata);
    return true;
}

bool AssetManager::burnAsset(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Asset Manager not initialized";
        return false;
    }

    auto it = assets_.find(id);
    if (it == assets_.end()) {
        lastError_ = "Asset not found";
        return false;
    }

    if (it->second.state == AssetState::BURNED) {
        lastError_ = "Asset already burned";
        return false;
    }

    it->second.state = AssetState::BURNED;
    it->second.history.push_back("Burned at " + 
        std::to_string(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())));

    updateAssetStats(id, AssetState::BURNED);
    notifyAssetChange(id, AssetState::BURNED);
    return true;
}

bool AssetManager::updateAssetMetadata(const std::string& id, const AssetMetadata& metadata) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "AssetManager::Asset Manager not initialized";
        return false;
    }

    auto it = assets_.find(id);
    if (it == assets_.end()) {
        lastError_ = "AssetManager::Asset not found";
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

    notifyMetadataChange(id, metadata);
    return true;
}

bool AssetManager::isAssetExists(const std::string& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return assets_.find(id) != assets_.end();
}

AssetManager::Asset AssetManager::getAsset(const std::string& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = assets_.find(id);
    if (it == assets_.end()) {
        return Asset{};
    }
    return it->second;
}

std::vector<AssetManager::Asset> AssetManager::getAssetsByType(AssetType type) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<AssetManager::Asset> result;
    for (const auto& [id, asset] : assets_) {
        if (asset.metadata.type == type) {
            result.push_back(asset);
        }
    }
    return result;
}

std::vector<AssetManager::Asset> AssetManager::getAssetsByOwner(const std::string& owner) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<AssetManager::Asset> result;
    for (const auto& [id, asset] : assets_) {
        if (asset.metadata.owner == owner) {
            result.push_back(asset);
        }
    }
    return result;
}

std::vector<AssetManager::Asset> AssetManager::getAssetsByState(AssetState state) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<AssetManager::Asset> result;
    for (const auto& [id, asset] : assets_) {
        if (asset.state == state) {
            result.push_back(asset);
        }
    }
    return result;
}

bool AssetManager::validateMetadata(const AssetMetadata& metadata) const {
    if (metadata.name.empty() || metadata.symbol.empty() || metadata.issuer.empty() || metadata.owner.empty()) {
        return false;
    }
    return true;
}

bool AssetManager::updateAssetURI(const std::string& id, const std::string& uri) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = assets_.find(id);
    if (it == assets_.end()) {
        lastError_ = "AssetManager::Asset not found";
        return false;
    }

    it->second.metadata.uri = uri;
    it->second.metadata.lastModified = std::chrono::system_clock::now();
    notifyMetadataChange(id, it->second.metadata);
    return true;
}

bool AssetManager::updateAssetHash(const std::string& id, const std::string& hash) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = assets_.find(id);
    if (it == assets_.end()) {
        lastError_ = "AssetManager::Asset not found";
        return false;
    }

    it->second.metadata.hash = hash;
    it->second.metadata.lastModified = std::chrono::system_clock::now();
    notifyMetadataChange(id, it->second.metadata);
    return true;
}

bool AssetManager::addAssetAttribute(const std::string& id, const std::string& key, const nlohmann::json& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = assets_.find(id);
    if (it == assets_.end()) {
        lastError_ = "AssetManager::Asset not found";
        return false;
    }

    it->second.metadata.attributes[key] = value;
    it->second.metadata.lastModified = std::chrono::system_clock::now();
    notifyMetadataChange(id, it->second.metadata);
    return true;
}

bool AssetManager::removeAssetAttribute(const std::string& id, const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = assets_.find(id);
    if (it == assets_.end()) {
        lastError_ = "AssetManager::Asset not found";
        return false;
    }

    it->second.metadata.attributes.erase(key);
    it->second.metadata.lastModified = std::chrono::system_clock::now();
    notifyMetadataChange(id, it->second.metadata);
    return true;
}

std::vector<std::string> AssetManager::getAssetHistory(const std::string& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = assets_.find(id);
    if (it == assets_.end()) {
        return {};
    }
    return it->second.history;
}

bool AssetManager::addAssetHistoryEntry(const std::string& id, const std::string& entry) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = assets_.find(id);
    if (it == assets_.end()) {
        lastError_ = "AssetManager::Asset not found";
        return false;
    }

    it->second.history.push_back(entry);
    return true;
}

bool AssetManager::clearAssetHistory(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = assets_.find(id);
    if (it == assets_.end()) {
        lastError_ = "AssetManager::Asset not found";
        return false;
    }

    it->second.history.clear();
    return true;
}

AssetManager::AssetStats AssetManager::getStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

void AssetManager::resetStats() {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_ = AssetStats{};
}

bool AssetManager::enableStats(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    statsEnabled_ = enable;
    return true;
}

void AssetManager::registerAssetCallback(AssetCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    assetCallbacks_["all"].push_back(callback);
}

void AssetManager::registerMetadataCallback(AssetMetadataCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    metadataCallbacks_["all"].push_back(callback);
}

void AssetManager::registerErrorCallback(AssetErrorCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    errorCallbacks_.push_back(callback);
}

void AssetManager::unregisterAssetCallback() {
    std::lock_guard<std::mutex> lock(mutex_);
    assetCallbacks_.clear();
}

void AssetManager::unregisterMetadataCallback() {
    std::lock_guard<std::mutex> lock(mutex_);
    metadataCallbacks_.clear();
}

void AssetManager::unregisterErrorCallback() {
    std::lock_guard<std::mutex> lock(mutex_);
    errorCallbacks_.clear();
}

std::string AssetManager::getLastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastError_;
}

void AssetManager::clearLastError() {
    std::lock_guard<std::mutex> lock(mutex_);
    lastError_.clear();
}

// Private helper methods
bool AssetManager::validateConfig(const nlohmann::json& config) const {
    return true; // Add validation logic as needed
}

bool AssetManager::validateAssetId(const std::string& id) const {
    return !id.empty() && id.length() <= 64;
}

bool AssetManager::validateOwner(const std::string& owner) const {
    return !owner.empty() && owner.length() <= 64;
}

bool AssetManager::checkAssetLimit() const {
    return assets_.size() < 1000000; // Add configurable limit
}

void AssetManager::updateAssetState(const std::string& id, AssetState state) {
    auto it = assets_.find(id);
    if (it != assets_.end()) {
        it->second.state = state;
    }
}

void AssetManager::updateAssetStats(const std::string& id, AssetState state) {
    if (!statsEnabled_) {
        return;
    }

    stats_.totalAssets = assets_.size();
    stats_.issuedAssets = std::count_if(assets_.begin(), assets_.end(),
        [](const auto& pair) { return pair.second.state == AssetState::ISSUED; });
    stats_.transferredAssets = std::count_if(assets_.begin(), assets_.end(),
        [](const auto& pair) { return pair.second.state == AssetState::TRANSFERRED; });
    stats_.burnedAssets = std::count_if(assets_.begin(), assets_.end(),
        [](const auto& pair) { return pair.second.state == AssetState::BURNED; });
    stats_.errorAssets = std::count_if(assets_.begin(), assets_.end(),
        [](const auto& pair) { return pair.second.state == AssetState::ERROR; });

    // Update type distribution
    stats_.typeDistribution = nlohmann::json::object();
    for (const auto& [id, asset] : assets_) {
        std::string typeStr = std::to_string(static_cast<int>(asset.metadata.type));
        stats_.typeDistribution[typeStr] = stats_.typeDistribution.value(typeStr, 0) + 1;
    }
}

void AssetManager::notifyAssetChange(const std::string& id, AssetState state) {
    for (const auto& callback : assetCallbacks_["all"]) {
        try {
            callback(id, state);
        } catch (const std::exception& e) {
            notifyError(id, e.what());
        }
    }
}

void AssetManager::notifyMetadataChange(const std::string& id, const AssetMetadata& metadata) {
    for (const auto& callback : metadataCallbacks_["all"]) {
        try {
            callback(id, metadata);
        } catch (const std::exception& e) {
            notifyError(id, e.what());
        }
    }
}

void AssetManager::notifyError(const std::string& id, const std::string& error) {
    for (const auto& callback : errorCallbacks_) {
        try {
            callback(id, error);
        } catch (...) {
            // Ignore errors in error callbacks
        }
    }
}

} // namespace satox::core 