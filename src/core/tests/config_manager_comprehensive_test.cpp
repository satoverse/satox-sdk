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
#include <gmock/gmock.h>
#include "satox/core/config_manager.hpp"
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <future>
#include <random>
#include <algorithm>

using namespace satox::core;
using namespace testing;

class ConfigManagerComprehensiveTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test configuration file
        std::ofstream configFile("test_config.json");
        configFile << R"({
            "database": {
                "host": "localhost",
                "port": 5432,
                "user": "test_user",
                "password": "test_pass",
                "ssl": true,
                "timeout": 30.5
            },
            "network": {
                "endpoints": ["endpoint1", "endpoint2", "endpoint3"],
                "settings": {
                    "timeout": 5000,
                    "retries": 3,
                    "keepalive": true
                }
            }
        })";
        configFile.close();

        // Initialize configuration manager
        ASSERT_TRUE(ConfigManager::getInstance().initialize("test_config.json"));
    }

    void TearDown() override {
        // Clean up test files
        std::filesystem::remove("test_config.json");
        std::filesystem::remove("test_config_modified.json");
        std::filesystem::remove("test_config_invalid.json");
        std::filesystem::remove("test_config_empty.json");
        std::filesystem::remove("test_config_large.json");
    }

    // Helper function to create a test configuration file
    void createTestConfigFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename);
        file << content;
        file.close();
    }

    // Helper function to read a configuration file
    std::string readConfigFile(const std::string& filename) {
        std::ifstream file(filename);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
};

// Test initialization
TEST_F(ConfigManagerComprehensiveTest, Initialization) {
    // Test valid initialization
    EXPECT_TRUE(ConfigManager::getInstance().initialize("test_config.json"));

    // Test invalid file
    EXPECT_FALSE(ConfigManager::getInstance().initialize("nonexistent.json"));

    // Test invalid JSON
    createTestConfigFile("test_config_invalid.json", "invalid json content");
    EXPECT_FALSE(ConfigManager::getInstance().initialize("test_config_invalid.json"));

    // Test empty file
    createTestConfigFile("test_config_empty.json", "");
    EXPECT_FALSE(ConfigManager::getInstance().initialize("test_config_empty.json"));
}

// Test section operations
TEST_F(ConfigManagerComprehensiveTest, SectionOperations) {
    auto& config = ConfigManager::getInstance();

    // Test section creation
    EXPECT_TRUE(config.createSection("test_section"));
    EXPECT_TRUE(config.hasSection("test_section"));

    // Test duplicate section
    EXPECT_FALSE(config.createSection("test_section"));

    // Test section deletion
    EXPECT_TRUE(config.deleteSection("test_section"));
    EXPECT_FALSE(config.hasSection("test_section"));

    // Test invalid section names
    EXPECT_FALSE(config.createSection(""));
    EXPECT_FALSE(config.createSection("section.with.dots"));

    // Test get sections
    auto sections = config.getSections();
    EXPECT_THAT(sections, UnorderedElementsAre("database", "network"));
}

// Test value operations
TEST_F(ConfigManagerComprehensiveTest, ValueOperations) {
    auto& config = ConfigManager::getInstance();

    // Test string values
    EXPECT_TRUE(config.setString("test_section", "string_key", "test_value"));
    auto stringValue = config.getString("test_section", "string_key");
    EXPECT_TRUE(stringValue.has_value());
    EXPECT_EQ(stringValue.value(), "test_value");

    // Test integer values
    EXPECT_TRUE(config.setInt("test_section", "int_key", 42));
    auto intValue = config.getInt("test_section", "int_key");
    EXPECT_TRUE(intValue.has_value());
    EXPECT_EQ(intValue.value(), 42);

    // Test float values
    EXPECT_TRUE(config.setFloat("test_section", "float_key", 3.14));
    auto floatValue = config.getFloat("test_section", "float_key");
    EXPECT_TRUE(floatValue.has_value());
    EXPECT_DOUBLE_EQ(floatValue.value(), 3.14);

    // Test boolean values
    EXPECT_TRUE(config.setBool("test_section", "bool_key", true));
    auto boolValue = config.getBool("test_section", "bool_key");
    EXPECT_TRUE(boolValue.has_value());
    EXPECT_TRUE(boolValue.value());

    // Test array values
    std::vector<ConfigManager::ConfigValue> array;
    ConfigManager::ConfigValue arrayItem;
    arrayItem.type = ConfigManager::ValueType::STRING;
    arrayItem.stringValue = "array_item";
    array.push_back(arrayItem);
    EXPECT_TRUE(config.setArray("test_section", "array_key", array));
    auto arrayValue = config.getArray("test_section", "array_key");
    EXPECT_TRUE(arrayValue.has_value());
    EXPECT_EQ(arrayValue.value().size(), 1);
    EXPECT_EQ(arrayValue.value()[0].stringValue, "array_item");

    // Test object values
    std::unordered_map<std::string, ConfigManager::ConfigValue> object;
    ConfigManager::ConfigValue objectItem;
    objectItem.type = ConfigManager::ValueType::INTEGER;
    objectItem.intValue = 42;
    object["nested_key"] = objectItem;
    EXPECT_TRUE(config.setObject("test_section", "object_key", object));
    auto objectValue = config.getObject("test_section", "object_key");
    EXPECT_TRUE(objectValue.has_value());
    EXPECT_EQ(objectValue.value().size(), 1);
    EXPECT_EQ(objectValue.value()["nested_key"].intValue, 42);

    // Test value deletion
    EXPECT_TRUE(config.deleteValue("test_section", "string_key"));
    EXPECT_FALSE(config.hasValue("test_section", "string_key"));

    // Test invalid keys
    EXPECT_FALSE(config.setString("test_section", "", "value"));
    EXPECT_FALSE(config.setString("test_section", "key.with.dots", "value"));
}

// Test default values
TEST_F(ConfigManagerComprehensiveTest, DefaultValues) {
    auto& config = ConfigManager::getInstance();

    // Set default values
    EXPECT_TRUE(config.setDefaultString("test_section", "default_string", "default"));
    EXPECT_TRUE(config.setDefaultInt("test_section", "default_int", 42));
    EXPECT_TRUE(config.setDefaultFloat("test_section", "default_float", 3.14));
    EXPECT_TRUE(config.setDefaultBool("test_section", "default_bool", true));

    // Test default value application
    config.updateDefaults();
    EXPECT_EQ(config.getString("test_section", "default_string").value(), "default");
    EXPECT_EQ(config.getInt("test_section", "default_int").value(), 42);
    EXPECT_DOUBLE_EQ(config.getFloat("test_section", "default_float").value(), 3.14);
    EXPECT_TRUE(config.getBool("test_section", "default_bool").value());

    // Test default value override
    EXPECT_TRUE(config.setString("test_section", "default_string", "override"));
    EXPECT_EQ(config.getString("test_section", "default_string").value(), "override");
}

// Test environment variables
TEST_F(ConfigManagerComprehensiveTest, EnvironmentVariables) {
    auto& config = ConfigManager::getInstance();

    // Set environment variables
    setenv("TEST_SECTION_STRING_KEY", "env_value", 1);
    setenv("TEST_SECTION_INT_KEY", "42", 1);
    setenv("TEST_SECTION_FLOAT_KEY", "3.14", 1);
    setenv("TEST_SECTION_BOOL_KEY", "true", 1);

    // Test environment variable loading
    EXPECT_TRUE(config.loadFromEnvironment("TEST_"));
    EXPECT_EQ(config.getString("SECTION", "STRING_KEY").value(), "env_value");
    EXPECT_EQ(config.getInt("SECTION", "INT_KEY").value(), 42);
    EXPECT_DOUBLE_EQ(config.getFloat("SECTION", "FLOAT_KEY").value(), 3.14);
    EXPECT_TRUE(config.getBool("SECTION", "BOOL_KEY").value());

    // Test individual environment variable setting
    setenv("CUSTOM_ENV_VAR", "custom_value", 1);
    EXPECT_TRUE(config.setFromEnvironment("test_section", "custom_key", "CUSTOM_ENV_VAR"));
    EXPECT_EQ(config.getString("test_section", "custom_key").value(), "custom_value");

    // Clean up environment variables
    unsetenv("TEST_SECTION_STRING_KEY");
    unsetenv("TEST_SECTION_INT_KEY");
    unsetenv("TEST_SECTION_FLOAT_KEY");
    unsetenv("TEST_SECTION_BOOL_KEY");
    unsetenv("CUSTOM_ENV_VAR");
}

// Test callbacks
TEST_F(ConfigManagerComprehensiveTest, Callbacks) {
    auto& config = ConfigManager::getInstance();
    std::vector<std::string> callbackKeys;
    std::vector<ConfigManager::ConfigValue> callbackValues;

    // Register callback
    auto callback = [&](const std::string& key, const ConfigManager::ConfigValue& value) {
        callbackKeys.push_back(key);
        callbackValues.push_back(value);
    };
    config.registerCallback(callback);

    // Test callback invocation
    EXPECT_TRUE(config.setString("test_section", "callback_key", "callback_value"));
    EXPECT_EQ(callbackKeys.size(), 1);
    EXPECT_EQ(callbackKeys[0], "test_section.callback_key");
    EXPECT_EQ(callbackValues[0].stringValue, "callback_value");

    // Test callback unregistration
    config.unregisterCallback(callback);
    EXPECT_TRUE(config.setString("test_section", "another_key", "another_value"));
    EXPECT_EQ(callbackKeys.size(), 1);
}

// Test error handling
TEST_F(ConfigManagerComprehensiveTest, ErrorHandling) {
    auto& config = ConfigManager::getInstance();

    // Test invalid section
    EXPECT_FALSE(config.createSection(""));
    EXPECT_FALSE(config.getLastError().empty());

    // Test invalid key
    EXPECT_FALSE(config.setString("test_section", "", "value"));
    EXPECT_FALSE(config.getLastError().empty());

    // Test invalid value
    ConfigManager::ConfigValue invalidValue;
    invalidValue.type = static_cast<ConfigManager::ValueType>(999);
    EXPECT_FALSE(config.setValue("test_section", "key", invalidValue));
    EXPECT_FALSE(config.getLastError().empty());

    // Test error clearing
    config.clearLastError();
    EXPECT_TRUE(config.getLastError().empty());
}

// Test concurrency
TEST_F(ConfigManagerComprehensiveTest, Concurrency) {
    auto& config = ConfigManager::getInstance();
    std::vector<std::future<void>> futures;

    // Test concurrent section creation
    for (int i = 0; i < 10; ++i) {
        futures.push_back(std::async(std::launch::async, [&, i]() {
            config.createSection("concurrent_section_" + std::to_string(i));
        }));
    }
    for (auto& future : futures) {
        future.wait();
    }

    // Test concurrent value setting
    futures.clear();
    for (int i = 0; i < 10; ++i) {
        futures.push_back(std::async(std::launch::async, [&, i]() {
            config.setString("concurrent_section_0", "key_" + std::to_string(i),
                           "value_" + std::to_string(i));
        }));
    }
    for (auto& future : futures) {
        future.wait();
    }

    // Verify results
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(config.hasSection("concurrent_section_" + std::to_string(i)));
    }
    for (int i = 0; i < 10; ++i) {
        auto value = config.getString("concurrent_section_0", "key_" + std::to_string(i));
        EXPECT_TRUE(value.has_value());
        EXPECT_EQ(value.value(), "value_" + std::to_string(i));
    }
}

// Test edge cases
TEST_F(ConfigManagerComprehensiveTest, EdgeCases) {
    auto& config = ConfigManager::getInstance();

    // Test empty values
    EXPECT_TRUE(config.setString("test_section", "empty_string", ""));
    EXPECT_TRUE(config.setArray("test_section", "empty_array", {}));
    EXPECT_TRUE(config.setObject("test_section", "empty_object", {}));

    // Test special characters
    std::string specialChars = "!@#$%^&*()_+-=[]{}|;:,.<>?/~`";
    EXPECT_TRUE(config.setString("test_section", "special_chars", specialChars));
    EXPECT_EQ(config.getString("test_section", "special_chars").value(), specialChars);

    // Test Unicode characters
    std::string unicodeChars = "你好世界";
    EXPECT_TRUE(config.setString("test_section", "unicode_chars", unicodeChars));
    EXPECT_EQ(config.getString("test_section", "unicode_chars").value(), unicodeChars);

    // Test maximum key length
    std::string longKey(1000, 'a');
    EXPECT_TRUE(config.setString("test_section", longKey, "value"));
    EXPECT_EQ(config.getString("test_section", longKey).value(), "value");
}

// Test cleanup
TEST_F(ConfigManagerComprehensiveTest, Cleanup) {
    auto& config = ConfigManager::getInstance();

    // Create test data
    EXPECT_TRUE(config.createSection("cleanup_section"));
    EXPECT_TRUE(config.setString("cleanup_section", "key", "value"));

    // Save configuration
    EXPECT_TRUE(config.saveConfig("test_config_modified.json"));

    // Shutdown configuration manager
    config.shutdown();

    // Verify cleanup
    EXPECT_FALSE(config.hasSection("cleanup_section"));
    EXPECT_FALSE(config.hasValue("cleanup_section", "key"));

    // Verify saved configuration
    std::string savedConfig = readConfigFile("test_config_modified.json");
    EXPECT_FALSE(savedConfig.empty());
    EXPECT_TRUE(savedConfig.find("cleanup_section") != std::string::npos);
    EXPECT_TRUE(savedConfig.find("key") != std::string::npos);
    EXPECT_TRUE(savedConfig.find("value") != std::string::npos);
}

// Test stress
TEST_F(ConfigManagerComprehensiveTest, Stress) {
    auto& config = ConfigManager::getInstance();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1000);

    // Test large number of sections and values
    for (int i = 0; i < 1000; ++i) {
        std::string section = "stress_section_" + std::to_string(i);
        EXPECT_TRUE(config.createSection(section));
        for (int j = 0; j < 100; ++j) {
            std::string key = "key_" + std::to_string(j);
            std::string value = "value_" + std::to_string(dis(gen));
            EXPECT_TRUE(config.setString(section, key, value));
        }
    }

    // Verify results
    for (int i = 0; i < 1000; ++i) {
        std::string section = "stress_section_" + std::to_string(i);
        EXPECT_TRUE(config.hasSection(section));
        for (int j = 0; j < 100; ++j) {
            std::string key = "key_" + std::to_string(j);
            EXPECT_TRUE(config.hasValue(section, key));
        }
    }

    // Test large values
    std::string largeValue(1000000, 'a');
    EXPECT_TRUE(config.setString("test_section", "large_value", largeValue));
    EXPECT_EQ(config.getString("test_section", "large_value").value(), largeValue);
}

} // namespace satox::core 