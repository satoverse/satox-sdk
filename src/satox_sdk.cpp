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
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <filesystem>
#include <iostream>

namespace satox {

// Remove LoggerManager and logger_manager_ usage for clean build

SDK& SDK::getInstance() {
    static SDK instance;
    return instance;
}

bool SDK::initialize(const nlohmann::json& config) {
    if (initialized_) {
        spdlog::warn("SDK already initialized");
        return true;
    }

    try {
        std::cout << "DEBUG: Starting SDK initialization" << std::endl;
        spdlog::info("Initializing Satox SDK v{}", getVersion());
        start_time_ = std::chrono::system_clock::now();
        config_ = config;

        // Create logs directory structure
        std::filesystem::create_directories("logs/components");
        std::filesystem::create_directories("logs/bindings");
        std::filesystem::create_directories("logs/ci_cd");
        std::filesystem::create_directories("logs/monitoring");

        // Register main SDK logger
        // logger_manager_.registerLogger("satox_sdk");

        // Initialize core component first
        core::CoreConfig coreConfig;
        if (config.contains("core")) {
            auto coreJson = config["core"];
            if (coreJson.contains("data_dir")) coreConfig.data_dir = coreJson["data_dir"];
            if (coreJson.contains("network")) coreConfig.network = coreJson["network"];
            if (coreJson.contains("network_config")) coreConfig.network_config = coreJson["network_config"];
            if (coreJson.contains("enable_mining")) coreConfig.enable_mining = coreJson["enable_mining"];
            if (coreJson.contains("enable_sync")) coreConfig.enable_sync = coreJson["enable_sync"];
            if (coreJson.contains("sync_interval_ms")) coreConfig.sync_interval_ms = coreJson["sync_interval_ms"];
            if (coreJson.contains("mining_threads")) coreConfig.mining_threads = coreJson["mining_threads"];
            if (coreJson.contains("rpc_endpoint")) coreConfig.rpc_endpoint = coreJson["rpc_endpoint"];
            if (coreJson.contains("rpc_username")) coreConfig.rpc_username = coreJson["rpc_username"];
            if (coreJson.contains("rpc_password")) coreConfig.rpc_password = coreJson["rpc_password"];
            if (coreJson.contains("timeout_ms")) coreConfig.timeout_ms = coreJson["timeout_ms"];
        }
        
        // Set default values for required fields if not provided
        if (coreConfig.data_dir.empty()) coreConfig.data_dir = "/tmp/satox_data";
        if (coreConfig.network.empty()) coreConfig.network = "testnet";
        if (coreConfig.sync_interval_ms == 0) coreConfig.sync_interval_ms = 1000;
        if (coreConfig.mining_threads == 0) coreConfig.mining_threads = 1;
        if (coreConfig.timeout_ms == 0) coreConfig.timeout_ms = 30000;
        
        if (!core::CoreManager::getInstance().initialize(coreConfig)) {
            spdlog::error("Failed to initialize core manager");
            return false;
        }
        // logger_manager_.registerLogger("core_manager");

        // Initialize database component
        database::DatabaseConfig dbConfig;
        if (config.contains("database")) {
            auto dbJson = config["database"];
            if (dbJson.contains("name")) dbConfig.name = dbJson["name"];
            if (dbJson.contains("enableLogging")) dbConfig.enableLogging = dbJson["enableLogging"];
            if (dbJson.contains("logPath")) dbConfig.logPath = dbJson["logPath"];
            if (dbJson.contains("maxConnections")) dbConfig.maxConnections = dbJson["maxConnections"];
            if (dbJson.contains("connectionTimeout")) dbConfig.connectionTimeout = dbJson["connectionTimeout"];
        }
        if (!database::DatabaseManager::getInstance().initialize(dbConfig)) {
            spdlog::error("Failed to initialize database manager");
            return false;
        }
        // logger_manager_.registerLogger("database_manager");

        // Initialize security component
        security::SecurityConfig secConfig;
        if (config.contains("security")) {
            auto secJson = config["security"];
            if (secJson.contains("name")) secConfig.name = secJson["name"];
            if (secJson.contains("enablePQC")) secConfig.enablePQC = secJson["enablePQC"];
            if (secJson.contains("enableInputValidation")) secConfig.enableInputValidation = secJson["enableInputValidation"];
            if (secJson.contains("enableRateLimiting")) secConfig.enableRateLimiting = secJson["enableRateLimiting"];
            if (secJson.contains("enableLogging")) secConfig.enableLogging = secJson["enableLogging"];
            if (secJson.contains("logPath")) secConfig.logPath = secJson["logPath"];
        }
        if (!security::SecurityManager::getInstance().initialize(secConfig)) {
            spdlog::error("Failed to initialize security manager");
            return false;
        }
        // logger_manager_.registerLogger("security_manager");

        // Initialize NFT component
        if (!nft::NFTManager::getInstance().initialize()) {
            spdlog::error("Failed to initialize NFT manager");
            return false;
        }
        // logger_manager_.registerLogger("nft_manager");

        // TODO: Initialize other components when their getInstance() methods are properly implemented
        // Initialize blockchain component
        // blockchain::BlockchainConfig blockchainConfig;
        // if (config.contains("blockchain")) {
        //     auto blockchainJson = config["blockchain"];
        //     if (blockchainJson.contains("name")) blockchainConfig.name = blockchainJson["name"];
        //     if (blockchainJson.contains("network")) blockchainConfig.networkConfig.host = blockchainJson["network"];
        //     if (blockchainJson.contains("enableLogging")) blockchainConfig.enableLogging = blockchainJson["enableLogging"];
        //     if (blockchainJson.contains("logPath")) blockchainConfig.logPath = blockchainJson["logPath"];
        // }
        // if (!blockchain::BlockchainManager::getInstance().initialize(blockchainConfig)) {
        //     spdlog::error("Failed to initialize blockchain manager");
        //     return false;
        // }

        // Initialize asset component
        // asset::AssetConfig assetConfig;
        // if (config.contains("asset")) {
        //     auto assetJson = config["asset"];
        //     if (assetJson.contains("name")) assetConfig.name = assetJson["name"];
        //     if (assetJson.contains("enableLogging")) assetConfig.enableLogging = assetJson["enableLogging"];
        //     if (assetJson.contains("logPath")) assetConfig.logPath = assetJson["logPath"];
        // }
        // if (!asset::AssetManager::getInstance().initialize(assetConfig)) {
        //     spdlog::error("Failed to initialize asset manager");
        //     return false;
        // }

        // Initialize IPFS component
        // ipfs::Config ipfsConfig;
        // if (config.contains("ipfs")) {
        //     auto ipfsJson = config["ipfs"];
        //     if (ipfsJson.contains("name")) ipfsConfig.api_endpoint = ipfsJson["name"];
        //     if (ipfsJson.contains("enableLogging")) ipfsConfig.enable_pinning = ipfsJson["enableLogging"];
        //     if (ipfsJson.contains("logPath")) ipfsConfig.pinning_service = ipfsJson["logPath"];
        // }
        // if (!ipfs::IPFSManager::getInstance().initialize(ipfsConfig)) {
        //     spdlog::error("Failed to initialize IPFS manager");
        //     return false;
        // }

        // Initialize network component
        // network::NetworkManager::NetworkConfig networkConfig;
        // if (config.contains("network")) {
        //     auto networkJson = config["network"];
        //     if (networkJson.contains("name")) networkConfig.host = networkJson["name"];
        //     if (networkJson.contains("enableLogging")) networkConfig.enable_ssl = networkJson["enableLogging"];
        //     if (networkJson.contains("logPath")) networkConfig.ssl_cert_path = networkJson["logPath"];
        // }
        // if (!network::NetworkManager::getInstance().initialize(networkConfig)) {
        //     spdlog::error("Failed to initialize network manager");
        //     return false;
        // }

        // Initialize wallet component
        // if (!wallet::WalletManager::getInstance().initialize()) {
        //     spdlog::error("Failed to initialize wallet manager");
        //     return false;
        // }

        initialized_ = true;
        std::cout << "DEBUG: SDK initialization completed successfully" << std::endl;
        spdlog::info("Satox SDK initialized successfully");
        return true;

    } catch (const std::exception& e) {
        spdlog::error("Exception during SDK initialization: {}", e.what());
        return false;
    }
}

void SDK::shutdown() {
    if (!initialized_) {
        spdlog::warn("SDK not initialized");
        return;
    }

    try {
        std::cout << "DEBUG: Starting SDK shutdown" << std::endl;
        spdlog::info("Shutting down Satox SDK");

        // Shutdown components in reverse order
        // wallet::WalletManager::getInstance().shutdown();
        // network::NetworkManager::getInstance().shutdown();
        // ipfs::IPFSManager::getInstance().shutdown();
        try {
            nft::NFTManager::getInstance().shutdown();
        } catch (const std::exception& e) {
            spdlog::warn("Error shutting down NFT manager: {}", e.what());
        }
        // asset::AssetManager::getInstance().shutdown();
        // blockchain::BlockchainManager::getInstance().shutdown();
        try {
            security::SecurityManager::getInstance().shutdown();
        } catch (const std::exception& e) {
            spdlog::warn("Error shutting down Security manager: {}", e.what());
        }
        try {
            database::DatabaseManager::getInstance().shutdown();
        } catch (const std::exception& e) {
            spdlog::warn("Error shutting down Database manager: {}", e.what());
        }
        try {
            core::CoreManager::getInstance().shutdown();
        } catch (const std::exception& e) {
            spdlog::warn("Error shutting down Core manager: {}", e.what());
        }

        initialized_ = false;
        std::cout << "DEBUG: All components shut down, now managing loggers" << std::endl;
        spdlog::info("Satox SDK shut down successfully");

        // Use explicit logger management instead of spdlog::drop_all()
        // logger_manager_.dropAllLoggers();

    } catch (const std::exception& e) {
        spdlog::error("Exception during SDK shutdown: {}", e.what());
        initialized_ = false;
    }
}

bool SDK::isInitialized() const {
    return initialized_;
}

core::CoreManager& SDK::getCoreManager() {
    if (!initialized_) {
        throw std::runtime_error("SDK not initialized");
    }
    return core::CoreManager::getInstance();
}

blockchain::BlockchainManager& SDK::getBlockchainManager() {
    if (!initialized_) {
        throw std::runtime_error("SDK not initialized");
    }
    return blockchain::BlockchainManager::getInstance();
}

database::DatabaseManager& SDK::getDatabaseManager() {
    if (!initialized_) {
        throw std::runtime_error("SDK not initialized");
    }
    return database::DatabaseManager::getInstance();
}

asset::AssetManager& SDK::getAssetManager() {
    if (!initialized_) {
        throw std::runtime_error("SDK not initialized");
    }
    return asset::AssetManager::getInstance();
}

security::SecurityManager& SDK::getSecurityManager() {
    if (!initialized_) {
        throw std::runtime_error("SDK not initialized");
    }
    return security::SecurityManager::getInstance();
}

nft::NFTManager& SDK::getNFTManager() {
    if (!initialized_) {
        throw std::runtime_error("SDK not initialized");
    }
    return nft::NFTManager::getInstance();
}

ipfs::IPFSManager& SDK::getIPFSManager() {
    if (!initialized_) {
        throw std::runtime_error("SDK not initialized");
    }
    return ipfs::IPFSManager::getInstance();
}

network::NetworkManager& SDK::getNetworkManager() {
    if (!initialized_) {
        throw std::runtime_error("SDK not initialized");
    }
    return network::NetworkManager::getInstance();
}

wallet::WalletManager& SDK::getWalletManager() {
    if (!initialized_) {
        throw std::runtime_error("SDK not initialized");
    }
    return wallet::WalletManager::getInstance();
}

std::string SDK::getVersion() const {
    return SATOX_SDK_VERSION;
}

nlohmann::json SDK::getHealthStatus() const {
    nlohmann::json status;
    
    try {
        status["sdk"] = {
            {"initialized", initialized_},
            {"version", getVersion()},
            {"uptime", std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now() - start_time_).count()}
        };

        status["components"] = nlohmann::json::object();
        
        // Only include components that are properly initialized
        if (initialized_) {
            try {
                // Core manager doesn't have getHealthStatus, use isHealthy instead
                status["components"]["core"]["healthy"] = core::CoreManager::getInstance().isHealthy();
                status["components"]["core"]["initialized"] = core::CoreManager::getInstance().isInitialized();
            } catch (const std::exception& e) {
                status["components"]["core"]["healthy"] = false;
                status["components"]["core"]["initialized"] = false;
                status["components"]["core"]["error"] = e.what();
            }
            
            try {
                status["components"]["database"] = database::DatabaseManager::getInstance().getHealthStatus();
            } catch (const std::exception& e) {
                status["components"]["database"]["healthy"] = false;
                status["components"]["database"]["initialized"] = false;
                status["components"]["database"]["error"] = e.what();
            }
            
            try {
                status["components"]["security"]["healthy"] = security::SecurityManager::getInstance().isHealthy();
                status["components"]["security"]["initialized"] = security::SecurityManager::getInstance().isInitialized();
            } catch (const std::exception& e) {
                status["components"]["security"]["healthy"] = false;
                status["components"]["security"]["initialized"] = false;
                status["components"]["security"]["error"] = e.what();
            }
            
            try {
                status["components"]["nft"]["healthy"] = nft::NFTManager::getInstance().isHealthy();
                status["components"]["nft"]["initialized"] = nft::NFTManager::getInstance().isInitialized();
            } catch (const std::exception& e) {
                status["components"]["nft"]["healthy"] = false;
                status["components"]["nft"]["initialized"] = false;
                status["components"]["nft"]["error"] = e.what();
            }
            
            // TODO: Add other components when their getInstance() methods are properly implemented
            // For now, mark them as not available
            status["components"]["blockchain"]["healthy"] = false;
            status["components"]["blockchain"]["initialized"] = false;
            status["components"]["blockchain"]["error"] = "Not implemented";
            
            status["components"]["asset"]["healthy"] = false;
            status["components"]["asset"]["initialized"] = false;
            status["components"]["asset"]["error"] = "Not implemented";
            
            status["components"]["ipfs"]["healthy"] = false;
            status["components"]["ipfs"]["initialized"] = false;
            status["components"]["ipfs"]["error"] = "Not implemented";
            
            status["components"]["network"]["healthy"] = false;
            status["components"]["network"]["initialized"] = false;
            status["components"]["network"]["error"] = "Not implemented";
            
            status["components"]["wallet"]["healthy"] = false;
            status["components"]["wallet"]["initialized"] = false;
            status["components"]["wallet"]["error"] = "Not implemented";
        }
        
    } catch (const std::exception& e) {
        status["error"] = e.what();
    }
    
    return status;
}

nlohmann::json SDK::getStats() const {
    nlohmann::json stats;
    
    try {
        stats["sdk"] = {
            {"initialized", initialized_},
            {"version", getVersion()},
            {"uptime_seconds", std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now() - start_time_).count()}
        };

        stats["components"] = nlohmann::json::object();
        
        // Only include components that are properly initialized
        if (initialized_) {
            try {
                stats["components"]["core"]["initialized"] = core::CoreManager::getInstance().isInitialized();
                stats["components"]["core"]["healthy"] = core::CoreManager::getInstance().isHealthy();
            } catch (const std::exception& e) {
                stats["components"]["core"]["initialized"] = false;
                stats["components"]["core"]["healthy"] = false;
                stats["components"]["core"]["error"] = e.what();
            }
            
            try {
                auto dbStats = database::DatabaseManager::getInstance().getStats();
                stats["components"]["database"]["initialized"] = true;
                stats["components"]["database"]["total_operations"] = dbStats.totalOperations;
                stats["components"]["database"]["successful_operations"] = dbStats.successfulOperations;
                stats["components"]["database"]["failed_operations"] = dbStats.failedOperations;
            } catch (const std::exception& e) {
                stats["components"]["database"]["initialized"] = false;
                stats["components"]["database"]["error"] = e.what();
            }
            
            try {
                stats["components"]["security"]["initialized"] = security::SecurityManager::getInstance().isInitialized();
                stats["components"]["security"]["healthy"] = security::SecurityManager::getInstance().isHealthy();
            } catch (const std::exception& e) {
                stats["components"]["security"]["initialized"] = false;
                stats["components"]["security"]["healthy"] = false;
                stats["components"]["security"]["error"] = e.what();
            }
            
            try {
                stats["components"]["nft"]["initialized"] = nft::NFTManager::getInstance().isInitialized();
                stats["components"]["nft"]["healthy"] = nft::NFTManager::getInstance().isHealthy();
            } catch (const std::exception& e) {
                stats["components"]["nft"]["initialized"] = false;
                stats["components"]["nft"]["healthy"] = false;
                stats["components"]["nft"]["error"] = e.what();
            }
            
            // TODO: Add other components when their getInstance() methods are properly implemented
            // For now, mark them as not available
            stats["components"]["blockchain"]["initialized"] = false;
            stats["components"]["blockchain"]["error"] = "Not implemented";
            
            stats["components"]["asset"]["initialized"] = false;
            stats["components"]["asset"]["error"] = "Not implemented";
            
            stats["components"]["ipfs"]["initialized"] = false;
            stats["components"]["ipfs"]["error"] = "Not implemented";
            
            stats["components"]["network"]["initialized"] = false;
            stats["components"]["network"]["error"] = "Not implemented";
            
            stats["components"]["wallet"]["initialized"] = false;
            stats["components"]["wallet"]["error"] = "Not implemented";
        }
        
    } catch (const std::exception& e) {
        stats["error"] = e.what();
    }
    
    return stats;
}

} // namespace satox 