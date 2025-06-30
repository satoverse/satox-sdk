/**
 * @file azure_config.hpp
 * @brief Azure configuration and connection management
 * @author Satox SDK Team
 * @date $(date '+%Y-%m-%d')
 * @version 1.0.0
 * @license MIT
 *
 * @defgroup cloud_azure Azure Cloud Integration
 * @ingroup cloud
 * @details
 *   Azure integration for Satox SDK. Provides connection, query, transaction, storage, and Functions support.
 */

#pragma once

#include <string>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>

namespace satox {
namespace core {
namespace cloud {

/**
 * @brief Azure configuration structure
 * @ingroup cloud_azure
 */
struct AzureConfig {
    std::string client_id;               ///< Azure client ID
    std::string client_secret;           ///< Azure client secret
    std::string tenant_id;               ///< Azure tenant ID
    std::string subscription_id;         ///< Azure subscription ID
    std::string resource_group;          ///< Resource group name
    std::string cosmosdb_account;        ///< Cosmos DB account name
    std::string sql_server;              ///< Azure SQL server name
    std::string storage_account;         ///< Storage account name
    std::string blob_container;          ///< Blob container name
    std::string function_app;            ///< Function app name
    int timeout_seconds = 30;            ///< Request timeout in seconds
    int max_retries = 3;                 ///< Maximum retry attempts
    bool enable_ssl_verification = true; ///< Enable SSL certificate verification
    std::map<std::string, std::string> headers; ///< Custom headers
};

// Forward declaration of AzureManager class
class AzureManager;

} // namespace cloud
} // namespace core
} // namespace satox 
// Forward declaration of AzureManager class
class AzureManager;

