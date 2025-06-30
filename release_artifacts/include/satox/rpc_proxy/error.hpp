/**
 * @file error.hpp
 * @brief Satox satox-sdk - Error
 * @details This header provides Error functionality for the Satox satox-sdk.
 *
 * @defgroup rpc_proxy rpc_proxy Component
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
 * @file error.hpp
 * @brief Satox satox-sdk - Error
 * @details This header provides Error functionality for the Satox satox-sdk.
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

/*
 * MIT License
 * Copyright (c) 2025 Satoxcoin Core Developer
 */
#pragma once

#include <string>

/** @ingroup rpc_proxy */
namespace satox::rpc_proxy {

inline std::string errorToString(int code) {
    switch (code) {
        case 0: return "NONE";
        case 1: return "CONNECTION_FAILED";
        case 2: return "TIMEOUT";
        case 3: return "AUTH_FAILED";
        case 4: return "INVALID_RESPONSE";
        default: return "UNKNOWN";
    }
}

} // namespace satox::rpc_proxy 
