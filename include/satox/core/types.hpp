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

namespace satox::core {

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

// Configuration structures
// struct CoreConfig {
//     std::string name = "satox_core";
//     bool enableLogging = true;
//     std::string logPath = "logs/components/core/";
//     nlohmann::json additionalConfig;
// };

// Statistics structures for core components
struct CoreComponentStats {
    int totalOperations = 0;
    int successfulOperations = 0;
    int failedOperations = 0;
    std::chrono::system_clock::time_point lastOperation;
    double averageOperationTime = 0.0;
    nlohmann::json additionalStats;
};

// Result structures
struct CoreResult {
    bool success = false;
    std::string error;
    nlohmann::json data;
    std::chrono::milliseconds duration;
};

// Component status
enum class CoreStatus {
    UNKNOWN,
    INITIALIZING,
    INITIALIZED,
    RUNNING,
    STOPPED,
    ERROR,
    DISABLED
};

// Component health
struct CoreHealth {
    CoreStatus status = CoreStatus::UNKNOWN;
    bool healthy = false;
    std::string message;
    std::chrono::system_clock::time_point lastCheck;
    int consecutiveFailures = 0;
    nlohmann::json details;
};

// Basic blockchain types
struct Input {
    std::string txid;
    uint32_t vout;
    std::string script_sig;
    uint64_t amount;
    std::string address;
};

struct Output {
    uint64_t amount;
    std::string script_pubkey;
    std::string address;
    uint32_t n;
};

enum class Status {
    PENDING,
    CONFIRMED,
    FAILED,
    CANCELLED,
    UNKNOWN
};

// System information types
struct OsSystemInfo {
    std::string os_name;
    std::string os_version;
    std::string architecture;
    size_t total_memory;
    size_t available_memory;
    size_t cpu_cores;
    std::string hostname;
    std::chrono::system_clock::time_point timestamp;
};

// Configuration manager types
struct ConfigValue {
    enum class Type {
        STRING,
        INTEGER,
        DOUBLE,
        BOOLEAN,
        JSON
    };
    
    Type type;
    std::string string_value;
    int64_t int_value;
    double double_value;
    bool bool_value;
    nlohmann::json json_value;
    
    ConfigValue() : type(Type::STRING), int_value(0), double_value(0.0), bool_value(false) {}
    ConfigValue(const std::string& val) : type(Type::STRING), string_value(val), int_value(0), double_value(0.0), bool_value(false) {}
    ConfigValue(int64_t val) : type(Type::INTEGER), int_value(val), double_value(0.0), bool_value(false) {}
    ConfigValue(double val) : type(Type::DOUBLE), int_value(0), double_value(val), bool_value(false) {}
    ConfigValue(bool val) : type(Type::BOOLEAN), int_value(0), double_value(0.0), bool_value(val) {}
    ConfigValue(const nlohmann::json& val) : type(Type::JSON), int_value(0), double_value(0.0), bool_value(false), json_value(val) {}
};

struct ConfigSection {
    std::string name;
    std::unordered_map<std::string, ConfigValue> values;
    std::chrono::system_clock::time_point last_modified;
};

} // namespace satox::core
