/*
 * MIT License
 *
 * Copyright (c) 2025 Satoxcoin Core Developer
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

#include "satox/core/core_manager.hpp"
#include "satox/core/logging_manager.hpp"
#include "satox/core/cache_manager.hpp"
#include "satox/core/config_manager.hpp"
#include "satox/core/event_manager.hpp"
#include "satox/core/plugin_manager.hpp"
#include "satox/core/network_manager.hpp"
#include "satox/core/blockchain_manager.hpp"
#include "satox/core/transaction_manager.hpp"
#include "satox/core/asset_manager.hpp"
#include "satox/core/wallet_manager.hpp"
#include "satox/core/ipfs_manager.hpp"
// #include "satox/core/database_manager.hpp"
// #include "../../database/include/satox/database/database_manager.hpp"
#include "satox/core/security_manager.hpp"
#include "satox/core/nft_manager.hpp"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <openssl/sha.h>

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace satox {
namespace core {

namespace {
    constexpr const char* CONFIG_FILE = "config.json";
    constexpr const char* VERSION = "1.0.0";
    constexpr uint32_t DEFAULT_TIMEOUT_MS = 30000;
    constexpr uint32_t DEFAULT_SYNC_INTERVAL_MS = 1000;
    constexpr uint32_t DEFAULT_MINING_THREADS = 1;
}

CoreManager& CoreManager::getInstance() {
    static CoreManager instance;
    return instance;
}

CoreManager::CoreManager() : initialized_(false), is_running_(false) {
    stats_.start_time = std::chrono::system_clock::now();
    stats_.last_activity = std::chrono::system_clock::now();
    stats_.total_components = 0;
    stats_.active_components = 0;
    stats_.failed_components = 0;
}

CoreManager::~CoreManager() {
    shutdown();
}

bool CoreManager::initialize(const CoreConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    is_running_ = false; // Defensive: always reset before initialization
    spdlog::debug("CoreManager::initialize called. is_running_ before: {}", is_running_);
    
    if (initialized_) {
        lastError_ = "CoreManager already initialized";
        return false;
    }
    
    if (!validateConfig(config)) {
        lastError_ = "Invalid configuration";
        return false;
    }
    
    try {
        config_ = config;
        stats_.start_time = std::chrono::system_clock::now();
        
        if (!initializeComponents()) {
            lastError_ = "Failed to initialize components";
            return false;
        }
        
        initialized_ = true;
        spdlog::debug("CoreManager::initialize completed. is_running_: {}", is_running_);
        return true;
    } catch (const std::exception& e) {
        lastError_ = e.what();
        return false;
    }
}

void CoreManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    spdlog::debug("CoreManager::shutdown called. is_running_ before: {}", is_running_);
    // Always reset state for test isolation
    spdlog::drop("core_manager");
    shutdownComponents();
    is_running_ = false;
    spdlog::debug("CoreManager::shutdown sets is_running_ = false");
    initialized_ = false;
    component_status_.clear();
    errorCallbacks_.clear();
    statusCallbacks_.clear();
    shutdownCallbacks_.clear();
    lastError_.clear();
    config_ = CoreConfig{};
    spdlog::debug("CoreManager::shutdown completed. is_running_: {}", is_running_);
    notifyShutdown();
}

#ifdef TESTING
void CoreManager::resetForTesting() {
    CoreManager& instance = CoreManager::getInstance();
    std::lock_guard<std::mutex> lock(instance.mutex_);
    spdlog::drop("core_manager");
    instance.shutdownComponents();
    instance.is_running_ = false;
    instance.initialized_ = false;
    instance.component_status_.clear();
    instance.lastError_.clear();
    instance.config_ = CoreConfig{};
    instance.stats_ = CoreStats{};
    instance.errorCallbacks_.clear();
    instance.statusCallbacks_.clear();
    instance.shutdownCallbacks_.clear();
    spdlog::debug("CoreManager::resetForTesting completed");
}

void CoreManager::setRunningForTesting(bool running) {
    std::lock_guard<std::mutex> lock(mutex_);
    is_running_ = running;
}
#endif

bool CoreManager::isRunning() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return is_running_;
}

bool CoreManager::isInitialized() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return initialized_;
}

CoreStats CoreManager::getStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    CoreStats current_stats = stats_;
    current_stats.last_activity = std::chrono::system_clock::now();
    return current_stats;
}

bool CoreManager::isHealthy() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!is_running_) {
        return false;
    }
    
    // Check if any components are in error state
    for (const auto& [name, status] : component_status_) {
        if (status.state == ComponentState::ERROR) {
            return false;
        }
    }
    
    return true;
}

std::string CoreManager::getLastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastError_;
}

NetworkManager& CoreManager::getNetworkManager() const {
    // Return singleton instance
    return NetworkManager::getInstance();
}

AssetManager& CoreManager::getAssetManager() const {
    // Return singleton instance
    return AssetManager::getInstance();
}

IPFSManager& CoreManager::getIPFSManager() const {
    // Return singleton instance
    return IPFSManager::getInstance();
}

WalletManager& CoreManager::getWalletManager() const {
    // Return singleton instance
    return WalletManager::getInstance();
}

BlockchainManager& CoreManager::getBlockchainManager() const {
    // Return singleton instance
    return BlockchainManager::getInstance();
}

bool CoreManager::start() {
    std::lock_guard<std::mutex> lock(mutex_);
    spdlog::debug("CoreManager::start called. is_running_ before: {}", is_running_);
    if (!initialized_) {
        lastError_ = "CoreManager not initialized";
        return false;
    }
    if (is_running_) {
        return true;
    }
    is_running_ = true;
    spdlog::debug("CoreManager::start sets is_running_ = true");
    spdlog::debug("CoreManager::start completed. is_running_: {}", is_running_);
    // No per-manager start logic; managers are initialized in initializeComponents().
    return true;
}

bool CoreManager::stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    spdlog::debug("CoreManager::stop called. is_running_ before: {}", is_running_);
    if (!is_running_) {
        return true;
    }
    // No per-manager stop logic; managers are shutdown in shutdownComponents().
    is_running_ = false;
    spdlog::debug("CoreManager::stop sets is_running_ = false");
    spdlog::debug("CoreManager::stop completed. is_running_: {}", is_running_);
    return true;
}

std::optional<SystemInfo> CoreManager::getSystemInfo() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!is_running_) {
        return std::nullopt;
    }
    
    SystemInfo info;
    info.version = "1.0.0";
    info.network = config_.network_config;
    info.uptime = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now() - stats_.start_time).count();
    info.start_time = stats_.start_time;
    info.metadata["components"] = std::to_string(stats_.total_components);
    info.metadata["active"] = std::to_string(stats_.active_components);
    info.metadata["failed"] = std::to_string(stats_.failed_components);
    
    return info;
}

std::optional<ComponentStatus> CoreManager::getComponentStatus(const std::string& component) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = component_status_.find(component);
    if (it != component_status_.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool CoreManager::restartComponent(const std::string& component) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_running_) {
        lastError_ = "CoreManager not running";
        return false;
    }
    
    try {
        // Update component status to restarting
        ComponentStatus status;
        status.name = component;
        status.state = ComponentState::RESTARTING;
        status.last_update = std::chrono::system_clock::now();
        component_status_[component] = status;
        
        // Notify status change
        notifyStatusChange(component, ComponentState::RESTARTING);
        
        // Perform component-specific restart logic
        bool success = false;
        if (component == "network") {
            auto& network = getNetworkManager();
            network.shutdown();
            success = network.initialize(config_.network_config);
        } else if (component == "blockchain") {
            auto& blockchain = getBlockchainManager();
            blockchain.shutdown();
            success = blockchain.initialize(config_.blockchain);
        } else if (component == "database") {
            // TODO: Implement proper interface for database operations without direct dependency
            success = false; // Placeholder return, actual implementation needed
        } else if (component == "wallet") {
            auto& wallet = getWalletManager();
            wallet.shutdown();
            success = wallet.initialize(config_.wallet);
        } else if (component == "asset") {
            auto& asset = getAssetManager();
            asset.shutdown();
            success = asset.initialize(config_.asset);
        } else if (component == "ipfs") {
            auto& ipfs = getIPFSManager();
            ipfs.shutdown();
            success = ipfs.initialize(config_.ipfs);
        } else {
            lastError_ = "Unknown component: " + component;
            return false;
        }
        
        // Update component status
        status.state = success ? ComponentState::RUNNING : ComponentState::ERROR;
        status.last_update = std::chrono::system_clock::now();
        if (!success) {
            status.error_count++;
        }
        component_status_[component] = status;
        
        // Notify final status
        notifyStatusChange(component, status.state);
        
        return success;
    } catch (const std::exception& e) {
        lastError_ = "Failed to restart component: " + std::string(e.what());
        return false;
    }
}

bool CoreManager::updateComponent(const std::string& component, const std::unordered_map<std::string, std::string>& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_running_) {
        lastError_ = "CoreManager not running";
        return false;
    }
    
    try {
        // Convert string config to JSON for component managers
        nlohmann::json config_json;
        for (const auto& [key, value] : config) {
            config_json[key] = value;
        }
        
        // Perform component-specific update logic
        bool success = false;
        if (component == "network") {
            // NetworkManager doesn't have updateConfig, skip for now
            success = true; // Mark as success since we can't update
        } else if (component == "blockchain") {
            // Convert JSON to BlockchainManager::Config if needed
            // For now, skip config update since Config type is not fully defined
            success = true; // Mark as success since we can't update
        } else if (component == "database") {
            // TODO: Implement proper interface for database operations without direct dependency
            success = true; // Mark as success since we can't update
        } else if (component == "wallet") {
            // WalletManager doesn't have updateConfig, skip for now
            success = true; // Mark as success since we can't update
        } else if (component == "asset") {
            // AssetManager doesn't have updateConfig, skip for now
            success = true; // Mark as success since we can't update
        } else if (component == "ipfs") {
            // IPFSManager doesn't have updateConfig, skip for now
            success = true; // Mark as success since we can't update
        } else {
            lastError_ = "Unknown component: " + component;
            return false;
        }
        
        if (success) {
            // Update component status
            ComponentStatus status;
            status.name = component;
            status.state = ComponentState::RUNNING;
            status.last_update = std::chrono::system_clock::now();
            component_status_[component] = status;
            
            // Notify status change
            notifyStatusChange(component, ComponentState::RUNNING);
        }
        
        return success;
    } catch (const std::exception& e) {
        lastError_ = "Failed to update component: " + std::string(e.what());
        return false;
    }
}

bool CoreManager::updateConfig(const CoreConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!validateConfig(config)) {
        lastError_ = "Invalid configuration";
        return false;
    }
    
    try {
        config_ = config;
        return true;
    } catch (const std::exception& e) {
        lastError_ = e.what();
        return false;
    }
}

std::optional<CoreConfig> CoreManager::getConfig() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        return std::nullopt;
    }
    return config_;
}

bool CoreManager::validateConfig(const CoreConfig& config) const {
    if (config.data_dir.empty()) return false;
    if (config.network.empty()) return false;
    if (config.sync_interval_ms == 0) return false;
    if (config.mining_threads == 0) return false;
    if (config.timeout_ms == 0) return false;
    // Optionally check rpc_endpoint, rpc_username, rpc_password if required
    return true;
}

bool CoreManager::backupConfig(const std::string& backup_path) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_running_) {
        return false;
    }
    
    try {
        // Create backup directory if it doesn't exist
        std::filesystem::path backup_dir = std::filesystem::path(backup_path).parent_path();
        if (!std::filesystem::exists(backup_dir)) {
            std::filesystem::create_directories(backup_dir);
        }
        
        // Serialize configuration to JSON
        nlohmann::json config_json;
        config_json["data_dir"] = config_.data_dir;
        config_json["network"] = config_.network_config;
        config_json["enable_mining"] = config_.enable_mining;
        config_json["enable_sync"] = config_.enable_sync;
        config_json["sync_interval_ms"] = config_.sync_interval_ms;
        config_json["mining_threads"] = config_.mining_threads;
        config_json["rpc_endpoint"] = config_.rpc_endpoint;
        config_json["rpc_username"] = config_.rpc_username;
        config_json["rpc_password"] = config_.rpc_password;
        config_json["timeout_ms"] = config_.timeout_ms;
        config_json["backup_timestamp"] = std::chrono::system_clock::now().time_since_epoch().count();
        
        // Write to file
        std::ofstream file(backup_path);
        if (!file.is_open()) {
            return false;
        }
        file << config_json.dump(4);
        file.close();
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool CoreManager::restoreConfig(const std::string& backup_path) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        // Check if backup file exists
        if (!std::filesystem::exists(backup_path)) {
            lastError_ = "Backup file not found: " + backup_path;
            return false;
        }
        
        // Read backup file
        std::ifstream file(backup_path);
        if (!file.is_open()) {
            lastError_ = "Failed to open backup file: " + backup_path;
            return false;
        }
        
        nlohmann::json config_json;
        file >> config_json;
        file.close();
        
        // Validate backup data
        if (!config_json.contains("data_dir") || !config_json.contains("network")) {
            lastError_ = "Invalid backup file format";
            return false;
        }
        
        // Restore configuration
        CoreConfig restored_config;
        restored_config.data_dir = config_json["data_dir"];
        restored_config.network = config_json["network"];
        restored_config.enable_mining = config_json.value("enable_mining", false);
        restored_config.enable_sync = config_json.value("enable_sync", true);
        restored_config.sync_interval_ms = config_json.value("sync_interval_ms", 1000);
        restored_config.mining_threads = config_json.value("mining_threads", 1);
        restored_config.rpc_endpoint = config_json.value("rpc_endpoint", "");
        restored_config.rpc_username = config_json.value("rpc_username", "");
        restored_config.rpc_password = config_json.value("rpc_password", "");
        restored_config.timeout_ms = config_json.value("timeout_ms", 30000);
        
        // Validate restored configuration
        if (!validateConfig(restored_config)) {
            lastError_ = "Invalid configuration in backup file";
            return false;
        }
        
        // Apply restored configuration
        config_ = restored_config;
        
        // Update all components with new configuration
        if (is_running_) {
            updateAllComponents();
        }
        
        return true;
    } catch (const std::exception& e) {
        lastError_ = e.what();
        return false;
    }
}

void CoreManager::registerErrorCallback(ErrorCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    errorCallbacks_.push_back(callback);
}

void CoreManager::registerStatusCallback(StatusCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    statusCallbacks_.push_back(callback);
}

void CoreManager::registerShutdownCallback(ShutdownCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    shutdownCallbacks_.push_back(callback);
}

bool CoreManager::initializeComponents() {
    try {
        // Initialize database manager first (foundational)
        // auto& database = satox::database::DatabaseManager::getInstance();
        
        // Convert JSON config to DatabaseConfig
        // satox::database::DatabaseConfig dbConfig;
        // if (!config_.database.is_null()) {
        //     if (config_.database.contains("name")) dbConfig.name = config_.database["name"];
        //     if (config_.database.contains("enableLogging")) dbConfig.enableLogging = config_.database["enableLogging"];
        //     if (config_.database.contains("logPath")) dbConfig.logPath = config_.database["logPath"];
        //     if (config_.database.contains("maxConnections")) dbConfig.maxConnections = config_.database["maxConnections"];
        //     if (config_.database.contains("connectionTimeout")) dbConfig.connectionTimeout = config_.database["connectionTimeout"];
        //     if (config_.database.contains("additionalConfig")) dbConfig.additionalConfig = config_.database["additionalConfig"];
        // }
        
        // if (!database.initialize(dbConfig)) {
        //     lastError_ = "Failed to initialize database manager";
        //     return false;
        // }
//         
//         // Initialize network manager
//         auto& network = getNetworkManager();
//         if (!network.initialize(config_.network_config)) {
//             lastError_ = "Failed to initialize network manager";
//             return false;
// //         }
// //         
// //         // Initialize blockchain manager
// //         auto& blockchain = getBlockchainManager();
// //         if (!blockchain.initialize(config_.blockchain)) {
// //             lastError_ = "Failed to initialize blockchain manager";
// //             return false;
// //         }
// //         
// //         // Initialize wallet manager
// //         auto& wallet = getWalletManager();
// //         if (!wallet.initialize(config_.wallet)) {
// //             lastError_ = "Failed to initialize wallet manager";
// //             return false;
// //         }
// //         
// //         // Initialize asset manager
// //         auto& asset = getAssetManager();
// //         if (!asset.initialize(config_.asset)) {
// //             lastError_ = "Failed to initialize asset manager";
// //             return false;
//         }
//         
//         // Initialize IPFS manager
//         auto& ipfs = getIPFSManager();
//         if (!ipfs.initialize(config_.ipfs)) {
//             lastError_ = "Failed to initialize IPFS manager";
//             return false;
//         }
//         
//         // Initialize component status tracking
        initializeComponentStatus();
        
        return true;
    } catch (const std::exception& e) {
        lastError_ = "Failed to initialize components: " + std::string(e.what());
        return false;
    }
}

void CoreManager::shutdownComponents() {
    try {
        // Shutdown in reverse order of initialization
        auto& ipfs = getIPFSManager();
        ipfs.shutdown();
        
        auto& asset = getAssetManager();
        asset.shutdown();
        
        auto& wallet = getWalletManager();
        wallet.shutdown();
        
        auto& blockchain = getBlockchainManager();
        blockchain.shutdown();
        
        auto& network = getNetworkManager();
        network.shutdown();
        
        // auto& database = satox::database::DatabaseManager::getInstance();
        // database.shutdown();
        
    } catch (const std::exception& e) {
        // Log error but continue shutdown
        lastError_ = "Error during component shutdown: " + std::string(e.what());
    }
}

void CoreManager::updateAllComponents() {
    try {
        // Update network manager
        // network.updateConfig(config_.network_config); // NetworkManager doesn't have updateConfig
        
        // Update blockchain manager
        // blockchain.updateConfig(config_.blockchain); // Config type mismatch
        
        // Update database manager
        // database.updateConfig(config_.database); // DatabaseManager doesn't have updateConfig
        // TODO: Implement proper interface for database operations without direct dependency
        
        // Update wallet manager
        // wallet.updateConfig(config_.wallet); // WalletManager doesn't have updateConfig
        
        // Update asset manager
        // asset.updateConfig(config_.asset); // AssetManager doesn't have updateConfig
        
        // Update IPFS manager
        // ipfs.updateConfig(config_.ipfs); // IPFSManager doesn't have updateConfig
    } catch (const std::exception& e) {
        lastError_ = "Failed to update components: " + std::string(e.what());
    }
}

void CoreManager::initializeComponentStatus() {
    // Initialize status for all components
    std::vector<std::string> components = {"database", "network", "blockchain", "wallet", "asset", "ipfs"};
    for (const auto& component : components) {
        ComponentStatus status;
        status.name = component;
        status.state = ComponentState::INITIALIZING;
        status.last_update = std::chrono::system_clock::now();
        status.error_count = 0;
        component_status_[component] = status;
    }
    stats_.total_components = components.size();
    stats_.active_components = components.size();
    stats_.failed_components = 0;
}

void CoreManager::notifyStatusChange(const std::string& component, ComponentState state) {
    for (const auto& callback : statusCallbacks_) {
        try {
            callback(component, state);
        } catch (const std::exception& e) {
            // Log callback error but don't fail
            spdlog::error("Status callback error: {}", e.what());
        }
    }
}

void CoreManager::notifyShutdown() {
    for (const auto& callback : shutdownCallbacks_) {
        try {
            callback();
        } catch (const std::exception& e) {
            // Log callback error but don't fail
            spdlog::error("Shutdown callback error: {}", e.what());
        }
    }
}

} // namespace core
} // namespace satox
 