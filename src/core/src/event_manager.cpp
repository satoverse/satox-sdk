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

#include <future>
#include "satox/core/event_manager.hpp"
#include <algorithm>
#include <chrono>
#include <thread>
#include <future>
#include <iostream>
#include <spdlog/spdlog.h>

namespace satox::core {

EventManager& EventManager::getInstance() {
    static EventManager instance;
    return instance;
}

bool EventManager::initialize(size_t maxQueueSize, size_t numWorkers) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        lastError_ = "EventManager already initialized";
        return false;
    }
    
    maxQueueSize_ = maxQueueSize;
    running_ = true;
    
    // Start worker threads
    for (size_t i = 0; i < numWorkers; ++i) {
        workers_.emplace_back(&EventManager::workerThread, this);
    }
    
    initialized_ = true;
    spdlog::info("EventManager initialized with {} workers and max queue size {}", numWorkers, maxQueueSize);
    return true;
}

void EventManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return;
    }
    
    running_ = false;
    condition_.notify_all();
    
    // Wait for worker threads to finish
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    
    workers_.clear();
    initialized_ = false;
    spdlog::info("EventManager shutdown complete");
}

EventManager::SubscriptionToken EventManager::generateToken() {
    return nextToken_++;
}

EventManager::SubscriptionToken EventManager::subscribe(EventType type, EventHandler handler,
                                        bool async, std::chrono::milliseconds timeout) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "EventManager not initialized";
        return 0;
    }
    
    SubscriptionToken token = generateToken();
    Subscription subscription{token, handler, nullptr, async, timeout};
    typeSubscriptions_[type].push_back(subscription);
    
    spdlog::debug("Event subscription created for type {} with token {}", 
                  static_cast<int>(type), token);
    return token;
}

EventManager::SubscriptionToken EventManager::subscribe(EventType type, const std::string& name, 
                                        EventHandler handler, bool async, 
                                        std::chrono::milliseconds timeout) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "EventManager not initialized";
        return 0;
    }
    
    SubscriptionToken token = generateToken();
    Subscription subscription{token, handler, nullptr, async, timeout};
    nameSubscriptions_[name].push_back(subscription);
    
    spdlog::debug("Event subscription created for name '{}' with token {}", name, token);
    return token;
}

EventManager::SubscriptionToken EventManager::subscribe(EventFilter filter, EventHandler handler,
                                        bool async, std::chrono::milliseconds timeout) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "EventManager not initialized";
        return 0;
    }
    
    SubscriptionToken token = generateToken();
    Subscription subscription{token, handler, filter, async, timeout};
    filterSubscriptions_.push_back(subscription);
    
    spdlog::debug("Event subscription created with filter and token {}", token);
    return token;
}

bool EventManager::unsubscribe(SubscriptionToken token) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "EventManager not initialized";
        return false;
    }
    
    // Remove from type subscriptions
    for (auto& [type, subscriptions] : typeSubscriptions_) {
        subscriptions.erase(
            std::remove_if(subscriptions.begin(), subscriptions.end(),
                          [token](const Subscription& sub) {
                              return sub.token == token;
                          }),
            subscriptions.end());
    }
    
    // Remove from name subscriptions
    for (auto& [name, subscriptions] : nameSubscriptions_) {
        subscriptions.erase(
            std::remove_if(subscriptions.begin(), subscriptions.end(),
                          [token](const Subscription& sub) {
                              return sub.token == token;
                          }),
            subscriptions.end());
    }
    
    // Remove from filter subscriptions
    filterSubscriptions_.erase(
        std::remove_if(filterSubscriptions_.begin(), filterSubscriptions_.end(),
                      [token](const Subscription& sub) {
                          return sub.token == token;
                      }),
        filterSubscriptions_.end());
    
    spdlog::debug("Event subscription with token {} removed", token);
    return true;
}

// Legacy unsubscribe methods (deprecated)
void EventManager::unsubscribe(EventType type, EventHandler handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = typeSubscriptions_.find(type);
    if (it != typeSubscriptions_.end()) {
        // Remove all subscriptions for this type (legacy behavior)
        it->second.clear();
        spdlog::warn("Legacy unsubscribe called for type {} - all subscriptions removed", 
                     static_cast<int>(type));
    }
}

void EventManager::unsubscribe(EventType type, const std::string& name, EventHandler handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = nameSubscriptions_.find(name);
    if (it != nameSubscriptions_.end()) {
        // Remove all subscriptions for this name (legacy behavior)
        it->second.clear();
        spdlog::warn("Legacy unsubscribe called for name '{}' - all subscriptions removed", name);
    }
}

void EventManager::unsubscribe(EventFilter filter, EventHandler handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Remove all filter subscriptions (legacy behavior)
    filterSubscriptions_.clear();
    spdlog::warn("Legacy unsubscribe called for filter - all filter subscriptions removed");
}

bool EventManager::publishEvent(const Event& event) {
    if (!initialized_) {
        lastError_ = "EventManager not initialized";
        return false;
    }
    
    if (!validateEvent(event)) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (eventQueue_.size() >= maxQueueSize_) {
        lastError_ = "Event queue is full";
        return false;
    }
    
    eventQueue_.push(event);
    condition_.notify_one();
    
    if (statsEnabled_) {
        stats_.totalEvents++;
        stats_.queuedEvents++;
    }
    
    spdlog::debug("Event published: type={}, name='{}', source='{}'", 
                  static_cast<int>(event.type), event.name, event.source);
    return true;
}

bool EventManager::publishEvent(EventType type, const std::string& name,
                               const nlohmann::json& data, Priority priority) {
    Event event;
    event.type = type;
    event.name = name;
    event.source = "EventManager";
    event.priority = priority;
    event.timestamp = std::chrono::system_clock::now();
    event.data = data;
    
    return publishEvent(event);
}

bool EventManager::publishEventAsync(const Event& event) {
    return publishEvent(event);
}

bool EventManager::publishEventAsync(EventType type, const std::string& name,
                                    const nlohmann::json& data, Priority priority) {
    return publishEvent(type, name, data, priority);
}

bool EventManager::processEvent(const Event& event) {
    if (!initialized_) {
        lastError_ = "EventManager not initialized";
        return false;
    }
    
    if (!validateEvent(event)) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Process type subscriptions
    auto typeIt = typeSubscriptions_.find(event.type);
    if (typeIt != typeSubscriptions_.end()) {
        for (const auto& subscription : typeIt->second) {
            if (subscription.async) {
                handleEventAsync(event, subscription);
            } else {
                handleEvent(event, subscription);
            }
        }
    }
    
    // Process name subscriptions
    auto nameIt = nameSubscriptions_.find(event.name);
    if (nameIt != nameSubscriptions_.end()) {
        for (const auto& subscription : nameIt->second) {
            if (subscription.async) {
                handleEventAsync(event, subscription);
            } else {
                handleEvent(event, subscription);
            }
        }
    }
    
    // Process filter subscriptions
    for (const auto& subscription : filterSubscriptions_) {
        if (matchEvent(event, subscription)) {
            if (subscription.async) {
                handleEventAsync(event, subscription);
            } else {
                handleEvent(event, subscription);
            }
        }
    }
    
    return true;
}

bool EventManager::processEventAsync(const Event& event) {
    return processEvent(event);
}

void EventManager::processEvents() {
    while (running_) {
        Event event;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            condition_.wait(lock, [this] { return !eventQueue_.empty() || !running_; });
            
            if (!running_) {
                break;
            }
            
            if (eventQueue_.empty()) {
                continue;
            }
            
            event = eventQueue_.front();
            eventQueue_.pop();
            
            if (statsEnabled_) {
                stats_.queuedEvents--;
            }
        }
        
        processEvent(event);
    }
}

void EventManager::workerThread() {
    processEvents();
}

void EventManager::waitForEvents(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (timeout.count() > 0) {
        condition_.wait_for(lock, timeout, [this] { return !eventQueue_.empty(); });
    } else {
        condition_.wait(lock, [this] { return !eventQueue_.empty(); });
    }
}

std::vector<EventManager::Event> EventManager::getEvents(EventType type,
                                         std::chrono::system_clock::time_point start,
                                         std::chrono::system_clock::time_point end) {
    // This is a placeholder implementation
    // In a real implementation, you would store events and filter them
    return {};
}

std::vector<EventManager::Event> EventManager::getEvents(const std::string& name,
                                         std::chrono::system_clock::time_point start,
                                         std::chrono::system_clock::time_point end) {
    // This is a placeholder implementation
    return {};
}

std::vector<EventManager::Event> EventManager::getEvents(EventFilter filter,
                                         std::chrono::system_clock::time_point start,
                                         std::chrono::system_clock::time_point end) {
    // This is a placeholder implementation
    return {};
}

EventManager::EventStats EventManager::getStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

void EventManager::resetStats() {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_ = EventStats{};
}

void EventManager::enableStats(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    statsEnabled_ = enable;
}

bool EventManager::addFilter(EventType type, EventFilter filter) {
    std::lock_guard<std::mutex> lock(mutex_);
    typeFilters_[type].push_back(filter);
    return true;
}

bool EventManager::addFilter(const std::string& name, EventFilter filter) {
    std::lock_guard<std::mutex> lock(mutex_);
    nameFilters_[name].push_back(filter);
    return true;
}

void EventManager::removeFilter(EventType type, EventFilter filter) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = typeFilters_.find(type);
    if (it != typeFilters_.end()) {
        it->second.clear();
    }
}

void EventManager::removeFilter(const std::string& name, EventFilter filter) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = nameFilters_.find(name);
    if (it != nameFilters_.end()) {
        it->second.clear();
    }
}

std::string EventManager::getLastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastError_;
}

void EventManager::clearLastError() {
    std::lock_guard<std::mutex> lock(mutex_);
    lastError_.clear();
}

bool EventManager::validateEvent(const Event& event) const {
    if (event.name.empty()) {
        return false;
    }
    return true;
}

void EventManager::updateStats(const Event& event, std::chrono::milliseconds processingTime) {
    if (!statsEnabled_) {
        return;
    }
    
    stats_.processedEvents++;
    stats_.averageProcessingTime = std::chrono::milliseconds(
        (stats_.averageProcessingTime.count() * (stats_.processedEvents - 1) + processingTime.count()) / stats_.processedEvents);
    
    if (processingTime > stats_.maxProcessingTime) {
        stats_.maxProcessingTime = processingTime;
    }
    
    if (stats_.minProcessingTime.count() == 0 || processingTime < stats_.minProcessingTime) {
        stats_.minProcessingTime = processingTime;
    }
}

void EventManager::handleEvent(const Event& event, const Subscription& subscription) {
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        subscription.handler(event);
        auto end = std::chrono::high_resolution_clock::now();
        auto processingTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        updateStats(event, processingTime);
    } catch (const std::exception& e) {
        if (statsEnabled_) {
            stats_.failedEvents++;
        }
        spdlog::error("Error handling event: {}", e.what());
    }
}

void EventManager::handleEventAsync(const Event& event, const Subscription& subscription) {
    std::async(std::launch::async, [this, event, subscription]() {
        handleEvent(event, subscription);
    });
}

bool EventManager::matchEvent(const Event& event, const Subscription& subscription) const {
    if (!subscription.filter) {
        return true;
    }
    
    try {
        return subscription.filter(event);
    } catch (const std::exception& e) {
        spdlog::error("Error in event filter: {}", e.what());
        return false;
    }
}

void EventManager::cleanupSubscriptions() {
    // This method can be used to clean up expired or invalid subscriptions
    // For now, it's a placeholder
}

void EventManager::cleanupFilters() {
    // This method can be used to clean up expired or invalid filters
    // For now, it's a placeholder
}

} // namespace satox::core 