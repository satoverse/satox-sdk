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

#include "security/monitoring/security_monitor.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <prometheus/text_serializer.h>

namespace satox {
namespace security {
namespace monitoring {

SecurityMonitor::SecurityMonitor() {
    initializePrometheusMetrics();
}

void SecurityMonitor::initializePrometheusMetrics() {
    registry = std::make_shared<prometheus::Registry>();

    // Initialize operation counters
    operationCounters["encrypt"] = &prometheus::BuildCounter()
        .Name("security_encrypt_operations_total")
        .Help("Total number of encryption operations")
        .Register(*registry);
    
    operationCounters["decrypt"] = &prometheus::BuildCounter()
        .Name("security_decrypt_operations_total")
        .Help("Total number of decryption operations")
        .Register(*registry);
    
    operationCounters["sign"] = &prometheus::BuildCounter()
        .Name("security_sign_operations_total")
        .Help("Total number of signing operations")
        .Register(*registry);
    
    operationCounters["verify"] = &prometheus::BuildCounter()
        .Name("security_verify_operations_total")
        .Help("Total number of verification operations")
        .Register(*registry);

    // Initialize operation duration histograms
    operationDurations["encrypt"] = &prometheus::BuildHistogram()
        .Name("security_encrypt_duration_seconds")
        .Help("Encryption operation duration in seconds")
        .Register(*registry);
    
    operationDurations["decrypt"] = &prometheus::BuildHistogram()
        .Name("security_decrypt_duration_seconds")
        .Help("Decryption operation duration in seconds")
        .Register(*registry);
    
    operationDurations["sign"] = &prometheus::BuildHistogram()
        .Name("security_sign_duration_seconds")
        .Help("Signing operation duration in seconds")
        .Register(*registry);
    
    operationDurations["verify"] = &prometheus::BuildHistogram()
        .Name("security_verify_duration_seconds")
        .Help("Verification operation duration in seconds")
        .Register(*registry);

    // Initialize error counters
    errorCounters["invalid_input"] = &prometheus::BuildCounter()
        .Name("security_errors_invalid_input_total")
        .Help("Total number of invalid input errors")
        .Register(*registry);
    
    errorCounters["key_error"] = &prometheus::BuildCounter()
        .Name("security_errors_key_error_total")
        .Help("Total number of key-related errors")
        .Register(*registry);
    
    errorCounters["crypto_error"] = &prometheus::BuildCounter()
        .Name("security_errors_crypto_error_total")
        .Help("Total number of cryptographic errors")
        .Register(*registry);

    // Initialize resource gauges
    memoryGauge = &prometheus::BuildGauge()
        .Name("security_memory_usage_bytes")
        .Help("Current memory usage in bytes")
        .Register(*registry);
    
    cpuGauge = &prometheus::BuildGauge()
        .Name("security_cpu_usage_percent")
        .Help("Current CPU usage percentage")
        .Register(*registry);

    // Initialize cache counters
    cacheHitCounter = &prometheus::BuildCounter()
        .Name("security_cache_hits_total")
        .Help("Total number of cache hits")
        .Register(*registry);
    
    cacheMissCounter = &prometheus::BuildCounter()
        .Name("security_cache_misses_total")
        .Help("Total number of cache misses")
        .Register(*registry);
}

void SecurityMonitor::recordOperation(const std::string& operation,
                                    const std::chrono::milliseconds& duration,
                                    bool success) {
    auto& metrics = operationMetrics[operation];
    metrics.totalOperations++;
    if (success) {
        metrics.successfulOperations++;
    } else {
        metrics.failedOperations++;
    }
    metrics.totalDuration += duration.count();

    {
        std::lock_guard<std::mutex> lock(metrics.mutex);
        metrics.recentDurations.push_back(duration);
        if (metrics.recentDurations.size() > 1000) {
            metrics.recentDurations.erase(metrics.recentDurations.begin());
        }
    }

    // Update Prometheus metrics
    if (operationCounters.count(operation)) {
        operationCounters[operation]->Increment();
    }
    if (operationDurations.count(operation)) {
        operationDurations[operation]->Observe(duration.count() / 1000.0);
    }
}

void SecurityMonitor::recordKeyOperation(const std::string& operation,
                                       const std::string& keyId,
                                       bool success) {
    std::string metricName = operation + "_" + keyId;
    auto& metrics = operationMetrics[metricName];
    metrics.totalOperations++;
    if (success) {
        metrics.successfulOperations++;
    } else {
        metrics.failedOperations++;
    }
}

void SecurityMonitor::recordCacheOperation(const std::string& operation,
                                         bool hit) {
    if (hit) {
        cacheMetrics.hits++;
        cacheHitCounter->Increment();
    } else {
        cacheMetrics.misses++;
        cacheMissCounter->Increment();
    }
}

void SecurityMonitor::recordResourceUsage(size_t memoryUsage,
                                        double cpuUsage) {
    resourceMetrics.currentMemoryUsage = memoryUsage;
    resourceMetrics.currentCpuUsage = cpuUsage;

    {
        std::lock_guard<std::mutex> lock(resourceMetrics.mutex);
        resourceMetrics.memoryHistory.push_back(memoryUsage);
        resourceMetrics.cpuHistory.push_back(cpuUsage);
        if (resourceMetrics.memoryHistory.size() > 1000) {
            resourceMetrics.memoryHistory.erase(resourceMetrics.memoryHistory.begin());
            resourceMetrics.cpuHistory.erase(resourceMetrics.cpuHistory.begin());
        }
    }

    // Update Prometheus metrics
    memoryGauge->Set(memoryUsage);
    cpuGauge->Set(cpuUsage);
}

void SecurityMonitor::recordError(const std::string& operation,
                                const std::string& errorType) {
    {
        std::lock_guard<std::mutex> lock(errorMetrics.mutex);
        errorMetrics.errorCounts[errorType]++;
    }

    // Update Prometheus metrics
    if (errorCounters.count(errorType)) {
        errorCounters[errorType]->Increment();
    }
}

double SecurityMonitor::getAverageOperationTime(const std::string& operation) const {
    auto it = operationMetrics.find(operation);
    if (it == operationMetrics.end()) {
        return 0.0;
    }

    const auto& metrics = it->second;
    if (metrics.totalOperations == 0) {
        return 0.0;
    }

    return static_cast<double>(metrics.totalDuration) / metrics.totalOperations;
}

double SecurityMonitor::getOperationSuccessRate(const std::string& operation) const {
    auto it = operationMetrics.find(operation);
    if (it == operationMetrics.end()) {
        return 0.0;
    }

    const auto& metrics = it->second;
    if (metrics.totalOperations == 0) {
        return 0.0;
    }

    return static_cast<double>(metrics.successfulOperations) / metrics.totalOperations;
}

double SecurityMonitor::getCacheHitRate() const {
    size_t total = cacheMetrics.hits + cacheMetrics.misses;
    if (total == 0) {
        return 0.0;
    }

    return static_cast<double>(cacheMetrics.hits) / total;
}

size_t SecurityMonitor::getCurrentMemoryUsage() const {
    return resourceMetrics.currentMemoryUsage;
}

double SecurityMonitor::getCurrentCpuUsage() const {
    return resourceMetrics.currentCpuUsage;
}

size_t SecurityMonitor::getErrorCount(const std::string& operation) const {
    std::lock_guard<std::mutex> lock(errorMetrics.mutex);
    auto it = errorMetrics.errorCounts.find(operation);
    return it != errorMetrics.errorCounts.end() ? it->second : 0;
}

std::string SecurityMonitor::exportMetrics() const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);

    // Export operation metrics
    ss << "# Operation Metrics\n";
    for (const auto& [operation, metrics] : operationMetrics) {
        ss << "security_operation_" << operation << "_total " 
           << metrics.totalOperations << "\n";
        ss << "security_operation_" << operation << "_success_rate "
           << getOperationSuccessRate(operation) << "\n";
        ss << "security_operation_" << operation << "_avg_duration_ms "
           << getAverageOperationTime(operation) << "\n";
    }

    // Export cache metrics
    ss << "\n# Cache Metrics\n";
    ss << "security_cache_hit_rate " << getCacheHitRate() << "\n";

    // Export resource metrics
    ss << "\n# Resource Metrics\n";
    ss << "security_memory_usage_bytes " << getCurrentMemoryUsage() << "\n";
    ss << "security_cpu_usage_percent " << getCurrentCpuUsage() << "\n";

    // Export error metrics
    ss << "\n# Error Metrics\n";
    {
        std::lock_guard<std::mutex> lock(errorMetrics.mutex);
        for (const auto& [error, count] : errorMetrics.errorCounts) {
            ss << "security_error_" << error << "_total " << count << "\n";
        }
    }

    return ss.str();
}

void SecurityMonitor::exportToPrometheus(const std::string& endpoint) {
    // Update all Prometheus metrics
    updatePrometheusMetrics();

    // Export metrics to Prometheus endpoint
    prometheus::TextSerializer serializer;
    std::string metrics = serializer.Serialize(registry->Collect());

    // TODO: Implement HTTP server to expose metrics
    // This would typically be done using a web server library
    // like cpp-httplib or similar
}

void SecurityMonitor::updatePrometheusMetrics() {
    // Update operation metrics
    for (const auto& [operation, metrics] : operationMetrics) {
        if (operationCounters.count(operation)) {
            operationCounters[operation]->Increment(metrics.totalOperations);
        }
        if (operationDurations.count(operation)) {
            operationDurations[operation]->Observe(
                static_cast<double>(metrics.totalDuration) / 1000.0
            );
        }
    }

    // Update resource metrics
    memoryGauge->Set(resourceMetrics.currentMemoryUsage);
    cpuGauge->Set(resourceMetrics.currentCpuUsage);

    // Update cache metrics
    cacheHitCounter->Increment(cacheMetrics.hits);
    cacheMissCounter->Increment(cacheMetrics.misses);

    // Update error metrics
    {
        std::lock_guard<std::mutex> lock(errorMetrics.mutex);
        for (const auto& [error, count] : errorMetrics.errorCounts) {
            if (errorCounters.count(error)) {
                errorCounters[error]->Increment(count);
            }
        }
    }
}

void SecurityMonitor::cleanupOldMetrics() {
    const size_t maxHistorySize = 1000;
    const std::chrono::hours maxAge(24);

    auto now = std::chrono::system_clock::now();

    // Cleanup operation metrics
    for (auto& [operation, metrics] : operationMetrics) {
        std::lock_guard<std::mutex> lock(metrics.mutex);
        if (metrics.recentDurations.size() > maxHistorySize) {
            metrics.recentDurations.erase(
                metrics.recentDurations.begin(),
                metrics.recentDurations.begin() + (metrics.recentDurations.size() - maxHistorySize)
            );
        }
    }

    // Cleanup resource metrics
    {
        std::lock_guard<std::mutex> lock(resourceMetrics.mutex);
        if (resourceMetrics.memoryHistory.size() > maxHistorySize) {
            resourceMetrics.memoryHistory.erase(
                resourceMetrics.memoryHistory.begin(),
                resourceMetrics.memoryHistory.begin() + (resourceMetrics.memoryHistory.size() - maxHistorySize)
            );
        }
        if (resourceMetrics.cpuHistory.size() > maxHistorySize) {
            resourceMetrics.cpuHistory.erase(
                resourceMetrics.cpuHistory.begin(),
                resourceMetrics.cpuHistory.begin() + (resourceMetrics.cpuHistory.size() - maxHistorySize)
            );
        }
    }
}

} // namespace monitoring
} // namespace security
} // namespace satox 