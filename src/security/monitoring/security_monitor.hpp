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

#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <atomic>
#include <memory>
#include <prometheus/counter.h>
#include <prometheus/gauge.h>
#include <prometheus/histogram.h>
#include <prometheus/registry.h>

namespace satox {
namespace security {
namespace monitoring {

class SecurityMonitor {
public:
    static SecurityMonitor& getInstance() {
        static SecurityMonitor instance;
        return instance;
    }

    // Operation metrics
    void recordOperation(const std::string& operation, 
                        const std::chrono::milliseconds& duration,
                        bool success);
    
    void recordKeyOperation(const std::string& operation,
                           const std::string& keyId,
                           bool success);
    
    void recordCacheOperation(const std::string& operation,
                             bool hit);
    
    void recordResourceUsage(size_t memoryUsage,
                            double cpuUsage);
    
    void recordError(const std::string& operation,
                    const std::string& errorType);

    // Performance metrics
    double getAverageOperationTime(const std::string& operation) const;
    double getOperationSuccessRate(const std::string& operation) const;
    double getCacheHitRate() const;
    size_t getCurrentMemoryUsage() const;
    double getCurrentCpuUsage() const;
    size_t getErrorCount(const std::string& operation) const;

    // Export metrics
    std::string exportMetrics() const;
    void exportToPrometheus(const std::string& endpoint);

private:
    SecurityMonitor();
    ~SecurityMonitor() = default;
    SecurityMonitor(const SecurityMonitor&) = delete;
    SecurityMonitor& operator=(const SecurityMonitor&) = delete;

    struct OperationMetrics {
        std::atomic<size_t> totalOperations{0};
        std::atomic<size_t> successfulOperations{0};
        std::atomic<size_t> failedOperations{0};
        std::atomic<size_t> totalDuration{0};
        std::vector<std::chrono::milliseconds> recentDurations;
        mutable std::mutex mutex;
    };

    struct CacheMetrics {
        std::atomic<size_t> hits{0};
        std::atomic<size_t> misses{0};
        mutable std::mutex mutex;
    };

    struct ResourceMetrics {
        std::atomic<size_t> currentMemoryUsage{0};
        std::atomic<double> currentCpuUsage{0};
        std::vector<size_t> memoryHistory;
        std::vector<double> cpuHistory;
        mutable std::mutex mutex;
    };

    struct ErrorMetrics {
        std::unordered_map<std::string, std::atomic<size_t>> errorCounts;
        mutable std::mutex mutex;
    };

    // Metrics storage
    std::unordered_map<std::string, OperationMetrics> operationMetrics;
    CacheMetrics cacheMetrics;
    ResourceMetrics resourceMetrics;
    ErrorMetrics errorMetrics;

    // Prometheus metrics
    std::shared_ptr<prometheus::Registry> registry;
    std::unordered_map<std::string, prometheus::Counter*> operationCounters;
    std::unordered_map<std::string, prometheus::Histogram*> operationDurations;
    std::unordered_map<std::string, prometheus::Counter*> errorCounters;
    prometheus::Gauge* memoryGauge;
    prometheus::Gauge* cpuGauge;
    prometheus::Counter* cacheHitCounter;
    prometheus::Counter* cacheMissCounter;

    // Helper methods
    void initializePrometheusMetrics();
    void updatePrometheusMetrics();
    void cleanupOldMetrics();
    std::string formatMetrics() const;
};

} // namespace monitoring
} // namespace security
} // namespace satox 