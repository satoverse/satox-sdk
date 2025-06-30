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
#include "satox/assets/asset_types.hpp"

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <nlohmann/json.hpp>

namespace satox {
namespace assets {

// Forward declaration
class AssetManagerImpl;

class AssetManager {
public:
    // Singleton instance
    static AssetManager& getInstance();

    // Prevent copying
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    // Initialization and shutdown
    bool initialize(const nlohmann::json& config);
    void shutdown();

    // Asset creation and management
    bool createAsset(const AssetMetadata& metadata, std::string& assetId);
    bool updateAsset(const std::string& assetId, const AssetMetadata& metadata);
    bool deleteAsset(const std::string& assetId);
    bool getAsset(const std::string& assetId, Asset& asset);
    bool listAssets(std::vector<Asset>& assets, const nlohmann::json& filters = nlohmann::json::object());

    // Asset operations
    bool transferAsset(const std::string& assetId, const std::string& from, const std::string& to);
    bool lockAsset(const std::string& assetId);
    bool unlockAsset(const std::string& assetId);
    bool burnAsset(const std::string& assetId, uint64_t amount);
    bool mintAsset(const std::string& assetId, uint64_t amount);

    // Asset validation
    bool validateAsset(const std::string& assetId);
    bool validateMetadata(const AssetMetadata& metadata);
    bool validateTransfer(const std::string& assetId, const std::string& from, const std::string& to);

    // Asset history
    bool getAssetHistory(const std::string& assetId, std::vector<nlohmann::json>& history);
    bool addAssetHistory(const std::string& assetId, const nlohmann::json& event);

    // Asset search
    bool searchAssets(const std::string& query, std::vector<Asset>& results);
    bool searchByMetadata(const nlohmann::json& metadata, std::vector<Asset>& results);

    // Asset permissions
    bool setAssetPermissions(const std::string& assetId, const nlohmann::json& permissions);
    bool getAssetPermissions(const std::string& assetId, nlohmann::json& permissions);
    bool checkAssetPermission(const std::string& assetId, const std::string& address, const std::string& permission);

    // Event handling
    using AssetCallback = std::function<void(const std::string&, const nlohmann::json&)>;
    using ErrorCallback = std::function<void(const std::string&)>;

    void registerAssetCallback(AssetCallback callback);
    void registerErrorCallback(ErrorCallback callback);

    // Event listener interface
    void addEventListener(AssetEventListener* listener);
    void removeEventListener(AssetEventListener* listener);

    // Error handling
    std::string getLastError() const;
    void clearLastError();

private:
    AssetManager();
    ~AssetManager();
    std::unique_ptr<AssetManagerImpl> impl_;

    // Internal state
    bool initialized_ = false;
    std::string lastError_;
    std::mutex mutex_;

    // Asset storage
    std::unordered_map<std::string, Asset> assets_;
    std::unordered_map<std::string, nlohmann::json> permissions_;

    // Callbacks
    std::vector<AssetCallback> assetCallbacks_;
    std::vector<ErrorCallback> errorCallbacks_;
    std::vector<AssetEventListener*> eventListeners_;

    // Helper methods
    void setLastError(const std::string& error);
    void notifyAssetCallbacks(const std::string& assetId, const nlohmann::json& event);
    void notifyErrorCallbacks(const std::string& error);
    std::string generateAssetId(const AssetMetadata& metadata);
    bool validateAssetId(const std::string& assetId);
};

} // namespace assets
} // namespace satox 
