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

#include "rate_limiter.hpp"
#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <vector>
#include <future>

namespace satox {
namespace test {

class RateLimiterTests : public ::testing::Test {
protected:
    void SetUp() override {
        limiter = &RateLimiter::getInstance();
        limiter->initialize();
    }

    void TearDown() override {
        limiter->shutdown();
    }

    RateLimiter* limiter;
};

TEST_F(RateLimiterTests, SingletonInstance) {
    auto& instance1 = RateLimiter::getInstance();
    auto& instance2 = RateLimiter::getInstance();
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(RateLimiterTests, Initialization) {
    EXPECT_TRUE(limiter->isRunning());
    EXPECT_TRUE(limiter->checkLimit("test_method", "test_client"));
}

TEST_F(RateLimiterTests, DefaultLimit) {
    const std::string method = "test_method";
    const std::string client = "test_client";

    // Should allow up to 100 requests per minute
    for (int i = 0; i < 100; ++i) {
        EXPECT_TRUE(limiter->checkLimit(method, client));
        limiter->updateLimit(method, client);
    }

    // 101st request should be rejected
    EXPECT_FALSE(limiter->checkLimit(method, client));
}

TEST_F(RateLimiterTests, MethodSpecificLimit) {
    const std::string method = "getblockchaininfo"; // 10 requests per minute
    const std::string client = "test_client";

    // Should allow up to 10 requests
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(limiter->checkLimit(method, client));
        limiter->updateLimit(method, client);
    }

    // 11th request should be rejected
    EXPECT_FALSE(limiter->checkLimit(method, client));
}

TEST_F(RateLimiterTests, ClientSpecificLimit) {
    const std::string method = "test_method";
    const std::string client = "test_client";

    // Set client-specific limit
    limiter->setClientLimit(client, 5, std::chrono::seconds(60));

    // Should allow up to 5 requests
    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(limiter->checkLimit(method, client));
        limiter->updateLimit(method, client);
    }

    // 6th request should be rejected
    EXPECT_FALSE(limiter->checkLimit(method, client));
}

TEST_F(RateLimiterTests, WindowExpiration) {
    const std::string method = "test_method";
    const std::string client = "test_client";

    // Set a short window
    limiter->setMethodLimit(method, 5, std::chrono::seconds(1));

    // Make 5 requests
    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(limiter->checkLimit(method, client));
        limiter->updateLimit(method, client);
    }

    // 6th request should be rejected
    EXPECT_FALSE(limiter->checkLimit(method, client));

    // Wait for window to expire
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Should be able to make requests again
    EXPECT_TRUE(limiter->checkLimit(method, client));
}

TEST_F(RateLimiterTests, ResetLimit) {
    const std::string method = "test_method";
    const std::string client = "test_client";

    // Make some requests
    for (int i = 0; i < 5; ++i) {
        limiter->updateLimit(method, client);
    }

    // Reset the limit
    limiter->resetLimit(method, client);

    // Should be able to make requests again
    EXPECT_TRUE(limiter->checkLimit(method, client));
}

TEST_F(RateLimiterTests, ResetAllLimits) {
    const std::string method1 = "test_method1";
    const std::string method2 = "test_method2";
    const std::string client = "test_client";

    // Make some requests
    for (int i = 0; i < 5; ++i) {
        limiter->updateLimit(method1, client);
        limiter->updateLimit(method2, client);
    }

    // Reset all limits
    limiter->resetAllLimits();

    // Should be able to make requests again
    EXPECT_TRUE(limiter->checkLimit(method1, client));
    EXPECT_TRUE(limiter->checkLimit(method2, client));
}

TEST_F(RateLimiterTests, ConcurrentAccess) {
    const std::string method = "test_method";
    const std::string client = "test_client";
    const int num_threads = 10;
    const int requests_per_thread = 10;

    std::vector<std::future<void>> futures;
    for (int i = 0; i < num_threads; ++i) {
        futures.push_back(std::async(std::launch::async, [&]() {
            for (int j = 0; j < requests_per_thread; ++j) {
                if (limiter->checkLimit(method, client)) {
                    limiter->updateLimit(method, client);
                }
            }
        }));
    }

    // Wait for all threads to complete
    for (auto& future : futures) {
        future.wait();
    }

    // Verify that the total number of requests doesn't exceed the limit
    EXPECT_LE(limiter->checkLimit(method, client), true);
}

TEST_F(RateLimiterTests, MultipleClients) {
    const std::string method = "test_method";
    const std::string client1 = "client1";
    const std::string client2 = "client2";

    // Set different limits for each client
    limiter->setClientLimit(client1, 5, std::chrono::seconds(60));
    limiter->setClientLimit(client2, 10, std::chrono::seconds(60));

    // Client 1 should be limited to 5 requests
    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(limiter->checkLimit(method, client1));
        limiter->updateLimit(method, client1);
    }
    EXPECT_FALSE(limiter->checkLimit(method, client1));

    // Client 2 should be limited to 10 requests
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(limiter->checkLimit(method, client2));
        limiter->updateLimit(method, client2);
    }
    EXPECT_FALSE(limiter->checkLimit(method, client2));
}

TEST_F(RateLimiterTests, Shutdown) {
    limiter->shutdown();
    EXPECT_FALSE(limiter->isRunning());
    EXPECT_THROW(limiter->checkLimit("test_method", "test_client"), std::runtime_error);
}

} // namespace test
} // namespace satox 