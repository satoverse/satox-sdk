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

#include "satox/core/satox_manager.hpp"
#include "satox/network/network_manager.hpp"
#include "satox/core/security_manager.hpp"
#include "satox/core/database_manager.hpp"
#include "satox/wallet/wallet_manager.hpp"
#include "satox/core/quantum_manager.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>
#include <mutex>
#include <filesystem>
#include <algorithm>
#include <chrono>

namespace satox {
namespace core {

SatoxManager& SatoxManager::getInstance() {
    static SatoxManager instance;
    return instance;
}

bool SatoxManager::initialize(const nlohmann::json& config) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    if (initialized_) {
        lastError_ = "SatoxManager already initialized";
        return false;
    }

    config_ = config;
    
    // Initialize core components
    if (!initializeCoreComponent()) {
        return false;
    }
    
    if (!initializeBlockchainComponent()) {
        return false;
    }
    
    if (!initializeTransactionComponent()) {
        return false;
    }
    
    if (!initializeWalletComponent()) {
        return false;
    }
    
    if (!initializeIPFSComponent()) {
        return false;
    }
    
    if (!initializeNetworkComponent()) {
        return false;
    }
    
    if (!initializeConfigComponent()) {
        return false;
    }
    
    initialized_ = true;
    stats_.start_time = std::chrono::system_clock::now();
    stats_.last_activity = std::chrono::system_clock::now();
    
    return true;
}

bool SatoxManager::shutdown() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!initialized_) {
        return true;
    }
    // Shutdown components in reverse order
    // Note: Components are singletons, so we just update our state
    initialized_ = false;
    stats_.last_activity = std::chrono::system_clock::now();
    components_.clear(); // Clear registered components for clean state
    return true;
}

bool SatoxManager::clearLastError() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    lastError_.clear();
    return true;
}

std::string SatoxManager::getLastError() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return lastError_;
}

// System state methods
bool SatoxManager::start() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = "SatoxManager not initialized";
        return false;
    }
    running_ = true;
    return true;
}

bool SatoxManager::stop() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    running_ = false;
    return true;
}

bool SatoxManager::pause() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!running_) {
        lastError_ = "SatoxManager not running";
        return false;
    }
    paused_ = true;
    return true;
}

bool SatoxManager::resume() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    paused_ = false;
    return true;
}

bool SatoxManager::isRunning() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return running_;
}

bool SatoxManager::isPaused() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return paused_;
}

// Configuration methods
bool SatoxManager::update(const nlohmann::json& config) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    config_ = config;
    return true;
}

std::optional<nlohmann::json> SatoxManager::getConfig() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return std::optional<nlohmann::json>(config_);
}

// Manager access methods
CoreManager& SatoxManager::getCoreManager() {
    return CoreManager::getInstance();
}

NetworkManager& SatoxManager::getNetworkManager() {
    return NetworkManager::getInstance();
}

BlockchainManager& SatoxManager::getBlockchainManager() {
    return BlockchainManager::getInstance();
}

TransactionManager& SatoxManager::getTransactionManager() {
    return TransactionManager::getInstance();
}

AssetManager& SatoxManager::getAssetManager() {
    return AssetManager::getInstance();
}

WalletManager& SatoxManager::getWalletManager() {
    return WalletManager::getInstance();
}

IPFSManager& SatoxManager::getIPFSManager() {
    return IPFSManager::getInstance();
}

// Comment out managers that don't have getInstance() methods yet
/*
DatabaseManager& SatoxManager::getDatabaseManager() {
    return DatabaseManager::getInstance();
}

SecurityManager& SatoxManager::getSecurityManager() {
    return SecurityManager::getInstance();
}
*/

NFTManager& SatoxManager::getNFTManager() {
    return NFTManager::getInstance();
}

PluginManager& SatoxManager::getPluginManager() {
    return PluginManager::getInstance();
}

EventManager& SatoxManager::getEventManager() {
    return EventManager::getInstance();
}

CacheManager& SatoxManager::getCacheManager() {
    return CacheManager::getInstance();
}

ConfigManager& SatoxManager::getConfigManager() {
    return ConfigManager::getInstance();
}

LoggingManager& SatoxManager::getLoggingManager() {
    static LoggingManager dummyLogger;
    return dummyLogger;
}

// System information methods
std::optional<SatoxManager::SystemInfo> SatoxManager::getSystemInfo() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!initialized_) {
        return std::nullopt;
    }
    
    SystemInfo info;
    info.initialized = initialized_;
    info.running = running_;
    info.paused = paused_;
    info.version = "1.0.0";
    info.build = "debug";
    
    // Add manager status
    info.managerStatus["core"] = true;
    info.managerStatus["network"] = true;
    info.managerStatus["blockchain"] = true;
    info.managerStatus["transaction"] = true;
    info.managerStatus["asset"] = true;
    info.managerStatus["wallet"] = true;
    info.managerStatus["ipfs"] = true;
    info.managerStatus["database"] = true;
    info.managerStatus["security"] = true;
    info.managerStatus["nft"] = true;
    
    return info;
}

std::optional<ComponentStatus> SatoxManager::getComponentStatus(const std::string& component) const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    // For now, return a default status for any component
    ComponentStatus status;
    status.name = component;
    status.state = ComponentState::RUNNING;
    // status.health = ComponentHealth::HEALTHY;  // Not available in current struct
    // status.last_activity = std::chrono::system_clock::now();  // Not available in current struct
    return status;
}

std::vector<std::string> SatoxManager::getAvailableComponents() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return {"core", "network", "blockchain", "transaction", "asset", "wallet", "ipfs", "database", "security", "nft"};
}

// System statistics methods
SatoxManager::SystemStats SatoxManager::getSystemStats() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return stats_;
}

bool SatoxManager::enableStats(bool enable) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    statsEnabled_ = enable;
    return true;
}

bool SatoxManager::resetStats() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    stats_ = SystemStats{};
    stats_.start_time = std::chrono::system_clock::now();
    stats_.last_activity = std::chrono::system_clock::now();
    return true;
}

// Callback methods
bool SatoxManager::registerStateCallback(StateCallback callback) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    stateCallback_ = callback;
    return true;
}

bool SatoxManager::registerErrorCallback(ErrorCallback callback) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    errorCallback_ = callback;
    return true;
}

bool SatoxManager::registerStatsCallback(StatsCallback callback) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    statsCallback_ = callback;
    return true;
}

bool SatoxManager::unregisterStateCallback() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    stateCallback_ = nullptr;
    return true;
}

bool SatoxManager::unregisterErrorCallback() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    errorCallback_ = nullptr;
    return true;
}

bool SatoxManager::unregisterStatsCallback() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    statsCallback_ = nullptr;
    return true;
}

// Configuration management methods
bool SatoxManager::updateConfig(const nlohmann::json& config) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    config_ = config;
    return true;
}

bool SatoxManager::reloadConfig() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    // For now, just return true
    return true;
}

// Statistics and monitoring methods
SatoxManager::Stats SatoxManager::getStats() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    Stats stats;
    stats.total_components = stats_.total_components;
    stats.active_components = stats_.active_components;
    stats.failed_components = stats_.failed_components;
    stats.start_time = stats_.start_time;
    stats.last_activity = stats_.last_activity;
    return stats;
}

bool SatoxManager::isHealthy() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return initialized_ && stats_.failed_components == 0;
}

// Private helper methods
void SatoxManager::setError(const std::string& error) {
    lastError_ = error;
    if (errorCallback_) {
        errorCallback_(error);
    }
}

bool SatoxManager::validateConfig(const nlohmann::json& config) const {
    // For now, accept any valid JSON
    return !config.is_null();
}

void SatoxManager::updateState() {
    if (stateCallback_) {
        auto info = getSystemInfo();
        if (info) {
            stateCallback_(*info);
        }
    }
}

void SatoxManager::handleError(const std::string& error) {
    setError(error);
}

void SatoxManager::updateStats() {
    stats_.last_activity = std::chrono::system_clock::now();
    if (statsCallback_) {
        statsCallback_(stats_);
    }
}

// Component initialization methods
bool SatoxManager::initializeCoreComponent() {
    try {
        // CoreManager is a singleton, just check if it's accessible
        CoreManager::getInstance();
        stats_.total_components++;
        stats_.active_components++;
        return true;
    } catch (const std::exception& e) {
        lastError_ = "Failed to initialize core component: " + std::string(e.what());
        stats_.failed_components++;
        return false;
    }
}

bool SatoxManager::initializeBlockchainComponent() {
    try {
        // BlockchainManager is a singleton, just check if it's accessible
        auto& blockchain = BlockchainManager::getInstance();
        stats_.total_components++;
        stats_.active_components++;
        return true;
    } catch (const std::exception& e) {
        lastError_ = "Failed to initialize blockchain component: " + std::string(e.what());
        stats_.failed_components++;
        return false;
    }
}

bool SatoxManager::initializeTransactionComponent() {
    try {
        // TransactionManager is a singleton, just check if it's accessible
        auto& transaction = TransactionManager::getInstance();
        stats_.total_components++;
        stats_.active_components++;
        return true;
    } catch (const std::exception& e) {
        lastError_ = "Failed to initialize transaction component: " + std::string(e.what());
        stats_.failed_components++;
        return false;
    }
}

bool SatoxManager::initializeWalletComponent() {
    try {
        // WalletManager is a singleton, just check if it's accessible
        auto& wallet = WalletManager::getInstance();
        stats_.total_components++;
        stats_.active_components++;
        return true;
    } catch (const std::exception& e) {
        lastError_ = "Failed to initialize wallet component: " + std::string(e.what());
        stats_.failed_components++;
        return false;
    }
}

bool SatoxManager::initializeIPFSComponent() {
    try {
        // IPFSManager is a singleton, just check if it's accessible
        auto& ipfs = IPFSManager::getInstance();
        stats_.total_components++;
        stats_.active_components++;
        return true;
    } catch (const std::exception& e) {
        lastError_ = "Failed to initialize IPFS component: " + std::string(e.what());
        stats_.failed_components++;
        return false;
    }
}

bool SatoxManager::initializeNetworkComponent() {
    try {
        // NetworkManager is a singleton, just check if it's accessible
        auto& network = NetworkManager::getInstance();
        stats_.total_components++;
        stats_.active_components++;
        return true;
    } catch (const std::exception& e) {
        lastError_ = "Failed to initialize network component: " + std::string(e.what());
        stats_.failed_components++;
        return false;
    }
}

bool SatoxManager::initializeConfigComponent() {
    try {
        // ConfigManager is a singleton, just check if it's accessible
        auto& config = ConfigManager::getInstance();
        stats_.total_components++;
        stats_.active_components++;
        return true;
    } catch (const std::exception& e) {
        lastError_ = "Failed to initialize config component: " + std::string(e.what());
        stats_.failed_components++;
        return false;
    }
}

} // namespace core
} // namespace satox 