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

#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <memory>
#include <chrono>
#include <optional>
#include <nlohmann/json.hpp>

namespace satox {
namespace core {

// Forward declarations
class NetworkManager;
class AssetManager;
class IPFSManager;
class WalletManager;
class BlockchainManager;
class DatabaseManager;

// Enums
enum class ComponentState {
    UNINITIALIZED,
    INITIALIZING,
    RUNNING,
    RESTARTING,
    ERROR,
    SHUTDOWN
};

// Configuration structures
struct CoreConfig {
    std::string data_dir;
    std::string network;  // "mainnet", "testnet", "regtest"
    bool enable_mining = false;
    bool enable_sync = true;
    uint32_t sync_interval_ms = 1000;
    uint32_t mining_threads = 1;
    std::string rpc_endpoint;
    std::string rpc_username;
    std::string rpc_password;
    uint32_t timeout_ms = 30000;
    
    // Component-specific configurations
    nlohmann::json database;
    nlohmann::json network_config;
    nlohmann::json blockchain;
    nlohmann::json wallet;
    nlohmann::json asset;
    nlohmann::json ipfs;
};

// Statistics structures
struct CoreStats {
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point last_activity;
    uint32_t total_components = 0;
    uint32_t active_components = 0;
    uint32_t failed_components = 0;
    nlohmann::json additional_stats;
};

// Component status structure
struct ComponentStatus {
    std::string name;
    ComponentState state = ComponentState::UNINITIALIZED;
    std::chrono::system_clock::time_point last_update;
    uint32_t error_count = 0;
    std::string last_error;
    nlohmann::json additional_info;
};

// System information structure
struct SystemInfo {
    std::string version;
    std::string network;
    uint64_t uptime;
    std::chrono::system_clock::time_point start_time;
    std::map<std::string, std::string> metadata;
};

// Callback types
using ErrorCallback = std::function<void(const std::string&)>;
using StatusCallback = std::function<void(const std::string&, ComponentState)>;
using ShutdownCallback = std::function<void()>;

// Main manager class
class CoreManager {
public:
    // Singleton pattern
    static CoreManager& getInstance();

    // Constructor/Destructor
    CoreManager();
    ~CoreManager();

    // Disable copy and assignment
    CoreManager(const CoreManager&) = delete;
    CoreManager& operator=(const CoreManager&) = delete;

    // Initialization and shutdown
    bool initialize(const CoreConfig& config);
    void shutdown();
    bool isRunning() const;
    bool isInitialized() const;
    std::string getLastError() const;

    // Component access (singleton pattern)
    NetworkManager& getNetworkManager() const;
    AssetManager& getAssetManager() const;
    IPFSManager& getIPFSManager() const;
    WalletManager& getWalletManager() const;
    BlockchainManager& getBlockchainManager() const;

    // System management
    bool start();
    bool stop();

    // Component management
    bool restartComponent(const std::string& component);
    bool updateComponent(const std::string& component, const std::unordered_map<std::string, std::string>& config);
    std::optional<ComponentStatus> getComponentStatus(const std::string& component) const;

    // Configuration management
    bool updateConfig(const CoreConfig& config);
    std::optional<CoreConfig> getConfig() const;
    bool validateConfig(const CoreConfig& config) const;
    bool backupConfig(const std::string& backup_path) const;
    bool restoreConfig(const std::string& backup_path);

    // Statistics and monitoring
    std::optional<SystemInfo> getSystemInfo() const;
    CoreStats getStats() const;
    bool isHealthy() const;

    // Callback registration
    void registerErrorCallback(ErrorCallback callback);
    void registerStatusCallback(StatusCallback callback);
    void registerShutdownCallback(ShutdownCallback callback);

#ifdef TESTING
    // Test-only method to reset singleton state
    static void resetForTesting();

    // Test-only method to forcibly set running state
    void setRunningForTesting(bool running);
#endif

private:
    // Helper methods
    bool initializeComponents();
    void shutdownComponents();
    void updateAllComponents();
    void initializeComponentStatus();
    void notifyStatusChange(const std::string& component, ComponentState state);
    void notifyShutdown();

    // Member variables
    CoreConfig config_;
    CoreStats stats_;
    std::map<std::string, ComponentStatus> component_status_;
    std::vector<ErrorCallback> errorCallbacks_;
    std::vector<StatusCallback> statusCallbacks_;
    std::vector<ShutdownCallback> shutdownCallbacks_;
    mutable std::mutex mutex_;
    std::string lastError_;
    bool initialized_;
    bool is_running_;
};

} // namespace core
} // namespace satox 