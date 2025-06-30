/**
 * @file firebase_manager.hpp
 * @brief Firebase database manager for Satox SDK
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
#include "firebase_config.hpp"

namespace satox {
namespace core {

/**
 * @brief Firebase database manager for handling Firebase operations
 */
class FirebaseManager {
public:
    /**
     * @brief Constructor
     */
    FirebaseManager();
    
    /**
     * @brief Destructor
     */
    ~FirebaseManager();
    
    /**
     * @brief Initialize the Firebase manager
     * @param config Configuration for Firebase connection
     * @return true if initialization successful, false otherwise
     */
    bool initialize(const FirebaseConfig& config);
    
    /**
     * @brief Connect to Firebase
     * @return Connection ID if successful, empty string otherwise
     */
    std::string connect();
    
    /**
     * @brief Disconnect from Firebase
     * @param connection_id Connection ID to disconnect
     * @return true if disconnection successful, false otherwise
     */
    bool disconnect(const std::string& connection_id);
    
    /**
     * @brief Firestore operations
     */
    class Firestore {
    public:
        /**
         * @brief Get a document
         * @param connection_id Connection ID
         * @param collection Collection name
         * @param document_id Document ID
         * @param result Document data
         * @return true if successful, false otherwise
         */
        bool getDocument(const std::string& connection_id,
                        const std::string& collection,
                        const std::string& document_id,
                        nlohmann::json& result);
        
        /**
         * @brief Set a document
         * @param connection_id Connection ID
         * @param collection Collection name
         * @param document_id Document ID
         * @param data Document data
         * @param result Operation result
         * @return true if successful, false otherwise
         */
        bool setDocument(const std::string& connection_id,
                        const std::string& collection,
                        const std::string& document_id,
                        const nlohmann::json& data,
                        nlohmann::json& result);
        
        /**
         * @brief Update a document
         * @param connection_id Connection ID
         * @param collection Collection name
         * @param document_id Document ID
         * @param data Document data to update
         * @param result Operation result
         * @return true if successful, false otherwise
         */
        bool updateDocument(const std::string& connection_id,
                           const std::string& collection,
                           const std::string& document_id,
                           const nlohmann::json& data,
                           nlohmann::json& result);
        
        /**
         * @brief Delete a document
         * @param connection_id Connection ID
         * @param collection Collection name
         * @param document_id Document ID
         * @param result Operation result
         * @return true if successful, false otherwise
         */
        bool deleteDocument(const std::string& connection_id,
                           const std::string& collection,
                           const std::string& document_id,
                           nlohmann::json& result);
        
        /**
         * @brief Query documents
         * @param connection_id Connection ID
         * @param collection Collection name
         * @param query Query conditions
         * @param result Query results
         * @return true if successful, false otherwise
         */
        bool queryDocuments(const std::string& connection_id,
                           const std::string& collection,
                           const nlohmann::json& query,
                           nlohmann::json& result);
    };
    
    /**
     * @brief Realtime Database operations
     */
    class RealtimeDatabase {
    public:
        /**
         * @brief Get data from a path
         * @param connection_id Connection ID
         * @param path Database path
         * @param result Data at path
         * @return true if successful, false otherwise
         */
        bool getData(const std::string& connection_id,
                    const std::string& path,
                    nlohmann::json& result);
        
        /**
         * @brief Set data at a path
         * @param connection_id Connection ID
         * @param path Database path
         * @param data Data to set
         * @param result Operation result
         * @return true if successful, false otherwise
         */
        bool setData(const std::string& connection_id,
                    const std::string& path,
                    const nlohmann::json& data,
                    nlohmann::json& result);
        
        /**
         * @brief Update data at a path
         * @param connection_id Connection ID
         * @param path Database path
         * @param data Data to update
         * @param result Operation result
         * @return true if successful, false otherwise
         */
        bool updateData(const std::string& connection_id,
                       const std::string& path,
                       const nlohmann::json& data,
                       nlohmann::json& result);
        
        /**
         * @brief Delete data at a path
         * @param connection_id Connection ID
         * @param path Database path
         * @param result Operation result
         * @return true if successful, false otherwise
         */
        bool deleteData(const std::string& connection_id,
                       const std::string& path,
                       nlohmann::json& result);
        
        /**
         * @brief Listen to real-time changes
         * @param connection_id Connection ID
         * @param path Database path
         * @param callback Callback function for real-time updates
         * @return Listener ID if successful, empty string otherwise
         */
        std::string listen(const std::string& connection_id,
                          const std::string& path,
                          std::function<void(const nlohmann::json&)> callback);
        
        /**
         * @brief Stop listening to real-time changes
         * @param connection_id Connection ID
         * @param listener_id Listener ID to stop
         * @return true if successful, false otherwise
         */
        bool stopListening(const std::string& connection_id, const std::string& listener_id);
    };
    
    /**
     * @brief Get Firestore instance
     * @return Firestore instance
     */
    Firestore& firestore();
    
    /**
     * @brief Get Realtime Database instance
     * @return Realtime Database instance
     */
    RealtimeDatabase& realtime();
    
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
