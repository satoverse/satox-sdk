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

#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <chrono>
#include <functional>
#include <unordered_map>
#include <vector>
#include <queue>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <nlohmann/json.hpp>

namespace satox::core {

class EventManager {
public:
    // Event types
    enum class EventType {
        SYSTEM,
        NETWORK,
        BLOCKCHAIN,
        WALLET,
        TRANSACTION,
        ASSET,
        NFT,
        IPFS,
        DATABASE,
        CACHE,
        CONFIG,
        SECURITY,
        CUSTOM
    };

    // Event priority levels
    enum class Priority {
        LOW,
        NORMAL,
        HIGH,
        CRITICAL
    };

    // Event structure
    struct Event {
        EventType type;
        std::string name;
        std::string source;
        Priority priority;
        std::chrono::system_clock::time_point timestamp;
        nlohmann::json data;
        std::string correlationId;
        std::string traceId;
    };

    // Event handler type
    using EventHandler = std::function<void(const Event&)>;

    // Event filter type
    using EventFilter = std::function<bool(const Event&)>;

    // Subscription token type
    using SubscriptionToken = size_t;

    // Event subscription structure
    struct Subscription {
        SubscriptionToken token;
        EventHandler handler;
        EventFilter filter;
        bool async;
        std::chrono::milliseconds timeout;
    };

    // Event statistics structure
    struct EventStats {
        size_t totalEvents;
        size_t processedEvents;
        size_t failedEvents;
        size_t queuedEvents;
        std::chrono::milliseconds averageProcessingTime;
        std::chrono::milliseconds maxProcessingTime;
        std::chrono::milliseconds minProcessingTime;
    };

    // Singleton instance
    static EventManager& getInstance();

    // Prevent copying
    EventManager(const EventManager&) = delete;
    EventManager& operator=(const EventManager&) = delete;

    // Initialization and shutdown
    bool initialize(size_t maxQueueSize = 1000, size_t numWorkers = 4);
    void shutdown();

    // Event publishing
    bool publishEvent(const Event& event);
    bool publishEvent(EventType type, const std::string& name,
                     const nlohmann::json& data = nlohmann::json::object(),
                     Priority priority = Priority::NORMAL);
    bool publishEventAsync(const Event& event);
    bool publishEventAsync(EventType type, const std::string& name,
                          const nlohmann::json& data = nlohmann::json::object(),
                          Priority priority = Priority::NORMAL);

    // Event subscription (token-based)
    SubscriptionToken subscribe(EventType type, EventHandler handler,
                               bool async = false,
                               std::chrono::milliseconds timeout = std::chrono::milliseconds(0));
    SubscriptionToken subscribe(EventType type, const std::string& name, EventHandler handler,
                               bool async = false,
                               std::chrono::milliseconds timeout = std::chrono::milliseconds(0));
    SubscriptionToken subscribe(EventFilter filter, EventHandler handler,
                               bool async = false,
                               std::chrono::milliseconds timeout = std::chrono::milliseconds(0));
    
    // Unsubscribe by token
    bool unsubscribe(SubscriptionToken token);
    
    // Legacy unsubscribe methods (deprecated - will be removed)
    void unsubscribe(EventType type, EventHandler handler);
    void unsubscribe(EventType type, const std::string& name, EventHandler handler);
    void unsubscribe(EventFilter filter, EventHandler handler);

    // Event handling
    bool processEvent(const Event& event);
    bool processEventAsync(const Event& event);
    void processEvents();
    void waitForEvents(std::chrono::milliseconds timeout = std::chrono::milliseconds(0));

    // Event querying
    std::vector<Event> getEvents(EventType type,
                               std::chrono::system_clock::time_point start,
                               std::chrono::system_clock::time_point end);
    std::vector<Event> getEvents(const std::string& name,
                               std::chrono::system_clock::time_point start,
                               std::chrono::system_clock::time_point end);
    std::vector<Event> getEvents(EventFilter filter,
                               std::chrono::system_clock::time_point start,
                               std::chrono::system_clock::time_point end);

    // Event statistics
    EventStats getStats() const;
    void resetStats();
    void enableStats(bool enable);

    // Event filtering
    bool addFilter(EventType type, EventFilter filter);
    bool addFilter(const std::string& name, EventFilter filter);
    void removeFilter(EventType type, EventFilter filter);
    void removeFilter(const std::string& name, EventFilter filter);

    // Error handling
    std::string getLastError() const;
    void clearLastError();

private:
    EventManager() = default;
    ~EventManager() = default;

    // Helper methods
    void workerThread();
    bool validateEvent(const Event& event) const;
    void updateStats(const Event& event, std::chrono::milliseconds processingTime);
    void handleEvent(const Event& event, const Subscription& subscription);
    void handleEventAsync(const Event& event, const Subscription& subscription);
    bool matchEvent(const Event& event, const Subscription& subscription) const;
    void cleanupSubscriptions();
    void cleanupFilters();
    SubscriptionToken generateToken();

    // Member variables
    bool initialized_ = false;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    std::queue<Event> eventQueue_;
    size_t maxQueueSize_ = 1000;
    std::vector<std::thread> workers_;
    std::atomic<bool> running_ = false;
    std::unordered_map<EventType, std::vector<Subscription>> typeSubscriptions_;
    std::unordered_map<std::string, std::vector<Subscription>> nameSubscriptions_;
    std::vector<Subscription> filterSubscriptions_;
    std::unordered_map<EventType, std::vector<EventFilter>> typeFilters_;
    std::unordered_map<std::string, std::vector<EventFilter>> nameFilters_;
    EventStats stats_;
    bool statsEnabled_ = false;
    std::string lastError_;
    std::atomic<SubscriptionToken> nextToken_ = 1;
};

} // namespace satox::core 