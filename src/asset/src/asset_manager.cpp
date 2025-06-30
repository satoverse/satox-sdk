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

#include <iostream>
#include <thread>
#include <mutex>
#include "satox/asset/asset_manager.hpp"
#include <nlohmann/json.hpp>
#include <filesystem>
#include <chrono>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

using json = nlohmann::json;

namespace satox::asset {

// Simple logging wrapper to avoid spdlog compatibility issues
class SimpleLogger {
public:
    static void debug(const std::string& msg) {
        std::cout << "[DEBUG] " << msg << std::endl;
    }
    
    static void info(const std::string& msg) {
        std::cout << "[INFO] " << msg << std::endl;
    }
    
    static void warn(const std::string& msg) {
        std::cout << "[WARN] " << msg << std::endl;
    }
    
    static void error(const std::string& msg) {
        std::cerr << "[ERROR] " << msg << std::endl;
    }
};

// Singleton instance
AssetManager& AssetManager::getInstance() {
    static AssetManager instance;
    return instance;
}

// Constructor
AssetManager::AssetManager() {
    SimpleLogger::debug("AssetManager: Constructor called");
}

// Destructor
AssetManager::~AssetManager() {
    SimpleLogger::debug("AssetManager: Destructor called");
    shutdown();
}

// Initialization and shutdown
bool AssetManager::initialize(const AssetConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        setLastError("AssetManager already initialized");
        return false;
    }

    try {
        SimpleLogger::info("AssetManager: Initializing with config: " + config.name);

        // Validate configuration
        if (!validateConfig(config)) {
            setLastError("Invalid configuration");
            return false;
        }

        // Set state to initializing
        state_ = AssetState::INITIALIZING;

        // Store configuration
        config_ = config;

        // Initialize logging if enabled
        if (config.enableLogging && !config.logPath.empty()) {
            try {
                std::filesystem::create_directories(
                    std::filesystem::path(config.logPath).parent_path()
                );
                SimpleLogger::info("Logging initialized to: " + config.logPath);
            } catch (const std::exception& e) {
                SimpleLogger::warn("Failed to setup logging: " + std::string(e.what()));
            }
        }

        // Initialize asset storage
        assets_.clear();
        balances_.clear();
        assetStatuses_.clear();

        // Set state to initialized
        state_ = AssetState::INITIALIZED;
        initialized_ = true;
        lastHealthCheck_ = std::chrono::system_clock::now();

        SimpleLogger::info("AssetManager: Initialized successfully");
        notifyAssetEvent("initialized", true);
        return true;

    } catch (const std::exception& e) {
        SimpleLogger::error("AssetManager: Initialization failed: " + std::string(e.what()));
        setLastError(e.what());
        state_ = AssetState::ERROR;
        return false;
    }
}

void AssetManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return;
    }

    try {
        SimpleLogger::info("AssetManager: Shutting down");

        // Clear all data
        assets_.clear();
        balances_.clear();
        assetStatuses_.clear();
        
        // Clear callbacks
        assetCallbacks_.clear();
        operationCallbacks_.clear();
        errorCallbacks_.clear();

        initialized_ = false;
        state_ = AssetState::SHUTDOWN;
        lastError_.clear();

        SimpleLogger::info("AssetManager: Shutdown completed");
        notifyAssetEvent("shutdown", true);

    } catch (const std::exception& e) {
        SimpleLogger::error("AssetManager: Shutdown failed: " + std::string(e.what()));
        setLastError(e.what());
    }
}

bool AssetManager::isInitialized() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return initialized_;
}

std::string AssetManager::getLastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastError_;
}

// Asset management
bool AssetManager::createAsset(const AssetCreationRequest& request) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        setLastError("AssetManager not initialized");
        return false;
    }

    try {
        auto startTime = std::chrono::high_resolution_clock::now();

        // Validate request
        AssetVerificationResult result;
        if (!validateAssetCreationRequest(request, result)) {
            setLastError(result.error_message);
            updateStats(false, 0.0);
            notifyErrorEvent("createAsset", result.error_message);
            notifyAssetEvent("asset_creation_failed", false);
            AssetResult opResult{"createAsset", false, result.error_message};
            notifyOperationEvent(opResult);
            return false;
        }

        // Check if asset already exists
        if (assets_.find(request.name) != assets_.end()) {
            setLastError("Asset already exists: " + request.name);
            updateStats(false, 0.0);
            notifyErrorEvent("createAsset", "Asset already exists: " + request.name);
            return false;
        }

        // Check max assets limit
        if (assets_.size() >= config_.maxAssets) {
            setLastError("Maximum number of assets reached: " + std::to_string(config_.maxAssets));
            updateStats(false, 0.0);
            notifyErrorEvent("createAsset", "Maximum number of assets reached: " + std::to_string(config_.maxAssets));
            return false;
        }

        // Validate metadata size
        if (request.metadata.is_object() && request.metadata.size() > 100) {
            setLastError("Metadata too large, maximum 100 fields allowed");
            updateStats(false, 0.0);
            notifyErrorEvent("createAsset", "Metadata too large, maximum 100 fields allowed");
            return false;
        }

        // Create asset metadata
        AssetMetadata metadata;
        metadata.name = request.name;
        metadata.symbol = request.symbol;
        metadata.description = request.metadata.is_object() && request.metadata.contains("description") 
            ? request.metadata["description"].get<std::string>() : "";
        metadata.issuer = request.owner_address;
        metadata.total_supply = request.amount;
        metadata.decimals = request.units;
        metadata.reissuable = request.reissuable;
        metadata.frozen = false;
        metadata.additional_data = request.metadata;

        // Store asset
        assets_[request.name] = metadata;
        assetStatuses_[request.name] = AssetStatus::ACTIVE;

        // Initialize balance for owner
        balances_[request.name][request.owner_address] = request.amount;

        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        updateStats(true, duration.count());
        logOperation("createAsset", true, duration.count());
        
        SimpleLogger::info("Asset created: " + request.name + " (" + request.symbol + ")");
        notifyAssetEvent("asset_created", true);
        
        AssetResult opResult{"createAsset", true, ""};
        notifyOperationEvent(opResult);
        
        return true;

    } catch (const std::exception& e) {
        SimpleLogger::error("Asset creation failed: " + std::string(e.what()));
        setLastError(e.what());
        updateStats(false, 0.0);
        notifyErrorEvent("createAsset", e.what());
        return false;
    }
}

bool AssetManager::transferAsset(const AssetTransferRequest& request) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        setLastError("AssetManager not initialized");
        return false;
    }

    try {
        auto startTime = std::chrono::high_resolution_clock::now();

        // Validate transfer
        AssetVerificationResult result;
        if (!validateAssetTransfer(request, result)) {
            setLastError(result.error_message);
            updateStats(false, 0.0);
            notifyErrorEvent("transferAsset", result.error_message);
            return false;
        }

        // Check if asset exists
        if (assets_.find(request.asset_name) == assets_.end()) {
            setLastError("Asset not found: " + request.asset_name);
            updateStats(false, 0.0);
            notifyErrorEvent("transferAsset", "Asset not found: " + request.asset_name);
            return false;
        }

        // Check if asset is frozen
        if (assetStatuses_[request.asset_name] == AssetStatus::FROZEN) {
            setLastError("Asset is frozen: " + request.asset_name);
            updateStats(false, 0.0);
            notifyErrorEvent("transferAsset", "Asset is frozen: " + request.asset_name);
            return false;
        }

        // Check sender balance
        auto& assetBalances = balances_[request.asset_name];
        if (assetBalances[request.from_address] < request.amount) {
            setLastError("Insufficient balance for transfer");
            updateStats(false, 0.0);
            notifyErrorEvent("transferAsset", "Insufficient balance for transfer");
            return false;
        }

        // Perform transfer
        assetBalances[request.from_address] -= request.amount;
        assetBalances[request.to_address] += request.amount;

        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        updateStats(true, duration.count());
        logOperation("transferAsset", true, duration.count());
        
        SimpleLogger::info("Asset transferred: " + request.asset_name + " from " + request.from_address + " to " + request.to_address + " (amount: " + std::to_string(request.amount) + ")");
        notifyAssetEvent("asset_transferred", true);
        
        return true;

    } catch (const std::exception& e) {
        SimpleLogger::error("Asset transfer failed: " + std::string(e.what()));
        setLastError(e.what());
        updateStats(false, 0.0);
        notifyErrorEvent("transferAsset", e.what());
        return false;
    }
}

bool AssetManager::reissueAsset(const AssetReissueRequest& request) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        setLastError("AssetManager not initialized");
        return false;
    }

    try {
        auto startTime = std::chrono::high_resolution_clock::now();

        // Validate reissue
        AssetVerificationResult result;
        if (!validateAssetReissue(request, result)) {
            setLastError(result.error_message);
            updateStats(false, 0.0);
            notifyErrorEvent("reissueAsset", result.error_message);
            return false;
        }

        // Check if asset exists
        if (assets_.find(request.asset_name) == assets_.end()) {
            setLastError("Asset not found: " + request.asset_name);
            updateStats(false, 0.0);
            notifyErrorEvent("reissueAsset", "Asset not found: " + request.asset_name);
            return false;
        }

        // Check if asset is reissuable
        if (!assets_[request.asset_name].reissuable) {
            setLastError("Asset is not reissuable: " + request.asset_name);
            updateStats(false, 0.0);
            notifyErrorEvent("reissueAsset", "Asset is not reissuable: " + request.asset_name);
            return false;
        }

        // Add to owner balance
        balances_[request.asset_name][request.owner_address] += request.amount;

        // Update total supply
        assets_[request.asset_name].total_supply += request.amount;

        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        updateStats(true, duration.count());
        logOperation("reissueAsset", true, duration.count());
        
        SimpleLogger::info("Asset reissued: " + request.asset_name + " by " + request.owner_address + " (amount: " + std::to_string(request.amount) + ")");
        notifyAssetEvent("asset_reissued", true);
        
        return true;

    } catch (const std::exception& e) {
        SimpleLogger::error("Asset reissue failed: " + std::string(e.what()));
        setLastError(e.what());
        updateStats(false, 0.0);
        notifyErrorEvent("reissueAsset", e.what());
        return false;
    }
}

bool AssetManager::burnAsset(const AssetBurnRequest& request) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        setLastError("AssetManager not initialized");
        return false;
    }

    try {
        auto startTime = std::chrono::high_resolution_clock::now();

        // Validate burn
        AssetVerificationResult result;
        if (!validateAssetBurn(request, result)) {
            setLastError(result.error_message);
            updateStats(false, 0.0);
            notifyErrorEvent("burnAsset", result.error_message);
            return false;
        }

        // Check if asset exists
        if (assets_.find(request.asset_name) == assets_.end()) {
            setLastError("Asset not found: " + request.asset_name);
            updateStats(false, 0.0);
            notifyErrorEvent("burnAsset", "Asset not found: " + request.asset_name);
            return false;
        }

        // Check owner balance
        auto& assetBalances = balances_[request.asset_name];
        if (assetBalances[request.owner_address] < request.amount) {
            setLastError("Insufficient balance for burn");
            updateStats(false, 0.0);
            notifyErrorEvent("burnAsset", "Insufficient balance for burn");
            return false;
        }

        // Perform burn
        assetBalances[request.owner_address] -= request.amount;
        assets_[request.asset_name].total_supply -= request.amount;

        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        updateStats(true, duration.count());
        logOperation("burnAsset", true, duration.count());
        
        SimpleLogger::info("Asset burned: " + request.asset_name + " by " + request.owner_address + " (amount: " + std::to_string(request.amount) + ")");
        notifyAssetEvent("asset_burned", true);
        
        return true;

    } catch (const std::exception& e) {
        SimpleLogger::error("Asset burn failed: " + std::string(e.what()));
        setLastError(e.what());
        updateStats(false, 0.0);
        notifyErrorEvent("burnAsset", e.what());
        return false;
    }
}

bool AssetManager::freezeAsset(const std::string& asset_name, const std::string& owner_address) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        setLastError("AssetManager not initialized");
        return false;
    }

    if (assets_.find(asset_name) == assets_.end()) {
        setLastError("Asset not found: " + asset_name);
        return false;
    }

    assetStatuses_[asset_name] = AssetStatus::FROZEN;
    assets_[asset_name].frozen = true;
    
    SimpleLogger::info("Asset frozen: " + asset_name + " by " + owner_address);
    notifyAssetEvent("asset_frozen", true);
    
    return true;
}

bool AssetManager::unfreezeAsset(const std::string& asset_name, const std::string& owner_address) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        setLastError("AssetManager not initialized");
        return false;
    }

    if (assets_.find(asset_name) == assets_.end()) {
        setLastError("Asset not found: " + asset_name);
        return false;
    }

    assetStatuses_[asset_name] = AssetStatus::ACTIVE;
    assets_[asset_name].frozen = false;
    
    SimpleLogger::info("Asset unfrozen: " + asset_name + " by " + owner_address);
    notifyAssetEvent("asset_unfrozen", true);
    
    return true;
}

bool AssetManager::destroyAsset(const std::string& asset_name, const std::string& owner_address) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        setLastError("AssetManager not initialized");
        return false;
    }
    if (assets_.find(asset_name) == assets_.end()) {
        setLastError("Asset not found: " + asset_name);
        return false;
    }
    assetStatuses_[asset_name] = AssetStatus::DESTROYED;
    assets_[asset_name].frozen = true;
    SimpleLogger::info("Asset destroyed: " + asset_name + " by " + owner_address);
    notifyAssetEvent("asset_destroyed", true);
    return true;
}

// Asset queries
bool AssetManager::getAssetMetadata(const std::string& asset_name, AssetMetadata& metadata) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return false;
    }

    auto it = assets_.find(asset_name);
    if (it == assets_.end()) {
        return false;
    }

    metadata = it->second;
    return true;
}

bool AssetManager::getAssetBalance(const std::string& address, const std::string& asset_name, uint64_t& balance) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return false;
    }

    auto assetIt = balances_.find(asset_name);
    if (assetIt == balances_.end()) {
        balance = 0;
        return true;
    }

    auto balanceIt = assetIt->second.find(address);
    if (balanceIt == assetIt->second.end()) {
        balance = 0;
        return true;
    }

    balance = balanceIt->second;
    return true;
}

bool AssetManager::getAssetOwners(const std::string& asset_name, std::vector<std::string>& owners) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return false;
    }

    auto it = balances_.find(asset_name);
    if (it == balances_.end()) {
        return false;
    }

    owners.clear();
    for (const auto& pair : it->second) {
        if (pair.second > 0) {
            owners.push_back(pair.first);
        }
    }
    
    return true;
}

bool AssetManager::getAssetStatus(const std::string& asset_name, AssetStatus& status) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = assetStatuses_.find(asset_name);
    if (it == assetStatuses_.end()) {
        return false;
    }
    status = it->second;
    return true;
}

bool AssetManager::getAssetType(const std::string& asset_name, AssetType& type) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return false;
    }

    auto it = assets_.find(asset_name);
    if (it == assets_.end()) {
        return false;
    }

    type = config_.type;
    return true;
}

// Asset verification
bool AssetManager::verifyAsset(const std::string& asset_name, AssetVerificationResult& result) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        result.valid = false;
        result.error_message = "AssetManager not initialized";
        return false;
    }

    auto it = assets_.find(asset_name);
    if (it == assets_.end()) {
        result.valid = false;
        result.error_message = "Asset not found";
        return false;
    }

    result.valid = true;
    result.error_message = "";
    result.warnings.clear();
    result.details = json::object();
    
    return true;
}

bool AssetManager::validateAssetMetadata(const AssetMetadata& metadata, AssetVerificationResult& result) const {
    result.valid = false;
    result.error_message.clear();
    result.warnings.clear();
    
    if (metadata.name.empty()) {
        result.error_message = "Asset name cannot be empty";
        return false;
    }
    if (metadata.symbol.empty()) {
        result.error_message = "Asset symbol cannot be empty";
        return false;
    }
    if (metadata.total_supply == 0) {
        result.error_message = "Asset total supply must be greater than 0";
        return false;
    }
    if (metadata.issuer.empty()) {
        result.error_message = "Asset issuer cannot be empty";
        return false;
    }
    if (metadata.decimals > 18) {
        result.warnings.push_back("Asset decimals should not exceed 18");
    }
    // Only set valid to true if all checks pass
    result.valid = true;
    return true;
}

bool AssetManager::validateAssetTransfer(const AssetTransferRequest& request, AssetVerificationResult& result) const {
    result.valid = true;
    result.error_message.clear();
    result.warnings.clear();
    
    if (request.asset_name.empty()) {
        result.valid = false;
        result.error_message = "Asset name cannot be empty";
        return false;
    }
    
    if (request.from_address.empty()) {
        result.valid = false;
        result.error_message = "From address cannot be empty";
        return false;
    }
    
    if (request.to_address.empty()) {
        result.valid = false;
        result.error_message = "To address cannot be empty";
        return false;
    }
    
    if (request.amount == 0) {
        result.valid = false;
        result.error_message = "Transfer amount must be greater than 0";
        return false;
    }
    
    if (request.from_address == request.to_address) {
        result.valid = false;
        result.error_message = "From and to addresses cannot be the same";
        return false;
    }
    
    return true;
}

bool AssetManager::validateAssetReissue(const AssetReissueRequest& request, AssetVerificationResult& result) const {
    result.valid = true;
    result.error_message.clear();
    result.warnings.clear();
    
    if (request.asset_name.empty()) {
        result.valid = false;
        result.error_message = "Asset name cannot be empty";
        return false;
    }
    
    if (request.owner_address.empty()) {
        result.valid = false;
        result.error_message = "Owner address cannot be empty";
        return false;
    }
    
    if (request.amount == 0) {
        result.valid = false;
        result.error_message = "Reissue amount must be greater than 0";
        return false;
    }
    
    return true;
}

bool AssetManager::validateAssetBurn(const AssetBurnRequest& request, AssetVerificationResult& result) const {
    result.valid = true;
    result.error_message.clear();
    result.warnings.clear();
    
    if (request.asset_name.empty()) {
        result.valid = false;
        result.error_message = "Asset name cannot be empty";
        return false;
    }
    
    if (request.owner_address.empty()) {
        result.valid = false;
        result.error_message = "Owner address cannot be empty";
        return false;
    }
    
    if (request.amount == 0) {
        result.valid = false;
        result.error_message = "Burn amount must be greater than 0";
        return false;
    }
    
    return true;
}

// State management
AssetState AssetManager::getState() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_;
}

bool AssetManager::isHealthy() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return initialized_ && state_ == AssetState::INITIALIZED;
}

// Configuration management
AssetConfig AssetManager::getConfig() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_;
}

bool AssetManager::updateConfig(const AssetConfig& new_config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        setLastError("AssetManager not initialized");
        return false;
    }

    // Validate new config
    if (new_config.maxAssets < assets_.size()) {
        setLastError("Cannot reduce max assets below current asset count");
        return false;
    }

    config_ = new_config;
    SimpleLogger::info("AssetManager configuration updated");
    
    return true;
}

bool AssetManager::validateConfig(const AssetConfig& config) const {
    return !config.name.empty() && config.maxAssets > 0 && config.timeout > 0;
}

// Statistics and monitoring
AssetStats AssetManager::getStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    AssetStats stats = stats_;
    stats.activeAssets = 0;
    stats.frozenAssets = 0;
    stats.destroyedAssets = 0;
    for (const auto& pair : assetStatuses_) {
        if (pair.second == AssetStatus::ACTIVE) stats.activeAssets++;
        if (pair.second == AssetStatus::FROZEN) stats.frozenAssets++;
        if (pair.second == AssetStatus::DESTROYED) stats.destroyedAssets++;
    }
    return stats;
}

void AssetManager::resetStats() {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_ = AssetStats{};
    SimpleLogger::info("AssetManager: Statistics reset");
}

bool AssetManager::enableStats(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    statsEnabled_ = enable;
    SimpleLogger::info("AssetManager: Statistics " + std::string(enable ? "enabled" : "disabled"));
    return true;
}

// Callback registration
void AssetManager::registerAssetCallback(AssetCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    assetCallbacks_.push_back(callback);
}

void AssetManager::registerOperationCallback(OperationCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    operationCallbacks_.push_back(callback);
}

void AssetManager::registerErrorCallback(ErrorCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    errorCallbacks_.push_back(callback);
}

void AssetManager::unregisterAssetCallback() {
    std::lock_guard<std::mutex> lock(mutex_);
    assetCallbacks_.clear();
}

void AssetManager::unregisterOperationCallback() {
    std::lock_guard<std::mutex> lock(mutex_);
    operationCallbacks_.clear();
}

void AssetManager::unregisterErrorCallback() {
    std::lock_guard<std::mutex> lock(mutex_);
    errorCallbacks_.clear();
}

// Health check
bool AssetManager::healthCheck() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return false;
    }

    lastHealthCheck_ = std::chrono::system_clock::now();
    return true;
}

nlohmann::json AssetManager::getHealthStatus() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    json status;
    status["initialized"] = initialized_;
    status["state"] = static_cast<int>(state_);
    status["total_assets"] = assets_.size();
    status["last_health_check"] = std::chrono::duration_cast<std::chrono::seconds>(
        lastHealthCheck_.time_since_epoch()).count();
    status["consecutive_failures"] = consecutiveFailures_;
    
    return status;
}

// Additional functionality
nlohmann::json AssetManager::getAssetInfo(const std::string& asset_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    json info;
    auto it = assets_.find(asset_name);
    if (it != assets_.end()) {
        info["name"] = it->second.name;
        info["symbol"] = it->second.symbol;
        info["description"] = it->second.description;
        info["issuer"] = it->second.issuer;
        info["total_supply"] = it->second.total_supply;
        info["decimals"] = it->second.decimals;
        info["reissuable"] = it->second.reissuable;
        info["frozen"] = it->second.frozen;
        info["status"] = static_cast<int>(assetStatuses_[asset_name]);
        info["additional_data"] = it->second.additional_data;
    }
    
    return info;
}

std::vector<nlohmann::json> AssetManager::listAssets() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<json> assets;
    for (const auto& pair : assets_) {
        json asset;
        asset["name"] = pair.second.name;
        asset["symbol"] = pair.second.symbol;
        asset["status"] = static_cast<int>(assetStatuses_[pair.first]);
        assets.push_back(asset);
    }
    
    return assets;
}

std::vector<nlohmann::json> AssetManager::getAssetHistory(const std::string& asset_name) {
    // TODO: Implement asset history tracking
    return std::vector<json>();
}

std::vector<nlohmann::json> AssetManager::getAssetBalances(const std::string& asset_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<json> balances;
    auto it = balances_.find(asset_name);
    if (it != balances_.end()) {
        for (const auto& pair : it->second) {
            json balance;
            balance["address"] = pair.first;
            balance["balance"] = pair.second;
            balances.push_back(balance);
        }
    }
    
    return balances;
}

std::vector<nlohmann::json> AssetManager::getAssetTransactions(const std::string& asset_name) {
    // TODO: Implement transaction history tracking
    return std::vector<json>();
}

// Private helper methods
bool AssetManager::validateAssetCreationRequest(const AssetCreationRequest& request, AssetVerificationResult& result) const {
    result.valid = true;
    result.error_message.clear();
    result.warnings.clear();
    
    // Validate required fields
    if (request.name.empty()) {
        result.valid = false;
        result.error_message = "Asset name cannot be empty";
        return false;
    }
    
    if (request.symbol.empty()) {
        result.valid = false;
        result.error_message = "Asset symbol cannot be empty";
        return false;
    }
    
    if (request.amount == 0) {
        result.valid = false;
        result.error_message = "Asset amount must be greater than 0";
        return false;
    }
    
    if (request.owner_address.empty()) {
        result.valid = false;
        result.error_message = "Owner address cannot be empty";
        return false;
    }
    
    // Validate units (decimals)
    if (request.units > 18) {
        result.warnings.push_back("Asset units should not exceed 18");
    }
    
    return true;
}

void AssetManager::setLastError(const std::string& error) {
    lastError_ = error;
    SimpleLogger::error("AssetManager error: " + error);
}

void AssetManager::notifyAssetEvent(const std::string& event, bool success) {
    for (const auto& callback : assetCallbacks_) {
        try {
            callback(event, success);
        } catch (const std::exception& e) {
            SimpleLogger::error("Asset callback error: " + std::string(e.what()));
        }
    }
}

void AssetManager::notifyOperationEvent(const AssetResult& result) {
    for (const auto& callback : operationCallbacks_) {
        try {
            callback(result.operation, result.success);
        } catch (const std::exception& e) {
            SimpleLogger::error("Operation callback error: " + std::string(e.what()));
        }
    }
}

void AssetManager::notifyErrorEvent(const std::string& operation, const std::string& error) {
    for (const auto& callback : errorCallbacks_) {
        try {
            callback(operation, error);
        } catch (const std::exception& e) {
            SimpleLogger::error("Error callback error: " + std::string(e.what()));
        }
    }
}

void AssetManager::updateStats(bool success, double operationTime) {
    if (!statsEnabled_) {
        return;
    }

    stats_.totalOperations++;
    if (success) {
        stats_.successfulOperations++;
    } else {
        stats_.failedOperations++;
    }

    stats_.lastOperation = std::chrono::system_clock::now();
    
    // Update average operation time
    if (stats_.totalOperations > 1) {
        stats_.averageOperationTime = (stats_.averageOperationTime * (stats_.totalOperations - 1) + operationTime) / stats_.totalOperations;
    } else {
        stats_.averageOperationTime = operationTime;
    }
}

bool AssetManager::validateState() const {
    return initialized_ && state_ == AssetState::INITIALIZED;
}

void AssetManager::logOperation(const std::string& operation, bool success, double duration) {
    if (config_.enableLogging) {
        SimpleLogger::info("Asset operation: " + operation + " - " + (success ? "SUCCESS" : "FAILED") + " (" + std::to_string(duration) + "ms)");
    }
}

void AssetManager::clearLastError() {
    std::lock_guard<std::mutex> lock(mutex_);
    lastError_.clear();
}

} // namespace satox::asset 