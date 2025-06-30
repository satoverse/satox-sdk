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
#include "satox/core/cache_manager.hpp"
#include <thread>
#include <future>
#include <chrono>
#include <random>
#include <nlohmann/json.hpp>

using namespace satox::core;
using namespace std::chrono_literals;

class CacheManagerComprehensiveTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize Cache Manager with test configuration
        CacheConfig config;
        config.maxSize = 1024 * 1024;  // 1MB
        config.maxEntries = 1000;
        config.defaultTTL = 3600s;
        config.enableLRU = true;
        config.enableLFU = true;
        config.cleanupInterval = 60;
        ASSERT_TRUE(CacheManager::getInstance().initialize(config));
    }

    void TearDown() override {
        CacheManager::getInstance().shutdown();
    }

    // Helper function to create a test value
    std::string createTestValue(size_t size) {
        std::string value;
        value.reserve(size);
        for (size_t i = 0; i < size; ++i) {
            value += static_cast<char>('a' + (i % 26));
        }
        return value;
    }

    // Helper function to create a test JSON value
    nlohmann::json createTestJson() {
        return {
            {"string", "test"},
            {"number", 42},
            {"boolean", true},
            {"array", {1, 2, 3}},
            {"object", {
                {"key", "value"}
            }}
        };
    }
};

// Basic Functionality Tests
TEST_F(CacheManagerComprehensiveTest, Initialization) {
    // Test initialization with valid config
    CacheConfig validConfig;
    validConfig.maxSize = 1024 * 1024;
    validConfig.maxEntries = 1000;
    validConfig.defaultTTL = 3600s;
    validConfig.enableLRU = true;
    validConfig.enableLFU = true;
    validConfig.cleanupInterval = 60;
    EXPECT_TRUE(CacheManager::getInstance().initialize(validConfig));

    // Test initialization with invalid config
    CacheConfig invalidConfig;
    invalidConfig.maxSize = 0;
    invalidConfig.maxEntries = 0;
    EXPECT_FALSE(CacheManager::getInstance().initialize(invalidConfig));
}

// Basic Cache Operations Tests
TEST_F(CacheManagerComprehensiveTest, BasicOperations) {
    // Test set and get
    EXPECT_TRUE(CacheManager::getInstance().set("key1", "value1"));
    auto value = CacheManager::getInstance().get("key1");
    EXPECT_TRUE(value.has_value());
    EXPECT_EQ(*value, "value1");

    // Test set and get with JSON
    auto jsonValue = createTestJson();
    EXPECT_TRUE(CacheManager::getInstance().set("key2", jsonValue));
    auto retrievedJson = CacheManager::getInstance().getJson("key2");
    EXPECT_TRUE(retrievedJson.has_value());
    EXPECT_EQ(*retrievedJson, jsonValue);

    // Test remove
    EXPECT_TRUE(CacheManager::getInstance().remove("key1"));
    EXPECT_FALSE(CacheManager::getInstance().get("key1").has_value());

    // Test exists
    EXPECT_TRUE(CacheManager::getInstance().exists("key2"));
    EXPECT_FALSE(CacheManager::getInstance().exists("key1"));

    // Test clear
    CacheManager::getInstance().clear();
    EXPECT_FALSE(CacheManager::getInstance().exists("key2"));
}

// Bulk Operations Tests
TEST_F(CacheManagerComprehensiveTest, BulkOperations) {
    // Test setMulti
    std::unordered_map<std::string, std::string> entries = {
        {"key1", "value1"},
        {"key2", "value2"},
        {"key3", "value3"}
    };
    EXPECT_TRUE(CacheManager::getInstance().setMulti(entries));

    // Test getMulti
    std::vector<std::string> keys = {"key1", "key2", "key3", "key4"};
    auto results = CacheManager::getInstance().getMulti(keys);
    EXPECT_EQ(results.size(), 3);
    EXPECT_EQ(results["key1"], "value1");
    EXPECT_EQ(results["key2"], "value2");
    EXPECT_EQ(results["key3"], "value3");

    // Test removeMulti
    std::vector<std::string> keysToRemove = {"key1", "key2"};
    EXPECT_TRUE(CacheManager::getInstance().removeMulti(keysToRemove));
    EXPECT_FALSE(CacheManager::getInstance().exists("key1"));
    EXPECT_FALSE(CacheManager::getInstance().exists("key2"));
    EXPECT_TRUE(CacheManager::getInstance().exists("key3"));
}

// TTL and Expiry Tests
TEST_F(CacheManagerComprehensiveTest, TTLAndExpiry) {
    // Test set with TTL
    EXPECT_TRUE(CacheManager::getInstance().set("key1", "value1", 1s));
    EXPECT_TRUE(CacheManager::getInstance().exists("key1"));
    std::this_thread::sleep_for(1100ms);
    EXPECT_FALSE(CacheManager::getInstance().exists("key1"));

    // Test setTTL
    EXPECT_TRUE(CacheManager::getInstance().set("key2", "value2"));
    EXPECT_TRUE(CacheManager::getInstance().setTTL("key2", 1s));
    std::this_thread::sleep_for(1100ms);
    EXPECT_FALSE(CacheManager::getInstance().exists("key2"));

    // Test getExpiry
    EXPECT_TRUE(CacheManager::getInstance().set("key3", "value3", 3600s));
    auto expiry = CacheManager::getInstance().getExpiry("key3");
    EXPECT_GT(expiry, std::chrono::system_clock::now());
}

// Size and Entry Count Tests
TEST_F(CacheManagerComprehensiveTest, SizeAndEntryCount) {
    // Test getSize
    std::string value = createTestValue(100);
    EXPECT_TRUE(CacheManager::getInstance().set("key1", value));
    EXPECT_GE(CacheManager::getInstance().getSize("key1"), 100);

    // Test getTotalSize
    EXPECT_GE(CacheManager::getInstance().getTotalSize(), 100);

    // Test getEntryCount
    EXPECT_EQ(CacheManager::getInstance().getEntryCount(), 1);
}

// Statistics Tests
TEST_F(CacheManagerComprehensiveTest, Statistics) {
    // Test hit and miss statistics
    EXPECT_TRUE(CacheManager::getInstance().set("key1", "value1"));
    EXPECT_TRUE(CacheManager::getInstance().get("key1").has_value());
    EXPECT_FALSE(CacheManager::getInstance().get("key2").has_value());

    auto stats = CacheManager::getInstance().getStats();
    EXPECT_GE(stats.hitCount, 1);
    EXPECT_GE(stats.missCount, 1);
    EXPECT_GT(stats.hitRate, 0.0);
    EXPECT_LE(stats.hitRate, 1.0);
}

// Callback Tests
TEST_F(CacheManagerComprehensiveTest, Callbacks) {
    bool callbackCalled = false;
    CacheCallback callback = [&callbackCalled](const std::string& key, const std::string& value) {
        callbackCalled = true;
    };

    // Test registerCallback
    CacheManager::getInstance().registerCallback(callback);

    // Test callback invocation
    EXPECT_TRUE(CacheManager::getInstance().set("key1", "value1"));
    EXPECT_TRUE(callbackCalled);

    // Test unregisterCallback
    callbackCalled = false;
    CacheManager::getInstance().unregisterCallback(callback);
    EXPECT_TRUE(CacheManager::getInstance().set("key2", "value2"));
    EXPECT_FALSE(callbackCalled);
}

// Error Handling Tests
TEST_F(CacheManagerComprehensiveTest, ErrorHandling) {
    // Test invalid key
    EXPECT_FALSE(CacheManager::getInstance().set("", "value1"));
    EXPECT_FALSE(CacheManager::getInstance().getLastError().empty());

    // Test invalid value
    EXPECT_FALSE(CacheManager::getInstance().set("key1", ""));
    EXPECT_FALSE(CacheManager::getInstance().getLastError().empty());

    // Test clearLastError
    CacheManager::getInstance().clearLastError();
    EXPECT_TRUE(CacheManager::getInstance().getLastError().empty());
}

// Concurrency Tests
TEST_F(CacheManagerComprehensiveTest, Concurrency) {
    const int numThreads = 10;
    std::vector<std::future<void>> futures;

    // Test concurrent set operations
    for (int i = 0; i < numThreads; ++i) {
        futures.push_back(std::async(std::launch::async, [i]() {
            EXPECT_TRUE(CacheManager::getInstance().set(
                "key" + std::to_string(i),
                "value" + std::to_string(i)));
        }));
    }

    // Wait for all threads to complete
    for (auto& future : futures) {
        future.wait();
    }

    // Verify all values were set
    for (int i = 0; i < numThreads; ++i) {
        auto value = CacheManager::getInstance().get("key" + std::to_string(i));
        EXPECT_TRUE(value.has_value());
        EXPECT_EQ(*value, "value" + std::to_string(i));
    }
}

// Edge Cases Tests
TEST_F(CacheManagerComprehensiveTest, EdgeCases) {
    // Test maximum value size
    std::string largeValue = createTestValue(1024 * 1024);  // 1MB
    EXPECT_TRUE(CacheManager::getInstance().set("key1", largeValue));

    // Test maximum number of entries
    for (int i = 0; i < 1000; ++i) {
        EXPECT_TRUE(CacheManager::getInstance().set(
            "key" + std::to_string(i),
            "value" + std::to_string(i)));
    }

    // Test resize
    CacheManager::getInstance().resize(512 * 1024);  // 512KB
    EXPECT_LE(CacheManager::getInstance().getTotalSize(), 512 * 1024);

    // Test setMaxEntries
    CacheManager::getInstance().setMaxEntries(500);
    EXPECT_LE(CacheManager::getInstance().getEntryCount(), 500);
}

// Cleanup Tests
TEST_F(CacheManagerComprehensiveTest, Cleanup) {
    // Test cleanup of expired entries
    EXPECT_TRUE(CacheManager::getInstance().set("key1", "value1", 1s));
    std::this_thread::sleep_for(1100ms);
    CacheManager::getInstance().cleanup();
    EXPECT_FALSE(CacheManager::getInstance().exists("key1"));
}

// Stress Tests
TEST_F(CacheManagerComprehensiveTest, StressTest) {
    const int numOperations = 1000;
    std::vector<std::string> keys;
    keys.reserve(numOperations);

    // Generate random keys
    for (int i = 0; i < numOperations; ++i) {
        keys.push_back("key" + std::to_string(i));
    }

    // Perform random operations
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> opDist(0, 2);  // 0: set, 1: get, 2: remove

    for (int i = 0; i < numOperations; ++i) {
        int op = opDist(gen);
        const std::string& key = keys[i % keys.size()];

        switch (op) {
            case 0:  // set
                EXPECT_TRUE(CacheManager::getInstance().set(key, "value" + std::to_string(i)));
                break;
            case 1:  // get
                CacheManager::getInstance().get(key);
                break;
            case 2:  // remove
                CacheManager::getInstance().remove(key);
                break;
        }
    }
} 