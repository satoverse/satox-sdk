/**
 * @file aws_config.cpp
 * @brief AWS configuration implementation
 * @author Satox SDK Team
 * @date $(date '+%Y-%m-%d')
 * @version 1.0.0
 * @license MIT
 */

#include "satox/core/cloud/aws_config.hpp"
#include <nlohmann/json.hpp>

namespace satox {
namespace core {
namespace cloud {

void to_json(nlohmann::json& j, const AWSConfig& config) {
    j = nlohmann::json{
        {"access_key_id", config.access_key_id},
        {"secret_access_key", config.secret_access_key},
        {"region", config.region},
        {"session_token", config.session_token},
        {"endpoint_url", config.endpoint_url},
        {"s3_bucket", config.s3_bucket},
        {"rds_endpoint", config.rds_endpoint},
        {"dynamodb_table", config.dynamodb_table},
        {"lambda_function", config.lambda_function},
        {"timeout_seconds", config.timeout_seconds},
        {"max_retries", config.max_retries},
        {"enable_ssl_verification", config.enable_ssl_verification},
        {"headers", config.headers}
    };
}

void from_json(const nlohmann::json& j, AWSConfig& config) {
    j.at("access_key_id").get_to(config.access_key_id);
    j.at("secret_access_key").get_to(config.secret_access_key);
    j.at("region").get_to(config.region);
    j.at("session_token").get_to(config.session_token);
    j.at("endpoint_url").get_to(config.endpoint_url);
    j.at("s3_bucket").get_to(config.s3_bucket);
    j.at("rds_endpoint").get_to(config.rds_endpoint);
    j.at("dynamodb_table").get_to(config.dynamodb_table);
    j.at("lambda_function").get_to(config.lambda_function);
    j.at("timeout_seconds").get_to(config.timeout_seconds);
    j.at("max_retries").get_to(config.max_retries);
    j.at("enable_ssl_verification").get_to(config.enable_ssl_verification);
    j.at("headers").get_to(config.headers);
}

} // namespace cloud
} // namespace core
} // namespace satox 