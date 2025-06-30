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
#include <unordered_map>
#include <memory>
#include <mutex>
#include <chrono>
#include <functional>
#include <list>
#include <optional>
#include <nlohmann/json.hpp>

namespace satox::core {

class CacheManager {
public:
    // Cache entry structure
    struct CacheEntry {
        std::string key;
        std::string value;
        std::chrono::system_clock::time_point expiry;
        size_t size;
        uint32_t accessCount;
        std::chrono::system_clock::time_point lastAccess;
    };

    // Cache statistics structure
    struct CacheStats {
        size_t totalEntries;
        size_t totalSize;
        size_t maxSize;
        size_t hitCount;
        size_t missCount;
        double hitRate;
        size_t evictionCount;
    };

    // Cache configuration structure
    struct CacheConfig {
        size_t maxSize;                    // Maximum cache size in bytes
        size_t maxEntries;                 // Maximum number of entries
        std::chrono::seconds defaultTTL;   // Default time-to-live
        bool enableLRU;                    // Enable Least Recently Used eviction
        bool enableLFU;                    // Enable Least Frequently Used eviction
        size_t cleanupInterval;            // Cleanup interval in seconds
    };

    // Cache callback type
    using CacheCallback = std::function<void(const std::string&, const std::string&)>;

    // Singleton instance
    static CacheManager& getInstance();

    // Prevent copying
    CacheManager(const CacheManager&) = delete;
    CacheManager& operator=(const CacheManager&) = delete;

    // Initialization and shutdown
    bool initialize(const CacheConfig& config);
    void shutdown();

    // Basic cache operations
    bool set(const std::string& key, const std::string& value,
             std::chrono::seconds ttl = std::chrono::seconds(0));
    bool set(const std::string& key, const nlohmann::json& value,
             std::chrono::seconds ttl = std::chrono::seconds(0));
    std::optional<std::string> get(const std::string& key);
    std::optional<nlohmann::json> getJson(const std::string& key);
    bool remove(const std::string& key);
    bool exists(const std::string& key);
    void clear();

    // Bulk operations
    bool setMulti(const std::unordered_map<std::string, std::string>& entries,
                  std::chrono::seconds ttl = std::chrono::seconds(0));
    std::unordered_map<std::string, std::string> getMulti(
        const std::vector<std::string>& keys);
    bool removeMulti(const std::vector<std::string>& keys);

    // Cache management
    bool setTTL(const std::string& key, std::chrono::seconds ttl);
    std::chrono::system_clock::time_point getExpiry(const std::string& key);
    size_t getSize(const std::string& key);
    size_t getTotalSize();
    size_t getEntryCount();
    CacheStats getStats();

    // Cache callbacks
    void registerCallback(CacheCallback callback);
    void unregisterCallback();

    // Cache maintenance
    void cleanup();
    void resize(size_t newMaxSize);
    void setMaxEntries(size_t newMaxEntries);

    // Error handling
    std::string getLastError() const;
    void clearLastError();

private:
    CacheManager() = default;
    ~CacheManager() = default;

    // Helper methods
    bool evictEntry();
    void updateAccessStats(const std::string& key);
    void notifyCallbacks(const std::string& key, const std::string& value);
    bool validateKey(const std::string& key);
    bool validateValue(const std::string& value);
    size_t calculateEntrySize(const CacheEntry& entry);
    void updateStats(bool hit);

    // Member variables
    bool initialized_ = false;
    std::mutex mutex_;
    std::unordered_map<std::string, CacheEntry> cache_;
    std::list<std::string> lruList_;  // For LRU eviction
    std::vector<CacheCallback> callbacks_;
    CacheConfig config_;
    CacheStats stats_;
    std::string lastError_;
    std::chrono::system_clock::time_point lastCleanup_;
};

} // namespace satox::core 