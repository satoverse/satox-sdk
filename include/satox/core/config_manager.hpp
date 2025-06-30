/*
 * MIT License
 * Copyright(c) 2025 Satoxcoin Core Developer
 */

#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <chrono>
#include <functional>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>
#include "types.hpp"

namespace satox::core {

class ConfigManager {
public:
    // Configuration value types
    enum class ValueType {
        STRING,
        INTEGER,
        FLOAT,
        BOOLEAN,
        ARRAY,
        OBJECT,
        NULL_VALUE
    };

    // Configuration value structure
    struct ConfigValue {
        ValueType type;
        std::string stringValue;
        int64_t intValue;
        double floatValue;
        bool boolValue;
        nlohmann::json complexValue; // Use JSON for arrays and objects
    };

    // Configuration section structure
    struct ConfigSection {
        std::string name;
        std::unordered_map<std::string, ConfigValue> values;
        std::chrono::system_clock::time_point last_modified;
    };

    // Configuration callback type
    using ConfigCallback = std::function<void(const std::string&, const ConfigValue&)>;
    using ErrorCallback = std::function<void(const std::string&)>;

    // Singleton instance
    static ConfigManager& getInstance();

    // Prevent copying
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    // Initialization and shutdown
    bool initialize(const std::string& config_path = "");
    bool shutdown();

    // Configuration loading and saving
    bool loadConfig(const std::string& path);
    bool saveConfig(const std::string& path = "");
    bool reloadConfig();
    bool validateConfig();

    // Section operations
    bool createSection(const std::string& section);
    bool deleteSection(const std::string& section);
    bool hasSection(const std::string& section) const;
    std::vector<std::string> getSections() const;

    // Value operations
    bool setValue(const std::string& section, const std::string& key,
                 const ConfigValue& value);
    bool setString(const std::string& section, const std::string& key,
                  const std::string& value);
    bool setInt(const std::string& section, const std::string& key, int64_t value);
    bool setFloat(const std::string& section, const std::string& key, double value);
    bool setBool(const std::string& section, const std::string& key, bool value);
    bool setArray(const std::string& section, const std::string& key,
                 const std::vector<ConfigValue>& value);
    bool setObject(const std::string& section, const std::string& key,
                  const std::unordered_map<std::string, ConfigValue>& value);

    std::optional<ConfigValue> getValue(const std::string& section,
                                      const std::string& key) const;
    std::optional<std::string> getString(const std::string& section,
                                       const std::string& key) const;
    std::optional<int64_t> getInt(const std::string& section,
                                 const std::string& key) const;
    std::optional<double> getFloat(const std::string& section,
                                 const std::string& key) const;
    std::optional<bool> getBool(const std::string& section,
                               const std::string& key) const;
    std::optional<std::vector<ConfigValue>> getArray(const std::string& section,
                                                   const std::string& key) const;
    std::optional<std::unordered_map<std::string, ConfigValue>> getObject(
        const std::string& section, const std::string& key) const;

    bool hasValue(const std::string& section, const std::string& key) const;
    bool deleteValue(const std::string& section, const std::string& key);
    std::vector<std::string> getKeys(const std::string& section) const;

    // Default values
    bool setDefaultValue(const std::string& section, const std::string& key,
                        const ConfigValue& value);
    bool setDefaultString(const std::string& section, const std::string& key,
                         const std::string& value);
    bool setDefaultInt(const std::string& section, const std::string& key,
                      int64_t value);
    bool setDefaultFloat(const std::string& section, const std::string& key,
                        double value);
    bool setDefaultBool(const std::string& section, const std::string& key,
                       bool value);
    bool setDefaultArray(const std::string& section, const std::string& key,
                        const std::vector<ConfigValue>& value);
    bool setDefaultObject(const std::string& section, const std::string& key,
                         const std::unordered_map<std::string, ConfigValue>& value);

    // Environment variables
    bool loadFromEnvironment(const std::string& prefix = "");
    bool setFromEnvironment(const std::string& section, const std::string& key,
                           const std::string& envVar);

    // Callbacks
    void registerConfigCallback(ConfigCallback callback);
    void registerErrorCallback(ErrorCallback callback);

    // Validation
    bool validateValue(const ConfigValue& value) const;
    bool validateSection(const std::string& section) const;
    bool validateKey(const std::string& key) const;

    // Error handling
    std::string getLastError() const;
    void clearLastError();

    // Statistics and monitoring
    struct Stats {
        size_t total_sections;
        size_t total_values;
        size_t load_count;
        size_t save_count;
        std::chrono::system_clock::time_point last_load;
        std::chrono::system_clock::time_point last_save;
    };
    Stats getStats() const;

    // Health check
    bool isHealthy() const;

private:
    ConfigManager() = default;
    ~ConfigManager() = default;

    // Helper methods
    bool parseConfigFile(const std::string& content);
    std::string serializeConfig() const;
    void notifyConfigChange(const std::string& section, const std::string& key, const ConfigValue& value);
    void notifyError(const std::string& error);
    void updateStats(bool is_load);
    
    // Additional helper methods
    ConfigValue createValue(const nlohmann::json& json) const;
    nlohmann::json valueToJson(const ConfigValue& value) const;
    bool validateConfigFile(const std::string& content) const;
    std::string getEnvironmentVariable(const std::string& name) const;

    // Member variables
    bool initialized_ = false;
    mutable std::mutex mutex_;
    std::string configPath_;
    std::unordered_map<std::string, ConfigSection> sections_;
    std::unordered_map<std::string, ConfigValue> defaultValues_;
    std::vector<ConfigCallback> configCallbacks_;
    std::vector<ErrorCallback> errorCallbacks_;
    Stats stats_;
    std::string lastError_;
};

} // namespace satox::core 