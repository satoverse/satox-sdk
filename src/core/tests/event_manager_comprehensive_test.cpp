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
#include "satox/core/event_manager.hpp"
#include <thread>
#include <chrono>
#include <future>
#include <random>
#include <algorithm>

using namespace satox::core;
using namespace testing;

class EventManagerComprehensiveTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize event manager
        ASSERT_TRUE(EventManager::getInstance().initialize(1000, 4));
    }

    void TearDown() override {
        // Shutdown event manager
        EventManager::getInstance().shutdown();
    }

    // Helper function to create a test event
    Event createTestEvent(EventType type, const std::string& name,
                        const nlohmann::json& data = nlohmann::json::object(),
                        Priority priority = Priority::NORMAL) {
        Event event;
        event.type = type;
        event.name = name;
        event.source = "test";
        event.priority = priority;
        event.timestamp = std::chrono::system_clock::now();
        event.data = data;
        return event;
    }

    // Helper function to wait for events to be processed
    void waitForEvents(size_t expectedCount, std::chrono::milliseconds timeout) {
        auto start = std::chrono::steady_clock::now();
        while (EventManager::getInstance().getStats().processedEvents < expectedCount) {
            if (std::chrono::steady_clock::now() - start > timeout) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
};

// Test initialization
TEST_F(EventManagerComprehensiveTest, Initialization) {
    auto& manager = EventManager::getInstance();

    // Test valid initialization
    EXPECT_TRUE(manager.initialize(1000, 4));

    // Test invalid queue size
    EXPECT_FALSE(manager.initialize(0, 4));

    // Test invalid worker count
    EXPECT_FALSE(manager.initialize(1000, 0));

    // Test double initialization
    EXPECT_FALSE(manager.initialize(1000, 4));
}

// Test event publishing
TEST_F(EventManagerComprehensiveTest, EventPublishing) {
    auto& manager = EventManager::getInstance();
    std::atomic<size_t> eventCount{0};

    // Subscribe to events
    manager.subscribe(EventType::SYSTEM, [&](const Event& event) {
        eventCount++;
    });

    // Test synchronous publishing
    Event event = createTestEvent(EventType::SYSTEM, "test_event");
    EXPECT_TRUE(manager.publishEvent(event));
    waitForEvents(1, std::chrono::milliseconds(100));
    EXPECT_EQ(eventCount, 1);

    // Test asynchronous publishing
    EXPECT_TRUE(manager.publishEventAsync(EventType::SYSTEM, "async_event"));
    waitForEvents(2, std::chrono::milliseconds(100));
    EXPECT_EQ(eventCount, 2);

    // Test invalid event
    Event invalidEvent;
    EXPECT_FALSE(manager.publishEvent(invalidEvent));
}

// Test event subscription
TEST_F(EventManagerComprehensiveTest, EventSubscription) {
    auto& manager = EventManager::getInstance();
    std::atomic<size_t> typeCount{0};
    std::atomic<size_t> nameCount{0};
    std::atomic<size_t> filterCount{0};

    // Test type subscription
    manager.subscribe(EventType::SYSTEM, [&](const Event& event) {
        typeCount++;
    });

    // Test name subscription
    manager.subscribe(EventType::SYSTEM, "test_event", [&](const Event& event) {
        nameCount++;
    });

    // Test filter subscription
    manager.subscribe(
        [](const Event& event) { return event.priority == Priority::HIGH; },
        [&](const Event& event) { filterCount++; });

    // Publish events
    manager.publishEvent(EventType::SYSTEM, "test_event");
    manager.publishEvent(EventType::SYSTEM, "other_event");
    manager.publishEvent(EventType::NETWORK, "test_event");
    manager.publishEvent(EventType::SYSTEM, "high_priority_event", {},
                        Priority::HIGH);

    waitForEvents(4, std::chrono::milliseconds(100));

    EXPECT_EQ(typeCount, 3);  // All SYSTEM events
    EXPECT_EQ(nameCount, 1);  // Only "test_event" in SYSTEM type
    EXPECT_EQ(filterCount, 1);  // Only HIGH priority event
}

// Test event handling
TEST_F(EventManagerComprehensiveTest, EventHandling) {
    auto& manager = EventManager::getInstance();
    std::atomic<size_t> syncCount{0};
    std::atomic<size_t> asyncCount{0};

    // Test synchronous handling
    manager.subscribe(EventType::SYSTEM, [&](const Event& event) {
        syncCount++;
    });

    // Test asynchronous handling
    manager.subscribe(EventType::NETWORK, [&](const Event& event) {
        asyncCount++;
    }, true);

    // Publish events
    manager.publishEvent(EventType::SYSTEM, "sync_event");
    manager.publishEvent(EventType::NETWORK, "async_event");

    waitForEvents(2, std::chrono::milliseconds(100));

    EXPECT_EQ(syncCount, 1);
    EXPECT_EQ(asyncCount, 1);
}

// Test event filtering
TEST_F(EventManagerComprehensiveTest, EventFiltering) {
    auto& manager = EventManager::getInstance();
    std::atomic<size_t> filterCount{0};

    // Add type filter
    manager.addFilter(EventType::SYSTEM, [](const Event& event) {
        return event.priority == Priority::HIGH;
    });

    // Add name filter
    manager.addFilter("test_event", [](const Event& event) {
        return event.type == EventType::SYSTEM;
    });

    // Subscribe to filtered events
    manager.subscribe(EventType::SYSTEM, [&](const Event& event) {
        filterCount++;
    });

    // Publish events
    manager.publishEvent(EventType::SYSTEM, "test_event", {}, Priority::HIGH);
    manager.publishEvent(EventType::SYSTEM, "test_event", {}, Priority::LOW);
    manager.publishEvent(EventType::NETWORK, "test_event", {}, Priority::HIGH);

    waitForEvents(3, std::chrono::milliseconds(100));

    EXPECT_EQ(filterCount, 1);  // Only SYSTEM type with HIGH priority
}

// Test event statistics
TEST_F(EventManagerComprehensiveTest, EventStatistics) {
    auto& manager = EventManager::getInstance();
    manager.enableStats(true);
    manager.resetStats();

    // Publish events
    for (int i = 0; i < 10; ++i) {
        manager.publishEvent(EventType::SYSTEM, "test_event_" + std::to_string(i));
    }

    waitForEvents(10, std::chrono::milliseconds(100));

    auto stats = manager.getStats();
    EXPECT_EQ(stats.totalEvents, 10);
    EXPECT_EQ(stats.processedEvents, 10);
    EXPECT_EQ(stats.queuedEvents, 0);
    EXPECT_GT(stats.averageProcessingTime.count(), 0);
    EXPECT_GT(stats.maxProcessingTime.count(), 0);
    EXPECT_GT(stats.minProcessingTime.count(), 0);
}

// Test error handling
TEST_F(EventManagerComprehensiveTest, ErrorHandling) {
    auto& manager = EventManager::getInstance();

    // Test invalid event
    Event invalidEvent;
    EXPECT_FALSE(manager.publishEvent(invalidEvent));
    EXPECT_FALSE(manager.getLastError().empty());

    // Test handler exception
    manager.subscribe(EventType::SYSTEM, [](const Event&) {
        throw std::runtime_error("Test exception");
    });

    manager.publishEvent(EventType::SYSTEM, "error_event");
    waitForEvents(1, std::chrono::milliseconds(100));

    EXPECT_FALSE(manager.getLastError().empty());
    EXPECT_EQ(manager.getStats().failedEvents, 1);

    // Test error clearing
    manager.clearLastError();
    EXPECT_TRUE(manager.getLastError().empty());
}

// Test concurrency
TEST_F(EventManagerComprehensiveTest, Concurrency) {
    auto& manager = EventManager::getInstance();
    std::atomic<size_t> eventCount{0};

    // Subscribe to events
    manager.subscribe(EventType::SYSTEM, [&](const Event&) {
        eventCount++;
    });

    // Publish events from multiple threads
    std::vector<std::future<void>> futures;
    for (int i = 0; i < 10; ++i) {
        futures.push_back(std::async(std::launch::async, [&, i]() {
            for (int j = 0; j < 10; ++j) {
                manager.publishEvent(EventType::SYSTEM,
                                   "event_" + std::to_string(i) + "_" +
                                       std::to_string(j));
            }
        }));
    }

    for (auto& future : futures) {
        future.wait();
    }

    waitForEvents(100, std::chrono::milliseconds(1000));

    EXPECT_EQ(eventCount, 100);
}

// Test edge cases
TEST_F(EventManagerComprehensiveTest, EdgeCases) {
    auto& manager = EventManager::getInstance();
    std::atomic<size_t> eventCount{0};

    // Subscribe to events
    manager.subscribe(EventType::SYSTEM, [&](const Event&) {
        eventCount++;
    });

    // Test empty event name
    Event emptyNameEvent = createTestEvent(EventType::SYSTEM, "");
    EXPECT_FALSE(manager.publishEvent(emptyNameEvent));

    // Test empty event source
    Event emptySourceEvent = createTestEvent(EventType::SYSTEM, "test_event");
    emptySourceEvent.source = "";
    EXPECT_FALSE(manager.publishEvent(emptySourceEvent));

    // Test large event data
    nlohmann::json largeData;
    for (int i = 0; i < 1000; ++i) {
        largeData["key_" + std::to_string(i)] = "value_" + std::to_string(i);
    }
    Event largeDataEvent = createTestEvent(EventType::SYSTEM, "large_data_event",
                                         largeData);
    EXPECT_TRUE(manager.publishEvent(largeDataEvent));

    waitForEvents(1, std::chrono::milliseconds(100));
    EXPECT_EQ(eventCount, 1);
}

// Test cleanup
TEST_F(EventManagerComprehensiveTest, Cleanup) {
    auto& manager = EventManager::getInstance();
    std::atomic<size_t> eventCount{0};

    // Subscribe to events
    manager.subscribe(EventType::SYSTEM, [&](const Event&) {
        eventCount++;
    });

    // Publish events
    for (int i = 0; i < 10; ++i) {
        manager.publishEvent(EventType::SYSTEM, "test_event_" + std::to_string(i));
    }

    // Shutdown manager
    manager.shutdown();

    // Verify cleanup
    EXPECT_FALSE(manager.publishEvent(EventType::SYSTEM, "test_event"));
    EXPECT_EQ(manager.getStats().queuedEvents, 0);
}

// Test stress
TEST_F(EventManagerComprehensiveTest, Stress) {
    auto& manager = EventManager::getInstance();
    std::atomic<size_t> eventCount{0};

    // Subscribe to events
    manager.subscribe(EventType::SYSTEM, [&](const Event&) {
        eventCount++;
    });

    // Publish events rapidly
    std::vector<std::future<void>> futures;
    for (int i = 0; i < 100; ++i) {
        futures.push_back(std::async(std::launch::async, [&, i]() {
            for (int j = 0; j < 100; ++j) {
                manager.publishEvent(EventType::SYSTEM,
                                   "event_" + std::to_string(i) + "_" +
                                       std::to_string(j));
            }
        }));
    }

    for (auto& future : futures) {
        future.wait();
    }

    waitForEvents(10000, std::chrono::milliseconds(5000));

    EXPECT_EQ(eventCount, 10000);
}

} // namespace satox::core 