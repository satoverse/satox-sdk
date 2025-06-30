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
#include <chrono>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <queue>
#include <atomic>
#include <thread>
#include <nlohmann/json.hpp>
#include <optional>
#include <unordered_set>

namespace satox {
namespace asset {

struct Asset {
    std::string id;
    std::string name;
    std::string symbol;
    std::string owner;
    uint64_t total_supply;
    uint8_t decimals;
    std::string status;
    std::map<std::string, std::string> metadata;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
};

class AssetManager {
public:
    // Singleton instance
    static AssetManager& getInstance();

    // Prevent copying
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    // Allow moving
    AssetManager(AssetManager&&) noexcept = default;
    AssetManager& operator=(AssetManager&&) noexcept = default;

    // Core functionality
    bool initialize(const std::string& network);
    void shutdown();
    std::string getLastError() const;

    // Asset operations
    bool createAsset(const Asset& asset);
    bool updateAsset(const std::string& asset_id, const Asset& asset);
    bool deleteAsset(const std::string& asset_id);
    std::optional<Asset> getAsset(const std::string& asset_id) const;
    std::vector<Asset> getAssetsByOwner(const std::string& owner) const;
    std::vector<Asset> getAllAssets() const;
    bool transferAsset(const std::string& asset_id, const std::string& from,
                      const std::string& to, uint64_t amount);
    uint64_t getAssetBalance(const std::string& asset_id, const std::string& address) const;

    // Performance configuration
    struct PerformanceConfig {
        size_t max_cache_size = 1000;
        size_t batch_size = 50;
        bool enable_async_processing = true;
        bool enable_caching = true;
        size_t connection_pool_size = 10;
        std::chrono::milliseconds cache_ttl = std::chrono::minutes(5);
        std::chrono::milliseconds operation_timeout = std::chrono::seconds(30);
    };

    // Error recovery configuration
    struct ErrorRecoveryConfig {
        size_t max_retry_attempts = 3;
        std::chrono::milliseconds retry_delay = std::chrono::seconds(1);
        std::chrono::milliseconds recovery_timeout = std::chrono::seconds(30);
        bool enable_auto_recovery = true;
        size_t max_recovery_queue_size = 1000;
    };

    // Recovery operation
    struct RecoveryOperation {
        enum class Type {
            ASSET_CREATION,
            ASSET_UPDATE,
            ASSET_DELETION,
            ASSET_TRANSFER,
            ASSET_VALIDATION,
            METADATA_UPDATE,
            PERMISSION_UPDATE
        };

        std::string operation_id;
        Type type;
        std::chrono::system_clock::time_point timestamp;
        std::string error_message;
        size_t attempt_count;
        nlohmann::json context;
    };

    // Performance optimization methods
    bool setPerformanceConfig(const PerformanceConfig& config);
    void clearCache();
    size_t getCacheSize() const;
    size_t getConnectionPoolSize() const;
    size_t getBatchSize() const;
    bool processBatch(const std::vector<Asset>& assets);
    void optimizeMemory();

    // Error recovery methods
    bool setErrorRecoveryConfig(const ErrorRecoveryConfig& config);
    bool recoverFromError(const std::string& operation_id);
    std::vector<RecoveryOperation> getRecoveryHistory() const;
    void clearRecoveryHistory();
    bool isRecoveryInProgress() const;
    std::string getLastRecoveryError() const;

    // Asset search
    bool searchAssets(const std::string& query, std::vector<Asset>& results);
    bool searchByMetadata(const std::unordered_map<std::string, std::string>& metadata_filters, std::vector<Asset>& results);

    // Asset validation
    bool validateAsset(const Asset& asset) const;
    bool validateAssetName(const std::string& name) const;
    bool validateMetadata(const std::unordered_map<std::string, std::string>& metadata) const;

    // Asset permissions
    bool setAssetPermissions(const std::string& asset_id, const std::unordered_map<std::string, std::vector<std::string>>& permissions);
    bool getAssetPermissions(const std::string& asset_id, std::unordered_map<std::string, std::vector<std::string>>& permissions) const;
    bool checkAssetPermission(const std::string& asset_id, const std::string& address, const std::string& permission) const;

    // Asset history
    bool getAssetHistory(const std::string& asset_id, std::vector<nlohmann::json>& history) const;
    bool addAssetHistory(const std::string& asset_id, const nlohmann::json& event);

    // Callbacks
    using AssetCallback = std::function<void(const std::string&, const nlohmann::json&)>;
    void registerAssetCallback(AssetCallback callback);

private:
    AssetManager();
    ~AssetManager();

    // Core members
    bool initialized_;
    std::string network_name_;
    std::string last_error_;
    std::map<std::string, Asset> assets_;
    std::map<std::string, std::map<std::string, uint64_t>> balances_;
    mutable std::mutex mutex_;

    // Performance optimization members
    PerformanceConfig perf_config_;
    std::unordered_map<std::string, std::pair<Asset, std::chrono::system_clock::time_point>> asset_cache_;
    std::unordered_map<std::string, std::unordered_set<std::string>> owner_cache_;
    std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_set<std::string>>> metadata_cache_;
    std::queue<std::vector<Asset>> batch_queue_;
    std::vector<std::thread> worker_threads_;
    std::atomic<bool> should_stop_workers_;
    std::mutex cache_mutex_;
    std::mutex batch_mutex_;
    std::mutex worker_mutex_;

    // Error recovery members
    ErrorRecoveryConfig recovery_config_;
    std::vector<RecoveryOperation> recovery_history_;
    std::unordered_map<std::string, RecoveryOperation> active_recoveries_;
    std::mutex recovery_mutex_;
    std::string last_recovery_error_;

    // Helper methods
    void cleanup();
    std::string generateAssetId(const Asset& asset);
    bool validateAsset(const Asset& asset);

    // Performance optimization methods
    void startWorkers();
    void stopWorkers();
    void workerFunction();
    void addTask(const std::function<void()>& task);
    void processTaskQueue();
    void updateCache(const std::string& asset_id, const Asset& asset);
    void cleanupCache();
    void cleanupConnections();
    void processBatchQueue();
    void optimizeMemoryUsage();

    // Error recovery methods
    bool attemptRecovery(const RecoveryOperation& operation);
    void logRecoveryAttempt(const RecoveryOperation& operation, bool success);
    bool validateRecoveryContext(const RecoveryOperation& operation);
    void cleanupRecoveryOperation(const std::string& operation_id);
    bool isRecoveryAllowed(const RecoveryOperation& operation);
    std::string generateRecoveryId();
};

} // namespace asset
} // namespace satox 