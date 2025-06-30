/**
 * @file firebase_config.hpp
 * @brief Firebase configuration and connection management
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
 * @brief Firebase configuration structure
 */
struct FirebaseConfig {
    std::string project_id;             ///< Firebase project ID
    std::string api_key;                ///< Firebase API key
    std::string auth_domain;            ///< Authentication domain
    std::string database_url;           ///< Realtime Database URL
    std::string storage_bucket;         ///< Storage bucket name
    std::string messaging_sender_id;    ///< Messaging sender ID
    std::string app_id;                 ///< Firebase app ID
    std::string service_account_key;    ///< Service account key file path
    int timeout_seconds = 30;           ///< Request timeout in seconds
    int max_retries = 3;                ///< Maximum retry attempts
    bool enable_real_time = true;       ///< Enable real-time database
    bool enable_ssl_verification = true; ///< Enable SSL certificate verification
    std::map<std::string, std::string> headers; ///< Custom headers
};

/**
 * @brief Firebase connection manager
 */
class FirebaseManager {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to FirebaseManager instance
     */
    static FirebaseManager& getInstance();

    /**
     * @brief Initialize Firebase connection
     * @param config Configuration parameters
     * @return true if successful, false otherwise
     */
    bool initialize(const FirebaseConfig& config);

    /**
     * @brief Check if connection is active
     * @return true if connected, false otherwise
     */
    bool isConnected() const;

    /**
     * @brief Read data from Realtime Database
     * @param path Database path
     * @return JSON result
     */
    nlohmann::json readData(const std::string& path);

    /**
     * @brief Write data to Realtime Database
     * @param path Database path
     * @param data Data to write
     * @return Write result
     */
    nlohmann::json writeData(const std::string& path, const nlohmann::json& data);

    /**
     * @brief Update data in Realtime Database
     * @param path Database path
     * @param data Data to update
     * @return Update result
     */
    nlohmann::json updateData(const std::string& path, const nlohmann::json& data);

    /**
     * @brief Delete data from Realtime Database
     * @param path Database path
     * @return Delete result
     */
    nlohmann::json deleteData(const std::string& path);

    /**
     * @brief Query data with conditions
     * @param path Database path
     * @param query Query parameters
     * @return Query result
     */
    nlohmann::json queryData(const std::string& path, const nlohmann::json& query);

    /**
     * @brief Subscribe to real-time changes
     * @param path Database path
     * @param callback Callback function
     * @return Subscription ID
     */
    std::string subscribe(const std::string& path, 
                         std::function<void(const nlohmann::json&)> callback);

    /**
     * @brief Unsubscribe from real-time changes
     * @param subscription_id Subscription ID
     */
    void unsubscribe(const std::string& subscription_id);

    /**
     * @brief Upload file to Firebase Storage
     * @param bucket Storage bucket
     * @param path Storage path
     * @param file_path Local file path
     * @return Upload result
     */
    nlohmann::json uploadFile(const std::string& bucket, const std::string& path, 
                             const std::string& file_path);

    /**
     * @brief Download file from Firebase Storage
     * @param bucket Storage bucket
     * @param path Storage path
     * @param local_path Local file path to save
     * @return Download result
     */
    nlohmann::json downloadFile(const std::string& bucket, const std::string& path, 
                               const std::string& local_path);

    /**
     * @brief Call Cloud Function
     * @param function_name Function name
     * @param payload Function payload
     * @return Function result
     */
    nlohmann::json callFunction(const std::string& function_name, const nlohmann::json& payload = {});

    /**
     * @brief Send push notification
     * @param token Device token
     * @param notification Notification data
     * @return Send result
     */
    nlohmann::json sendNotification(const std::string& token, const nlohmann::json& notification);

    /**
     * @brief Get connection status
     * @return Status information
     */
    nlohmann::json getStatus() const;

    /**
     * @brief Disconnect from Firebase
     */
    void disconnect();

    /**
     * @brief Destructor
     */
    ~FirebaseManager();

private:
    FirebaseManager() = default;
    FirebaseManager(const FirebaseManager&) = delete;
    FirebaseManager& operator=(const FirebaseManager&) = delete;

    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace cloud
} // namespace core
} // namespace satox 