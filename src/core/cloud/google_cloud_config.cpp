/**
 * @file google_cloud_config.cpp
 * @brief Google Cloud configuration implementation
 * @author Satox SDK Team
 * @date $(date '+%Y-%m-%d')
 * @version 1.0.0
 * @license MIT
 */

#include "satox/core/cloud/google_cloud_config.hpp"
#include <nlohmann/json.hpp>

namespace satox {
namespace core {
namespace cloud {

void to_json(nlohmann::json& j, const GoogleCloudConfig& config) {
    j = nlohmann::json{
        {"project_id", config.project_id},
        {"service_account_key", config.service_account_key},
        {"storage_bucket", config.storage_bucket},
        {"firestore_database", config.firestore_database},
        {"cloud_function_name", config.cloud_function_name},
        {"timeout_seconds", config.timeout_seconds},
        {"max_retries", config.max_retries},
        {"enable_ssl_verification", config.enable_ssl_verification},
        {"headers", config.headers}
    };
}

void from_json(const nlohmann::json& j, GoogleCloudConfig& config) {
    j.at("project_id").get_to(config.project_id);
    j.at("service_account_key").get_to(config.service_account_key);
    j.at("storage_bucket").get_to(config.storage_bucket);
    j.at("firestore_database").get_to(config.firestore_database);
    j.at("cloud_function_name").get_to(config.cloud_function_name);
    j.at("timeout_seconds").get_to(config.timeout_seconds);
    j.at("max_retries").get_to(config.max_retries);
    j.at("enable_ssl_verification").get_to(config.enable_ssl_verification);
    j.at("headers").get_to(config.headers);
}

} // namespace cloud
} // namespace core
} // namespace satox 