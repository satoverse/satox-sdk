/**
 * @file aws_config.hpp
 * @brief AWS configuration and connection management
 * @author Satox SDK Team
 * @date $(date '+%Y-%m-%d')
 * @version 1.0.0
 * @license MIT
 *
 * @defgroup cloud_aws AWS Cloud Integration
 * @ingroup cloud
 * @details
 *   AWS integration for Satox SDK. Provides connection, query, transaction, storage, and Lambda support.
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
 * @brief AWS configuration structure
 * @ingroup cloud_aws
 */
struct AWSConfig {
    std::string access_key_id;           ///< AWS access key ID
    std::string secret_access_key;       ///< AWS secret access key
    std::string region;                  ///< AWS region
    std::string session_token;           ///< AWS session token (optional)
    std::string endpoint_url;            ///< Custom endpoint URL (optional)
    std::string s3_bucket;               ///< S3 bucket name
    std::string rds_endpoint;            ///< RDS endpoint
    std::string dynamodb_table;          ///< DynamoDB table name
    std::string lambda_function;         ///< Lambda function name
    int timeout_seconds = 30;            ///< Request timeout in seconds
    int max_retries = 3;                 ///< Maximum retry attempts
    bool enable_ssl_verification = true; ///< Enable SSL certificate verification
    std::map<std::string, std::string> headers; ///< Custom headers
};

/**
 * @brief AWS connection manager
 * @ingroup cloud_aws
 *
 * Example usage:
 * @code
 * satox::core::cloud::AWSConfig config;
 * config.access_key_id = "...";
 * config.secret_access_key = "...";
 * config.region = "us-east-1";
 * auto& aws = satox::core::cloud::AWSManager::getInstance();
 * aws.initialize(config);
 * @endcode
 */
class AWSManager {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to AWSManager instance
     */
    static AWSManager& getInstance();
    /**
     * @brief Initialize AWS connection
     * @param config Configuration parameters
     * @return true if successful, false otherwise
     */
    bool initialize(const AWSConfig& config);
    /**
     * @brief Check if connection is active
     * @return true if connected, false otherwise
     */
    bool isConnected() const;
    /**
     * @brief Execute a query on a given AWS service
     * @param service Service name (e.g., "dynamodb", "rds")
     * @param params Query parameters
     * @return JSON result
     */
    nlohmann::json executeQuery(const std::string& service, const nlohmann::json& params = {});
    /**
     * @brief Execute a transaction on a given AWS service
     * @param service Service name
     * @param actions List of actions
     * @return JSON result
     */
    nlohmann::json executeTransaction(const std::string& service, const std::vector<nlohmann::json>& actions = {});
    /**
     * @brief Upload file to S3
     * @param bucket Bucket name
     * @param path File path in bucket
     * @param file_path Local file path
     * @return Upload result
     */
    nlohmann::json uploadFile(const std::string& bucket, const std::string& path, const std::string& file_path);
    /**
     * @brief Download file from S3
     * @param bucket Bucket name
     * @param path File path in bucket
     * @param local_path Local file path to save
     * @return Download result
     */
    nlohmann::json downloadFile(const std::string& bucket, const std::string& path, const std::string& local_path);
    /**
     * @brief Invoke Lambda function
     * @param function_name Lambda function name
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
     * @brief Disconnect from AWS
     */
    void disconnect();
    /**
     * @brief Destructor
     */
    ~AWSManager();
private:
    AWSManager() = default;
    AWSManager(const AWSManager&) = delete;
    AWSManager& operator=(const AWSManager&) = delete;
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace cloud
} // namespace core
} // namespace satox 