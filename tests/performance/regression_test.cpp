#include "security/security_manager.hpp"
#include "security/monitoring/security_monitor.hpp"
#include <gtest/gtest.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <chrono>
#include <thread>
#include <vector>
#include <cmath>

using namespace satox::security;

class PerformanceRegressionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Load baseline metrics if they exist
        loadBaselineMetrics();
        
        // Initialize security manager
        securityManager = std::make_unique<SecurityManager>();
    }

    void TearDown() override {
        // Save current metrics as new baseline if test passed
        if (::testing::Test::HasFailure()) {
            std::cout << "Performance regression detected!\n";
        } else {
            saveBaselineMetrics();
        }
    }

    void loadBaselineMetrics() {
        try {
            std::ifstream file("performance_baseline.json");
            if (file.is_open()) {
                file >> baselineMetrics;
            }
        } catch (const std::exception& e) {
            std::cerr << "Warning: Could not load baseline metrics: " << e.what() << "\n";
        }
    }

    void saveBaselineMetrics() {
        try {
            std::ofstream file("performance_baseline.json");
            if (file.is_open()) {
                file << std::setw(4) << currentMetrics << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Warning: Could not save baseline metrics: " << e.what() << "\n";
        }
    }

    bool isWithinThreshold(double current, double baseline, double threshold) {
        if (baseline == 0) return true;
        double percentChange = std::abs((current - baseline) / baseline * 100);
        return percentChange <= threshold;
    }

    void measureOperation(const std::string& operation, size_t dataSize, int iterations) {
        std::vector<uint8_t> data = generateRandomData(dataSize);
        std::vector<uint8_t> result;
        std::vector<double> times;

        // Warmup
        for (int i = 0; i < 5; ++i) {
            if (operation == "encrypt") {
                securityManager->encrypt(data, result);
            } else if (operation == "decrypt") {
                securityManager->decrypt(data, result);
            } else if (operation == "sign") {
                securityManager->sign(data, result);
            } else if (operation == "verify") {
                securityManager->verify(data, result);
            }
        }

        // Measure
        for (int i = 0; i < iterations; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            
            if (operation == "encrypt") {
                securityManager->encrypt(data, result);
            } else if (operation == "decrypt") {
                securityManager->decrypt(data, result);
            } else if (operation == "sign") {
                securityManager->sign(data, result);
            } else if (operation == "verify") {
                securityManager->verify(data, result);
            }

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            times.push_back(duration.count() / 1000.0); // Convert to milliseconds
        }

        // Calculate statistics
        double avgTime = calculateAverage(times);
        double minTime = *std::min_element(times.begin(), times.end());
        double maxTime = *std::max_element(times.begin(), times.end());
        double stdDev = calculateStdDev(times, avgTime);

        // Store metrics
        currentMetrics[operation][std::to_string(dataSize)] = {
            {"avg_time_ms", avgTime},
            {"min_time_ms", minTime},
            {"max_time_ms", maxTime},
            {"std_dev_ms", stdDev},
            {"memory_usage_bytes", monitoring::SecurityMonitor::getInstance().getCurrentMemoryUsage()},
            {"cpu_usage_percent", monitoring::SecurityMonitor::getInstance().getCurrentCpuUsage()}
        };

        // Compare with baseline
        if (baselineMetrics.contains(operation) && 
            baselineMetrics[operation].contains(std::to_string(dataSize))) {
            auto baseline = baselineMetrics[operation][std::to_string(dataSize)];
            
            EXPECT_TRUE(isWithinThreshold(avgTime, baseline["avg_time_ms"], 10.0))
                << "Average time for " << operation << " with " << dataSize 
                << " bytes exceeded threshold. Current: " << avgTime 
                << " ms, Baseline: " << baseline["avg_time_ms"] << " ms";
            
            EXPECT_TRUE(isWithinThreshold(maxTime, baseline["max_time_ms"], 20.0))
                << "Maximum time for " << operation << " with " << dataSize 
                << " bytes exceeded threshold. Current: " << maxTime 
                << " ms, Baseline: " << baseline["max_time_ms"] << " ms";
        }
    }

    std::vector<uint8_t> generateRandomData(size_t size) {
        std::vector<uint8_t> data(size);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        
        for (size_t i = 0; i < size; ++i) {
            data[i] = static_cast<uint8_t>(dis(gen));
        }
        
        return data;
    }

    double calculateAverage(const std::vector<double>& values) {
        return std::accumulate(values.begin(), values.end(), 0.0) / values.size();
    }

    double calculateStdDev(const std::vector<double>& values, double mean) {
        double sq_sum = std::inner_product(values.begin(), values.end(), values.begin(), 0.0);
        return std::sqrt(sq_sum / values.size() - mean * mean);
    }

    std::unique_ptr<SecurityManager> securityManager;
    nlohmann::json baselineMetrics;
    nlohmann::json currentMetrics;
};

// Test cases for different data sizes
TEST_F(PerformanceRegressionTest, SmallData) {
    measureOperation("encrypt", 1024, 1000);
    measureOperation("decrypt", 1024, 1000);
    measureOperation("sign", 1024, 1000);
    measureOperation("verify", 1024, 1000);
}

TEST_F(PerformanceRegressionTest, MediumData) {
    measureOperation("encrypt", 102400, 100);
    measureOperation("decrypt", 102400, 100);
    measureOperation("sign", 102400, 100);
    measureOperation("verify", 102400, 100);
}

TEST_F(PerformanceRegressionTest, LargeData) {
    measureOperation("encrypt", 1048576, 10);
    measureOperation("decrypt", 1048576, 10);
    measureOperation("sign", 1048576, 10);
    measureOperation("verify", 1048576, 10);
}

TEST_F(PerformanceRegressionTest, ConcurrentOperations) {
    const int numThreads = 4;
    const size_t dataSize = 102400;
    std::vector<std::thread> threads;
    std::vector<uint8_t> data = generateRandomData(dataSize);
    std::vector<uint8_t> result;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([this, &data, &result]() {
            for (int j = 0; j < 100; ++j) {
                securityManager->encrypt(data, result);
                securityManager->decrypt(result, data);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Store concurrent operation metrics
    currentMetrics["concurrent_operations"] = {
        {"total_time_ms", duration.count()},
        {"operations_per_second", (numThreads * 100 * 1000.0) / duration.count()},
        {"memory_usage_bytes", monitoring::SecurityMonitor::getInstance().getCurrentMemoryUsage()},
        {"cpu_usage_percent", monitoring::SecurityMonitor::getInstance().getCurrentCpuUsage()}
    };

    // Compare with baseline
    if (baselineMetrics.contains("concurrent_operations")) {
        auto baseline = baselineMetrics["concurrent_operations"];
        
        EXPECT_TRUE(isWithinThreshold(duration.count(), baseline["total_time_ms"], 15.0))
            << "Concurrent operations total time exceeded threshold. Current: " 
            << duration.count() << " ms, Baseline: " << baseline["total_time_ms"] << " ms";
        
        EXPECT_TRUE(isWithinThreshold(
            (numThreads * 100 * 1000.0) / duration.count(),
            baseline["operations_per_second"],
            15.0))
            << "Concurrent operations throughput exceeded threshold";
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 