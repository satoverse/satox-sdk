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
#include <vector>
#include <map>
#include <chrono>
#include <nlohmann/json.hpp>

namespace satox::database {

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
struct DatabaseConfig {
    std::string name = "satox_database";
    bool enableLogging = true;
    std::string logPath = "logs/components/database/";
    int maxConnections = 10;
    int connectionTimeout = 5000;
    nlohmann::json additionalConfig;
};

// Statistics structures
struct DatabaseStats {
    int totalOperations = 0;
    int successfulOperations = 0;
    int failedOperations = 0;
    std::chrono::system_clock::time_point lastOperation;
    double averageOperationTime = 0.0;
    nlohmann::json additionalStats;
};

// Result structures
struct DatabaseResult {
    bool success = false;
    std::string error;
    nlohmann::json data;
    std::chrono::milliseconds duration;

    bool operator==(const DatabaseResult& other) const {
        return success == other.success &&
               error == other.error &&
               data == other.data &&
               duration == other.duration;
    }
};

// GoogleTest PrintTo for DatabaseResult
inline void PrintTo(const DatabaseResult& result, std::ostream* os) {
    *os << "DatabaseResult{";
    *os << "success: " << result.success << ", ";
    *os << "error: '" << result.error << "', ";
    *os << "data: " << result.data.dump() << ", ";
    *os << "duration: " << result.duration.count() << "ms";
    *os << "}";
}

// Component status
enum class DatabaseStatus {
    UNKNOWN,
    INITIALIZING,
    INITIALIZED,
    RUNNING,
    STOPPED,
    ERROR,
    DISABLED
};

// Component health
struct DatabaseHealth {
    DatabaseStatus status = DatabaseStatus::UNKNOWN;
    bool healthy = false;
    std::string message;
    std::chrono::system_clock::time_point lastCheck;
    int consecutiveFailures = 0;
    nlohmann::json details;
};

// Database storage structures
struct TableData {
    nlohmann::json schema;
    std::vector<nlohmann::json> records;
    std::map<std::string, std::vector<std::string>> indexes;
};

struct DatabaseData {
    std::string name;
    std::map<std::string, TableData> tables;
};

} // namespace satox::database
