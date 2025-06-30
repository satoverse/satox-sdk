/**
 * @file supabase_config.hpp
 * @brief Supabase configuration structures for Satox SDK
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace satox {
namespace core {

/**
 * @brief Configuration structure for Supabase database connections
 */
struct SupabaseConfig {
    std::string url;                    ///< Supabase project URL
    std::string anon_key;               ///< Anonymous key for client access
    std::string service_role_key;       ///< Service role key for admin access
    std::string database_url;           ///< Direct database connection URL
    bool enable_realtime = true;        ///< Enable real-time subscriptions
    int connection_timeout = 30;        ///< Connection timeout in seconds
    std::string auth_scheme = "bearer"; ///< Authentication scheme
    bool enable_ssl = true;             ///< Enable SSL/TLS
    std::string schema = "public";      ///< Database schema
    int max_connections = 10;           ///< Maximum connection pool size
    bool enable_logging = true;         ///< Enable request/response logging
    std::string log_level = "info";     ///< Log level (debug, info, warn, error)
    
    /**
     * @brief Convert configuration to JSON
     * @return JSON representation of the configuration
     */
    nlohmann::json toJson() const {
        nlohmann::json j;
        j["url"] = url;
        j["anon_key"] = anon_key;
        j["service_role_key"] = service_role_key;
        j["database_url"] = database_url;
        j["enable_realtime"] = enable_realtime;
        j["connection_timeout"] = connection_timeout;
        j["auth_scheme"] = auth_scheme;
        j["enable_ssl"] = enable_ssl;
        j["schema"] = schema;
        j["max_connections"] = max_connections;
        j["enable_logging"] = enable_logging;
        j["log_level"] = log_level;
        return j;
    }
    
    /**
     * @brief Create configuration from JSON
     * @param j JSON object containing configuration
     * @return SupabaseConfig instance
     */
    static SupabaseConfig fromJson(const nlohmann::json& j) {
        SupabaseConfig config;
        if (j.contains("url")) config.url = j["url"];
        if (j.contains("anon_key")) config.anon_key = j["anon_key"];
        if (j.contains("service_role_key")) config.service_role_key = j["service_role_key"];
        if (j.contains("database_url")) config.database_url = j["database_url"];
        if (j.contains("enable_realtime")) config.enable_realtime = j["enable_realtime"];
        if (j.contains("connection_timeout")) config.connection_timeout = j["connection_timeout"];
        if (j.contains("auth_scheme")) config.auth_scheme = j["auth_scheme"];
        if (j.contains("enable_ssl")) config.enable_ssl = j["enable_ssl"];
        if (j.contains("schema")) config.schema = j["schema"];
        if (j.contains("max_connections")) config.max_connections = j["max_connections"];
        if (j.contains("enable_logging")) config.enable_logging = j["enable_logging"];
        if (j.contains("log_level")) config.log_level = j["log_level"];
        return config;
    }
    
    /**
     * @brief Validate configuration
     * @return true if configuration is valid, false otherwise
     */
    bool isValid() const {
        return !url.empty() && !anon_key.empty();
    }
};

} // namespace core
} // namespace satox
