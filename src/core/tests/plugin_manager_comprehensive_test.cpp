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

#include <gtest/gtest.h>
#include "satox/core/plugin_manager.hpp"
#include <filesystem>
#include <thread>
#include <chrono>

using namespace satox::core;

class PluginManagerComprehensiveTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test plugin directory
        testPluginDir = std::filesystem::temp_directory_path() / "satox_test_plugins";
        std::filesystem::create_directories(testPluginDir);

        // Create test plugin config
        config.pluginDir = testPluginDir.string();
        config.autoLoad = false;
        config.maxPlugins = 10;
        config.pluginTimeout = std::chrono::seconds(5);
    }

    void TearDown() override {
        // Clean up test plugin directory
        std::filesystem::remove_all(testPluginDir);
    }

    std::filesystem::path testPluginDir;
    PluginManager::PluginConfig config;
};

// Helper function to create a test plugin
std::string createTestPlugin(const std::filesystem::path& pluginDir,
                           const std::string& name) {
    // This is a simplified implementation. In a real system, you would
    // want to create actual plugin files with proper implementations.
    return (pluginDir / (name + ".so")).string();
}

// Helper function to create test plugin info
PluginManager::PluginInfo createTestPluginInfo(const std::string& name) {
    PluginManager::PluginInfo info;
    info.name = name;
    info.version = "1.0.0";
    info.description = "Test plugin " + name;
    info.author = "Test Author";
    info.license = "MIT";
    info.type = PluginManager::PluginType::CUSTOM;
    info.dependencies = {};
    info.conflicts = {};
    info.metadata = {
        {"path", (testPluginDir / (name + ".so")).string()},
        {"config", nlohmann::json::object()}
    };
    return info;
}

// Helper function to create test plugin config
nlohmann::json createTestPluginConfig() {
    return {
        {"setting1", "value1"},
        {"setting2", 42},
        {"setting3", true}
    };
}

// Initialization Tests
TEST_F(PluginManagerComprehensiveTest, InitializeWithValidConfig) {
    auto& manager = PluginManager::getInstance();
    EXPECT_TRUE(manager.initialize(config));
    EXPECT_TRUE(manager.isInitialized());
}

TEST_F(PluginManagerComprehensiveTest, InitializeWithInvalidConfig) {
    auto& manager = PluginManager::getInstance();
    config.pluginDir = "/nonexistent/directory";
    EXPECT_FALSE(manager.initialize(config));
    EXPECT_FALSE(manager.isInitialized());
}

TEST_F(PluginManagerComprehensiveTest, InitializeWithZeroMaxPlugins) {
    auto& manager = PluginManager::getInstance();
    config.maxPlugins = 0;
    EXPECT_FALSE(manager.initialize(config));
    EXPECT_FALSE(manager.isInitialized());
}

TEST_F(PluginManagerComprehensiveTest, InitializeWithZeroTimeout) {
    auto& manager = PluginManager::getInstance();
    config.pluginTimeout = std::chrono::seconds(0);
    EXPECT_FALSE(manager.initialize(config));
    EXPECT_FALSE(manager.isInitialized());
}

// Plugin Loading Tests
TEST_F(PluginManagerComprehensiveTest, LoadValidPlugin) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::string pluginPath = createTestPlugin(testPluginDir, "test_plugin");
    EXPECT_TRUE(manager.loadPlugin(pluginPath));
    EXPECT_TRUE(manager.isPluginLoaded("test_plugin"));
}

TEST_F(PluginManagerComprehensiveTest, LoadNonexistentPlugin) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_FALSE(manager.loadPlugin("/nonexistent/plugin.so"));
    EXPECT_FALSE(manager.isPluginLoaded("nonexistent"));
}

TEST_F(PluginManagerComprehensiveTest, LoadInvalidPlugin) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    // Create an invalid plugin file
    std::filesystem::path invalidPlugin = testPluginDir / "invalid.so";
    std::ofstream file(invalidPlugin);
    file << "invalid content";
    file.close();

    EXPECT_FALSE(manager.loadPlugin(invalidPlugin.string()));
    EXPECT_FALSE(manager.isPluginLoaded("invalid"));
}

TEST_F(PluginManagerComprehensiveTest, LoadDuplicatePlugin) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::string pluginPath = createTestPlugin(testPluginDir, "test_plugin");
    EXPECT_TRUE(manager.loadPlugin(pluginPath));
    EXPECT_FALSE(manager.loadPlugin(pluginPath));
}

// Plugin Unloading Tests
TEST_F(PluginManagerComprehensiveTest, UnloadLoadedPlugin) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::string pluginPath = createTestPlugin(testPluginDir, "test_plugin");
    ASSERT_TRUE(manager.loadPlugin(pluginPath));
    EXPECT_TRUE(manager.unloadPlugin("test_plugin"));
    EXPECT_FALSE(manager.isPluginLoaded("test_plugin"));
}

TEST_F(PluginManagerComprehensiveTest, UnloadNonexistentPlugin) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_FALSE(manager.unloadPlugin("nonexistent"));
}

TEST_F(PluginManagerComprehensiveTest, UnloadRunningPlugin) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::string pluginPath = createTestPlugin(testPluginDir, "test_plugin");
    ASSERT_TRUE(manager.loadPlugin(pluginPath));
    ASSERT_TRUE(manager.initializePlugin("test_plugin"));
    ASSERT_TRUE(manager.startPlugin("test_plugin"));

    EXPECT_TRUE(manager.unloadPlugin("test_plugin"));
    EXPECT_FALSE(manager.isPluginLoaded("test_plugin"));
}

// Plugin Reloading Tests
TEST_F(PluginManagerComprehensiveTest, ReloadLoadedPlugin) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::string pluginPath = createTestPlugin(testPluginDir, "test_plugin");
    ASSERT_TRUE(manager.loadPlugin(pluginPath));
    EXPECT_TRUE(manager.reloadPlugin("test_plugin"));
    EXPECT_TRUE(manager.isPluginLoaded("test_plugin"));
}

TEST_F(PluginManagerComprehensiveTest, ReloadNonexistentPlugin) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_FALSE(manager.reloadPlugin("nonexistent"));
}

// Plugin Initialization Tests
TEST_F(PluginManagerComprehensiveTest, InitializeLoadedPlugin) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::string pluginPath = createTestPlugin(testPluginDir, "test_plugin");
    ASSERT_TRUE(manager.loadPlugin(pluginPath));
    EXPECT_TRUE(manager.initializePlugin("test_plugin"));
    EXPECT_EQ(manager.getPluginState("test_plugin"),
              PluginManager::PluginState::INITIALIZED);
}

TEST_F(PluginManagerComprehensiveTest, InitializeNonexistentPlugin) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_FALSE(manager.initializePlugin("nonexistent"));
}

TEST_F(PluginManagerComprehensiveTest, InitializePluginWithMissingDependencies) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::string pluginPath = createTestPlugin(testPluginDir, "test_plugin");
    ASSERT_TRUE(manager.loadPlugin(pluginPath));

    // Set up a plugin with a dependency
    auto info = createTestPluginInfo("test_plugin");
    info.dependencies = {"nonexistent_dependency"};
    // In a real system, you would update the plugin's info here

    EXPECT_FALSE(manager.initializePlugin("test_plugin"));
}

// Plugin Start/Stop Tests
TEST_F(PluginManagerComprehensiveTest, StartInitializedPlugin) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::string pluginPath = createTestPlugin(testPluginDir, "test_plugin");
    ASSERT_TRUE(manager.loadPlugin(pluginPath));
    ASSERT_TRUE(manager.initializePlugin("test_plugin"));
    EXPECT_TRUE(manager.startPlugin("test_plugin"));
    EXPECT_EQ(manager.getPluginState("test_plugin"),
              PluginManager::PluginState::RUNNING);
}

TEST_F(PluginManagerComprehensiveTest, StartNonexistentPlugin) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_FALSE(manager.startPlugin("nonexistent"));
}

TEST_F(PluginManagerComprehensiveTest, StartUninitializedPlugin) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::string pluginPath = createTestPlugin(testPluginDir, "test_plugin");
    ASSERT_TRUE(manager.loadPlugin(pluginPath));
    EXPECT_FALSE(manager.startPlugin("test_plugin"));
}

TEST_F(PluginManagerComprehensiveTest, StopRunningPlugin) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::string pluginPath = createTestPlugin(testPluginDir, "test_plugin");
    ASSERT_TRUE(manager.loadPlugin(pluginPath));
    ASSERT_TRUE(manager.initializePlugin("test_plugin"));
    ASSERT_TRUE(manager.startPlugin("test_plugin"));
    EXPECT_TRUE(manager.stopPlugin("test_plugin"));
    EXPECT_EQ(manager.getPluginState("test_plugin"),
              PluginManager::PluginState::INITIALIZED);
}

TEST_F(PluginManagerComprehensiveTest, StopNonexistentPlugin) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_FALSE(manager.stopPlugin("nonexistent"));
}

TEST_F(PluginManagerComprehensiveTest, StopUninitializedPlugin) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::string pluginPath = createTestPlugin(testPluginDir, "test_plugin");
    ASSERT_TRUE(manager.loadPlugin(pluginPath));
    EXPECT_FALSE(manager.stopPlugin("test_plugin"));
}

// Plugin Pause/Resume Tests
TEST_F(PluginManagerComprehensiveTest, PauseRunningPlugin) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::string pluginPath = createTestPlugin(testPluginDir, "test_plugin");
    ASSERT_TRUE(manager.loadPlugin(pluginPath));
    ASSERT_TRUE(manager.initializePlugin("test_plugin"));
    ASSERT_TRUE(manager.startPlugin("test_plugin"));
    EXPECT_TRUE(manager.pausePlugin("test_plugin"));
    EXPECT_EQ(manager.getPluginState("test_plugin"),
              PluginManager::PluginState::PAUSED);
}

TEST_F(PluginManagerComprehensiveTest, PauseNonexistentPlugin) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_FALSE(manager.pausePlugin("nonexistent"));
}

TEST_F(PluginManagerComprehensiveTest, PauseUninitializedPlugin) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::string pluginPath = createTestPlugin(testPluginDir, "test_plugin");
    ASSERT_TRUE(manager.loadPlugin(pluginPath));
    EXPECT_FALSE(manager.pausePlugin("test_plugin"));
}

TEST_F(PluginManagerComprehensiveTest, ResumePausedPlugin) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::string pluginPath = createTestPlugin(testPluginDir, "test_plugin");
    ASSERT_TRUE(manager.loadPlugin(pluginPath));
    ASSERT_TRUE(manager.initializePlugin("test_plugin"));
    ASSERT_TRUE(manager.startPlugin("test_plugin"));
    ASSERT_TRUE(manager.pausePlugin("test_plugin"));
    EXPECT_TRUE(manager.resumePlugin("test_plugin"));
    EXPECT_EQ(manager.getPluginState("test_plugin"),
              PluginManager::PluginState::RUNNING);
}

TEST_F(PluginManagerComprehensiveTest, ResumeNonexistentPlugin) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_FALSE(manager.resumePlugin("nonexistent"));
}

TEST_F(PluginManagerComprehensiveTest, ResumeUninitializedPlugin) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::string pluginPath = createTestPlugin(testPluginDir, "test_plugin");
    ASSERT_TRUE(manager.loadPlugin(pluginPath));
    EXPECT_FALSE(manager.resumePlugin("test_plugin"));
}

// Plugin Configuration Tests
TEST_F(PluginManagerComprehensiveTest, SetPluginConfig) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::string pluginPath = createTestPlugin(testPluginDir, "test_plugin");
    ASSERT_TRUE(manager.loadPlugin(pluginPath));
    ASSERT_TRUE(manager.initializePlugin("test_plugin"));

    auto pluginConfig = createTestPluginConfig();
    EXPECT_TRUE(manager.setPluginConfig("test_plugin", pluginConfig));
    EXPECT_EQ(manager.getPluginConfig("test_plugin"), pluginConfig);
}

TEST_F(PluginManagerComprehensiveTest, SetConfigForNonexistentPlugin) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    auto pluginConfig = createTestPluginConfig();
    EXPECT_FALSE(manager.setPluginConfig("nonexistent", pluginConfig));
}

TEST_F(PluginManagerComprehensiveTest, SetInvalidPluginConfig) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::string pluginPath = createTestPlugin(testPluginDir, "test_plugin");
    ASSERT_TRUE(manager.loadPlugin(pluginPath));
    ASSERT_TRUE(manager.initializePlugin("test_plugin"));

    // Create an invalid config
    nlohmann::json invalidConfig = "invalid";
    EXPECT_FALSE(manager.setPluginConfig("test_plugin", invalidConfig));
}

// Plugin Discovery Tests
TEST_F(PluginManagerComprehensiveTest, DiscoverPlugins) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    // Create test plugins
    createTestPlugin(testPluginDir, "plugin1");
    createTestPlugin(testPluginDir, "plugin2");
    createTestPlugin(testPluginDir, "plugin3");

    auto plugins = manager.discoverPlugins();
    EXPECT_EQ(plugins.size(), 3);
}

TEST_F(PluginManagerComprehensiveTest, DiscoverPluginsInEmptyDirectory) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    auto plugins = manager.discoverPlugins();
    EXPECT_TRUE(plugins.empty());
}

// Plugin Dependency Tests
TEST_F(PluginManagerComprehensiveTest, CheckPluginDependencies) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::string pluginPath = createTestPlugin(testPluginDir, "test_plugin");
    ASSERT_TRUE(manager.loadPlugin(pluginPath));

    // Set up a plugin with a dependency
    auto info = createTestPluginInfo("test_plugin");
    info.dependencies = {"dependency"};
    // In a real system, you would update the plugin's info here

    EXPECT_FALSE(manager.checkDependencies("test_plugin"));
}

TEST_F(PluginManagerComprehensiveTest, ResolvePluginDependencies) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    // Create and load dependency plugin
    std::string depPath = createTestPlugin(testPluginDir, "dependency");
    ASSERT_TRUE(manager.loadPlugin(depPath));
    ASSERT_TRUE(manager.initializePlugin("dependency"));
    ASSERT_TRUE(manager.startPlugin("dependency"));

    // Create and load dependent plugin
    std::string pluginPath = createTestPlugin(testPluginDir, "test_plugin");
    ASSERT_TRUE(manager.loadPlugin(pluginPath));

    // Set up a plugin with a dependency
    auto info = createTestPluginInfo("test_plugin");
    info.dependencies = {"dependency"};
    // In a real system, you would update the plugin's info here

    EXPECT_TRUE(manager.resolveDependencies("test_plugin"));
}

// Plugin Callback Tests
TEST_F(PluginManagerComprehensiveTest, StateChangeCallback) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::string pluginPath = createTestPlugin(testPluginDir, "test_plugin");
    ASSERT_TRUE(manager.loadPlugin(pluginPath));

    bool callbackCalled = false;
    PluginManager::PluginState callbackState;
    std::string callbackPlugin;

    auto callback = [&](const std::string& pluginName,
                       PluginManager::PluginState state) {
        callbackCalled = true;
        callbackPlugin = pluginName;
        callbackState = state;
    };

    manager.registerStateCallback(callback);
    ASSERT_TRUE(manager.initializePlugin("test_plugin"));

    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(callbackPlugin, "test_plugin");
    EXPECT_EQ(callbackState, PluginManager::PluginState::INITIALIZED);
}

TEST_F(PluginManagerComprehensiveTest, ErrorCallback) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::string pluginPath = createTestPlugin(testPluginDir, "test_plugin");
    ASSERT_TRUE(manager.loadPlugin(pluginPath));

    bool callbackCalled = false;
    std::string callbackPlugin;
    std::string callbackError;

    auto callback = [&](const std::string& pluginName,
                       const std::string& error) {
        callbackCalled = true;
        callbackPlugin = pluginName;
        callbackError = error;
    };

    manager.registerErrorCallback(callback);
    // In a real system, you would trigger an error here

    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(callbackPlugin, "test_plugin");
    EXPECT_FALSE(callbackError.empty());
}

TEST_F(PluginManagerComprehensiveTest, ConfigChangeCallback) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::string pluginPath = createTestPlugin(testPluginDir, "test_plugin");
    ASSERT_TRUE(manager.loadPlugin(pluginPath));
    ASSERT_TRUE(manager.initializePlugin("test_plugin"));

    bool callbackCalled = false;
    std::string callbackPlugin;
    nlohmann::json callbackConfig;

    auto callback = [&](const std::string& pluginName,
                       const nlohmann::json& config) {
        callbackCalled = true;
        callbackPlugin = pluginName;
        callbackConfig = config;
    };

    manager.registerConfigCallback(callback);
    auto pluginConfig = createTestPluginConfig();
    ASSERT_TRUE(manager.setPluginConfig("test_plugin", pluginConfig));

    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(callbackPlugin, "test_plugin");
    EXPECT_EQ(callbackConfig, pluginConfig);
}

// Plugin Statistics Tests
TEST_F(PluginManagerComprehensiveTest, PluginStatistics) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    manager.enableStats(true);

    // Create and load test plugins
    std::string plugin1Path = createTestPlugin(testPluginDir, "plugin1");
    std::string plugin2Path = createTestPlugin(testPluginDir, "plugin2");
    ASSERT_TRUE(manager.loadPlugin(plugin1Path));
    ASSERT_TRUE(manager.loadPlugin(plugin2Path));

    ASSERT_TRUE(manager.initializePlugin("plugin1"));
    ASSERT_TRUE(manager.initializePlugin("plugin2"));
    ASSERT_TRUE(manager.startPlugin("plugin1"));

    auto stats = manager.getStats();
    EXPECT_EQ(stats.totalPlugins, 2);
    EXPECT_EQ(stats.loadedPlugins, 2);
    EXPECT_EQ(stats.runningPlugins, 1);
    EXPECT_EQ(stats.failedPlugins, 0);
    EXPECT_GT(stats.averageLoadTime.count(), 0);
    EXPECT_GT(stats.averageInitTime.count(), 0);
    EXPECT_GT(stats.averageStartTime.count(), 0);
}

// Error Handling Tests
TEST_F(PluginManagerComprehensiveTest, GetLastError) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_FALSE(manager.loadPlugin("/nonexistent/plugin.so"));
    EXPECT_FALSE(manager.getLastError().empty());
}

TEST_F(PluginManagerComprehensiveTest, ClearLastError) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    EXPECT_FALSE(manager.loadPlugin("/nonexistent/plugin.so"));
    EXPECT_FALSE(manager.getLastError().empty());

    manager.clearLastError();
    EXPECT_TRUE(manager.getLastError().empty());
}

// Concurrency Tests
TEST_F(PluginManagerComprehensiveTest, ConcurrentPluginLoading) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&, i]() {
            std::string pluginPath = createTestPlugin(
                testPluginDir, "test_plugin_" + std::to_string(i));
            manager.loadPlugin(pluginPath);
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(manager.isPluginLoaded("test_plugin_" + std::to_string(i)));
    }
}

// Edge Cases Tests
TEST_F(PluginManagerComprehensiveTest, EmptyPluginName) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::string pluginPath = createTestPlugin(testPluginDir, "");
    EXPECT_FALSE(manager.loadPlugin(pluginPath));
}

TEST_F(PluginManagerComprehensiveTest, LongPluginName) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::string longName(256, 'a');
    std::string pluginPath = createTestPlugin(testPluginDir, longName);
    EXPECT_FALSE(manager.loadPlugin(pluginPath));
}

TEST_F(PluginManagerComprehensiveTest, SpecialCharactersInPluginName) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::string pluginPath = createTestPlugin(testPluginDir, "test@plugin#123");
    EXPECT_FALSE(manager.loadPlugin(pluginPath));
}

// Cleanup Tests
TEST_F(PluginManagerComprehensiveTest, ShutdownWithLoadedPlugins) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::string pluginPath = createTestPlugin(testPluginDir, "test_plugin");
    ASSERT_TRUE(manager.loadPlugin(pluginPath));
    ASSERT_TRUE(manager.initializePlugin("test_plugin"));
    ASSERT_TRUE(manager.startPlugin("test_plugin"));

    manager.shutdown();
    EXPECT_FALSE(manager.isInitialized());
    EXPECT_FALSE(manager.isPluginLoaded("test_plugin"));
}

TEST_F(PluginManagerComprehensiveTest, ShutdownWithoutInitialization) {
    auto& manager = PluginManager::getInstance();
    manager.shutdown();
    EXPECT_FALSE(manager.isInitialized());
}

// Stress Tests
TEST_F(PluginManagerComprehensiveTest, LoadManyPlugins) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    const int numPlugins = 100;
    for (int i = 0; i < numPlugins; ++i) {
        std::string pluginPath = createTestPlugin(
            testPluginDir, "test_plugin_" + std::to_string(i));
        ASSERT_TRUE(manager.loadPlugin(pluginPath));
    }

    auto stats = manager.getStats();
    EXPECT_EQ(stats.totalPlugins, numPlugins);
}

TEST_F(PluginManagerComprehensiveTest, RapidPluginOperations) {
    auto& manager = PluginManager::getInstance();
    ASSERT_TRUE(manager.initialize(config));

    std::string pluginPath = createTestPlugin(testPluginDir, "test_plugin");
    ASSERT_TRUE(manager.loadPlugin(pluginPath));

    for (int i = 0; i < 100; ++i) {
        ASSERT_TRUE(manager.initializePlugin("test_plugin"));
        ASSERT_TRUE(manager.startPlugin("test_plugin"));
        ASSERT_TRUE(manager.pausePlugin("test_plugin"));
        ASSERT_TRUE(manager.resumePlugin("test_plugin"));
        ASSERT_TRUE(manager.stopPlugin("test_plugin"));
    }
} 