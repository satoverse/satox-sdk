/**
 * @file satox.hpp
 * @brief Satox satox-sdk - Satox
 * @details This header provides Satox functionality for the Satox satox-sdk.
 *
 * @defgroup satox-sdk satox-sdk Component
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
 * @file satox.hpp
 * @brief Satox satox-sdk - Satox
 * @details This header provides Satox functionality for the Satox satox-sdk.
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

#pragma once

/**
 * @file satox.hpp
 * @brief Main header file for the Satox SDK
 * @version 1.0.0
 * @date 
 * 
 * This header provides access to all Satox SDK components:
 * - Core functionality
 * - Blockchain operations
 * - Database management
 * - Asset management
 * - Security (PQC + General)
 * - NFT operations
 * - IPFS integration
 * - Network operations
 * - Wallet management
 */

// Version information
#define SATOX_SDK_VERSION_MAJOR 1
#define SATOX_SDK_VERSION_MINOR 0
#define SATOX_SDK_VERSION_PATCH 0
#define SATOX_SDK_VERSION "1.0.0"

// Core component
#include "satox/core/core_manager.hpp"

// Blockchain component
#include "satox/blockchain/blockchain_manager.hpp"

// Database component
#include "satox/database/database_manager.hpp"

// Asset component
#include "satox/asset/asset_manager.hpp"

// Security component
#include "satox/security/security_manager.hpp"

// NFT component
#include "satox/nft/nft_manager.hpp"

// IPFS component
#include "satox/ipfs/ipfs_manager.hpp"

// Network component
#include "satox/network/network_manager.hpp"

// Wallet component
#include "satox/wallet/wallet_manager.hpp"

// Common types and utilities
#include "satox/core/types.hpp"
#include "satox/core/error.hpp"

/** @ingroup satox-sdk */
namespace satox {

/**
 * @brief Main SDK class providing access to all components
 * 
 * This class serves as the main entry point for the Satox SDK,
 * providing access to all component managers and unified functionality.
 */
/** @ingroup satox-sdk */
class SDK {
public:
    /**
     * @brief Get the SDK instance (singleton pattern)
     * @return Reference to the SDK instance
     */
    static SDK& getInstance();

    /**
     * @brief Initialize the SDK with configuration
     * @param config Configuration for all components
     * @return true if initialization successful, false otherwise
     */
    bool initialize(const nlohmann::json& config = {});

    /**
     * @brief Shutdown the SDK and all components
     */
    void shutdown();

    /**
     * @brief Check if the SDK is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const;

    /**
     * @brief Get the core manager
     * @return Reference to the core manager
     */
    core::CoreManager& getCoreManager();

    /**
     * @brief Get the blockchain manager
     * @return Reference to the blockchain manager
     */
    blockchain::BlockchainManager& getBlockchainManager();

    /**
     * @brief Get the database manager
     * @return Reference to the database manager
     */
    database::DatabaseManager& getDatabaseManager();

    /**
     * @brief Get the asset manager
     * @return Reference to the asset manager
     */
    asset::AssetManager& getAssetManager();

    /**
     * @brief Get the security manager
     * @return Reference to the security manager
     */
    security::SecurityManager& getSecurityManager();

    /**
     * @brief Get the NFT manager
     * @return Reference to the NFT manager
     */
    nft::NFTManager& getNFTManager();

    /**
     * @brief Get the IPFS manager
     * @return Reference to the IPFS manager
     */
    ipfs::IPFSManager& getIPFSManager();

    /**
     * @brief Get the network manager
     * @return Reference to the network manager
     */
    network::NetworkManager& getNetworkManager();

    /**
     * @brief Get the wallet manager
     * @return Reference to the wallet manager
     */
    wallet::WalletManager& getWalletManager();

    /**
     * @brief Get SDK version information
     * @return Version string
     */
    std::string getVersion() const;

    /**
     * @brief Get SDK health status
     * @return Health status JSON object
     */
    nlohmann::json getHealthStatus() const;

    /**
     * @brief Get SDK statistics
     * @return Statistics JSON object
     */
    nlohmann::json getStats() const;

private:
    SDK() = default;
    ~SDK() = default;
    SDK(const SDK&) = delete;
    SDK& operator=(const SDK&) = delete;

    bool initialized_ = false;
    nlohmann::json config_;
    std::chrono::system_clock::time_point start_time_;
};

// Convenience functions for quick access to managers
/** @ingroup satox-sdk */
namespace managers {
    inline core::CoreManager& core() { return SDK::getInstance().getCoreManager(); }
    inline blockchain::BlockchainManager& blockchain() { return SDK::getInstance().getBlockchainManager(); }
    inline database::DatabaseManager& database() { return SDK::getInstance().getDatabaseManager(); }
    inline asset::AssetManager& asset() { return SDK::getInstance().getAssetManager(); }
    inline security::SecurityManager& security() { return SDK::getInstance().getSecurityManager(); }
    inline nft::NFTManager& nft() { return SDK::getInstance().getNFTManager(); }
    inline ipfs::IPFSManager& ipfs() { return SDK::getInstance().getIPFSManager(); }
    inline network::NetworkManager& network() { return SDK::getInstance().getNetworkManager(); }
    inline wallet::WalletManager& wallet() { return SDK::getInstance().getWalletManager(); }
}

} // namespace satox

// Global SDK instance access
#define SATOX_SDK satox::SDK::getInstance()
#define SATOX_CORE satox::managers::core()
#define SATOX_BLOCKCHAIN satox::managers::blockchain()
#define SATOX_DATABASE satox::managers::database()
#define SATOX_ASSET satox::managers::asset()
#define SATOX_SECURITY satox::managers::security()
#define SATOX_NFT satox::managers::nft()
#define SATOX_IPFS satox::managers::ipfs()
#define SATOX_NETWORK satox::managers::network()
#define SATOX_WALLET satox::managers::wallet() 
