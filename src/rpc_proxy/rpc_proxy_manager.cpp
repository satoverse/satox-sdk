/*
 * MIT License
 * Copyright (c) 2025 Satoxcoin Core Developer
 */
#include "../../include/satox/rpc_proxy/rpc_proxy_manager.hpp"
#include "../../include/satox/rpc_proxy/types.hpp"
#include "../../include/satox/rpc_proxy/error.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <chrono>
#include <thread>

namespace satox::rpc_proxy {

// Global logger instance
static std::shared_ptr<spdlog::logger> g_logger;
static std::mutex g_logger_mutex;

static std::shared_ptr<spdlog::logger>& getLogger() {
    return g_logger;
}

static void initializeLogging(const std::string& logPath) {
    std::lock_guard<std::mutex> lock(g_logger_mutex);
    try {
        if (g_logger) {
            return; // Logger already exists
        }
        std::filesystem::create_directories(logPath);
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            logPath + "/rpc_proxy.log", 1024*1024*5, 3);
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        g_logger = std::make_shared<spdlog::logger>("satox-rpc-proxy", spdlog::sinks_init_list{file_sink, console_sink});
        g_logger->set_level(spdlog::level::debug);
        g_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] [satox-rpc-proxy] %v");
        spdlog::register_logger(g_logger);
        g_logger->info("RPC Proxy logging system initialized");
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize RPC proxy logging: " << e.what() << std::endl;
    }
}

RpcProxyManager& RpcProxyManager::getInstance() {
    static RpcProxyManager instance;
    return instance;
}

RpcProxyManager::RpcProxyManager() {}
RpcProxyManager::~RpcProxyManager() { shutdown(); }

bool RpcProxyManager::initialize(const RpcProxyConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) return true;
    config_ = config;
    try {
        if (config_.enableLogging) {
            initializeLogging(config_.logPath);
        }
        initialized_ = true;
        healthy_ = true;
        logInfo("RPC Proxy initialized");
        return true;
    } catch (const std::exception& e) {
        lastError_ = e.what();
        logError(lastError_);
        healthy_ = false;
        return false;
    }
}

void RpcProxyManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) return;
    
    auto& logger = getLogger();
    if (logger) {
        logger->info("RPC Proxy shutdown completed");
        try {
            spdlog::drop("satox-rpc-proxy");
            logger.reset();
        } catch (const std::exception& e) {
            // Ignore logger cleanup errors
        }
    }
    
    initialized_ = false;
    healthy_ = false;
    logInfo("RPC Proxy shutdown");
}

bool RpcProxyManager::isInitialized() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return initialized_;
}

bool RpcProxyManager::isHealthy() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return healthy_;
}

std::string RpcProxyManager::getLastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastError_;
}

RpcProxyStats RpcProxyManager::getStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

bool RpcProxyManager::sendRpcRequest(const nlohmann::json& request, nlohmann::json& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = "Not initialized";
        logError(lastError_);
        stats_.errors_total++;
        return false;
    }
    // TODO: Implement actual HTTP/HTTPS request logic
    // For now, just simulate a response
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    response = nlohmann::json{{"result", "ok"}, {"id", request.value("id", "0")}};
    stats_.requests_total++;
    stats_.last_latency_ms = 10;
    return true;
}

void RpcProxyManager::registerErrorCallback(std::function<void(const std::string&)> cb) {
    std::lock_guard<std::mutex> lock(mutex_);
    errorCallbacks_.push_back(cb);
}

void RpcProxyManager::registerHealthCallback(std::function<void(bool)> cb) {
    std::lock_guard<std::mutex> lock(mutex_);
    healthCallbacks_.push_back(cb);
}

void RpcProxyManager::logError(const std::string& msg) const {
    auto& logger = getLogger();
    if (logger) logger->error(msg);
    notifyError(msg);
}

void RpcProxyManager::logInfo(const std::string& msg) const {
    auto& logger = getLogger();
    if (logger) logger->info(msg);
}

void RpcProxyManager::notifyError(const std::string& msg) const {
    for (const auto& cb : errorCallbacks_) {
        try { cb(msg); } catch (...) {}
    }
}

void RpcProxyManager::notifyHealth(bool healthy) const {
    for (const auto& cb : healthCallbacks_) {
        try { cb(healthy); } catch (...) {}
    }
}

} // namespace satox::rpc_proxy 