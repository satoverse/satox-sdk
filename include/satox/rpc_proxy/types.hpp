/**
 * @file types.hpp
 * @brief Satox satox-sdk - Types
 * @details This header provides Types functionality for the Satox satox-sdk.
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
 * @file types.hpp
 * @brief Satox satox-sdk - Types
 * @details This header provides Types functionality for the Satox satox-sdk.
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
#include <nlohmann/json.hpp>

/** @ingroup rpc_proxy */
namespace satox::rpc_proxy {

/** @ingroup rpc_proxy */
struct RpcRequest {
    std::string method;
    nlohmann::json params;
    std::string id;
};

/** @ingroup rpc_proxy */
struct RpcResponse {
    nlohmann::json result;
    std::string error;
    std::string id;
};

/** @ingroup rpc_proxy */
enum class RpcProxyError {
    NONE = 0,
    CONNECTION_FAILED,
    TIMEOUT,
    AUTH_FAILED,
    INVALID_RESPONSE,
    UNKNOWN
};

} // namespace satox::rpc_proxy 
