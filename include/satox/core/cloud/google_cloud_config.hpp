/**
 * @file google_cloud_config.hpp
 * @brief Google Cloud configuration and connection management
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

struct GoogleCloudConfig {
    std::string project_id;
    std::string client_email;
    std::string private_key;
    std::string region;
    std::string storage_bucket;
    std::string sql_instance;
    std::string firestore_collection;
    std::string function_name;
    int timeout_seconds = 30;
    int max_retries = 3;
    bool enable_ssl_verification = true;
    std::map<std::string, std::string> headers;
};

// Forward declaration of GoogleCloudManager class
class GoogleCloudManager;

} // namespace cloud
} // namespace core
} // namespace satox 
// Forward declaration of GoogleCloudManager class
class GoogleCloudManager;

