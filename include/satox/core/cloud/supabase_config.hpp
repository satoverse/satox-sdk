/**
 * @file supabase_config.hpp
 * @brief Supabase configuration and connection management
 * @author Satox SDK Team
 * @date $(date '+%Y-%m-%d')
 * @version 1.0.0
 * @license MIT
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
 * @brief Supabase configuration structure
 */
struct SupabaseConfig {
    std::string url;                    ///< Supabase project URL
    std::string api_key;                ///< Supabase API key
    std::string service_role_key;       ///< Service role key for admin operations
    std::string anon_key;               ///< Anonymous key for public operations
    std::string database_url;           ///< Direct database connection URL
    std::string auth_url;               ///< Authentication endpoint
    std::string storage_url;            ///< Storage endpoint
    std::string functions_url;          ///< Edge functions endpoint
    int timeout_seconds = 30;           ///< Request timeout in seconds
    int max_retries = 3;                ///< Maximum retry attempts
    bool enable_real_time = true;       ///< Enable real-time subscriptions
    bool enable_ssl_verification = true; ///< Enable SSL certificate verification
    std::map<std::string, std::string> headers; ///< Custom headers
};

/**
 * @brief Supabase connection manager
 */
class SupabaseManager {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to SupabaseManager instance
     */
    static SupabaseManager& getInstance();

    /**
     * @brief Initialize Supabase connection
     * @param config Configuration parameters
     * @return true if successful, false otherwise
     */
    bool initialize(const SupabaseConfig& config);

    /**
     * @brief Check if connection is active
     * @return true if connected, false otherwise
     */
    bool isConnected() const;

    /**
     * @brief Execute a query
     * @param query SQL query string
     * @param params Query parameters
     * @return JSON result
     */
    nlohmann::json executeQuery(const std::string& query, const nlohmann::json& params = {});

    /**
     * @brief Execute a transaction
     * @param queries Vector of SQL queries
     * @param params Vector of parameter sets
     * @return JSON result
     */
    nlohmann::json executeTransaction(const std::vector<std::string>& queries, 
                                     const std::vector<nlohmann::json>& params = {});

    /**
     * @brief Subscribe to real-time changes
     * @param table Table name
     * @param event Event type (INSERT, UPDATE, DELETE, ALL)
     * @param callback Callback function
     * @return Subscription ID
     */
    std::string subscribe(const std::string& table, const std::string& event, 
                         std::function<void(const nlohmann::json&)> callback);

    /**
     * @brief Unsubscribe from real-time changes
     * @param subscription_id Subscription ID
     */
    void unsubscribe(const std::string& subscription_id);

    /**
     * @brief Upload file to storage
     * @param bucket Bucket name
     * @param path File path in bucket
     * @param file_path Local file path
     * @return Upload result
     */
    nlohmann::json uploadFile(const std::string& bucket, const std::string& path, 
                             const std::string& file_path);

    /**
     * @brief Download file from storage
     * @param bucket Bucket name
     * @param path File path in bucket
     * @param local_path Local file path to save
     * @return Download result
     */
    nlohmann::json downloadFile(const std::string& bucket, const std::string& path, 
                               const std::string& local_path);

    /**
     * @brief Call edge function
     * @param function_name Function name
     * @param payload Function payload
     * @return Function result
     */
    nlohmann::json callFunction(const std::string& function_name, const nlohmann::json& payload = {});

    /**
     * @brief Get connection status
     * @return Status information
     */
    nlohmann::json getStatus() const;

    /**
     * @brief Disconnect from Supabase
     */
    void disconnect();

    /**
     * @brief Destructor
     */
    ~SupabaseManager();

private:
    SupabaseManager() = default;
    SupabaseManager(const SupabaseManager&) = delete;
    SupabaseManager& operator=(const SupabaseManager&) = delete;

    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace cloud
} // namespace core
} // namespace satox 