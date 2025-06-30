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

#include "satox/core/plugin_manager.hpp"
#include <dlfcn.h>
#include <algorithm>
#include <chrono>
#include <thread>

namespace satox::core {

PluginManager& PluginManager::getInstance() {
    static PluginManager instance;
    return instance;
}

bool PluginManager::initialize(const PluginConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        lastError_ = "Plugin manager already initialized";
        return false;
    }

    config_ = config;
    if (!std::filesystem::exists(config_.pluginDir)) {
        lastError_ = "Plugin directory does not exist: " + config_.pluginDir;
        return false;
    }

    if (config_.autoLoad) {
        if (!scanPluginDirectory()) {
            return false;
        }
    }

    initialized_ = true;
    return true;
}

void PluginManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return;
    }

    // Stop and unload all plugins
    for (const auto& [name, plugin] : plugins_) {
        if (plugin->getState() == PluginState::RUNNING) {
            plugin->stop();
        }
        unloadPluginLibrary(name);
    }

    plugins_.clear();
    factories_.clear();
    stateCallbacks_.clear();
    errorCallbacks_.clear();
    configCallbacks_.clear();
    initialized_ = false;
}

bool PluginManager::loadPlugin(const std::string& pluginPath) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Plugin manager not initialized";
        return false;
    }

    if (!std::filesystem::exists(pluginPath)) {
        lastError_ = "Plugin file does not exist: " + pluginPath;
        return false;
    }

    if (!validatePlugin(pluginPath)) {
        return false;
    }

    auto startTime = std::chrono::steady_clock::now();
    if (!loadPluginLibrary(pluginPath)) {
        return false;
    }

    if (statsEnabled_) {
        auto endTime = std::chrono::steady_clock::now();
        auto loadTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime);
        updateStats(pluginPath, loadTime, "load");
    }

    return true;
}

bool PluginManager::unloadPlugin(const std::string& pluginName) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Plugin manager not initialized";
        return false;
    }

    auto it = plugins_.find(pluginName);
    if (it == plugins_.end()) {
        lastError_ = "Plugin not found: " + pluginName;
        return false;
    }

    if (it->second->getState() == PluginState::RUNNING) {
        if (!it->second->stop()) {
            lastError_ = "Failed to stop plugin: " + pluginName;
            return false;
        }
    }

    if (!unloadPluginLibrary(pluginName)) {
        return false;
    }

    plugins_.erase(it);
    return true;
}

bool PluginManager::reloadPlugin(const std::string& pluginName) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Plugin manager not initialized";
        return false;
    }

    auto it = plugins_.find(pluginName);
    if (it == plugins_.end()) {
        lastError_ = "Plugin not found: " + pluginName;
        return false;
    }

    std::string pluginPath = it->second->getInfo().metadata["path"];
    if (!unloadPlugin(pluginName)) {
        return false;
    }

    return loadPlugin(pluginPath);
}

bool PluginManager::isPluginLoaded(const std::string& pluginName) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return plugins_.find(pluginName) != plugins_.end();
}

bool PluginManager::initializePlugin(const std::string& pluginName) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Plugin manager not initialized";
        return false;
    }

    auto it = plugins_.find(pluginName);
    if (it == plugins_.end()) {
        lastError_ = "Plugin not found: " + pluginName;
        return false;
    }

    if (!checkDependencies(pluginName)) {
        return false;
    }

    auto startTime = std::chrono::steady_clock::now();
    if (!it->second->initialize(getPluginConfig(pluginName))) {
        lastError_ = "Failed to initialize plugin: " + pluginName;
        return false;
    }

    if (statsEnabled_) {
        auto endTime = std::chrono::steady_clock::now();
        auto initTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime);
        updateStats(pluginName, initTime, "init");
    }

    updatePluginState(pluginName, PluginState::INITIALIZED);
    return true;
}

bool PluginManager::startPlugin(const std::string& pluginName) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Plugin manager not initialized";
        return false;
    }

    auto it = plugins_.find(pluginName);
    if (it == plugins_.end()) {
        lastError_ = "Plugin not found: " + pluginName;
        return false;
    }

    if (it->second->getState() != PluginState::INITIALIZED) {
        lastError_ = "Plugin not initialized: " + pluginName;
        return false;
    }

    auto startTime = std::chrono::steady_clock::now();
    if (!it->second->start()) {
        lastError_ = "Failed to start plugin: " + pluginName;
        return false;
    }

    if (statsEnabled_) {
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime);
        updateStats(pluginName, duration, "start");
    }

    updatePluginState(pluginName, PluginState::RUNNING);
    return true;
}

bool PluginManager::stopPlugin(const std::string& pluginName) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Plugin manager not initialized";
        return false;
    }

    auto it = plugins_.find(pluginName);
    if (it == plugins_.end()) {
        lastError_ = "Plugin not found: " + pluginName;
        return false;
    }

    if (it->second->getState() != PluginState::RUNNING) {
        lastError_ = "Plugin not running: " + pluginName;
        return false;
    }

    if (!it->second->stop()) {
        lastError_ = "Failed to stop plugin: " + pluginName;
        return false;
    }

    updatePluginState(pluginName, PluginState::INITIALIZED);
    return true;
}

bool PluginManager::pausePlugin(const std::string& pluginName) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Plugin manager not initialized";
        return false;
    }

    auto it = plugins_.find(pluginName);
    if (it == plugins_.end()) {
        lastError_ = "Plugin not found: " + pluginName;
        return false;
    }

    if (it->second->getState() != PluginState::RUNNING) {
        lastError_ = "Plugin not running: " + pluginName;
        return false;
    }

    if (!it->second->pause()) {
        lastError_ = "Failed to pause plugin: " + pluginName;
        return false;
    }

    updatePluginState(pluginName, PluginState::PAUSED);
    return true;
}

bool PluginManager::resumePlugin(const std::string& pluginName) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Plugin manager not initialized";
        return false;
    }

    auto it = plugins_.find(pluginName);
    if (it == plugins_.end()) {
        lastError_ = "Plugin not found: " + pluginName;
        return false;
    }

    if (it->second->getState() != PluginState::PAUSED) {
        lastError_ = "Plugin not paused: " + pluginName;
        return false;
    }

    if (!it->second->resume()) {
        lastError_ = "Failed to resume plugin: " + pluginName;
        return false;
    }

    updatePluginState(pluginName, PluginState::RUNNING);
    return true;
}

bool PluginManager::updatePlugin(const std::string& pluginName,
                               const nlohmann::json& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Plugin manager not initialized";
        return false;
    }

    auto it = plugins_.find(pluginName);
    if (it == plugins_.end()) {
        lastError_ = "Plugin not found: " + pluginName;
        return false;
    }

    if (!validatePluginConfig(pluginName, config)) {
        return false;
    }

    if (!it->second->update(config)) {
        lastError_ = "Failed to update plugin: " + pluginName;
        return false;
    }

    notifyConfigChange(pluginName, config);
    return true;
}

std::vector<PluginManager::PluginInfo> PluginManager::getPluginInfo() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<PluginInfo> result;
    result.reserve(plugins_.size());
    for (const auto& [name, plugin] : plugins_) {
        result.push_back(plugin->getInfo());
    }
    return result;
}

PluginManager::PluginInfo PluginManager::getPluginInfo(
    const std::string& pluginName) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = plugins_.find(pluginName);
    if (it == plugins_.end()) {
        return PluginInfo{};
    }
    return it->second->getInfo();
}

PluginManager::PluginState PluginManager::getPluginState(
    const std::string& pluginName) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = plugins_.find(pluginName);
    if (it == plugins_.end()) {
        return PluginState::UNLOADED;
    }
    return it->second->getState();
}

std::string PluginManager::getPluginError(const std::string& pluginName) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = plugins_.find(pluginName);
    if (it == plugins_.end()) {
        return "Plugin not found";
    }
    return it->second->getLastError();
}

std::vector<std::string> PluginManager::discoverPlugins() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> result;
    for (const auto& entry : std::filesystem::directory_iterator(config_.pluginDir)) {
        if (entry.path().extension() == ".so" || entry.path().extension() == ".dll") {
            result.push_back(entry.path().string());
        }
    }
    return result;
}

bool PluginManager::scanPluginDirectory() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Plugin manager not initialized";
        return false;
    }

    auto plugins = discoverPlugins();
    for (const auto& pluginPath : plugins) {
        if (!loadPlugin(pluginPath)) {
            return false;
        }
    }

    return true;
}

bool PluginManager::validatePlugin(const std::string& pluginPath) const {
    void* handle = dlopen(pluginPath.c_str(), RTLD_NOW);
    if (!handle) {
        lastError_ = "Failed to load plugin: " + std::string(dlerror());
        return false;
    }

    auto createPlugin = reinterpret_cast<IPluginFactory* (*)()>(
        dlsym(handle, "createPluginFactory"));
    if (!createPlugin) {
        lastError_ = "Invalid plugin: missing createPluginFactory symbol";
        dlclose(handle);
        return false;
    }

    auto factory = std::unique_ptr<IPluginFactory>(createPlugin());
    if (!factory) {
        lastError_ = "Failed to create plugin factory";
        dlclose(handle);
        return false;
    }

    auto plugin = factory->createPlugin();
    if (!plugin) {
        lastError_ = "Failed to create plugin instance";
        dlclose(handle);
        return false;
    }

    auto info = plugin->getInfo();
    if (!validatePluginInfo(info)) {
        dlclose(handle);
        return false;
    }

    dlclose(handle);
    return true;
}

bool PluginManager::checkDependencies(const std::string& pluginName) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = plugins_.find(pluginName);
    if (it == plugins_.end()) {
        lastError_ = "Plugin not found: " + pluginName;
        return false;
    }

    const auto& dependencies = it->second->getInfo().dependencies;
    for (const auto& dep : dependencies) {
        auto depIt = plugins_.find(dep);
        if (depIt == plugins_.end()) {
            lastError_ = "Missing dependency: " + dep;
            return false;
        }

        if (depIt->second->getState() != PluginState::RUNNING) {
            lastError_ = "Dependency not running: " + dep;
            return false;
        }
    }

    return true;
}

bool PluginManager::resolveDependencies(const std::string& pluginName) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = plugins_.find(pluginName);
    if (it == plugins_.end()) {
        lastError_ = "Plugin not found: " + pluginName;
        return false;
    }

    const auto& dependencies = it->second->getInfo().dependencies;
    for (const auto& dep : dependencies) {
        if (!isPluginLoaded(dep)) {
            lastError_ = "Cannot resolve dependency: " + dep;
            return false;
        }

        if (!initializePlugin(dep)) {
            return false;
        }

        if (!startPlugin(dep)) {
            return false;
        }
    }

    return true;
}

std::vector<std::string> PluginManager::getDependencies(
    const std::string& pluginName) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = plugins_.find(pluginName);
    if (it == plugins_.end()) {
        return {};
    }
    return it->second->getInfo().dependencies;
}

std::vector<std::string> PluginManager::getDependents(
    const std::string& pluginName) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::string> result;
    for (const auto& [name, plugin] : plugins_) {
        const auto& dependencies = plugin->getInfo().dependencies;
        if (std::find(dependencies.begin(), dependencies.end(), pluginName) !=
            dependencies.end()) {
            result.push_back(name);
        }
    }
    return result;
}

bool PluginManager::setPluginConfig(const std::string& pluginName,
                                  const nlohmann::json& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "Plugin manager not initialized";
        return false;
    }

    auto it = plugins_.find(pluginName);
    if (it == plugins_.end()) {
        lastError_ = "Plugin not found: " + pluginName;
        return false;
    }

    if (!validatePluginConfig(pluginName, config)) {
        return false;
    }

    return updatePlugin(pluginName, config);
}

nlohmann::json PluginManager::getPluginConfig(
    const std::string& pluginName) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = plugins_.find(pluginName);
    if (it == plugins_.end()) {
        return nlohmann::json::object();
    }

    return it->second->getInfo().metadata["config"];
}

bool PluginManager::validatePluginConfig(const std::string& pluginName,
                                       const nlohmann::json& config) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = plugins_.find(pluginName);
    if (it == plugins_.end()) {
        lastError_ = "Plugin not found: " + pluginName;
        return false;
    }

    // This is a simplified implementation. In a real system, you would
    // want to validate the configuration against a schema or other rules.
    return true;
}

void PluginManager::registerStateCallback(PluginStateCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    stateCallbacks_.push_back(callback);
}

void PluginManager::registerErrorCallback(PluginErrorCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    errorCallbacks_.push_back(callback);
}

void PluginManager::registerConfigCallback(PluginConfigCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    configCallbacks_.push_back(callback);
}

void PluginManager::unregisterStateCallback() {
    std::lock_guard<std::mutex> lock(mutex_);
    stateCallbacks_.clear();
}

void PluginManager::unregisterErrorCallback() {
    std::lock_guard<std::mutex> lock(mutex_);
    errorCallbacks_.clear();
}

void PluginManager::unregisterConfigCallback() {
    std::lock_guard<std::mutex> lock(mutex_);
    configCallbacks_.clear();
}

PluginManager::PluginStats PluginManager::getStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

void PluginManager::resetStats() {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_ = PluginStats{};
}

void PluginManager::enableStats(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    statsEnabled_ = enable;
}

std::string PluginManager::getLastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastError_;
}

void PluginManager::clearLastError() {
    std::lock_guard<std::mutex> lock(mutex_);
    lastError_.clear();
}

bool PluginManager::loadPluginLibrary(const std::string& pluginPath) {
    void* handle = dlopen(pluginPath.c_str(), RTLD_NOW);
    if (!handle) {
        lastError_ = "Failed to load plugin: " + std::string(dlerror());
        return false;
    }

    auto createPlugin = reinterpret_cast<IPluginFactory* (*)()>(
        dlsym(handle, "createPluginFactory"));
    if (!createPlugin) {
        lastError_ = "Invalid plugin: missing createPluginFactory symbol";
        dlclose(handle);
        return false;
    }

    auto factory = std::shared_ptr<IPluginFactory>(createPlugin());
    if (!factory) {
        lastError_ = "Failed to create plugin factory";
        dlclose(handle);
        return false;
    }

    auto plugin = factory->createPlugin();
    if (!plugin) {
        lastError_ = "Failed to create plugin instance";
        dlclose(handle);
        return false;
    }

    auto info = plugin->getInfo();
    if (!validatePluginInfo(info)) {
        dlclose(handle);
        return false;
    }

    plugins_[info.name] = plugin;
    factories_[info.name] = factory;
    return true;
}

bool PluginManager::unloadPluginLibrary(const std::string& pluginName) {
    auto it = plugins_.find(pluginName);
    if (it == plugins_.end()) {
        lastError_ = "Plugin not found: " + pluginName;
        return false;
    }

    auto factoryIt = factories_.find(pluginName);
    if (factoryIt == factories_.end()) {
        lastError_ = "Plugin factory not found: " + pluginName;
        return false;
    }

    plugins_.erase(it);
    factories_.erase(factoryIt);
    return true;
}

bool PluginManager::validatePluginInfo(const PluginInfo& info) const {
    if (info.name.empty()) {
        lastError_ = "Plugin name cannot be empty";
        return false;
    }

    if (info.version.empty()) {
        lastError_ = "Plugin version cannot be empty";
        return false;
    }

    if (info.description.empty()) {
        lastError_ = "Plugin description cannot be empty";
        return false;
    }

    if (info.author.empty()) {
        lastError_ = "Plugin author cannot be empty";
        return false;
    }

    if (info.license.empty()) {
        lastError_ = "Plugin license cannot be empty";
        return false;
    }

    return true;
}

bool PluginManager::checkPluginCompatibility(const PluginInfo& info) const {
    // This is a simplified implementation. In a real system, you would
    // want to check version compatibility, platform compatibility, etc.
    return true;
}

void PluginManager::updatePluginState(const std::string& pluginName,
                                    PluginState state) {
    auto it = plugins_.find(pluginName);
    if (it != plugins_.end()) {
        notifyStateChange(pluginName, state);
    }
}

void PluginManager::handlePluginError(const std::string& pluginName,
                                    const std::string& error) {
    auto it = plugins_.find(pluginName);
    if (it != plugins_.end()) {
        notifyError(pluginName, error);
        updatePluginState(pluginName, PluginState::ERROR);
    }
}

void PluginManager::updateStats(const std::string& pluginName,
                              const std::chrono::milliseconds& time,
                              const std::string& operation) {
    stats_.totalPlugins = plugins_.size();
    stats_.loadedPlugins = std::count_if(
        plugins_.begin(), plugins_.end(),
        [](const auto& pair) {
            return pair.second->getState() != PluginState::UNLOADED;
        });
    stats_.runningPlugins = std::count_if(
        plugins_.begin(), plugins_.end(),
        [](const auto& pair) {
            return pair.second->getState() == PluginState::RUNNING;
        });
    stats_.failedPlugins = std::count_if(
        plugins_.begin(), plugins_.end(),
        [](const auto& pair) {
            return pair.second->getState() == PluginState::ERROR;
        });

    if (operation == "load") {
        stats_.averageLoadTime = std::chrono::milliseconds(
            (stats_.averageLoadTime.count() * (stats_.loadedPlugins - 1) +
             time.count()) /
            stats_.loadedPlugins);
    } else if (operation == "init") {
        stats_.averageInitTime = std::chrono::milliseconds(
            (stats_.averageInitTime.count() * (stats_.loadedPlugins - 1) +
             time.count()) /
            stats_.loadedPlugins);
    } else if (operation == "start") {
        stats_.averageStartTime = std::chrono::milliseconds(
            (stats_.averageStartTime.count() * (stats_.runningPlugins - 1) +
             time.count()) /
            stats_.runningPlugins);
    }
}

void PluginManager::cleanupPlugins() {
    for (const auto& [name, plugin] : plugins_) {
        if (plugin->getState() == PluginState::RUNNING) {
            plugin->stop();
        }
        unloadPluginLibrary(name);
    }
    plugins_.clear();
    factories_.clear();
}

void PluginManager::notifyStateChange(const std::string& pluginName,
                                    PluginState state) {
    for (const auto& callback : stateCallbacks_) {
        callback(pluginName, state);
    }
}

void PluginManager::notifyError(const std::string& pluginName,
                              const std::string& error) {
    for (const auto& callback : errorCallbacks_) {
        callback(pluginName, error);
    }
}

void PluginManager::notifyConfigChange(const std::string& pluginName,
                                     const nlohmann::json& config) {
    for (const auto& callback : configCallbacks_) {
        callback(pluginName, config);
    }
}

} // namespace satox::core 