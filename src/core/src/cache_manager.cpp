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

#include "satox/core/cache_manager.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace satox::core {

CacheManager& CacheManager::getInstance() {
    static CacheManager instance;
    return instance;
}

bool CacheManager::initialize(const CacheConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        lastError_ = "Cache manager already initialized";
        return false;
    }

    if (config.maxSize == 0 || config.maxEntries == 0) {
        lastError_ = "Invalid cache configuration";
        return false;
    }

    config_ = config;
    stats_ = CacheStats{};
    stats_.maxSize = config.maxSize;
    lastCleanup_ = std::chrono::system_clock::now();
    initialized_ = true;
    return true;
}

void CacheManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) {
        clear();
        initialized_ = false;
    }
}

bool CacheManager::set(const std::string& key, const std::string& value,
                      std::chrono::seconds ttl) {
    if (!initialized_) {
        lastError_ = "Cache manager not initialized";
        return false;
    }

    if (!validateKey(key) || !validateValue(value)) {
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    // Calculate entry size
    CacheEntry entry;
    entry.key = key;
    entry.value = value;
    entry.size = value.size() + key.size();
    entry.accessCount = 0;
    entry.lastAccess = std::chrono::system_clock::now();
    entry.expiry = ttl.count() > 0 ? 
        std::chrono::system_clock::now() + ttl :
        std::chrono::system_clock::now() + config_.defaultTTL;

    // Check if we need to evict entries
    while (stats_.totalSize + entry.size > config_.maxSize ||
           cache_.size() >= config_.maxEntries) {
        if (!evictEntry()) {
            lastError_ = "Failed to evict entry for new value";
            return false;
        }
    }

    // Add or update entry
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        stats_.totalSize -= it->second.size;
        lruList_.remove(key);
    }

    cache_[key] = entry;
    lruList_.push_front(key);
    stats_.totalSize += entry.size;
    stats_.totalEntries = cache_.size();

    notifyCallbacks(key, value);
    return true;
}

bool CacheManager::set(const std::string& key, const nlohmann::json& value,
                      std::chrono::seconds ttl) {
    return set(key, value.dump(), ttl);
}

std::optional<std::string> CacheManager::get(const std::string& key) {
    if (!initialized_) {
        lastError_ = "Cache manager not initialized";
        return std::nullopt;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    auto it = cache_.find(key);
    if (it == cache_.end()) {
        updateStats(false);
        return std::nullopt;
    }

    // Check if entry has expired
    if (std::chrono::system_clock::now() > it->second.expiry) {
        cache_.erase(it);
        lruList_.remove(key);
        stats_.totalSize -= it->second.size;
        stats_.totalEntries = cache_.size();
        updateStats(false);
        return std::nullopt;
    }

    // Update access statistics
    it->second.accessCount++;
    it->second.lastAccess = std::chrono::system_clock::now();
    lruList_.remove(key);
    lruList_.push_front(key);

    updateStats(true);
    return it->second.value;
}

std::optional<nlohmann::json> CacheManager::getJson(const std::string& key) {
    auto value = get(key);
    if (!value) {
        return std::nullopt;
    }

    try {
        return nlohmann::json::parse(*value);
    } catch (const std::exception& e) {
        lastError_ = "Failed to parse JSON: " + std::string(e.what());
        return std::nullopt;
    }
}

bool CacheManager::remove(const std::string& key) {
    if (!initialized_) {
        lastError_ = "Cache manager not initialized";
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    auto it = cache_.find(key);
    if (it == cache_.end()) {
        return false;
    }

    stats_.totalSize -= it->second.size;
    cache_.erase(it);
    lruList_.remove(key);
    stats_.totalEntries = cache_.size();
    return true;
}

bool CacheManager::exists(const std::string& key) {
    if (!initialized_) {
        lastError_ = "Cache manager not initialized";
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    auto it = cache_.find(key);
    if (it == cache_.end()) {
        return false;
    }

    // Check if entry has expired
    if (std::chrono::system_clock::now() > it->second.expiry) {
        cache_.erase(it);
        lruList_.remove(key);
        stats_.totalSize -= it->second.size;
        stats_.totalEntries = cache_.size();
        return false;
    }

    return true;
}

void CacheManager::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    cache_.clear();
    lruList_.clear();
    stats_.totalSize = 0;
    stats_.totalEntries = 0;
}

bool CacheManager::setMulti(const std::unordered_map<std::string, std::string>& entries,
                           std::chrono::seconds ttl) {
    if (!initialized_) {
        lastError_ = "Cache manager not initialized";
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    for (const auto& [key, value] : entries) {
        if (!set(key, value, ttl)) {
            return false;
        }
    }

    return true;
}

std::unordered_map<std::string, std::string> CacheManager::getMulti(
    const std::vector<std::string>& keys) {
    std::unordered_map<std::string, std::string> result;
    
    if (!initialized_) {
        lastError_ = "Cache manager not initialized";
        return result;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    for (const auto& key : keys) {
        auto value = get(key);
        if (value) {
            result[key] = *value;
        }
    }

    return result;
}

bool CacheManager::removeMulti(const std::vector<std::string>& keys) {
    if (!initialized_) {
        lastError_ = "Cache manager not initialized";
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    for (const auto& key : keys) {
        if (!remove(key)) {
            return false;
        }
    }

    return true;
}

bool CacheManager::setTTL(const std::string& key, std::chrono::seconds ttl) {
    if (!initialized_) {
        lastError_ = "Cache manager not initialized";
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    auto it = cache_.find(key);
    if (it == cache_.end()) {
        return false;
    }

    it->second.expiry = std::chrono::system_clock::now() + ttl;
    return true;
}

std::chrono::system_clock::time_point CacheManager::getExpiry(const std::string& key) {
    if (!initialized_) {
        lastError_ = "Cache manager not initialized";
        return std::chrono::system_clock::time_point();
    }

    std::lock_guard<std::mutex> lock(mutex_);

    auto it = cache_.find(key);
    if (it == cache_.end()) {
        return std::chrono::system_clock::time_point();
    }

    return it->second.expiry;
}

size_t CacheManager::getSize(const std::string& key) {
    if (!initialized_) {
        lastError_ = "Cache manager not initialized";
        return 0;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    auto it = cache_.find(key);
    if (it == cache_.end()) {
        return 0;
    }

    return it->second.size;
}

size_t CacheManager::getTotalSize() {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_.totalSize;
}

size_t CacheManager::getEntryCount() {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_.totalEntries;
}

CacheManager::CacheStats CacheManager::getStats() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Calculate hit rate
    size_t totalAccesses = stats_.hitCount + stats_.missCount;
    stats_.hitRate = totalAccesses > 0 ? 
        static_cast<double>(stats_.hitCount) / totalAccesses : 0.0;

    return stats_;
}

void CacheManager::registerCallback(CacheCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_.push_back(callback);
}

void CacheManager::unregisterCallback() {
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_.clear();
}

void CacheManager::cleanup() {
    if (!initialized_) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    auto now = std::chrono::system_clock::now();
    if (now - lastCleanup_ < std::chrono::seconds(config_.cleanupInterval)) {
        return;
    }

    // Remove expired entries
    for (auto it = cache_.begin(); it != cache_.end();) {
        if (now > it->second.expiry) {
            stats_.totalSize -= it->second.size;
            lruList_.remove(it->first);
            it = cache_.erase(it);
        } else {
            ++it;
        }
    }

    stats_.totalEntries = cache_.size();
    lastCleanup_ = now;
}

void CacheManager::resize(size_t newMaxSize) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (newMaxSize == 0) {
        lastError_ = "Invalid cache size";
        return;
    }

    config_.maxSize = newMaxSize;
    stats_.maxSize = newMaxSize;

    // Evict entries if necessary
    while (stats_.totalSize > config_.maxSize) {
        if (!evictEntry()) {
            break;
        }
    }
}

void CacheManager::setMaxEntries(size_t newMaxEntries) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (newMaxEntries == 0) {
        lastError_ = "Invalid max entries";
        return;
    }

    config_.maxEntries = newMaxEntries;

    // Evict entries if necessary
    while (cache_.size() > config_.maxEntries) {
        if (!evictEntry()) {
            break;
        }
    }
}

std::string CacheManager::getLastError() const {
    return lastError_;
}

void CacheManager::clearLastError() {
    lastError_.clear();
}

bool CacheManager::evictEntry() {
    if (cache_.empty()) {
        return false;
    }

    std::string keyToEvict;

    if (config_.enableLRU && config_.enableLFU) {
        // Use both LRU and LFU for eviction
        auto it = std::min_element(cache_.begin(), cache_.end(),
            [](const auto& a, const auto& b) {
                return a.second.accessCount < b.second.accessCount ||
                       (a.second.accessCount == b.second.accessCount &&
                        a.second.lastAccess < b.second.lastAccess);
            });
        keyToEvict = it->first;
    } else if (config_.enableLRU) {
        // Use LRU for eviction
        keyToEvict = lruList_.back();
    } else if (config_.enableLFU) {
        // Use LFU for eviction
        auto it = std::min_element(cache_.begin(), cache_.end(),
            [](const auto& a, const auto& b) {
                return a.second.accessCount < b.second.accessCount;
            });
        keyToEvict = it->first;
    } else {
        // Default to LRU
        keyToEvict = lruList_.back();
    }

    auto it = cache_.find(keyToEvict);
    if (it != cache_.end()) {
        stats_.totalSize -= it->second.size;
        cache_.erase(it);
        lruList_.remove(keyToEvict);
        stats_.totalEntries = cache_.size();
        stats_.evictionCount++;
        return true;
    }

    return false;
}

void CacheManager::updateAccessStats(const std::string& key) {
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        it->second.accessCount++;
        it->second.lastAccess = std::chrono::system_clock::now();
        lruList_.remove(key);
        lruList_.push_front(key);
    }
}

void CacheManager::notifyCallbacks(const std::string& key, const std::string& value) {
    for (const auto& callback : callbacks_) {
        callback(key, value);
    }
}

bool CacheManager::validateKey(const std::string& key) {
    if (key.empty()) {
        lastError_ = "Invalid key: empty key";
        return false;
    }
    return true;
}

bool CacheManager::validateValue(const std::string& value) {
    if (value.empty()) {
        lastError_ = "Invalid value: empty value";
        return false;
    }
    return true;
}

size_t CacheManager::calculateEntrySize(const CacheEntry& entry) {
    return entry.key.size() + entry.value.size();
}

void CacheManager::updateStats(bool hit) {
    if (hit) {
        stats_.hitCount++;
    } else {
        stats_.missCount++;
    }
}

} // namespace satox::core 