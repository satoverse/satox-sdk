/**
 * @file supabase_config.cpp
 * @brief Supabase configuration implementation
 * @author Satox SDK Team
 * @date $(date '+%Y-%m-%d')
 * @version 1.0.0
 * @license MIT
 */

#include "satox/core/cloud/supabase_config.hpp"
#include <nlohmann/json.hpp>

namespace satox {
namespace core {
namespace cloud {

void to_json(nlohmann::json& j, const SupabaseConfig& config) {
    j = nlohmann::json{
        {"url", config.url},
        {"anon_key", config.anon_key},
        {"service_role_key", config.service_role_key},
        {"database_url", config.database_url},
        {"storage_bucket", config.storage_bucket},
        {"functions_url", config.functions_url},
        {"timeout_seconds", config.timeout_seconds},
        {"max_retries", config.max_retries},
        {"enable_ssl_verification", config.enable_ssl_verification},
        {"headers", config.headers}
    };
}

void from_json(const nlohmann::json& j, SupabaseConfig& config) {
    j.at("url").get_to(config.url);
    j.at("anon_key").get_to(config.anon_key);
    j.at("service_role_key").get_to(config.service_role_key);
    j.at("database_url").get_to(config.database_url);
    j.at("storage_bucket").get_to(config.storage_bucket);
    j.at("functions_url").get_to(config.functions_url);
    j.at("timeout_seconds").get_to(config.timeout_seconds);
    j.at("max_retries").get_to(config.max_retries);
    j.at("enable_ssl_verification").get_to(config.enable_ssl_verification);
    j.at("headers").get_to(config.headers);
}

} // namespace cloud
} // namespace core
} // namespace satox 