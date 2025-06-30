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
#include <functional>

namespace satox::asset {

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

// Asset state
enum class AssetState {
    UNINITIALIZED,
    INITIALIZING,
    INITIALIZED,
    ERROR,
    SHUTDOWN
};

// Asset type
enum class AssetType {
    TOKEN,
    NFT,
    UTILITY,
    SECURITY,
    CURRENCY
};

// Asset status
enum class AssetStatus {
    ACTIVE,
    FROZEN,
    DESTROYED,
    PENDING,
    REJECTED
};

// Asset configuration
struct AssetConfig {
    AssetType type = AssetType::TOKEN;
    std::string name;
    int maxAssets = 1000;
    int timeout = 30;
    bool enableLogging = true;
    std::string logPath;
    nlohmann::json additionalConfig;
};

// Asset statistics
struct AssetStats {
    int totalOperations = 0;
    int successfulOperations = 0;
    int failedOperations = 0;
    uint64_t activeAssets = 0;
    uint64_t frozenAssets = 0;
    uint64_t destroyedAssets = 0;
    std::chrono::system_clock::time_point lastOperation;
    double averageOperationTime = 0.0;
    nlohmann::json additionalStats;
};

// Asset result
struct AssetResult {
    std::string operation;
    bool success;
    std::string error_message;
    nlohmann::json data;
    std::chrono::system_clock::time_point timestamp;
};

// Asset creation request
struct AssetCreationRequest {
    std::string name;
    std::string symbol;
    uint64_t amount;
    uint8_t units;
    bool reissuable;
    std::string owner_address;
    nlohmann::json metadata;
};

// Asset transfer request
struct AssetTransferRequest {
    std::string asset_name;
    std::string from_address;
    std::string to_address;
    uint64_t amount;
    std::string transaction_id;
    uint64_t timestamp;
};

// Asset reissue request
struct AssetReissueRequest {
    std::string asset_name;
    std::string owner_address;
    uint64_t amount;
    bool reissuable;
    std::string transaction_id;
    uint64_t timestamp;
};

// Asset burn request
struct AssetBurnRequest {
    std::string asset_name;
    std::string owner_address;
    uint64_t amount;
    std::string transaction_id;
    uint64_t timestamp;
};

// Asset metadata
struct AssetMetadata {
    std::string name;
    std::string symbol;
    std::string description;
    std::string issuer;
    uint64_t total_supply;
    uint8_t decimals;
    bool reissuable;
    bool frozen;
    nlohmann::json additional_data;
};

// Asset verification result
struct AssetVerificationResult {
    bool valid;
    std::string error_message;
    std::vector<std::string> warnings;
    nlohmann::json details;
};

// Callback types
using AssetCallback = std::function<void(const std::string& event, bool success)>;
using OperationCallback = std::function<void(const std::string& operation, bool success)>;
using ErrorCallback = std::function<void(const std::string& operation, const std::string& error)>;

} // namespace satox::asset
