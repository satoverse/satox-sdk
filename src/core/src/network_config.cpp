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

#include "satox/core/network_config.h"
#include <fstream>
#include <filesystem>
#include <stdexcept>

namespace satox {
namespace core {

NetworkConfigManager& NetworkConfigManager::getInstance() {
    static NetworkConfigManager instance;
    return instance;
}

NetworkConfigManager::NetworkConfigManager() : isInitialized_(false) {
    // Set default configuration for mainnet
    currentConfig_ = NetworkConfig{
        NetworkType::MAINNET,
        "localhost",  // Default RPC host
        8332,        // Default RPC port
        "",          // RPC user
        "",          // RPC password
        false,       // Use SSL
        30,          // Timeout in seconds
        3            // Retry attempts
    };
}

bool NetworkConfigManager::loadConfig(const std::string& configPath) {
    try {
        if (!std::filesystem::exists(configPath)) {
            // Create default config file if it doesn't exist
            return saveConfig(configPath);
        }

        std::ifstream file(configPath);
        if (!file.is_open()) {
            return false;
        }

        nlohmann::json j;
        file >> j;

        currentConfig_.type = stringToNetworkType(j["network_type"]);
        currentConfig_.rpcHost = j["rpc_host"];
        currentConfig_.rpcPort = j["rpc_port"];
        currentConfig_.rpcUser = j["rpc_user"];
        currentConfig_.rpcPassword = j["rpc_password"];
        currentConfig_.useSSL = j["use_ssl"];
        currentConfig_.timeoutSeconds = j["timeout_seconds"];
        currentConfig_.retryAttempts = j["retry_attempts"];

        isInitialized_ = true;
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool NetworkConfigManager::saveConfig(const std::string& configPath) {
    try {
        // Create directory if it doesn't exist
        std::filesystem::create_directories(
            std::filesystem::path(configPath).parent_path());

        std::ofstream file(configPath);
        if (!file.is_open()) {
            return false;
        }

        nlohmann::json j;
        j["network_type"] = networkTypeToString(currentConfig_.type);
        j["rpc_host"] = currentConfig_.rpcHost;
        j["rpc_port"] = currentConfig_.rpcPort;
        j["rpc_user"] = currentConfig_.rpcUser;
        j["rpc_password"] = currentConfig_.rpcPassword;
        j["use_ssl"] = currentConfig_.useSSL;
        j["timeout_seconds"] = currentConfig_.timeoutSeconds;
        j["retry_attempts"] = currentConfig_.retryAttempts;

        file << j.dump(4);
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

const NetworkConfig& NetworkConfigManager::getConfig() const {
    return currentConfig_;
}

void NetworkConfigManager::updateConfig(const NetworkConfig& config) {
    currentConfig_ = config;
    isInitialized_ = true;
}

std::string NetworkConfigManager::networkTypeToString(NetworkType type) {
    switch (type) {
        case NetworkType::MAINNET:
            return "mainnet";
        case NetworkType::TESTNET:
            return "testnet";
        case NetworkType::REGTEST:
            return "regtest";
        default:
            return "unknown";
    }
}

NetworkType NetworkConfigManager::stringToNetworkType(const std::string& type) {
    if (type == "mainnet") return NetworkType::MAINNET;
    if (type == "testnet") return NetworkType::TESTNET;
    if (type == "regtest") return NetworkType::REGTEST;
    throw std::invalid_argument("Invalid network type: " + type);
}

} // namespace core
} // namespace satox 