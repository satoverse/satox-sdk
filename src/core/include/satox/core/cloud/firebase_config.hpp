/**
 * @file firebase_config.hpp
 * @brief Firebase configuration structures for Satox SDK
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace satox {
namespace core {

/**
 * @brief Configuration structure for Firebase database connections
 */
struct FirebaseConfig {
    std::string project_id;                    ///< Firebase project ID
    std::string private_key_id;                ///< Private key ID
    std::string private_key;                   ///< Private key for authentication
    std::string client_email;                  ///< Client email for service account
    std::string client_id;                     ///< Client ID
    std::string auth_uri;                      ///< Authentication URI
    std::string token_uri;                     ///< Token URI
    std::string auth_provider_x509_cert_url;   ///< Auth provider certificate URL
    std::string client_x509_cert_url;          ///< Client certificate URL
    std::string database_url;                  ///< Realtime Database URL
    bool enable_auth = true;                   ///< Enable Firebase Auth
    bool enable_firestore = true;              ///< Enable Firestore
    bool enable_realtime_db = false;           ///< Enable Realtime Database
    bool enable_storage = false;               ///< Enable Cloud Storage
    bool enable_functions = false;             ///< Enable Cloud Functions
    int connection_timeout = 30;               ///< Connection timeout in seconds
    int max_connections = 10;                  ///< Maximum connection pool size
    bool enable_ssl = true;                    ///< Enable SSL/TLS
    bool enable_logging = true;                ///< Enable request/response logging
    std::string log_level = "info";            ///< Log level (debug, info, warn, error)
    
    /**
     * @brief Convert configuration to JSON
     * @return JSON representation of the configuration
     */
    nlohmann::json toJson() const {
        nlohmann::json j;
        j["project_id"] = project_id;
        j["private_key_id"] = private_key_id;
        j["private_key"] = private_key;
        j["client_email"] = client_email;
        j["client_id"] = client_id;
        j["auth_uri"] = auth_uri;
        j["token_uri"] = token_uri;
        j["auth_provider_x509_cert_url"] = auth_provider_x509_cert_url;
        j["client_x509_cert_url"] = client_x509_cert_url;
        j["database_url"] = database_url;
        j["enable_auth"] = enable_auth;
        j["enable_firestore"] = enable_firestore;
        j["enable_realtime_db"] = enable_realtime_db;
        j["enable_storage"] = enable_storage;
        j["enable_functions"] = enable_functions;
        j["connection_timeout"] = connection_timeout;
        j["max_connections"] = max_connections;
        j["enable_ssl"] = enable_ssl;
        j["enable_logging"] = enable_logging;
        j["log_level"] = log_level;
        return j;
    }
    
    /**
     * @brief Create configuration from JSON
     * @param j JSON object containing configuration
     * @return FirebaseConfig instance
     */
    static FirebaseConfig fromJson(const nlohmann::json& j) {
        FirebaseConfig config;
        if (j.contains("project_id")) config.project_id = j["project_id"];
        if (j.contains("private_key_id")) config.private_key_id = j["private_key_id"];
        if (j.contains("private_key")) config.private_key = j["private_key"];
        if (j.contains("client_email")) config.client_email = j["client_email"];
        if (j.contains("client_id")) config.client_id = j["client_id"];
        if (j.contains("auth_uri")) config.auth_uri = j["auth_uri"];
        if (j.contains("token_uri")) config.token_uri = j["token_uri"];
        if (j.contains("auth_provider_x509_cert_url")) config.auth_provider_x509_cert_url = j["auth_provider_x509_cert_url"];
        if (j.contains("client_x509_cert_url")) config.client_x509_cert_url = j["client_x509_cert_url"];
        if (j.contains("database_url")) config.database_url = j["database_url"];
        if (j.contains("enable_auth")) config.enable_auth = j["enable_auth"];
        if (j.contains("enable_firestore")) config.enable_firestore = j["enable_firestore"];
        if (j.contains("enable_realtime_db")) config.enable_realtime_db = j["enable_realtime_db"];
        if (j.contains("enable_storage")) config.enable_storage = j["enable_storage"];
        if (j.contains("enable_functions")) config.enable_functions = j["enable_functions"];
        if (j.contains("connection_timeout")) config.connection_timeout = j["connection_timeout"];
        if (j.contains("max_connections")) config.max_connections = j["max_connections"];
        if (j.contains("enable_ssl")) config.enable_ssl = j["enable_ssl"];
        if (j.contains("enable_logging")) config.enable_logging = j["enable_logging"];
        if (j.contains("log_level")) config.log_level = j["log_level"];
        return config;
    }
    
    /**
     * @brief Validate configuration
     * @return true if configuration is valid, false otherwise
     */
    bool isValid() const {
        return !project_id.empty() && !private_key.empty() && !client_email.empty();
    }
};

} // namespace core
} // namespace satox
