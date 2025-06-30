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

#include "satox/assets/asset_types.hpp"
#include "satox/assets/asset_manager.hpp"
#include <chrono>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <openssl/evp.h>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <queue>
#include <condition_variable>
#include <functional>

namespace satox {
namespace assets {

class AssetManagerImpl {
public:
    AssetManagerImpl() : initialized_(false), cache_size_(0), max_cache_size_(1000) {
        cache_.reserve(max_cache_size_);
    }

    bool initialize(const nlohmann::json& config) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (initialized_) {
            setLastError("AssetManager already initialized");
            return false;
        }

        try {
            // Parse configuration
            if (config.contains("max_cache_size")) {
                max_cache_size_ = config["max_cache_size"].get<size_t>();
                cache_.reserve(max_cache_size_);
            } else if (config.contains("cache_size")) {
                max_cache_size_ = config["cache_size"].get<size_t>();
                cache_.reserve(max_cache_size_);
            }

            // Start background cache cleanup thread
            cleanup_thread_ = std::thread(&AssetManagerImpl::cleanupCache, this);

            initialized_ = true;
            return true;
        } catch (const std::exception& e) {
            setLastError(std::string("Failed to initialize: ") + e.what());
            return false;
        }
    }

    void shutdown() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!initialized_) {
            return;
        }

        // Signal cleanup thread to stop
        {
            std::lock_guard<std::mutex> cleanup_lock(cleanup_mutex_);
            should_stop_ = true;
        }
        cleanup_cv_.notify_one();
        if (cleanup_thread_.joinable()) {
            cleanup_thread_.join();
        }

        assets_.clear();
        cache_.clear();
        initialized_ = false;
    }

    bool createAsset(const AssetMetadata& metadata, std::string& assetId) {
        if (!initialized_) {
            setLastError("AssetManager not initialized");
            return false;
        }

        std::lock_guard<std::mutex> lock(mutex_);

        try {
            // Validate metadata
            if (!validateMetadata(metadata)) {
                return false;
            }

            // Generate asset ID
            assetId = generateAssetId(metadata);

            // Create asset
            Asset asset;
            asset.id = assetId;
            asset.metadata = metadata;
            asset.status = "active";
            asset.isLocked = false;
            asset.createdAt = std::chrono::system_clock::now();
            asset.updatedAt = asset.createdAt;

            // Store asset
            assets_[assetId] = asset;

            // Add to cache
            addToCache(assetId, asset);

            // Notify listeners
            notifyListeners(AssetEventType::CREATED, asset);

            return true;
        } catch (const std::exception& e) {
            setLastError(std::string("Failed to create asset: ") + e.what());
            return false;
        }
    }

    bool updateAsset(const std::string& assetId, const AssetMetadata& metadata) {
        if (!initialized_) {
            setLastError("AssetManager not initialized");
            return false;
        }

        std::lock_guard<std::mutex> lock(mutex_);

        try {
            // Validate metadata
            if (!validateMetadata(metadata)) {
                return false;
            }

            auto it = assets_.find(assetId);
            if (it == assets_.end()) {
                setLastError("Asset not found");
                return false;
            }

            // Update asset
            it->second.metadata = metadata;
            it->second.updatedAt = std::chrono::system_clock::now();

            // Update cache
            updateCache(assetId, it->second);

            // Notify listeners
            notifyListeners(AssetEventType::UPDATED, it->second);

            return true;
        } catch (const std::exception& e) {
            setLastError(std::string("Failed to update asset: ") + e.what());
            return false;
        }
    }

    bool deleteAsset(const std::string& assetId) {
        if (!initialized_) {
            setLastError("AssetManager not initialized");
            return false;
        }

        std::lock_guard<std::mutex> lock(mutex_);

        try {
            auto it = assets_.find(assetId);
            if (it == assets_.end()) {
                setLastError("Asset not found");
                return false;
            }

            // Get asset info before removal
            const auto& asset = it->second;

            // Remove asset
            assets_.erase(it);

            // Remove from cache
            removeFromCache(assetId);

            // Notify listeners
            notifyListeners(AssetEventType::DELETED, asset);

            return true;
        } catch (const std::exception& e) {
            setLastError(std::string("Failed to delete asset: ") + e.what());
            return false;
        }
    }

    bool getAsset(const std::string& assetId, Asset& asset) {
        if (!initialized_) {
            setLastError("AssetManager not initialized");
            return false;
        }

        std::lock_guard<std::mutex> lock(mutex_);

        try {
            // Try cache first
            auto cache_it = cache_.find(assetId);
            if (cache_it != cache_.end()) {
                asset = cache_it->second.asset;
                return true;
            }

            // Try storage
            auto it = assets_.find(assetId);
            if (it == assets_.end()) {
                setLastError("Asset not found");
                return false;
            }

            asset = it->second;
            addToCache(assetId, asset);
            return true;
        } catch (const std::exception& e) {
            setLastError(std::string("Failed to get asset: ") + e.what());
            return false;
        }
    }

    bool listAssets(std::vector<Asset>& assets, const nlohmann::json& filters) {
        if (!initialized_) {
            setLastError("AssetManager not initialized");
            return false;
        }

        std::lock_guard<std::mutex> lock(mutex_);

        try {
            assets.clear();
            for (const auto& pair : assets_) {
                if (matchesFilters(pair.second, filters)) {
                    assets.push_back(pair.second);
                }
            }
            return true;
        } catch (const std::exception& e) {
            setLastError(std::string("Failed to list assets: ") + e.what());
            return false;
        }
    }

    bool transferAsset(const std::string& assetId, const std::string& from, const std::string& to) {
        if (!initialized_) {
            setLastError("AssetManager not initialized");
            return false;
        }

        std::lock_guard<std::mutex> lock(mutex_);

        try {
            auto it = assets_.find(assetId);
            if (it == assets_.end()) {
                setLastError("Asset not found");
                return false;
            }

            if (it->second.isLocked) {
                setLastError("Asset is locked");
                return false;
            }

            if (it->second.metadata.owner != from) {
                setLastError("Invalid owner");
                return false;
            }

            // Update asset
            it->second.metadata.owner = to;
            it->second.updatedAt = std::chrono::system_clock::now();

            // Update cache
            updateCache(assetId, it->second);

            // Notify listeners
            notifyListeners(AssetEventType::TRANSFERRED, it->second);

            return true;
        } catch (const std::exception& e) {
            setLastError(std::string("Failed to transfer asset: ") + e.what());
            return false;
        }
    }

    bool lockAsset(const std::string& assetId) {
        if (!initialized_) {
            setLastError("AssetManager not initialized");
            return false;
        }

        std::lock_guard<std::mutex> lock(mutex_);

        try {
            auto it = assets_.find(assetId);
            if (it == assets_.end()) {
                setLastError("Asset not found");
                return false;
            }

            if (it->second.isLocked) {
                setLastError("Asset is already locked");
                return false;
            }

            // Update asset
            it->second.isLocked = true;
            it->second.updatedAt = std::chrono::system_clock::now();

            // Update cache
            updateCache(assetId, it->second);

            // Notify listeners
            notifyListeners(AssetEventType::LOCKED, it->second);

            return true;
        } catch (const std::exception& e) {
            setLastError(std::string("Failed to lock asset: ") + e.what());
            return false;
        }
    }

    bool unlockAsset(const std::string& assetId) {
        if (!initialized_) {
            setLastError("AssetManager not initialized");
            return false;
        }

        std::lock_guard<std::mutex> lock(mutex_);

        try {
            auto it = assets_.find(assetId);
            if (it == assets_.end()) {
                setLastError("Asset not found");
                return false;
            }

            if (!it->second.isLocked) {
                setLastError("Asset is not locked");
                return false;
            }

            // Update asset
            it->second.isLocked = false;
            it->second.updatedAt = std::chrono::system_clock::now();

            // Update cache
            updateCache(assetId, it->second);

            // Notify listeners
            notifyListeners(AssetEventType::UNLOCKED, it->second);

            return true;
        } catch (const std::exception& e) {
            setLastError(std::string("Failed to unlock asset: ") + e.what());
            return false;
        }
    }

    bool burnAsset(const std::string& assetId, uint64_t amount) {
        if (!initialized_) {
            setLastError("AssetManager not initialized");
            return false;
        }

        std::lock_guard<std::mutex> lock(mutex_);

        try {
            auto it = assets_.find(assetId);
            if (it == assets_.end()) {
                setLastError("Asset not found");
                return false;
            }

            if (it->second.isLocked) {
                setLastError("Asset is locked");
                return false;
            }

            if (it->second.metadata.totalSupply < amount) {
                setLastError("Insufficient supply");
                return false;
            }

            // Update asset
            it->second.metadata.totalSupply -= amount;
            it->second.updatedAt = std::chrono::system_clock::now();

            // Update cache
            updateCache(assetId, it->second);

            // Notify listeners
            notifyListeners(AssetEventType::BURNED, it->second);

            return true;
        } catch (const std::exception& e) {
            setLastError(std::string("Failed to burn asset: ") + e.what());
            return false;
        }
    }

    bool mintAsset(const std::string& assetId, uint64_t amount) {
        if (!initialized_) {
            setLastError("AssetManager not initialized");
            return false;
        }

        std::lock_guard<std::mutex> lock(mutex_);

        try {
            auto it = assets_.find(assetId);
            if (it == assets_.end()) {
                setLastError("Asset not found");
                return false;
            }

            if (it->second.isLocked) {
                setLastError("Asset is locked");
                return false;
            }

            // Update asset
            it->second.metadata.totalSupply += amount;
            it->second.updatedAt = std::chrono::system_clock::now();

            // Update cache
            updateCache(assetId, it->second);

            // Notify listeners
            notifyListeners(AssetEventType::MINTED, it->second);

            return true;
        } catch (const std::exception& e) {
            setLastError(std::string("Failed to mint asset: ") + e.what());
            return false;
        }
    }

    bool searchAssets(const std::string& query, std::vector<Asset>& results) {
        if (!initialized_) {
            setLastError("AssetManager not initialized");
            return false;
        }

        std::lock_guard<std::mutex> lock(mutex_);

        try {
            results.clear();
            for (const auto& pair : assets_) {
                const auto& asset = pair.second;
                if (asset.metadata.name.find(query) != std::string::npos ||
                    asset.metadata.symbol.find(query) != std::string::npos ||
                    asset.metadata.creator.find(query) != std::string::npos) {
                    results.push_back(asset);
                }
            }
            return true;
        } catch (const std::exception& e) {
            setLastError(std::string("Failed to search assets: ") + e.what());
            return false;
        }
    }

    bool setAssetPermissions(const std::string& assetId, const nlohmann::json& permissions) {
        if (!initialized_) {
            setLastError("AssetManager not initialized");
            return false;
        }

        std::lock_guard<std::mutex> lock(mutex_);

        try {
            auto it = assets_.find(assetId);
            if (it == assets_.end()) {
                setLastError("Asset not found");
                return false;
            }

            // Update asset
            it->second.metadata.permissions = permissions;
            it->second.updatedAt = std::chrono::system_clock::now();

            // Update cache
            updateCache(assetId, it->second);

            // Notify listeners
            notifyListeners(AssetEventType::PERMISSIONS_UPDATED, it->second);

            return true;
        } catch (const std::exception& e) {
            setLastError(std::string("Failed to set asset permissions: ") + e.what());
            return false;
        }
    }

    bool getAssetPermissions(const std::string& assetId, nlohmann::json& permissions) {
        if (!initialized_) {
            setLastError("AssetManager not initialized");
            return false;
        }

        std::lock_guard<std::mutex> lock(mutex_);

        try {
            auto it = assets_.find(assetId);
            if (it == assets_.end()) {
                setLastError("Asset not found");
                return false;
            }

            permissions = it->second.metadata.permissions;
            return true;
        } catch (const std::exception& e) {
            setLastError(std::string("Failed to get asset permissions: ") + e.what());
            return false;
        }
    }

    bool validateAsset(const std::string& assetId) {
        if (!initialized_) {
            setLastError("AssetManager not initialized");
            return false;
        }

        std::lock_guard<std::mutex> lock(mutex_);

        try {
            auto it = assets_.find(assetId);
            if (it == assets_.end()) {
                setLastError("Asset not found");
                return false;
            }

            return validateMetadata(it->second.metadata);
        } catch (const std::exception& e) {
            setLastError(std::string("Failed to validate asset: ") + e.what());
            return false;
        }
    }

    bool validateTransfer(const std::string& assetId, const std::string& from, const std::string& to) {
        if (!initialized_) {
            setLastError("AssetManager not initialized");
            return false;
        }

        std::lock_guard<std::mutex> lock(mutex_);

        try {
            auto it = assets_.find(assetId);
            if (it == assets_.end()) {
                setLastError("Asset not found");
                return false;
            }

            if (it->second.isLocked) {
                setLastError("Asset is locked");
                return false;
            }

            if (it->second.metadata.owner != from) {
                setLastError("Invalid owner");
                return false;
            }

            return true;
        } catch (const std::exception& e) {
            setLastError(std::string("Failed to validate transfer: ") + e.what());
            return false;
        }
    }

    bool getAssetHistory(const std::string& assetId, std::vector<nlohmann::json>& history) {
        if (!initialized_) {
            setLastError("AssetManager not initialized");
            return false;
        }

        std::lock_guard<std::mutex> lock(mutex_);

        try {
            auto it = assets_.find(assetId);
            if (it == assets_.end()) {
                setLastError("Asset not found");
                return false;
            }

            history = it->second.history;
            return true;
        } catch (const std::exception& e) {
            setLastError(std::string("Failed to get asset history: ") + e.what());
            return false;
        }
    }

    bool addAssetHistory(const std::string& assetId, const nlohmann::json& event) {
        if (!initialized_) {
            setLastError("AssetManager not initialized");
            return false;
        }

        std::lock_guard<std::mutex> lock(mutex_);

        try {
            auto it = assets_.find(assetId);
            if (it == assets_.end()) {
                setLastError("Asset not found");
                return false;
            }

            it->second.history.push_back(event);
            notifyListeners(AssetEventType::HISTORY_UPDATED, it->second);
            return true;
        } catch (const std::exception& e) {
            setLastError(std::string("Failed to add asset history: ") + e.what());
            return false;
        }
    }

    bool searchByMetadata(const nlohmann::json& metadata, std::vector<Asset>& results) {
        if (!initialized_) {
            setLastError("AssetManager not initialized");
            return false;
        }

        std::lock_guard<std::mutex> lock(mutex_);

        try {
            results.clear();
            for (const auto& pair : assets_) {
                const auto& asset = pair.second;
                bool matches = true;
                for (auto it = metadata.begin(); it != metadata.end(); ++it) {
                    if (asset.metadata.permissions[it.key()] != it.value()) {
                        matches = false;
                        break;
                    }
                }
                if (matches) {
                    results.push_back(asset);
                }
            }
            return true;
        } catch (const std::exception& e) {
            setLastError(std::string("Failed to search by metadata: ") + e.what());
            return false;
        }
    }

    bool checkAssetPermission(const std::string& assetId, const std::string& address, const std::string& permission) {
        if (!initialized_) {
            setLastError("AssetManager not initialized");
            return false;
        }

        std::lock_guard<std::mutex> lock(mutex_);

        try {
            auto it = assets_.find(assetId);
            if (it == assets_.end()) {
                setLastError("Asset not found");
                return false;
            }

            const auto& permissions = it->second.metadata.permissions;
            if (!permissions.contains(permission)) {
                return false;
            }

            const auto& allowed = permissions[permission];
            if (allowed.is_array()) {
                for (const auto& addr : allowed) {
                    if (addr == address) {
                        return true;
                    }
                }
            } else if (allowed.is_string()) {
                return allowed == address;
            }

            return false;
        } catch (const std::exception& e) {
            setLastError(std::string("Failed to check asset permission: ") + e.what());
            return false;
        }
    }

    void addEventListener(AssetEventListener* listener) {
        std::lock_guard<std::mutex> lock(mutex_);
        listeners_.push_back(listener);
    }

    void removeEventListener(AssetEventListener* listener) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = std::find(listeners_.begin(), listeners_.end(), listener);
        if (it != listeners_.end()) {
            listeners_.erase(it);
        }
    }

    std::string getLastError() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return lastError_;
    }

    void registerAssetCallback(AssetManager::AssetCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        assetCallbacks_.push_back(callback);
    }

    void registerErrorCallback(AssetManager::ErrorCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        errorCallbacks_.push_back(callback);
    }

    void clearLastError() {
        std::lock_guard<std::mutex> lock(mutex_);
        lastError_.clear();
    }

private:
    struct CacheEntry {
        Asset asset;
        std::chrono::system_clock::time_point lastAccessed;
    };

    std::string generateAssetId(const AssetMetadata& metadata) {
        std::stringstream ss;
        ss << metadata.name << metadata.symbol << metadata.creator << metadata.totalSupply;
        
        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int hash_len;
        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
        EVP_DigestUpdate(ctx, ss.str().c_str(), ss.str().length());
        EVP_DigestFinal_ex(ctx, hash, &hash_len);
        EVP_MD_CTX_free(ctx);

        std::stringstream hex;
        for (unsigned int i = 0; i < hash_len; i++) {
            hex << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
        }
        return hex.str();
    }

    bool validateMetadata(const AssetMetadata& metadata) {
        if (metadata.name.empty()) {
            setLastError("Asset name cannot be empty");
            return false;
        }

        if (metadata.symbol.empty()) {
            setLastError("Asset symbol cannot be empty");
            return false;
        }

        if (metadata.creator.empty()) {
            setLastError("Asset creator cannot be empty");
            return false;
        }

        if (metadata.totalSupply == 0) {
            setLastError("Asset total supply must be greater than 0");
            return false;
        }

        return true;
    }

    bool matchesFilters(const Asset& asset, const nlohmann::json& filters) {
        for (auto it = filters.begin(); it != filters.end(); ++it) {
            if (asset.metadata.permissions[it.key()] != it.value()) {
                return false;
            }
        }
        return true;
    }

    void addToCache(const std::string& assetId, const Asset& asset) {
        if (cache_.size() >= max_cache_size_) {
            // Remove least recently used entry
            auto oldest = cache_.begin();
            for (auto it = cache_.begin(); it != cache_.end(); ++it) {
                if (it->second.lastAccessed < oldest->second.lastAccessed) {
                    oldest = it;
                }
            }
            cache_.erase(oldest);
        }

        CacheEntry entry;
        entry.asset = asset;
        entry.lastAccessed = std::chrono::system_clock::now();
        cache_[assetId] = entry;
    }

    void updateCache(const std::string& assetId, const Asset& asset) {
        auto it = cache_.find(assetId);
        if (it != cache_.end()) {
            it->second.asset = asset;
            it->second.lastAccessed = std::chrono::system_clock::now();
        }
    }

    void removeFromCache(const std::string& assetId) {
        cache_.erase(assetId);
    }

    void cleanupCache() {
        while (true) {
            std::unique_lock<std::mutex> lock(cleanup_mutex_);
            cleanup_cv_.wait_for(lock, std::chrono::minutes(5), [this] { return should_stop_; });
            if (should_stop_) {
                break;
            }

            std::lock_guard<std::mutex> cache_lock(mutex_);
            auto now = std::chrono::system_clock::now();
            for (auto it = cache_.begin(); it != cache_.end();) {
                if (now - it->second.lastAccessed > std::chrono::minutes(30)) {
                    it = cache_.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    void notifyListeners(AssetEventType type, const Asset& asset) {
        for (auto listener : listeners_) {
            listener->onAssetEvent(type, asset);
        }
    }

    void setLastError(const std::string& error) {
        lastError_ = error;
        for (auto callback : errorCallbacks_) {
            callback(error);
        }
    }

    std::unordered_map<std::string, Asset> assets_;
    std::unordered_map<std::string, CacheEntry> cache_;
    std::vector<AssetEventListener*> listeners_;
    mutable std::mutex mutex_;
    std::mutex cleanup_mutex_;
    std::condition_variable cleanup_cv_;
    std::thread cleanup_thread_;
    bool initialized_;
    bool should_stop_ = false;
    size_t cache_size_;
    size_t max_cache_size_;
    std::string lastError_;
    std::vector<AssetManager::AssetCallback> assetCallbacks_;
    std::vector<AssetManager::ErrorCallback> errorCallbacks_;
};

// Singleton instance
AssetManager& AssetManager::getInstance() {
    static AssetManager instance;
    return instance;
}

// Constructor
AssetManager::AssetManager() : impl_(std::make_unique<AssetManagerImpl>()) {}

// Destructor
AssetManager::~AssetManager() = default;

// Initialize
bool AssetManager::initialize(const nlohmann::json& config) {
    return impl_->initialize(config);
}

// Shutdown
void AssetManager::shutdown() {
    impl_->shutdown();
}

// Create Asset
bool AssetManager::createAsset(const AssetMetadata& metadata, std::string& assetId) {
    return impl_->createAsset(metadata, assetId);
}

// Update Asset
bool AssetManager::updateAsset(const std::string& assetId, const AssetMetadata& metadata) {
    return impl_->updateAsset(assetId, metadata);
}

// Delete Asset
bool AssetManager::deleteAsset(const std::string& assetId) {
    return impl_->deleteAsset(assetId);
}

// Get Asset
bool AssetManager::getAsset(const std::string& assetId, Asset& asset) {
    return impl_->getAsset(assetId, asset);
}

// List Assets
bool AssetManager::listAssets(std::vector<Asset>& assets, const nlohmann::json& filters) {
    return impl_->listAssets(assets, filters);
}

// Transfer Asset
bool AssetManager::transferAsset(const std::string& assetId, const std::string& from, const std::string& to) {
    return impl_->transferAsset(assetId, from, to);
}

// Lock Asset
bool AssetManager::lockAsset(const std::string& assetId) {
    return impl_->lockAsset(assetId);
}

// Unlock Asset
bool AssetManager::unlockAsset(const std::string& assetId) {
    return impl_->unlockAsset(assetId);
}

// Burn Asset
bool AssetManager::burnAsset(const std::string& assetId, uint64_t amount) {
    return impl_->burnAsset(assetId, amount);
}

// Add Event Listener
void AssetManager::addEventListener(AssetEventListener* listener) {
    impl_->addEventListener(listener);
}

// Remove Event Listener
void AssetManager::removeEventListener(AssetEventListener* listener) {
    impl_->removeEventListener(listener);
}

// Get last error
std::string AssetManager::getLastError() const {
    return impl_->getLastError();
}

// Mint asset
bool AssetManager::mintAsset(const std::string& assetId, uint64_t amount) {
    return impl_->mintAsset(assetId, amount);
}

// Search assets
bool AssetManager::searchAssets(const std::string& query, std::vector<Asset>& results) {
    return impl_->searchAssets(query, results);
}

// Set asset permissions
bool AssetManager::setAssetPermissions(const std::string& assetId, const nlohmann::json& permissions) {
    return impl_->setAssetPermissions(assetId, permissions);
}

// Get asset permissions
bool AssetManager::getAssetPermissions(const std::string& assetId, nlohmann::json& permissions) {
    return impl_->getAssetPermissions(assetId, permissions);
}

// Validate asset
bool AssetManager::validateAsset(const std::string& assetId) {
    return impl_->validateAsset(assetId);
}

// Validate transfer
bool AssetManager::validateTransfer(const std::string& assetId, const std::string& from, const std::string& to) {
    return impl_->validateTransfer(assetId, from, to);
}

// Get asset history
bool AssetManager::getAssetHistory(const std::string& assetId, std::vector<nlohmann::json>& history) {
    return impl_->getAssetHistory(assetId, history);
}

// Add asset history
bool AssetManager::addAssetHistory(const std::string& assetId, const nlohmann::json& event) {
    return impl_->addAssetHistory(assetId, event);
}

// Search by metadata
bool AssetManager::searchByMetadata(const nlohmann::json& metadata, std::vector<Asset>& results) {
    return impl_->searchByMetadata(metadata, results);
}

// Check asset permission
bool AssetManager::checkAssetPermission(const std::string& assetId, const std::string& address, const std::string& permission) {
    return impl_->checkAssetPermission(assetId, address, permission);
}

// Register asset callback
void AssetManager::registerAssetCallback(AssetCallback callback) {
    impl_->registerAssetCallback(callback);
}

// Register error callback
void AssetManager::registerErrorCallback(ErrorCallback callback) {
    impl_->registerErrorCallback(callback);
}

// Clear last error
void AssetManager::clearLastError() {
    impl_->clearLastError();
}

} // namespace assets
} // namespace satox 