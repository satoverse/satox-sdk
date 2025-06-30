/**
 * @file firebase_config.cpp
 * @brief Firebase configuration implementation
 * @author Satox SDK Team
 * @date $(date '+%Y-%m-%d')
 * @version 1.0.0
 * @license MIT
 */

#include "satox/core/cloud/firebase_config.hpp"
#include <nlohmann/json.hpp>

namespace satox {
namespace core {
namespace cloud {

void to_json(nlohmann::json& j, const FirebaseConfig& config) {
    j = nlohmann::json{
        {"project_id", config.project_id},
        {"api_key", config.api_key},
        {"auth_domain", config.auth_domain},
        {"storage_bucket", config.storage_bucket},
        {"messaging_sender_id", config.messaging_sender_id},
        {"app_id", config.app_id},
        {"database_url", config.database_url},
        {"functions_url", config.functions_url},
        {"timeout_seconds", config.timeout_seconds},
        {"max_retries", config.max_retries},
        {"enable_ssl_verification", config.enable_ssl_verification},
        {"headers", config.headers}
    };
}

void from_json(const nlohmann::json& j, FirebaseConfig& config) {
    j.at("project_id").get_to(config.project_id);
    j.at("api_key").get_to(config.api_key);
    j.at("auth_domain").get_to(config.auth_domain);
    j.at("storage_bucket").get_to(config.storage_bucket);
    j.at("messaging_sender_id").get_to(config.messaging_sender_id);
    j.at("app_id").get_to(config.app_id);
    j.at("database_url").get_to(config.database_url);
    j.at("functions_url").get_to(config.functions_url);
    j.at("timeout_seconds").get_to(config.timeout_seconds);
    j.at("max_retries").get_to(config.max_retries);
    j.at("enable_ssl_verification").get_to(config.enable_ssl_verification);
    j.at("headers").get_to(config.headers);
}

} // namespace cloud
} // namespace core
} // namespace satox 