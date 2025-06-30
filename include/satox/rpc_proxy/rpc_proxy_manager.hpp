/**
 * @file rpc_proxy_manager.hpp
 * @brief Satox satox-sdk - Rpc Proxy Manager
 * @details This header provides Rpc Proxy Manager functionality for the Satox satox-sdk.
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
 * @file rpc_proxy_manager.hpp
 * @brief Satox satox-sdk - Rpc Proxy Manager
 * @details This header provides Rpc Proxy Manager functionality for the Satox satox-sdk.
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
#include <mutex>
#include <memory>
#include <nlohmann/json.hpp>
#include <functional>
#include <vector>

/** @ingroup rpc_proxy */
namespace satox::rpc_proxy {

/** @ingroup rpc_proxy */
struct RpcProxyConfig {
    std::string endpoint;
    std::string username;
    std::string password;
    uint32_t timeout_ms = 30000;
    bool enableLogging = true;
    std::string logPath = "logs/components/rpc_proxy/";
};

/** @ingroup rpc_proxy */
struct RpcProxyStats {
    uint64_t requests_total = 0;
    uint64_t errors_total = 0;
    uint64_t last_latency_ms = 0;
    nlohmann::json additional_stats;
};

/** @ingroup rpc_proxy */
class RpcProxyManager {
public:
    static RpcProxyManager& getInstance();
    bool initialize(const RpcProxyConfig& config);
    void shutdown();
    bool isInitialized() const;
    bool isHealthy() const;
    std::string getLastError() const;
    RpcProxyStats getStats() const;
    bool sendRpcRequest(const nlohmann::json& request, nlohmann::json& response);
    void registerErrorCallback(std::function<void(const std::string&)> cb);
    void registerHealthCallback(std::function<void(bool)> cb);
private:
    RpcProxyManager();
    ~RpcProxyManager();
    RpcProxyManager(const RpcProxyManager&) = delete;
    RpcProxyManager& operator=(const RpcProxyManager&) = delete;
    void logError(const std::string& msg) const;
    void logInfo(const std::string& msg) const;
    void notifyError(const std::string& msg) const;
    void notifyHealth(bool healthy) const;
    mutable std::mutex mutex_;
    RpcProxyConfig config_;
    RpcProxyStats stats_;
    std::string lastError_;
    bool initialized_ = false;
    bool healthy_ = true;
    std::vector<std::function<void(const std::string&)>> errorCallbacks_;
    std::vector<std::function<void(bool)>> healthCallbacks_;
};

} // namespace satox::rpc_proxy 
