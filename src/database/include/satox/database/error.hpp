/*
 * MIT License
 * Copyright(c) 2025 Satoxcoin Core Developer
 */

#pragma once

#include <string>
#include <exception>
#include <system_error>

namespace satox::database {

// Error codes
enum class DatabaseErrorCode {
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
    INVALID_STATE = 12,
    INITIALIZATION_ERROR = 13
};

// Main exception class
class DatabaseException : public std::exception {
public:
    DatabaseException(DatabaseErrorCode code, const std::string& message = "")
        : code_(code), message_(message) {}
    
    const char* what() const noexcept override {
        return message_.c_str();
    }
    
    DatabaseErrorCode code() const {
        return code_;
    }
    
    const std::string& message() const {
        return message_;
    }

private:
    DatabaseErrorCode code_;
    std::string message_;
};

// Specific exception types
class DatabaseInitializationException : public DatabaseException {
public:
    DatabaseInitializationException(const std::string& message = "")
        : DatabaseException(DatabaseErrorCode::NOT_INITIALIZED, message) {}
};

class DatabaseOperationException : public DatabaseException {
public:
    DatabaseOperationException(const std::string& message = "")
        : DatabaseException(DatabaseErrorCode::OPERATION_FAILED, message) {}
};

} // namespace satox::database
