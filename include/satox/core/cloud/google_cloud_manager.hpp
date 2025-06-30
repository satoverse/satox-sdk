/**
 * @file google_cloud_manager.hpp
 * @brief Google Cloud integration manager
 * @author Satox SDK Team
 * @date $(date '+%Y-%m-%d')
 * @version 1.0.0
 * @license MIT
 *
 * @defgroup cloud_google_cloud Google Cloud Integration
 * @ingroup cloud
 * @details
 *   Google Cloud integration for Satox SDK. Provides connection, query, transaction, storage, and Functions support.
 */

#pragma once

#include "satox/core/cloud/google_cloud_config.hpp"
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
 * @brief Google Cloud manager implementation
 * @ingroup cloud_google_cloud
 *
 * Provides comprehensive Google Cloud integration including:
 * - Cloud SQL operations
 * - Firestore operations
 * - Cloud Storage operations
 * - Cloud Functions invocation
 * - Authentication and connection management
 *
 * Example usage:
 * @code
 * satox::core::cloud::GoogleCloudConfig config;
 * config.project_id = "your-project-id";
 * config.client_email = "your-service-account@project.iam.gserviceaccount.com";
 * config.private_key = "-----BEGIN PRIVATE KEY-----\n...\n-----END PRIVATE KEY-----\n";
 * config.region = "us-central1";
 * config.storage_bucket = "your-storage-bucket";
 * config.sql_instance = "your-sql-instance";
 * config.firestore_collection = "your-collection";
 * config.function_name = "your-function";
 * 
 * auto& gcp = satox::core::cloud::GoogleCloudManager::getInstance();
 * if (gcp.initialize(config)) {
 *     // Execute queries
 *     auto result = gcp.executeQuery("firestore", {{"query", "SELECT * FROM users"}});
 *     
 *     // Upload files
 *     gcp.uploadFile("mybucket", "path/to/file.txt", "/local/path/file.txt");
 *     
 *     // Invoke functions
 *     gcp.invokeFunction("myfunction", {{"data", "value"}});
 * }
 * @endcode
 */
class GoogleCloudManager {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to GoogleCloudManager instance
     */
    static GoogleCloudManager& getInstance();

    /**
     * @brief Initialize Google Cloud connection
     * @param config Configuration parameters
     * @return true if successful, false otherwise
     */
    bool initialize(const GoogleCloudConfig& config);

    /**
     * @brief Check if connection is active
     * @return true if connected, false otherwise
     */
    bool isConnected() const;

    /**
     * @brief Execute a query on a given Google Cloud service
     * @param service Service name (e.g., "firestore", "sql")
     * @param params Query parameters
     * @return JSON result
     */
    nlohmann::json executeQuery(const std::string& service, const nlohmann::json& params = {});

    /**
     * @brief Execute a transaction on a given Google Cloud service
     * @param service Service name
     * @param actions List of actions
     * @return JSON result
     */
    nlohmann::json executeTransaction(const std::string& service, const std::vector<nlohmann::json>& actions = {});

    /**
     * @brief Upload file to Google Cloud Storage
     * @param bucket Bucket name
     * @param path File path in bucket
     * @param file_path Local file path
     * @return Upload result
     */
    nlohmann::json uploadFile(const std::string& bucket, const std::string& path, const std::string& file_path);

    /**
     * @brief Download file from Google Cloud Storage
     * @param bucket Bucket name
     * @param path File path in bucket
     * @param local_path Local file path to save
     * @return Download result
     */
    nlohmann::json downloadFile(const std::string& bucket, const std::string& path, const std::string& local_path);

    /**
     * @brief Invoke Google Cloud Function
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
     * @brief Disconnect from Google Cloud
     */
    void disconnect();

    /**
     * @brief Destructor
     */
    ~GoogleCloudManager();

private:
    GoogleCloudManager() = default;
    GoogleCloudManager(const GoogleCloudManager&) = delete;
    GoogleCloudManager& operator=(const GoogleCloudManager&) = delete;

    /**
     * @brief Get Google Cloud access token
     * @return Access token string
     */
    std::string getAccessToken();

    /**
     * @brief Make HTTP request to Google Cloud REST API
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
     * @brief Build Google Cloud REST API URL
     * @param service Service name
     * @param resource Resource path
     * @return Full URL
     */
    std::string buildUrl(const std::string& service, const std::string& resource);

    /**
     * @brief Get default headers for Google Cloud requests
     * @return Headers map
     */
    std::map<std::string, std::string> getDefaultHeaders();

    /**
     * @brief Create JWT token for service account authentication
     * @return JWT token string
     */
    std::string createJWT();

    GoogleCloudConfig config_;
    bool connected_ = false;
    std::string access_token_;
    std::time_t token_expiry_ = 0;
    CURL* curl_ = nullptr;
};

} // namespace cloud
} // namespace core
} // namespace satox 