/**
 * @file error_handling.hpp
 * @brief Satox satox-sdk - Error Handling
 * @details This header provides Error Handling functionality for the Satox satox-sdk.
 *
 * @defgroup core core Component
 * @ingroup satox-sdk
 * @author Satox Development Team
 * @date 2025-06-23
 * @version 1.0.0
 * @copyright Copyright (c) 2024 Satox
 *
 * @note This file is part of the Satox satox-sdk
 * @warning This is a public API header - changes may break compatibility
 *
 * @see satox_sdk.h
 * @see https://github.com/satoverse/satox-satox-sdk
 */

/**
 * @file error_handling.hpp
 * @brief Satox satox-sdk - Error Handling
 * @details This header provides Error Handling functionality for the Satox satox-sdk.
 * 
 * @author Satox Development Team
 * @date 2025-06-23
 * @version 1.0.0
 * @copyright Copyright (c) 2024 Satox
 * 
 * @note This file is part of the Satox satox-sdk
 * @warning This is a public API header - changes may break compatibility
 * 
 * @see satox_sdk.h
 * @see https://github.com/satoverse/satox-satox-sdk
 */

/**
 * @file error_handling.hpp
 * @brief Error handling utilities for Satox SDK
 * @copyright Copyright (c) 2025 Satoxcoin Core Developer
 * @license MIT License
 */

#ifndef SATOX_CORE_ERROR_HANDLING_HPP
#define SATOX_CORE_ERROR_HANDLING_HPP

#include <stdexcept>
#include <string>

/** @ingroup core */
namespace satox {
/** @ingroup core */
namespace core {

/**
 * @brief Custom exception class for Satox SDK errors
 */
/** @ingroup core */
class SatoxError : public std::runtime_error {
public:
    /**
     * @brief Constructor with error message
     * @param message Error message
     */
    explicit SatoxError(const std::string& message) 
        : std::runtime_error(message) {}
    
    /**
     * @brief Constructor with error message
     * @param message Error message
     */
    explicit SatoxError(const char* message) 
        : std::runtime_error(message) {}
    
    /**
     * @brief Virtual destructor
     */
    virtual ~SatoxError() = default;
};

/**
 * @brief Error codes for different types of errors
 */
/** @ingroup core */
enum class ErrorCode {
    SUCCESS = 0,
    INVALID_ARGUMENT,
    NOT_FOUND,
    ALREADY_EXISTS,
    PERMISSION_DENIED,
    TIMEOUT,
    NETWORK_ERROR,
    DATABASE_ERROR,
    CRYPTOGRAPHIC_ERROR,
    UNKNOWN_ERROR
};

/**
 * @brief Convert error code to string
 * @param code Error code
 * @return String representation of error code
 */
inline std::string errorCodeToString(ErrorCode code) {
    switch (code) {
        case ErrorCode::SUCCESS: return "SUCCESS";
        case ErrorCode::INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case ErrorCode::NOT_FOUND: return "NOT_FOUND";
        case ErrorCode::ALREADY_EXISTS: return "ALREADY_EXISTS";
        case ErrorCode::PERMISSION_DENIED: return "PERMISSION_DENIED";
        case ErrorCode::TIMEOUT: return "TIMEOUT";
        case ErrorCode::NETWORK_ERROR: return "NETWORK_ERROR";
        case ErrorCode::DATABASE_ERROR: return "DATABASE_ERROR";
        case ErrorCode::CRYPTOGRAPHIC_ERROR: return "CRYPTOGRAPHIC_ERROR";
        case ErrorCode::UNKNOWN_ERROR: return "UNKNOWN_ERROR";
        default: return "UNKNOWN_ERROR";
    }
}

} // namespace core
} // namespace satox

#endif // SATOX_CORE_ERROR_HANDLING_HPP 
