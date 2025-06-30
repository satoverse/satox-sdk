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
#include <chrono>
#include <nlohmann/json.hpp>

namespace satox::security {

// Version information
struct Version {
    int major = 0;
    int minor = 1;
    int patch = 0;
    std::string build = "";
    std::string commit = "";
    
    std::string toString() const {
        return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
    }
};

// Security levels
enum class SecurityLevel {
    LOW = 0,
    MEDIUM = 1,
    HIGH = 2,
    CRITICAL = 3
};

// PQC algorithms
enum class PQCAlgorithm {
    ML_DSA,
    ML_KEM,
    HYBRID_ML_DSA,
    HYBRID_ML_KEM
};

// Input validation types
enum class ValidationType {
    EMAIL,
    URL,
    IP_ADDRESS,
    PHONE_NUMBER,
    CREDIT_CARD,
    CUSTOM
};

// Rate limit configuration
struct RateLimitConfig {
    std::string identifier;
    int maxRequests = 100;
    int timeWindow = 3600; // seconds
    SecurityLevel level = SecurityLevel::MEDIUM;
};

// Security event
struct SecurityEvent {
    std::string type;
    std::string source;
    std::string description;
    SecurityLevel level;
    std::chrono::system_clock::time_point timestamp;
    nlohmann::json data;
    
    SecurityEvent(const std::string& t, const std::string& s, const std::string& d, SecurityLevel l)
        : type(t), source(s), description(d), level(l), timestamp(std::chrono::system_clock::now()) {}
};

// PQC key information
struct PQCKeyInfo {
    std::string algorithm;
    std::string publicKey;
    std::string privateKey;
    std::chrono::system_clock::time_point createdAt;
    std::chrono::system_clock::time_point expiresAt;
    bool isActive = true;
};

// Validation result
struct ValidationResult {
    bool isValid = false;
    std::string sanitizedValue;
    std::string error;
    ValidationType type;
    SecurityLevel level;
};

// Rate limit status
struct RateLimitStatus {
    std::string identifier;
    int currentRequests = 0;
    int maxRequests = 0;
    int timeWindow = 0;
    std::chrono::system_clock::time_point resetTime;
    bool isBlocked = false;
};

} // namespace satox::security
