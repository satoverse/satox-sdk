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

#include "satox/satox.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    try {
        std::cout << "=== Satox SDK Example ===" << std::endl;
        std::cout << "Version: " << SATOX_SDK_VERSION << std::endl;
        std::cout << std::endl;

        // Initialize SDK with configuration
        nlohmann::json config = {
            {"core", {
                {"name", "example_core"},
                {"enableLogging", true},
                {"logPath", "logs/components/core/"}
            }},
            {"database", {
                {"name", "example_database"},
                {"type", "memory"},
                {"enableLogging", true},
                {"logPath", "logs/components/database/"}
            }},
            {"security", {
                {"name", "example_security"},
                {"enablePQC", true},
                {"enableInputValidation", true},
                {"enableRateLimiting", true},
                {"enableLogging", true},
                {"logPath", "logs/components/security/"}
            }},
            {"blockchain", {
                {"name", "example_blockchain"},
                {"network", "testnet"},
                {"enableLogging", true},
                {"logPath", "logs/components/blockchain/"}
            }},
            {"asset", {
                {"name", "example_asset"},
                {"enableLogging", true},
                {"logPath", "logs/components/asset/"}
            }},
            {"nft", {
                {"name", "example_nft"},
                {"enableLogging", true},
                {"logPath", "logs/components/nft/"}
            }},
            {"ipfs", {
                {"name", "example_ipfs"},
                {"enableLogging", true},
                {"logPath", "logs/components/ipfs/"}
            }},
            {"network", {
                {"name", "example_network"},
                {"enableLogging", true},
                {"logPath", "logs/components/network/"}
            }},
            {"wallet", {
                {"name", "example_wallet"},
                {"enableLogging", true},
                {"logPath", "logs/components/wallet/"}
            }}
        };

        std::cout << "Initializing Satox SDK..." << std::endl;
        auto& sdk = satox::SDK::getInstance();
        
        if (!sdk.initialize(config)) {
            std::cerr << "Failed to initialize SDK!" << std::endl;
            return 1;
        }

        std::cout << "SDK initialized successfully!" << std::endl;
        std::cout << std::endl;

        // Demonstrate component access using different methods
        std::cout << "=== Component Access Examples ===" << std::endl;

        // Method 1: Direct SDK access
        std::cout << "1. Direct SDK access:" << std::endl;
        auto& core = sdk.getCoreManager();
        auto& database = sdk.getDatabaseManager();
        auto& security = sdk.getSecurityManager();
        auto& nft = satox::managers::nft();
        std::cout << "   - Core manager: " << (core.isInitialized() ? "OK" : "FAIL") << std::endl;
        std::cout << "   - Database manager: " << (database.isInitialized() ? "OK" : "FAIL") << std::endl;
        std::cout << "   - Security manager: " << (security.isInitialized() ? "OK" : "FAIL") << std::endl;
        std::cout << "   - NFT manager: " << (nft.isInitialized() ? "OK" : "FAIL") << std::endl;

        // Method 2: SDK convenience functions
        std::cout << "2. SDK convenience functions:" << std::endl;
        // TODO: Add other components when their getInstance() methods are properly implemented
        // auto& blockchain = satox::managers::blockchain();
        // auto& asset = satox::managers::asset();
        // auto& ipfs = satox::managers::ipfs();
        // auto& network = satox::managers::network();
        // auto& wallet = satox::managers::wallet();
        // std::cout << "   - Blockchain manager: " << (blockchain.isInitialized() ? "OK" : "FAIL") << std::endl;
        // std::cout << "   - Asset manager: " << (asset.isInitialized() ? "OK" : "FAIL") << std::endl;
        // std::cout << "   - IPFS manager: " << (ipfs.isInitialized() ? "OK" : "FAIL") << std::endl;
        // std::cout << "   - Network manager: " << (network.isInitialized() ? "OK" : "FAIL") << std::endl;
        // std::cout << "   - Wallet manager: " << (wallet.isInitialized() ? "OK" : "FAIL") << std::endl;

        std::cout << std::endl;

        // Demonstrate basic operations
        std::cout << "=== Basic Operations ===" << std::endl;

        // Core operations
        std::cout << "1. Core operations:" << std::endl;
        auto coreStats = core.getStats();
        std::cout << "   - Core components: " << coreStats.total_components << std::endl;
        std::cout << "   - Core health: " << (core.isHealthy() ? "HEALTHY" : "UNHEALTHY") << std::endl;

        // Security operations
        std::cout << "2. Security operations:" << std::endl;
        if (security.validateEmail("user@example.com")) {
            std::cout << "   - Email validation: OK" << std::endl;
        }
        if (security.checkRateLimit("user123", "login")) {
            std::cout << "   - Rate limit check: OK" << std::endl;
        }

        // Database operations
        std::cout << "3. Database operations:" << std::endl;
        auto dbHealth = database.getHealthStatus();
        std::cout << "   - Database health: " << (database.isHealthy() ? "HEALTHY" : "UNHEALTHY") << std::endl;
        std::cout << "   - Database type: " << dbHealth["config"]["type"] << std::endl;

        std::cout << std::endl;

        // Demonstrate health monitoring
        std::cout << "=== Health Monitoring ===" << std::endl;
        auto health = sdk.getHealthStatus();
        std::cout << "SDK Health Status:" << std::endl;
        std::cout << "  - Initialized: " << (health["sdk"]["initialized"] ? "YES" : "NO") << std::endl;
        std::cout << "  - Version: " << health["sdk"]["version"] << std::endl;
        std::cout << "  - Uptime: " << health["sdk"]["uptime"] << " seconds" << std::endl;

        std::cout << "Component Health:" << std::endl;
        for (const auto& [name, status] : health["components"].items()) {
            std::cout << "  - " << name << ": " << (status["healthy"] ? "HEALTHY" : "UNHEALTHY") << std::endl;
        }

        std::cout << std::endl;

        // Demonstrate statistics
        std::cout << "=== Statistics ===" << std::endl;
        auto stats = sdk.getStats();
        std::cout << "SDK Statistics:" << std::endl;
        std::cout << "  - Version: " << stats["sdk"]["version"] << std::endl;
        std::cout << "  - Uptime: " << stats["sdk"]["uptime_seconds"] << " seconds" << std::endl;

        std::cout << "Component Statistics:" << std::endl;
        for (const auto& [name, componentStats] : stats["components"].items()) {
            if (componentStats.contains("totalOperations")) {
                std::cout << "  - " << name << ": " << componentStats["totalOperations"] << " operations" << std::endl;
            }
        }

        std::cout << std::endl;

        // Demonstrate error handling
        std::cout << "=== Error Handling ===" << std::endl;
        try {
            // This should throw an exception
            auto& coreManager = sdk.getCoreManager(); // This is valid
            std::cout << "   - Core manager access: OK" << std::endl;
            std::cout << "   - Core manager initialized: " << (coreManager.isInitialized() ? "YES" : "NO") << std::endl;
        } catch (const std::exception& e) {
            std::cout << "   - Core manager access: FAIL - " << e.what() << std::endl;
        }

        std::cout << std::endl;

        // Demonstrate shutdown
        std::cout << "=== Shutdown ===" << std::endl;
        std::cout << "Shutting down SDK..." << std::endl;
        sdk.shutdown();
        std::cout << "SDK shut down successfully!" << std::endl;

        std::cout << std::endl;
        std::cout << "=== Example Complete ===" << std::endl;

        // Check component status
        std::cout << "\n=== Component Status ===" << std::endl;
        std::cout << "   - Core manager: " << (core.isInitialized() ? "OK" : "FAIL") << std::endl;
        std::cout << "   - Database manager: " << (database.isInitialized() ? "OK" : "FAIL") << std::endl;
        std::cout << "   - Security manager: " << (security.isInitialized() ? "OK" : "FAIL") << std::endl;
        std::cout << "   - NFT manager: " << (nft.isInitialized() ? "OK" : "FAIL") << std::endl;
        
        // TODO: Add other components when their getInstance() methods are properly implemented
        // std::cout << "   - Blockchain manager: " << (blockchain.isInitialized() ? "OK" : "FAIL") << std::endl;
        // std::cout << "   - Asset manager: " << (asset.isInitialized() ? "OK" : "FAIL") << std::endl;
        // std::cout << "   - IPFS manager: " << (ipfs.isInitialized() ? "OK" : "FAIL") << std::endl;
        // std::cout << "   - Network manager: " << (network.isInitialized() ? "OK" : "FAIL") << std::endl;
        // std::cout << "   - Wallet manager: " << (wallet.isInitialized() ? "OK" : "FAIL") << std::endl;

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
} 