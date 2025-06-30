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
#include <memory>
#include <mutex>
#include <chrono>
#include <functional>
#include <unordered_map>
#include <vector>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace satox::core {

class PluginManager {
public:
    // Plugin states
    enum class PluginState {
        UNLOADED,
        LOADED,
        INITIALIZED,
        RUNNING,
        PAUSED,
        ERROR
    };

    // Plugin types
    enum class PluginType {
        CORE,
        NETWORK,
        BLOCKCHAIN,
        WALLET,
        TRANSACTION,
        ASSET,
        NFT,
        IPFS,
        DATABASE,
        CACHE,
        CONFIG,
        SECURITY,
        CUSTOM
    };

    // Plugin information structure
    struct PluginInfo {
        std::string name;
        std::string version;
        std::string description;
        std::string author;
        std::string license;
        PluginType type;
        std::vector<std::string> dependencies;
        std::vector<std::string> conflicts;
        nlohmann::json metadata;
    };

    // Plugin configuration structure
    struct PluginConfig {
        std::string pluginDir;
        bool autoLoad;
        bool autoInitialize;
        bool autoStart;
        std::chrono::milliseconds loadTimeout;
        std::chrono::milliseconds initTimeout;
        std::chrono::milliseconds startTimeout;
        nlohmann::json defaultSettings;
    };

    // Plugin interface
    class IPlugin {
    public:
        virtual ~IPlugin() = default;
        virtual bool initialize(const nlohmann::json& config) = 0;
        virtual bool start() = 0;
        virtual bool stop() = 0;
        virtual bool pause() = 0;
        virtual bool resume() = 0;
        virtual bool update(const nlohmann::json& config) = 0;
        virtual PluginInfo getInfo() const = 0;
        virtual PluginState getState() const = 0;
        virtual std::string getLastError() const = 0;
    };

    // Plugin factory interface
    class IPluginFactory {
    public:
        virtual ~IPluginFactory() = default;
        virtual std::shared_ptr<IPlugin> createPlugin() = 0;
    };

    // Plugin callback types
    using PluginStateCallback = std::function<void(const std::string&, PluginState)>;
    using PluginErrorCallback = std::function<void(const std::string&, const std::string&)>;
    using PluginConfigCallback = std::function<void(const std::string&, const nlohmann::json&)>;

    // Singleton instance
    static PluginManager& getInstance();

    // Prevent copying
    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;

    // Initialization and shutdown
    bool initialize(const PluginConfig& config);
    void shutdown();

    // Plugin loading and unloading
    bool loadPlugin(const std::string& pluginPath);
    bool unloadPlugin(const std::string& pluginName);
    bool reloadPlugin(const std::string& pluginName);
    bool isPluginLoaded(const std::string& pluginName) const;

    // Plugin lifecycle management
    bool initializePlugin(const std::string& pluginName);
    bool startPlugin(const std::string& pluginName);
    bool stopPlugin(const std::string& pluginName);
    bool pausePlugin(const std::string& pluginName);
    bool resumePlugin(const std::string& pluginName);
    bool updatePlugin(const std::string& pluginName, const nlohmann::json& config);

    // Plugin information
    std::vector<PluginInfo> getPluginInfo() const;
    PluginInfo getPluginInfo(const std::string& pluginName) const;
    PluginState getPluginState(const std::string& pluginName) const;
    std::string getPluginError(const std::string& pluginName) const;

    // Plugin discovery
    std::vector<std::string> discoverPlugins() const;
    bool scanPluginDirectory();
    bool validatePlugin(const std::string& pluginPath) const;

    // Plugin dependencies
    bool checkDependencies(const std::string& pluginName) const;
    bool resolveDependencies(const std::string& pluginName);
    std::vector<std::string> getDependencies(const std::string& pluginName) const;
    std::vector<std::string> getDependents(const std::string& pluginName) const;

    // Plugin configuration
    bool setPluginConfig(const std::string& pluginName, const nlohmann::json& config);
    nlohmann::json getPluginConfig(const std::string& pluginName) const;
    bool validatePluginConfig(const std::string& pluginName, const nlohmann::json& config) const;

    // Plugin callbacks
    void registerStateCallback(PluginStateCallback callback);
    void registerErrorCallback(PluginErrorCallback callback);
    void registerConfigCallback(PluginConfigCallback callback);
    void unregisterStateCallback();
    void unregisterErrorCallback();
    void unregisterConfigCallback();

    // Plugin statistics
    struct PluginStats {
        size_t totalPlugins;
        size_t loadedPlugins;
        size_t runningPlugins;
        size_t failedPlugins;
        std::chrono::milliseconds averageLoadTime;
        std::chrono::milliseconds averageInitTime;
        std::chrono::milliseconds averageStartTime;
    };
    PluginStats getStats() const;
    void resetStats();
    void enableStats(bool enable);

    // Error handling
    std::string getLastError() const;
    void clearLastError();

private:
    PluginManager() = default;
    ~PluginManager() = default;

    // Helper methods
    bool loadPluginLibrary(const std::string& pluginPath);
    bool unloadPluginLibrary(const std::string& pluginName);
    bool validatePluginInfo(const PluginInfo& info) const;
    bool checkPluginCompatibility(const PluginInfo& info) const;
    void updatePluginState(const std::string& pluginName, PluginState state);
    void handlePluginError(const std::string& pluginName, const std::string& error);
    void updateStats(const std::string& pluginName, const std::chrono::milliseconds& time,
                    const std::string& operation);
    void cleanupPlugins();
    void notifyStateChange(const std::string& pluginName, PluginState state);
    void notifyError(const std::string& pluginName, const std::string& error);
    void notifyConfigChange(const std::string& pluginName, const nlohmann::json& config);

    // Member variables
    bool initialized_ = false;
    mutable std::mutex mutex_;
    PluginConfig config_;
    std::unordered_map<std::string, std::shared_ptr<IPlugin>> plugins_;
    std::unordered_map<std::string, std::shared_ptr<IPluginFactory>> factories_;
    std::vector<PluginStateCallback> stateCallbacks_;
    std::vector<PluginErrorCallback> errorCallbacks_;
    std::vector<PluginConfigCallback> configCallbacks_;
    PluginStats stats_;
    bool statsEnabled_ = false;
    mutable std::string lastError_;
};

} // namespace satox::core 