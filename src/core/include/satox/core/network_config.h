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
#include <nlohmann/json.hpp>

namespace satox {
namespace core {

enum class NetworkType {
    MAINNET,
    TESTNET,
    REGTEST
};

struct NetworkConfig {
    NetworkType type;
    std::string rpcHost;
    uint16_t rpcPort;
    std::string rpcUser;
    std::string rpcPassword;
    bool useSSL;
    int timeoutSeconds;
    int retryAttempts;
};

class NetworkConfigManager {
public:
    static NetworkConfigManager& getInstance();

    // Load configuration from file
    bool loadConfig(const std::string& configPath = "config/network.json");
    
    // Save current configuration to file
    bool saveConfig(const std::string& configPath = "config/network.json");
    
    // Get current network configuration
    const NetworkConfig& getConfig() const;
    
    // Update network configuration
    void updateConfig(const NetworkConfig& config);
    
    // Get network type as string
    static std::string networkTypeToString(NetworkType type);
    
    // Get network type from string
    static NetworkType stringToNetworkType(const std::string& type);

private:
    NetworkConfigManager();
    ~NetworkConfigManager() = default;
    
    NetworkConfigManager(const NetworkConfigManager&) = delete;
    NetworkConfigManager& operator=(const NetworkConfigManager&) = delete;

    NetworkConfig currentConfig_;
    bool isInitialized_;
};

} // namespace core
} // namespace satox 