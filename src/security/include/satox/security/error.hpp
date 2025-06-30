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
#include <exception>
#include <system_error>

namespace satox::security {

// Error codes
enum class SecurityErrorCode {
    SUCCESS = 0,
    UNKNOWN_ERROR = 1,
    INVALID_ARGUMENT = 2,
    NOT_INITIALIZED = 3,
    ALREADY_INITIALIZED = 4,
    NOT_RUNNING = 5,
    ALREADY_RUNNING = 6,
    PQC_ERROR = 7,
    VALIDATION_ERROR = 8,
    RATE_LIMIT_ERROR = 9,
    SECURITY_VIOLATION = 10,
    AUTHENTICATION_ERROR = 11,
    AUTHORIZATION_ERROR = 12,
    ENCRYPTION_ERROR = 13,
    DECRYPTION_ERROR = 14,
    SIGNATURE_ERROR = 15,
    VERIFICATION_ERROR = 16,
    KEY_GENERATION_ERROR = 17,
    KEY_STORAGE_ERROR = 18,
    CONFIGURATION_ERROR = 19
};

// Main exception class
class SecurityException : public std::exception {
public:
    SecurityException(SecurityErrorCode code, const std::string& message = "")
        : code_(code), message_(message) {}
    
    const char* what() const noexcept override {
        return message_.c_str();
    }
    
    SecurityErrorCode code() const {
        return code_;
    }
    
    const std::string& message() const {
        return message_;
    }

private:
    SecurityErrorCode code_;
    std::string message_;
};

// Specific exception types
class SecurityInitializationException : public SecurityException {
public:
    SecurityInitializationException(const std::string& message = "")
        : SecurityException(SecurityErrorCode::NOT_INITIALIZED, message) {}
};

class PQCException : public SecurityException {
public:
    PQCException(const std::string& message = "")
        : SecurityException(SecurityErrorCode::PQC_ERROR, message) {}
};

class ValidationException : public SecurityException {
public:
    ValidationException(const std::string& message = "")
        : SecurityException(SecurityErrorCode::VALIDATION_ERROR, message) {}
};

class RateLimitException : public SecurityException {
public:
    RateLimitException(const std::string& message = "")
        : SecurityException(SecurityErrorCode::RATE_LIMIT_ERROR, message) {}
};

class SecurityViolationException : public SecurityException {
public:
    SecurityViolationException(const std::string& message = "")
        : SecurityException(SecurityErrorCode::SECURITY_VIOLATION, message) {}
};

} // namespace satox::security
