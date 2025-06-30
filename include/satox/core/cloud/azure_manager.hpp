/**
 * @file azure_manager.hpp
 * @brief Azure cloud integration manager
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

#include "satox/core/cloud/azure_config.hpp"
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

namespace satox {
namespace core {
namespace cloud {

/**
 * @brief Azure cloud manager implementation
 * @ingroup cloud_azure
 *
 * Provides comprehensive Azure cloud integration including:
 * - Azure SQL Database operations
 * - Cosmos DB operations
 * - Blob Storage operations
 * - Azure Functions invocation
 * - Authentication and connection management
 *
 * Example usage:
 * @code
 * satox::core::cloud::AzureConfig config;
 * config.client_id = "your-client-id";
 * config.client_secret = "your-client-secret";
 * config.tenant_id = "your-tenant-id";
 * config.subscription_id = "your-subscription-id";
 * config.resource_group = "your-resource-group";
 * config.cosmosdb_account = "your-cosmosdb-account";
 * config.sql_server = "your-sql-server";
 * config.storage_account = "your-storage-account";
 * config.blob_container = "your-blob-container";
 * config.function_app = "your-function-app";
 * 
 * auto& azure = satox::core::cloud::AzureManager::getInstance();
 * if (azure.initialize(config)) {
 *     // Execute queries
 *     auto result = azure.executeQuery("cosmosdb", {{"query", "SELECT * FROM c"}});
 *     
 *     // Upload files
 *     azure.uploadFile("mycontainer", "path/to/file.txt", "/local/path/file.txt");
 *     
 *     // Invoke functions
 *     azure.invokeFunction("myfunction", {{"data", "value"}});
 * }
 * @endcode
 */
class AzureManager {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to AzureManager instance
     */
    static AzureManager& getInstance();

    /**
     * @brief Initialize Azure connection
     * @param config Configuration parameters
     * @return true if successful, false otherwise
     */
    bool initialize(const AzureConfig& config);

    /**
     * @brief Check if connection is active
     * @return true if connected, false otherwise
     */
    bool isConnected() const;

    /**
     * @brief Execute a query on a given Azure service
     * @param service Service name (e.g., "cosmosdb", "sql")
     * @param params Query parameters
     * @return JSON result
     */
    nlohmann::json executeQuery(const std::string& service, const nlohmann::json& params = {});

    /**
     * @brief Execute a transaction on a given Azure service
     * @param service Service name
     * @param actions List of actions
     * @return JSON result
     */
    nlohmann::json executeTransaction(const std::string& service, const std::vector<nlohmann::json>& actions = {});

    /**
     * @brief Upload file to Azure Blob Storage
     * @param container Container name
     * @param path File path in container
     * @param file_path Local file path
     * @return Upload result
     */
    nlohmann::json uploadFile(const std::string& container, const std::string& path, const std::string& file_path);

    /**
     * @brief Download file from Azure Blob Storage
     * @param container Container name
     * @param path File path in container
     * @param local_path Local file path to save
     * @return Download result
     */
    nlohmann::json downloadFile(const std::string& container, const std::string& path, const std::string& local_path);

    /**
     * @brief Invoke Azure Function
     * @param function_name Function name
     * @param payload Function payload
     * @return Function result
     */
    nlohmann::json invokeFunction(const std::string& function_name, const nlohmann::json& payload = {});

    /**
     * @brief Get connection status
     * @return Status information
     */
    nlohmann::json getStatus() const;

    /**
     * @brief Disconnect from Azure
     */
    void disconnect();

    /**
     * @brief Destructor
     */
    ~AzureManager();

private:
    AzureManager() = default;
    AzureManager(const AzureManager&) = delete;
    AzureManager& operator=(const AzureManager&) = delete;

    /**
     * @brief Get Azure access token
     * @return Access token string
     */
    std::string getAccessToken();

    /**
     * @brief Make HTTP request to Azure REST API
     * @param url Request URL
     * @param method HTTP method
     * @param headers Request headers
     * @param body Request body
     * @return Response JSON
     */
    nlohmann::json makeRequest(const std::string& url, const std::string& method, 
                              const std::map<std::string, std::string>& headers, 
                              const std::string& body = "");

    /**
     * @brief Build Azure REST API URL
     * @param service Service name
     * @param resource Resource path
     * @return Full URL
     */
    std::string buildUrl(const std::string& service, const std::string& resource);

    /**
     * @brief Get default headers for Azure requests
     * @return Headers map
     */
    std::map<std::string, std::string> getDefaultHeaders();

    AzureConfig config_;
    bool connected_ = false;
    std::string access_token_;
    std::time_t token_expiry_ = 0;
    CURL* curl_ = nullptr;
};

} // namespace cloud
} // namespace core
} // namespace satox 