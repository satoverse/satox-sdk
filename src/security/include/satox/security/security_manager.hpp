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
#include <vector>
#include <map>
#include <functional>
#include <mutex>
#include <memory>
#include <chrono>
#include <variant>
#include <nlohmann/json.hpp>

// Include PQC headers (commented out until PQC implementation is complete)
// #include "satox/security/pqc/key_manager.hpp"
// #include "satox/security/pqc/ml_dsa.hpp"
// #include "satox/security/pqc/ml_kem.hpp"
// #include "satox/security/pqc/hybrid.hpp"

// Include general security headers
#include "satox/security/input_validator.hpp"
#include "satox/security/rate_limiter.hpp"

namespace satox::security {

// Forward declarations
class SecurityManager;

// Enums
enum class SecurityState {
    UNINITIALIZED,
    INITIALIZING,
    INITIALIZED,
    RUNNING,
    STOPPED,
    ERROR,
    SHUTDOWN
};

// Configuration structures
struct SecurityConfig {
    std::string name = "satox_security";
    bool enablePQC = true;
    bool enableInputValidation = true;
    bool enableRateLimiting = true;
    bool enableLogging = true;
    std::string logPath = "logs/components/security/";
    nlohmann::json additionalConfig;
};

// Statistics structures
struct SecurityStats {
    int totalOperations = 0;
    int successfulOperations = 0;
    int failedOperations = 0;
    int securityViolations = 0;
    int pqcOperations = 0;
    int validationOperations = 0;
    std::chrono::system_clock::time_point lastOperation;
    double averageOperationTime = 0.0;
    nlohmann::json additionalStats;
};

// Result structures
struct SecurityResult {
    bool success = false;
    std::string error;
    nlohmann::json data;
    std::chrono::milliseconds duration;
};

// Callback types
using SecurityCallback = std::function<void(const std::string&, bool)>;
using SecurityViolationCallback = std::function<void(const std::string&, const std::string&)>;

// Main unified security manager class
class SecurityManager {
public:
    // Singleton pattern
    static SecurityManager& getInstance();

    // Constructor/Destructor
    SecurityManager();
    ~SecurityManager();

    // Initialization and shutdown
    bool initialize(const SecurityConfig& config);
    void shutdown();

    // Core operations
    bool start();
    bool stop();
    bool performOperation(const std::string& operation, const nlohmann::json& params);
    SecurityResult executeOperation(const std::string& operation, const nlohmann::json& params);
    bool validateOperation(const std::string& operation, const nlohmann::json& params);

    // State management
    SecurityState getState() const;
    bool isInitialized() const;
    bool isRunning() const;
    bool isHealthy() const;

    // Configuration management
    SecurityConfig getConfig() const;
    bool updateConfig(const SecurityConfig& config);
    bool validateConfig(const SecurityConfig& config);

    // Statistics and monitoring
    SecurityStats getStats() const;
    void resetStats();
    bool enableStats(bool enable);

    // Callback registration
    void registerSecurityCallback(SecurityCallback callback);
    void registerViolationCallback(SecurityViolationCallback callback);
    void unregisterSecurityCallback();
    void unregisterViolationCallback();

    // Error handling
    std::string getLastError() const;
    void clearLastError();

    // Health check
    bool healthCheck();
    nlohmann::json getHealthStatus();

    // PQC Operations
    bool generatePQCKeyPair(const std::string& algorithm);
    bool signWithPQC(const std::string& algorithm, const std::string& data);
    bool verifyWithPQC(const std::string& algorithm, const std::string& data, const std::string& signature);
    bool encryptWithPQC(const std::string& algorithm, const std::string& data);
    bool decryptWithPQC(const std::string& algorithm, const std::string& encryptedData);

    // Input Validation Operations
    bool validateInput(const std::string& input, const std::string& type);
    bool sanitizeInput(const std::string& input, std::string& sanitized);
    bool validateEmail(const std::string& email);
    bool validateURL(const std::string& url);
    bool validateIPAddress(const std::string& ip);
    bool validateJson(const std::string& json);
    bool validateToken(const std::string& token);

    // Rate Limiting Operations
    bool checkRateLimit(const std::string& identifier, const std::string& operation);
    bool setRateLimit(const std::string& identifier, int maxRequests, int timeWindow);
    bool resetRateLimit(const std::string& identifier);

    // Performance Configuration
    bool setPerformanceConfig(const nlohmann::json& config);
    size_t getCacheSize() const;
    size_t getConnectionPoolSize() const;
    size_t getBatchSize() const;

private:
    // Disable copy and assignment
    SecurityManager(const SecurityManager&) = delete;
    SecurityManager& operator=(const SecurityManager&) = delete;

    // Private helper methods
    void setLastError(const std::string& error);
    void notifySecurityEvent(const std::string& event, bool success);
    void notifyViolationEvent(const std::string& operation, const std::string& violation);
    void updateStats(bool success, double operationTime);
    bool validateState() const;
    void logOperation(const std::string& operation, bool success, double duration);
    bool initializeComponents();
    void shutdownComponents();

    // Member variables
    mutable std::mutex mutex_;
    bool initialized_ = false;
    bool is_running_ = false;
    SecurityState state_ = SecurityState::UNINITIALIZED;
    SecurityConfig config_;
    SecurityStats stats_;
    std::string lastError_;
    bool statsEnabled_ = false;

    // Callbacks
    std::vector<SecurityCallback> securityCallbacks_;
    std::vector<SecurityViolationCallback> violationCallbacks_;

    // Internal state
    std::chrono::system_clock::time_point start_time_;
    std::chrono::system_clock::time_point lastHealthCheck_;
    int consecutiveFailures_ = 0;
    nlohmann::json internalState_;

    // Component managers
    // std::unique_ptr<pqc::KeyManager> pqcKeyManager_;  // Commented out until PQC implementation
    std::unique_ptr<InputValidator> inputValidator_;
    std::unique_ptr<RateLimiter> rateLimiter_;
    
    // Rate limiting storage
    std::map<std::string, std::pair<int, int>> rateLimits_; // identifier -> {maxRequests, timeWindow}
    
    // Performance configuration storage
    nlohmann::json performanceConfig_;
};

} // namespace satox::security
