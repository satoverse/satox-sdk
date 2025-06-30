/*
 * MIT License
 * Copyright(c) 2025 Satoxcoin Core Developer
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <nlohmann/json.hpp>

namespace satox::blockchain {

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

// Network configuration
struct NetworkConfig {
    std::string host = "127.0.0.1";
    int port = 7777;
    std::string username;
    std::string password;
    int timeout = 30;
    bool enableSSL = false;
    std::string sslCertPath;
    std::string sslKeyPath;
    nlohmann::json additionalConfig;
};

// Configuration structures
struct BlockchainConfig {
    std::string name = "satox_blockchain";
    bool enableLogging = true;
    std::string logPath = "logs/components/blockchain/";
    NetworkConfig networkConfig;
    int maxConnections = 10;
    int timeout = 30;
    bool enableKawpowValidation = true;
    nlohmann::json additionalConfig;
};

// Statistics structures
struct BlockchainStats {
    int totalOperations = 0;
    int successfulOperations = 0;
    int failedOperations = 0;
    int activeConnections = 0;
    uint64_t currentHeight = 0;
    uint64_t totalBlocks = 0;
    uint64_t totalTransactions = 0;
    std::chrono::system_clock::time_point lastOperation;
    double averageOperationTime = 0.0;
    nlohmann::json additionalStats;
};

// Result structures
struct BlockchainResult {
    bool success = false;
    std::string error;
    nlohmann::json data;
    std::chrono::milliseconds duration;
};

// Component status
enum class BlockchainStatus {
    UNKNOWN,
    INITIALIZING,
    INITIALIZED,
    RUNNING,
    STOPPED,
    ERROR,
    DISABLED
};

// Component health
struct BlockchainHealth {
    BlockchainStatus status = BlockchainStatus::UNKNOWN;
    bool healthy = false;
    std::string message;
    std::chrono::system_clock::time_point lastCheck;
    int consecutiveFailures = 0;
    nlohmann::json details;
};

} // namespace satox::blockchain
