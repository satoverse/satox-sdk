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

#include "satox/core/config_manager.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <spdlog/spdlog.h>

namespace satox::core {

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::initialize(const std::string& config_path) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        lastError_ = "Configuration manager already initialized";
        return false;
    }

    configPath_ = config_path;
    if (!configPath_.empty() && !loadConfig(configPath_)) {
        return false;
    }

    initialized_ = true;
    spdlog::info("ConfigManager initialized with config path: {}", configPath_);
    return true;
}

bool ConfigManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return false;
    }

    sections_.clear();
    defaultValues_.clear();
    configCallbacks_.clear();
    errorCallbacks_.clear();
    initialized_ = false;
    spdlog::info("ConfigManager shutdown complete");
    return true;
}

bool ConfigManager::loadConfig(const std::string& path) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!std::filesystem::exists(path)) {
        lastError_ = "Configuration file does not exist: " + path;
        return false;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        lastError_ = "Failed to open configuration file: " + path;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    if (!validateConfigFile(content)) {
        return false;
    }

    if (!parseConfigFile(content)) {
        return false;
    }

    configPath_ = path;
    updateStats(true);
    spdlog::info("Config loaded from: {}", path);
    return true;
}

bool ConfigManager::saveConfig(const std::string& path) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string targetFile = path.empty() ? configPath_ : path;
    if (targetFile.empty()) {
        lastError_ = "No configuration file specified";
        return false;
    }

    std::string content = serializeConfig();
    std::ofstream file(targetFile);
    if (!file.is_open()) {
        lastError_ = "Failed to open configuration file for writing: " + targetFile;
        return false;
    }

    file << content;
    updateStats(false);
    spdlog::info("Config saved to: {}", targetFile);
    return true;
}

bool ConfigManager::reloadConfig() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (configPath_.empty()) {
        lastError_ = "No configuration file specified";
        return false;
    }

    return loadConfig(configPath_);
}

bool ConfigManager::validateConfig() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& [sectionName, section] : sections_) {
        if (!validateSection(sectionName)) {
            return false;
        }

        for (const auto& [key, value] : section.values) {
            if (!validateKey(key) || !validateValue(value)) {
                return false;
            }
        }
    }

    return true;
}

bool ConfigManager::createSection(const std::string& section) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!validateSection(section)) {
        return false;
    }

    if (sections_.find(section) != sections_.end()) {
        lastError_ = "Section already exists: " + section;
        return false;
    }

    sections_[section] = ConfigSection{section};
    return true;
}

bool ConfigManager::deleteSection(const std::string& section) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (sections_.find(section) == sections_.end()) {
        lastError_ = "Section does not exist: " + section;
        return false;
    }

    sections_.erase(section);
    return true;
}

bool ConfigManager::hasSection(const std::string& section) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return sections_.find(section) != sections_.end();
}

std::vector<std::string> ConfigManager::getSections() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> result;
    result.reserve(sections_.size());
    for (const auto& [name, _] : sections_) {
        result.push_back(name);
    }
    return result;
}

bool ConfigManager::setValue(const std::string& section, const std::string& key,
                           const ConfigValue& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!validateSection(section) || !validateKey(key) || !validateValue(value)) {
        return false;
    }

    if (sections_.find(section) == sections_.end()) {
        sections_[section] = ConfigSection{section};
    }

    sections_[section].values[key] = value;
    notifyConfigChange(section, key, value);
    return true;
}

bool ConfigManager::setString(const std::string& section, const std::string& key,
                            const std::string& value) {
    ConfigValue configValue;
    configValue.type = ValueType::STRING;
    configValue.stringValue = value;
    return setValue(section, key, configValue);
}

bool ConfigManager::setInt(const std::string& section, const std::string& key,
                         int64_t value) {
    ConfigValue configValue;
    configValue.type = ValueType::INTEGER;
    configValue.intValue = value;
    return setValue(section, key, configValue);
}

bool ConfigManager::setFloat(const std::string& section, const std::string& key,
                           double value) {
    ConfigValue configValue;
    configValue.type = ValueType::FLOAT;
    configValue.floatValue = value;
    return setValue(section, key, configValue);
}

bool ConfigManager::setBool(const std::string& section, const std::string& key,
                          bool value) {
    ConfigValue configValue;
    configValue.type = ValueType::BOOLEAN;
    configValue.boolValue = value;
    return setValue(section, key, configValue);
}

bool ConfigManager::setArray(const std::string& section, const std::string& key,
                           const std::vector<ConfigValue>& value) {
    ConfigValue configValue;
    configValue.type = ValueType::ARRAY;
    configValue.complexValue = nlohmann::json::array();
    for (const auto& item : value) {
        configValue.complexValue.push_back(valueToJson(item));
    }
    return setValue(section, key, configValue);
}

bool ConfigManager::setObject(const std::string& section, const std::string& key,
                            const std::unordered_map<std::string, ConfigValue>& value) {
    ConfigValue configValue;
    configValue.type = ValueType::OBJECT;
    configValue.complexValue = nlohmann::json::object();
    for (const auto& [k, v] : value) {
        configValue.complexValue[k] = valueToJson(v);
    }
    return setValue(section, key, configValue);
}

std::optional<ConfigManager::ConfigValue> ConfigManager::getValue(const std::string& section,
                                                               const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto sectionIt = sections_.find(section);
    if (sectionIt == sections_.end()) {
        return std::nullopt;
    }
    
    auto valueIt = sectionIt->second.values.find(key);
    if (valueIt == sectionIt->second.values.end()) {
        return std::nullopt;
    }
    
    return valueIt->second;
}

std::optional<std::string> ConfigManager::getString(const std::string& section,
                                                  const std::string& key) const {
    auto value = getValue(section, key);
    if (!value || value->type != ValueType::STRING) {
        return std::nullopt;
    }
    return value->stringValue;
}

std::optional<int64_t> ConfigManager::getInt(const std::string& section,
                                            const std::string& key) const {
    auto value = getValue(section, key);
    if (!value || value->type != ValueType::INTEGER) {
        return std::nullopt;
    }
    return value->intValue;
}

std::optional<double> ConfigManager::getFloat(const std::string& section,
                                            const std::string& key) const {
    auto value = getValue(section, key);
    if (!value || value->type != ValueType::FLOAT) {
        return std::nullopt;
    }
    return value->floatValue;
}

std::optional<bool> ConfigManager::getBool(const std::string& section,
                                          const std::string& key) const {
    auto value = getValue(section, key);
    if (!value || value->type != ValueType::BOOLEAN) {
        return std::nullopt;
    }
    return value->boolValue;
}

std::optional<std::vector<ConfigManager::ConfigValue>> ConfigManager::getArray(const std::string& section,
                                                                             const std::string& key) const {
    auto value = getValue(section, key);
    if (!value || value->type != ValueType::ARRAY) {
        return std::nullopt;
    }
    
    std::vector<ConfigValue> result;
    for (const auto& item : value->complexValue) {
        result.push_back(createValue(item));
    }
    return result;
}

std::optional<std::unordered_map<std::string, ConfigManager::ConfigValue>> ConfigManager::getObject(const std::string& section,
                                                                                                   const std::string& key) const {
    auto value = getValue(section, key);
    if (!value || value->type != ValueType::OBJECT) {
        return std::nullopt;
    }
    
    std::unordered_map<std::string, ConfigValue> result;
    for (const auto& [k, v] : value->complexValue.items()) {
        result[k] = createValue(v);
    }
    return result;
}

bool ConfigManager::hasValue(const std::string& section, const std::string& key) const {
    return getValue(section, key).has_value();
}

bool ConfigManager::deleteValue(const std::string& section, const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto sectionIt = sections_.find(section);
    if (sectionIt == sections_.end()) {
        return false;
    }
    
    auto valueIt = sectionIt->second.values.find(key);
    if (valueIt == sectionIt->second.values.end()) {
        return false;
    }
    
    sectionIt->second.values.erase(valueIt);
    return true;
}

std::vector<std::string> ConfigManager::getKeys(const std::string& section) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto sectionIt = sections_.find(section);
    if (sectionIt == sections_.end()) {
        return {};
    }
    
    std::vector<std::string> result;
    result.reserve(sectionIt->second.values.size());
    for (const auto& [key, _] : sectionIt->second.values) {
        result.push_back(key);
    }
    return result;
}

bool ConfigManager::setDefaultValue(const std::string& section, const std::string& key,
                                  const ConfigValue& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!validateSection(section) || !validateKey(key) || !validateValue(value)) {
        return false;
    }
    
    std::string fullKey = section + "." + key;
    defaultValues_[fullKey] = value;
    return true;
}

bool ConfigManager::setDefaultString(const std::string& section, const std::string& key,
                                   const std::string& value) {
    ConfigValue configValue;
    configValue.type = ValueType::STRING;
    configValue.stringValue = value;
    return setDefaultValue(section, key, configValue);
}

bool ConfigManager::setDefaultInt(const std::string& section, const std::string& key,
                                int64_t value) {
    ConfigValue configValue;
    configValue.type = ValueType::INTEGER;
    configValue.intValue = value;
    return setDefaultValue(section, key, configValue);
}

bool ConfigManager::setDefaultFloat(const std::string& section, const std::string& key,
                                  double value) {
    ConfigValue configValue;
    configValue.type = ValueType::FLOAT;
    configValue.floatValue = value;
    return setDefaultValue(section, key, configValue);
}

bool ConfigManager::setDefaultBool(const std::string& section, const std::string& key,
                                 bool value) {
    ConfigValue configValue;
    configValue.type = ValueType::BOOLEAN;
    configValue.boolValue = value;
    return setDefaultValue(section, key, configValue);
}

bool ConfigManager::setDefaultArray(const std::string& section, const std::string& key,
                                  const std::vector<ConfigValue>& value) {
    ConfigValue configValue;
    configValue.type = ValueType::ARRAY;
    configValue.complexValue = nlohmann::json::array();
    for (const auto& item : value) {
        configValue.complexValue.push_back(valueToJson(item));
    }
    return setDefaultValue(section, key, configValue);
}

bool ConfigManager::setDefaultObject(const std::string& section, const std::string& key,
                                   const std::unordered_map<std::string, ConfigValue>& value) {
    ConfigValue configValue;
    configValue.type = ValueType::OBJECT;
    configValue.complexValue = nlohmann::json::object();
    for (const auto& [k, v] : value) {
        configValue.complexValue[k] = valueToJson(v);
    }
    return setDefaultValue(section, key, configValue);
}

bool ConfigManager::loadFromEnvironment(const std::string& prefix) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& [key, value] : defaultValues_) {
        std::string envVar = prefix + key;
        std::transform(envVar.begin(), envVar.end(), envVar.begin(),
                      [](unsigned char c) { return std::toupper(c); });
        
        std::string envValue = getEnvironmentVariable(envVar);
        if (!envValue.empty()) {
            size_t dotPos = key.find('.');
            if (dotPos != std::string::npos) {
                std::string section = key.substr(0, dotPos);
                std::string configKey = key.substr(dotPos + 1);
                setValue(section, configKey, value);
            }
        }
    }
    
    return true;
}

bool ConfigManager::setFromEnvironment(const std::string& section, const std::string& key,
                                     const std::string& envVar) {
    std::string envValue = getEnvironmentVariable(envVar);
    if (envValue.empty()) {
        return false;
    }
    
    ConfigValue value;
    value.type = ValueType::STRING;
    value.stringValue = envValue;
    return setValue(section, key, value);
}

void ConfigManager::registerConfigCallback(ConfigCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    configCallbacks_.push_back(callback);
}

void ConfigManager::registerErrorCallback(ErrorCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    errorCallbacks_.push_back(callback);
}

bool ConfigManager::validateValue(const ConfigValue& value) const {
    switch (value.type) {
        case ValueType::STRING:
            return !value.stringValue.empty();
        case ValueType::INTEGER:
            return true;
        case ValueType::FLOAT:
            return !std::isnan(value.floatValue) && !std::isinf(value.floatValue);
        case ValueType::BOOLEAN:
            return true;
        case ValueType::ARRAY:
            return value.complexValue.is_array();
        case ValueType::OBJECT:
            return value.complexValue.is_object();
        case ValueType::NULL_VALUE:
            return true;
        default:
            return false;
    }
}

bool ConfigManager::validateSection(const std::string& section) const {
    if (section.empty()) {
        return false;
    }
    
    // Check for invalid characters
    for (char c : section) {
        if (!std::isalnum(c) && c != '_' && c != '-') {
            return false;
        }
    }
    
    return true;
}

bool ConfigManager::validateKey(const std::string& key) const {
    if (key.empty()) {
        return false;
    }
    
    // Check for invalid characters
    for (char c : key) {
        if (!std::isalnum(c) && c != '_' && c != '-') {
            return false;
        }
    }
    
    return true;
}

std::string ConfigManager::getLastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastError_;
}

void ConfigManager::clearLastError() {
    std::lock_guard<std::mutex> lock(mutex_);
    lastError_.clear();
}

ConfigManager::Stats ConfigManager::getStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

bool ConfigManager::isHealthy() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return initialized_ && lastError_.empty();
}

bool ConfigManager::parseConfigFile(const std::string& content) {
    try {
        nlohmann::json config = nlohmann::json::parse(content);
        
        for (const auto& [sectionName, sectionData] : config.items()) {
            if (!validateSection(sectionName)) {
                lastError_ = "Invalid section name: " + sectionName;
                return false;
            }
            
            ConfigSection configSection;
            configSection.name = sectionName;
            
            for (const auto& [key, value] : sectionData.items()) {
                if (!validateKey(key)) {
                    lastError_ = "Invalid key name: " + key;
                    return false;
                }
                
                configSection.values[key] = createValue(value);
            }
            
            sections_[sectionName] = configSection;
        }
        
        return true;
    } catch (const std::exception& e) {
        lastError_ = "Failed to parse config file: " + std::string(e.what());
        return false;
    }
}

std::string ConfigManager::serializeConfig() const {
    nlohmann::json config = nlohmann::json::object();
    
    for (const auto& [sectionName, section] : sections_) {
        nlohmann::json sectionJson = nlohmann::json::object();
        
        for (const auto& [key, value] : section.values) {
            sectionJson[key] = valueToJson(value);
        }
        
        config[sectionName] = sectionJson;
    }
    
    return config.dump(2);
}

void ConfigManager::notifyConfigChange(const std::string& section,
                                     const std::string& key, const ConfigValue& value) {
    for (const auto& callback : configCallbacks_) {
        try {
            callback(section, value);
        } catch (const std::exception& e) {
            spdlog::error("Config callback error: {}", e.what());
        }
    }
}

void ConfigManager::notifyError(const std::string& error) {
    for (const auto& callback : errorCallbacks_) {
        try {
            callback(error);
        } catch (const std::exception& e) {
            spdlog::error("Error callback error: {}", e.what());
        }
    }
}

void ConfigManager::updateStats(bool is_load) {
    if (is_load) {
        stats_.load_count++;
        stats_.last_load = std::chrono::system_clock::now();
    } else {
        stats_.save_count++;
        stats_.last_save = std::chrono::system_clock::now();
    }
    
    stats_.total_sections = sections_.size();
    stats_.total_values = 0;
    for (const auto& [_, section] : sections_) {
        stats_.total_values += section.values.size();
    }
}

ConfigManager::ConfigValue ConfigManager::createValue(const nlohmann::json& json) const {
    ConfigValue value;
    
    if (json.is_string()) {
        value.type = ValueType::STRING;
        value.stringValue = json.get<std::string>();
    } else if (json.is_number_integer()) {
        value.type = ValueType::INTEGER;
        value.intValue = json.get<int64_t>();
    } else if (json.is_number_float()) {
        value.type = ValueType::FLOAT;
        value.floatValue = json.get<double>();
    } else if (json.is_boolean()) {
        value.type = ValueType::BOOLEAN;
        value.boolValue = json.get<bool>();
    } else if (json.is_array()) {
        value.type = ValueType::ARRAY;
        value.complexValue = json;
    } else if (json.is_object()) {
        value.type = ValueType::OBJECT;
        value.complexValue = json;
    } else if (json.is_null()) {
        value.type = ValueType::NULL_VALUE;
    }
    
    return value;
}

nlohmann::json ConfigManager::valueToJson(const ConfigValue& value) const {
    switch (value.type) {
        case ValueType::STRING:
            return value.stringValue;
        case ValueType::INTEGER:
            return value.intValue;
        case ValueType::FLOAT:
            return value.floatValue;
        case ValueType::BOOLEAN:
            return value.boolValue;
        case ValueType::ARRAY:
        case ValueType::OBJECT:
            return value.complexValue;
        case ValueType::NULL_VALUE:
            return nullptr;
        default:
            return nullptr;
    }
}

bool ConfigManager::validateConfigFile(const std::string& content) const {
    try {
        nlohmann::json::parse(content);
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

std::string ConfigManager::getEnvironmentVariable(const std::string& name) const {
    const char* value = std::getenv(name.c_str());
    return value ? std::string(value) : "";
}

} // namespace satox::core 