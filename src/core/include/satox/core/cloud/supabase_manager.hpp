/**
 * @file supabase_manager.hpp
 * @brief Supabase database manager for Satox SDK
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <functional>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "supabase_config.hpp"

namespace satox {
namespace core {

/**
 * @brief Supabase database manager for handling Supabase operations
 */
class SupabaseManager {
public:
    /**
     * @brief Constructor
     */
    SupabaseManager();
    
    /**
     * @brief Destructor
     */
    ~SupabaseManager();
    
    /**
     * @brief Initialize the Supabase manager
     * @param config Configuration for Supabase connection
     * @return true if initialization successful, false otherwise
     */
    bool initialize(const SupabaseConfig& config);
    
    /**
     * @brief Connect to Supabase
     * @return Connection ID if successful, empty string otherwise
     */
    std::string connect();
    
    /**
     * @brief Disconnect from Supabase
     * @param connection_id Connection ID to disconnect
     * @return true if disconnection successful, false otherwise
     */
    bool disconnect(const std::string& connection_id);
    
    /**
     * @brief Execute a SQL query
     * @param connection_id Connection ID
     * @param query SQL query to execute
     * @param params Query parameters
     * @param result Query result
     * @return true if query successful, false otherwise
     */
    bool executeQuery(const std::string& connection_id, 
                     const std::string& query, 
                     const nlohmann::json& params,
                     nlohmann::json& result);
    
    /**
     * @brief Execute a transaction
     * @param connection_id Connection ID
     * @param queries Vector of SQL queries
     * @param result Transaction result
     * @return true if transaction successful, false otherwise
     */
    bool executeTransaction(const std::string& connection_id,
                           const std::vector<std::string>& queries,
                           nlohmann::json& result);
    
    /**
     * @brief Subscribe to real-time changes
     * @param connection_id Connection ID
     * @param table Table name to subscribe to
     * @param callback Callback function for real-time updates
     * @return Subscription ID if successful, empty string otherwise
     */
    std::string subscribe(const std::string& connection_id,
                         const std::string& table,
                         std::function<void(const nlohmann::json&)> callback);
    
    /**
     * @brief Unsubscribe from real-time changes
     * @param connection_id Connection ID
     * @param subscription_id Subscription ID to unsubscribe from
     * @return true if unsubscription successful, false otherwise
     */
    bool unsubscribe(const std::string& connection_id, const std::string& subscription_id);
    
    /**
     * @brief Get connection status
     * @param connection_id Connection ID
     * @return true if connected, false otherwise
     */
    bool isConnected(const std::string& connection_id) const;
    
    /**
     * @brief Get last error message
     * @return Last error message
     */
    std::string getLastError() const;
    
    /**
     * @brief Clear last error
     */
    void clearError();
    
    /**
     * @brief Shutdown the manager
     */
    void shutdown();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace core
} // namespace satox
