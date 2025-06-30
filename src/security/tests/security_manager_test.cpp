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
#include "satox/security/security_manager.hpp"
#include <filesystem>
#include <iostream>
#include <spdlog/spdlog.h>

namespace satox {
namespace security {
namespace tests {

class SecurityManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test log directory
        std::filesystem::create_directories("logs/components/security/");
    }

    void TearDown() override {
        // Cleanup test log directory
        std::filesystem::remove_all("logs/components/security/");
    }
    
    SecurityConfig createTestConfig() {
        SecurityConfig config;
        config.enableLogging = true;
        config.logPath = "logs/components/security/";
        config.enableInputValidation = true;
        config.enableRateLimiting = true;
        return config;
    }
};

TEST_F(SecurityManagerTest, Initialization) {
    // Create a new instance for this test
    std::unique_ptr<SecurityManager> manager = std::make_unique<SecurityManager>();
    SecurityConfig config = createTestConfig();
    EXPECT_TRUE(manager->initialize(config));
}

TEST_F(SecurityManagerTest, RateLimiting) {
    // Create a new instance for this test
    std::unique_ptr<SecurityManager> manager = std::make_unique<SecurityManager>();
    SecurityConfig config = createTestConfig();
    bool result = manager->initialize(config);
    if (!result) {
        std::cout << "Initialization failed: " << manager->getLastError() << std::endl;
    }
    EXPECT_TRUE(result);
    
    std::string key = "test_key";
    std::string operation = "op";
    EXPECT_TRUE(manager->checkRateLimit(key, operation));
    EXPECT_TRUE(manager->setRateLimit(key, 10, 60)); // Set rate limit
    EXPECT_TRUE(manager->checkRateLimit(key, operation)); // Should still be allowed
}

TEST_F(SecurityManagerTest, InputValidation) {
    // Create a new instance for this test
    std::unique_ptr<SecurityManager> manager = std::make_unique<SecurityManager>();
    SecurityConfig config = createTestConfig();
    EXPECT_TRUE(manager->initialize(config));
    
    std::string validJson = "{\"test\": \"value\"}";
    EXPECT_TRUE(manager->validateJson(validJson));
    
    std::string invalidJson = "{invalid json}";
    EXPECT_FALSE(manager->validateJson(invalidJson));
}

TEST_F(SecurityManagerTest, TokenValidation) {
    // Create a new instance for this test
    std::unique_ptr<SecurityManager> manager = std::make_unique<SecurityManager>();
    SecurityConfig config = createTestConfig();
    EXPECT_TRUE(manager->initialize(config));
    
    // Test with empty token (should fail)
    EXPECT_FALSE(manager->validateToken(""));
    
    // Test with a short token (should fail based on stub implementation)
    EXPECT_FALSE(manager->validateToken("short"));
    
    // Test with a valid token (should pass based on stub implementation)
    EXPECT_TRUE(manager->validateToken("valid_token_123"));
}

TEST_F(SecurityManagerTest, PerformanceConfig) {
    // Create a new instance for this test
    std::unique_ptr<SecurityManager> manager = std::make_unique<SecurityManager>();
    SecurityConfig config = createTestConfig();
    EXPECT_TRUE(manager->initialize(config));
    
    nlohmann::json perfConfig;
    perfConfig["max_cache_size"] = 2000;
    perfConfig["connection_pool_size"] = 2000;
    perfConfig["batch_size"] = 200;
    perfConfig["enable_async_processing"] = true;
    
    manager->setPerformanceConfig(perfConfig);
    
    EXPECT_EQ(manager->getCacheSize(), 0); // Should be 0 initially
    EXPECT_EQ(manager->getConnectionPoolSize(), 2000); // Should reflect new config
    EXPECT_EQ(manager->getBatchSize(), 200); // Should reflect new config from performance config
}

} // namespace tests
} // namespace security
} // namespace satox 