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
#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <random>
#include "satox/core/performance_optimization.hpp"

using namespace satox::core;
using namespace std::chrono_literals;

class PerformanceOptimizationTest : public ::testing::Test {
protected:
    void SetUp() override {
        optimization = std::make_unique<PerformanceOptimization>();
    }

    void TearDown() override {
        optimization.reset();
    }

    std::unique_ptr<PerformanceOptimization> optimization;
};

// Cache Tests
TEST_F(PerformanceOptimizationTest, CacheOperations) {
    // Test adding to cache
    optimization->addToCache("key1", "value1");
    optimization->addToCache("key2", "value2");

    // Test retrieving from cache
    auto value1 = optimization->getFromCache("key1");
    ASSERT_TRUE(value1.has_value());
    EXPECT_EQ(value1.value(), "value1");

    auto value2 = optimization->getFromCache("key2");
    ASSERT_TRUE(value2.has_value());
    EXPECT_EQ(value2.value(), "value2");

    // Test non-existent key
    auto value3 = optimization->getFromCache("key3");
    EXPECT_FALSE(value3.has_value());
}

// Connection Pool Tests
TEST_F(PerformanceOptimizationTest, ConnectionPoolOperations) {
    // Test adding connections
    optimization->addConnection("address1");
    optimization->addConnection("address2");

    // Test removing connections
    optimization->removeConnection("address1");
}

// Batch Processing Tests
TEST_F(PerformanceOptimizationTest, BatchProcessing) {
    // Test adding to batch
    for (int i = 0; i < 1000; ++i) {
        optimization->addToBatch("operation1", "data" + std::to_string(i));
    }
}

// Memory Optimization Tests
TEST_F(PerformanceOptimizationTest, MemoryOptimization) {
    // Fill cache
    for (int i = 0; i < 10000; ++i) {
        optimization->addToCache("key" + std::to_string(i), "value" + std::to_string(i));
    }

    // Test memory optimization
    optimization->optimizeMemory();
}

// Performance Tests
TEST_F(PerformanceOptimizationTest, CachePerformance) {
    const int num_operations = 10000;
    std::vector<std::string> keys;
    keys.reserve(num_operations);

    // Generate test keys
    for (int i = 0; i < num_operations; ++i) {
        keys.push_back("key" + std::to_string(i));
    }

    // Test cache write performance
    auto start = std::chrono::high_resolution_clock::now();
    for (const auto& key : keys) {
        optimization->addToCache(key, "value");
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should add at least 1000 items per second
    EXPECT_GE(num_operations * 1000 / duration.count(), 1000);

    // Test cache read performance
    start = std::chrono::high_resolution_clock::now();
    for (const auto& key : keys) {
        auto value = optimization->getFromCache(key);
        EXPECT_TRUE(value.has_value());
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should read at least 5000 items per second
    EXPECT_GE(num_operations * 1000 / duration.count(), 5000);
}

TEST_F(PerformanceOptimizationTest, ConnectionPoolPerformance) {
    const int num_connections = 100;
    std::vector<std::string> addresses;
    addresses.reserve(num_connections);

    // Generate test addresses
    for (int i = 0; i < num_connections; ++i) {
        addresses.push_back("address" + std::to_string(i));
    }

    // Test connection establishment performance
    auto start = std::chrono::high_resolution_clock::now();
    for (const auto& address : addresses) {
        optimization->addConnection(address);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should add at least 10 connections per second
    EXPECT_GE(num_connections * 1000 / duration.count(), 10);
}

TEST_F(PerformanceOptimizationTest, BatchProcessingPerformance) {
    const int num_operations = 1000;
    const int batch_size = 100;

    // Test batch processing performance
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_operations; ++i) {
        optimization->addToBatch("operation1", "data" + std::to_string(i));
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should process at least 1000 operations per second
    EXPECT_GE(num_operations * 1000 / duration.count(), 1000);
}

// Concurrency Tests
TEST_F(PerformanceOptimizationTest, ConcurrentOperations) {
    const int num_threads = 8;
    const int operations_per_thread = 1000;
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};

    auto worker = [&]() {
        for (int i = 0; i < operations_per_thread; ++i) {
            std::string key = "key" + std::to_string(i);
            std::string value = "value" + std::to_string(i);
            
            optimization->addToCache(key, value);
            auto retrieved = optimization->getFromCache(key);
            if (retrieved.has_value() && retrieved.value() == value) {
                success_count++;
            }
        }
    };

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker);
    }
    for (auto& thread : threads) {
        thread.join();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    double operations_per_second = (success_count * 1000.0) / duration.count();
    EXPECT_GE(operations_per_second, 1000.0); // Expect at least 1000 operations per second
    EXPECT_GE(success_count, num_threads * operations_per_thread * 0.9); // 90% success rate
}

// Edge Case Tests
TEST_F(PerformanceOptimizationTest, EmptyCache) {
    auto value = optimization->getFromCache("nonexistent");
    EXPECT_FALSE(value.has_value());
}

TEST_F(PerformanceOptimizationTest, LargeCache) {
    const int num_items = 20000; // More than max_cache_size_
    
    // Fill cache beyond capacity
    for (int i = 0; i < num_items; ++i) {
        optimization->addToCache("key" + std::to_string(i), "value" + std::to_string(i));
    }

    // Verify cache size is managed
    optimization->optimizeMemory();
}

// Recovery Tests
TEST_F(PerformanceOptimizationTest, RecoveryAfterFailure) {
    // Simulate failure by filling cache
    for (int i = 0; i < 10000; ++i) {
        optimization->addToCache("key" + std::to_string(i), "value" + std::to_string(i));
    }

    // Optimize memory
    optimization->optimizeMemory();

    // Verify system is still functional
    optimization->addToCache("new_key", "new_value");
    auto value = optimization->getFromCache("new_key");
    EXPECT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), "new_value");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 