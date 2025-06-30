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

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <chrono>
#include <nlohmann/json.hpp>
#include "satox/asset/types.hpp"

namespace satox::asset {

// Main manager class
class AssetManager {
public:
    // Singleton pattern
    static AssetManager& getInstance();

    // Constructor/Destructor
    AssetManager();
    ~AssetManager();

    // Initialization and shutdown
    bool initialize(const AssetConfig& config);
    void shutdown();

    // Core operations
    bool createAsset(const AssetCreationRequest& request);
    bool transferAsset(const AssetTransferRequest& request);
    bool reissueAsset(const AssetReissueRequest& request);
    bool burnAsset(const AssetBurnRequest& request);
    bool freezeAsset(const std::string& asset_name, const std::string& owner_address);
    bool unfreezeAsset(const std::string& asset_name, const std::string& owner_address);
    bool destroyAsset(const std::string& asset_name, const std::string& owner_address);
    
    // Asset queries
    bool getAssetMetadata(const std::string& asset_name, AssetMetadata& metadata) const;
    bool getAssetBalance(const std::string& address, const std::string& asset_name, uint64_t& balance) const;
    bool getAssetOwners(const std::string& asset_name, std::vector<std::string>& owners) const;
    bool getAssetStatus(const std::string& asset_name, AssetStatus& status) const;
    bool getAssetType(const std::string& asset_name, AssetType& type) const;
    
    // Asset validation
    bool verifyAsset(const std::string& asset_name, AssetVerificationResult& result) const;
    bool validateAssetMetadata(const AssetMetadata& metadata, AssetVerificationResult& result) const;
    bool validateAssetTransfer(const AssetTransferRequest& request, AssetVerificationResult& result) const;
    bool validateAssetReissue(const AssetReissueRequest& request, AssetVerificationResult& result) const;
    bool validateAssetBurn(const AssetBurnRequest& request, AssetVerificationResult& result) const;

    // State management
    AssetState getState() const;
    bool isInitialized() const;
    bool isHealthy() const;

    // Configuration management
    AssetConfig getConfig() const;
    bool updateConfig(const AssetConfig& config);
    bool validateConfig(const AssetConfig& config) const;

    // Statistics and monitoring
    AssetStats getStats() const;
    void resetStats();
    bool enableStats(bool enable);

    // Callback registration
    void registerAssetCallback(AssetCallback callback);
    void registerOperationCallback(OperationCallback callback);
    void registerErrorCallback(ErrorCallback callback);
    void unregisterAssetCallback();
    void unregisterOperationCallback();
    void unregisterErrorCallback();

    // Error handling
    std::string getLastError() const;
    void clearLastError();

    // Health check
    bool healthCheck();
    nlohmann::json getHealthStatus();

    // Additional functionality
    nlohmann::json getAssetInfo(const std::string& asset_name);
    std::vector<nlohmann::json> listAssets();
    std::vector<nlohmann::json> getAssetHistory(const std::string& asset_name);
    std::vector<nlohmann::json> getAssetBalances(const std::string& asset_name);
    std::vector<nlohmann::json> getAssetTransactions(const std::string& asset_name);

private:
    // Disable copy and assignment
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    // Private helper methods
    void setLastError(const std::string& error);
    void notifyAssetEvent(const std::string& event, bool success);
    void notifyOperationEvent(const AssetResult& result);
    void notifyErrorEvent(const std::string& operation, const std::string& error);
    void updateStats(bool success, double operationTime);
    bool validateState() const;
    void logOperation(const std::string& operation, bool success, double duration);
    bool validateAssetCreationRequest(const AssetCreationRequest& request, AssetVerificationResult& result) const;

    // Member variables
    mutable std::mutex mutex_;
    bool initialized_ = false;
    AssetState state_ = AssetState::UNINITIALIZED;
    AssetConfig config_;
    AssetStats stats_;
    bool statsEnabled_ = true;
    std::string lastError_;
    int consecutiveFailures_ = 0;
    std::chrono::system_clock::time_point lastHealthCheck_;

    // Asset storage
    std::unordered_map<std::string, AssetMetadata> assets_;
    std::unordered_map<std::string, std::unordered_map<std::string, uint64_t>> balances_;
    std::unordered_map<std::string, AssetStatus> assetStatuses_;

    // Callbacks
    std::vector<AssetCallback> assetCallbacks_;
    std::vector<OperationCallback> operationCallbacks_;
    std::vector<ErrorCallback> errorCallbacks_;
};

} // namespace satox::asset 