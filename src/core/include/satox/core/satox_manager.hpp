/*
 * MIT License
 * Copyright(c) 2025 Satoxcoin Core Developer
 */

#pragma once

#include "satox/core/core_manager.hpp"
#include "satox/core/network_manager.hpp"
#include "satox/core/blockchain_manager.hpp"
#include "satox/core/transaction_manager.hpp"
#include "satox/core/asset_manager.hpp"
#include "satox/core/wallet_manager.hpp"
#include "satox/core/ipfs_manager.hpp"
#include "satox/core/database_manager.hpp"
#include "satox/core/security_manager.hpp"
#include "satox/core/nft_manager.hpp"
#include "satox/core/plugin_manager.hpp"
#include "satox/core/event_manager.hpp"
#include "satox/core/cache_manager.hpp"
#include "satox/core/config_manager.hpp"
#include "satox/core/logging_manager.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <optional>
#include <vector>
#include <map>

namespace satox::core {

/**
 * @brief Interface for components that can be shut down
 */
class IShutdownable {
public:
    virtual ~IShutdownable() = default;
    virtual void shutdown() = 0;
};

/**
 * @brief Main manager class for Satox SDK core functionality
 * 
 * This class serves as the central point for managing core SDK functionality,
 * including initialization, shutdown, and component management.
 */
class SatoxManager {
public:
    /**
     * @brief Get the singleton instance of SatoxManager
     * @return Reference to the SatoxManager instance
     */
    static SatoxManager& getInstance();

    /**
     * @brief Initialize the SDK with configuration
     * @param configPath Path to configuration file
     * @return true if initialization successful, false otherwise
     */
    bool initialize(const nlohmann::json& config = nlohmann::json::object());

    /**
     * @brief Shutdown the SDK and cleanup resources
     */
    bool shutdown();

    /**
     * @brief Register a component with the manager
     * @tparam T Component type
     * @param name Component name
     * @param component Component instance
     * @return true if registration successful, false otherwise
     */
    template<typename T>
    bool registerComponent(const std::string& name, std::shared_ptr<T> component);

    /**
     * @brief Get a registered component
     * @tparam T Component type
     * @param name Component name
     * @return Shared pointer to component, or nullptr if not found
     */
    template<typename T>
    std::shared_ptr<T> getComponent(const std::string& name);

    /**
     * @brief Check if a component is registered
     * @param name Component name
     * @return true if component exists, false otherwise
     */
    bool hasComponent(const std::string& name) const;

    /**
     * @brief Get the last error message
     * @return Last error message
     */
    std::string getLastError() const;

    /**
     * @brief Clear the last error message
     */
    bool clearLastError();

    // System state
    bool start();
    bool stop();
    bool pause();
    bool resume();
    bool isRunning() const;
    bool isPaused() const;

    // Configuration
    bool update(const nlohmann::json& config);
    std::optional<nlohmann::json> getConfig() const;

    // Manager access
    CoreManager& getCoreManager();
    NetworkManager& getNetworkManager();
    BlockchainManager& getBlockchainManager();
    TransactionManager& getTransactionManager();
    AssetManager& getAssetManager();
    WalletManager& getWalletManager();
    IPFSManager& getIPFSManager();
    DatabaseManager& getDatabaseManager();
    SecurityManager& getSecurityManager();
    NFTManager& getNFTManager();
    PluginManager& getPluginManager();
    EventManager& getEventManager();
    CacheManager& getCacheManager();
    ConfigManager& getConfigManager();
    LoggingManager& getLoggingManager();

    // System information
    struct SystemInfo {
        bool initialized;
        bool running;
        bool paused;
        std::string version;
        std::string build;
        std::unordered_map<std::string, bool> managerStatus;
    };
    std::optional<SystemInfo> getSystemInfo() const;
    std::optional<ComponentStatus> getComponentStatus(const std::string& component) const;
    std::vector<std::string> getAvailableComponents() const;

    // System statistics
    struct SystemStats {
        uint64_t uptime;
        uint64_t totalEvents;
        uint64_t totalTransactions;
        uint64_t totalAssets;
        uint64_t totalNFTs;
        std::unordered_map<std::string, nlohmann::json> managerStats;
        size_t total_components = 0;
        size_t active_components = 0;
        size_t failed_components = 0;
        std::chrono::system_clock::time_point last_activity;
        std::chrono::system_clock::time_point start_time;
    };
    SystemStats getSystemStats() const;
    bool enableStats(bool enable);
    bool resetStats();

    // Callbacks
    using StateCallback = std::function<void(const SystemInfo&)>;
    using ErrorCallback = std::function<void(const std::string&)>;
    using StatsCallback = std::function<void(const SystemStats&)>;

    bool registerStateCallback(StateCallback callback);
    bool registerErrorCallback(ErrorCallback callback);
    bool registerStatsCallback(StatsCallback callback);
    bool unregisterStateCallback();
    bool unregisterErrorCallback();
    bool unregisterStatsCallback();

    // Configuration management
    bool updateConfig(const nlohmann::json& config);
    bool reloadConfig();

    // Statistics and monitoring
    struct Stats {
        size_t total_components;
        size_t active_components;
        size_t failed_components;
        std::chrono::system_clock::time_point start_time;
        std::chrono::system_clock::time_point last_activity;
        std::map<std::string, nlohmann::json> component_stats;
    };
    Stats getStats() const;
    bool isHealthy() const;

private:
    SatoxManager() = default;
    ~SatoxManager() = default;
    SatoxManager(const SatoxManager&) = delete;
    SatoxManager& operator=(const SatoxManager&) = delete;

    void setError(const std::string& error);
    bool initializeComponents();
    bool initializeCoreComponent();
    bool initializeBlockchainComponent();
    bool initializeNetworkComponent();
    bool initializeConfigComponent();
    bool initializeTransactionComponent();
    bool initializeIPFSComponent();
    bool initializeWalletComponent();

    bool validateConfig(const nlohmann::json& config) const;
    void updateState();
    void handleError(const std::string& error);
    void updateStats();

    std::unordered_map<std::string, std::shared_ptr<void>> components_;
    mutable std::recursive_mutex mutex_;
    std::string lastError_;
    bool initialized_ = false;
    bool running_ = false;
    bool paused_ = false;
    nlohmann::json config_;
    SystemStats stats_;
    bool statsEnabled_ = false;

    StateCallback stateCallback_;
    ErrorCallback errorCallback_;
    StatsCallback statsCallback_;
};

// Template implementations
template<typename T>
bool SatoxManager::registerComponent(const std::string& name, std::shared_ptr<T> component) {
    if (!component) {
        setError("Invalid component pointer");
        return false;
    }

    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (components_.find(name) != components_.end()) {
        setError("Component already registered: " + name);
        return false;
    }

    components_[name] = std::static_pointer_cast<void>(component);
    return true;
}

template<typename T>
std::shared_ptr<T> SatoxManager::getComponent(const std::string& name) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto it = components_.find(name);
    if (it == components_.end()) {
        setError("Component not found: " + name);
        return nullptr;
    }

    return std::static_pointer_cast<T>(it->second);
}

} // namespace satox::core 