/**
 * @file azure_config.cpp
 * @brief Azure configuration implementation
 * @author Satox SDK Team
 * @date $(date '+%Y-%m-%d')
 * @version 1.0.0
 * @license MIT
 */

#include "satox/core/cloud/azure_config.hpp"
#include <nlohmann/json.hpp>

namespace satox {
namespace core {
namespace cloud {

void to_json(nlohmann::json& j, const AzureConfig& config) {
    j = nlohmann::json{
        {"tenant_id", config.tenant_id},
        {"client_id", config.client_id},
        {"client_secret", config.client_secret},
        {"subscription_id", config.subscription_id},
        {"resource_group", config.resource_group},
        {"storage_account", config.storage_account},
        {"cosmos_db_endpoint", config.cosmos_db_endpoint},
        {"cosmos_db_key", config.cosmos_db_key},
        {"function_app_name", config.function_app_name},
        {"timeout_seconds", config.timeout_seconds},
        {"max_retries", config.max_retries},
        {"enable_ssl_verification", config.enable_ssl_verification},
        {"headers", config.headers}
    };
}

void from_json(const nlohmann::json& j, AzureConfig& config) {
    j.at("tenant_id").get_to(config.tenant_id);
    j.at("client_id").get_to(config.client_id);
    j.at("client_secret").get_to(config.client_secret);
    j.at("subscription_id").get_to(config.subscription_id);
    j.at("resource_group").get_to(config.resource_group);
    j.at("storage_account").get_to(config.storage_account);
    j.at("cosmos_db_endpoint").get_to(config.cosmos_db_endpoint);
    j.at("cosmos_db_key").get_to(config.cosmos_db_key);
    j.at("function_app_name").get_to(config.function_app_name);
    j.at("timeout_seconds").get_to(config.timeout_seconds);
    j.at("max_retries").get_to(config.max_retries);
    j.at("enable_ssl_verification").get_to(config.enable_ssl_verification);
    j.at("headers").get_to(config.headers);
}

} // namespace cloud
} // namespace core
} // namespace satox 