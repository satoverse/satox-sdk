#include <gtest/gtest.h>
#include "satox/security/rate_limiter.hpp"
#include <thread>
#include <chrono>

using namespace satox::security;

class RateLimiterTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_TRUE(limiter.initialize());
    }

    void TearDown() override {
        limiter.shutdown();
    }

    RateLimiter limiter;
};

TEST_F(RateLimiterTest, Initialization) {
    EXPECT_TRUE(limiter.isInitialized());
}

TEST_F(RateLimiterTest, BasicLimit) {
    const std::string key = "test_key";
    const int maxRequests = 5;
    const int timeWindow = 1; // 1 second

    // Should allow maxRequests requests
    for (int i = 0; i < maxRequests; ++i) {
        EXPECT_TRUE(limiter.checkLimit(key, maxRequests, timeWindow));
    }

    // Should reject the next request
    EXPECT_FALSE(limiter.checkLimit(key, maxRequests, timeWindow));
}

TEST_F(RateLimiterTest, TimeWindowExpiration) {
    const std::string key = "test_key";
    const int maxRequests = 3;
    const int timeWindow = 1; // 1 second

    // Use up all requests
    for (int i = 0; i < maxRequests; ++i) {
        EXPECT_TRUE(limiter.checkLimit(key, maxRequests, timeWindow));
    }

    // Wait for time window to expire
    std::this_thread::sleep_for(std::chrono::seconds(timeWindow + 1));

    // Should allow requests again
    EXPECT_TRUE(limiter.checkLimit(key, maxRequests, timeWindow));
}

TEST_F(RateLimiterTest, ResetLimit) {
    const std::string key = "test_key";
    const int maxRequests = 3;
    const int timeWindow = 1;

    // Use up all requests
    for (int i = 0; i < maxRequests; ++i) {
        EXPECT_TRUE(limiter.checkLimit(key, maxRequests, timeWindow));
    }

    // Reset the limit
    limiter.resetLimit(key);

    // Should allow requests again
    EXPECT_TRUE(limiter.checkLimit(key, maxRequests, timeWindow));
}

TEST_F(RateLimiterTest, RequestCount) {
    const std::string key = "test_key";
    const int maxRequests = 5;
    const int timeWindow = 1;

    EXPECT_EQ(limiter.getRequestCount(key), 0);

    for (int i = 0; i < 3; ++i) {
        EXPECT_TRUE(limiter.checkLimit(key, maxRequests, timeWindow));
        EXPECT_EQ(limiter.getRequestCount(key), i + 1);
    }
}

TEST_F(RateLimiterTest, WindowStart) {
    const std::string key = "test_key";
    const int maxRequests = 3;
    const int timeWindow = 1;

    auto before = std::chrono::system_clock::now();
    EXPECT_TRUE(limiter.checkLimit(key, maxRequests, timeWindow));
    auto after = std::chrono::system_clock::now();

    auto windowStart = limiter.getWindowStart(key);
    EXPECT_GE(windowStart, before);
    EXPECT_LE(windowStart, after);
}

TEST_F(RateLimiterTest, TimeWindow) {
    const std::string key = "test_key";
    const int maxRequests = 3;
    const int timeWindow = 5;

    EXPECT_TRUE(limiter.checkLimit(key, maxRequests, timeWindow));
    EXPECT_EQ(limiter.getTimeWindow(key), timeWindow);
}

TEST_F(RateLimiterTest, MaxRequests) {
    const std::string key = "test_key";
    const int maxRequests = 10;
    const int timeWindow = 1;

    EXPECT_TRUE(limiter.checkLimit(key, maxRequests, timeWindow));
    EXPECT_EQ(limiter.getMaxRequests(key), maxRequests);
}

TEST_F(RateLimiterTest, MultipleKeys) {
    const std::string key1 = "key1";
    const std::string key2 = "key2";
    const int maxRequests = 3;
    const int timeWindow = 1;

    // Use up all requests for key1
    for (int i = 0; i < maxRequests; ++i) {
        EXPECT_TRUE(limiter.checkLimit(key1, maxRequests, timeWindow));
    }
    EXPECT_FALSE(limiter.checkLimit(key1, maxRequests, timeWindow));

    // key2 should still have all requests available
    EXPECT_TRUE(limiter.checkLimit(key2, maxRequests, timeWindow));
}

TEST_F(RateLimiterTest, ShutdownReinitialize) {
    limiter.shutdown();
    EXPECT_FALSE(limiter.isInitialized());

    EXPECT_TRUE(limiter.initialize());
    EXPECT_TRUE(limiter.isInitialized());

    const std::string key = "test_key";
    EXPECT_TRUE(limiter.checkLimit(key, 1, 1));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 