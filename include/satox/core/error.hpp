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

namespace satox::core {

// Error codes
enum class CoreErrorCode {
    SUCCESS = 0,
    UNKNOWN_ERROR = 1,
    INVALID_ARGUMENT = 2,
    NOT_INITIALIZED = 3,
    ALREADY_INITIALIZED = 4,
    NOT_RUNNING = 5,
    ALREADY_RUNNING = 6,
    OPERATION_FAILED = 7,
    VALIDATION_ERROR = 8,
    TIMEOUT_ERROR = 9,
    RESOURCE_EXHAUSTED = 10,
    PERMISSION_DENIED = 11,
    INVALID_STATE = 12
};

// Main exception class
class CoreException : public std::exception {
public:
    CoreException(CoreErrorCode code, const std::string& message = "")
        : code_(code), message_(message) {}
    
    const char* what() const noexcept override {
        return message_.c_str();
    }
    
    CoreErrorCode code() const {
        return code_;
    }
    
    const std::string& message() const {
        return message_;
    }

private:
    CoreErrorCode code_;
    std::string message_;
};

// Specific exception types
class CoreInitializationException : public CoreException {
public:
    CoreInitializationException(const std::string& message = "")
        : CoreException(CoreErrorCode::NOT_INITIALIZED, message) {}
};

class CoreOperationException : public CoreException {
public:
    CoreOperationException(const std::string& message = "")
        : CoreException(CoreErrorCode::OPERATION_FAILED, message) {}
};

} // namespace satox::core
