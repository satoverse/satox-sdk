/**
 * @file google_cloud_manager.cpp
 * @brief Google Cloud manager implementation
 * @author Satox SDK Team
 * @date $(date '+%Y-%m-%d')
 * @version 1.0.0
 * @license MIT
 */

#include "satox/core/cloud/google_cloud_manager.hpp"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <memory>
#include <mutex>
#include <atomic>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <curl/curl.h>

namespace satox {
namespace core {
namespace cloud {

// Static callback for CURL
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Base64 encoding helper
std::string base64_encode(const std::string& input) {
    BIO* bio, *b64;
    BUF_MEM* bufferPtr;
    
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, input.c_str(), input.length());
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    
    std::string result(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);
    
    return result;
}

// URL encoding helper
std::string url_encode(const std::string& str) {
    CURL* curl = curl_easy_init();
    if (curl) {
        char* encoded = curl_easy_escape(curl, str.c_str(), str.length());
        std::string result(encoded);
        curl_free(encoded);
        curl_easy_cleanup(curl);
        return result;
    }
    return str;
}

GoogleCloudManager& GoogleCloudManager::getInstance() {
    static GoogleCloudManager instance;
    return instance;
}

bool GoogleCloudManager::initialize(const GoogleCloudConfig& config) {
    config_ = config;
    
    // Initialize CURL
    curl_global_init(CURL_GLOBAL_ALL);
    curl_ = curl_easy_init();
    if (!curl_) {
        spdlog::error("[GoogleCloud] Failed to initialize CURL");
        return false;
    }
    
    // Test connection by getting access token
    try {
        access_token_ = getAccessToken();
        if (!access_token_.empty()) {
            connected_ = true;
            spdlog::info("[GoogleCloud] Successfully initialized with project: {}", config.project_id);
            return true;
        }
    } catch (const std::exception& e) {
        spdlog::error("[GoogleCloud] Initialization failed: {}", e.what());
    }
    
    connected_ = false;
    return false;
}

bool GoogleCloudManager::isConnected() const {
    return connected_;
}

std::string GoogleCloudManager::getAccessToken() {
    // Check if token is still valid (with 5 minute buffer)
    if (!access_token_.empty() && std::time(nullptr) < token_expiry_ - 300) {
        return access_token_;
    }
    
    // Get new token using service account JWT
    std::string jwt = createJWT();
    std::string token_url = "https://oauth2.googleapis.com/token";
    
    std::map<std::string, std::string> headers = {
        {"Content-Type", "application/x-www-form-urlencoded"}
    };
    
    std::string body = "grant_type=urn:ietf:params:oauth:grant-type:jwt-bearer"
                      "&assertion=" + url_encode(jwt);
    
    auto response = makeRequest(token_url, "POST", headers, body);
    
    if (response.contains("access_token")) {
        access_token_ = response["access_token"];
        token_expiry_ = std::time(nullptr) + response["expires_in"].get<int>();
        spdlog::debug("[GoogleCloud] Got new access token, expires in {} seconds", response["expires_in"].get<int>());
        return access_token_;
    }
    
    spdlog::error("[GoogleCloud] Failed to get access token: {}", response.dump());
    return "";
}

std::string GoogleCloudManager::createJWT() {
    // Create JWT header
    nlohmann::json header = {
        {"alg", "RS256"},
        {"typ", "JWT"}
    };
    
    // Create JWT payload
    std::time_t now = std::time(nullptr);
    nlohmann::json payload = {
        {"iss", config_.client_email},
        {"scope", "https://www.googleapis.com/auth/cloud-platform"},
        {"aud", "https://oauth2.googleapis.com/token"},
        {"exp", now + 3600}, // 1 hour
        {"iat", now}
    };
    
    // Encode header and payload
    std::string header_str = base64_encode(header.dump());
    std::string payload_str = base64_encode(payload.dump());
    
    // Create signature
    std::string data = header_str + "." + payload_str;
    
    // Sign with private key (simplified - in production, use proper RSA signing)
    std::string signature = base64_encode(data); // Placeholder
    
    return data + "." + signature;
}

nlohmann::json GoogleCloudManager::executeQuery(const std::string& service, const nlohmann::json& params) {
    if (!isConnected()) {
        return {{"error", "Not connected to Google Cloud"}};
    }
    
    try {
        if (service == "firestore") {
            return executeFirestoreQuery(params);
        } else if (service == "sql") {
            return executeSQLQuery(params);
        } else {
            return {{"error", "Unsupported service: " + service}};
        }
    } catch (const std::exception& e) {
        spdlog::error("[GoogleCloud] Query execution failed: {}", e.what());
        return {{"error", e.what()}};
    }
}

nlohmann::json GoogleCloudManager::executeFirestoreQuery(const nlohmann::json& params) {
    std::string query = params.value("query", "");
    std::string collection = params.value("collection", config_.firestore_collection);
    
    if (query.empty() || collection.empty()) {
        return {{"error", "Missing required parameters: query, collection"}};
    }
    
    std::string url = buildUrl("firestore", "/projects/" + config_.project_id + "/databases/(default)/documents/" + collection);
    
    std::map<std::string, std::string> headers = getDefaultHeaders();
    
    // For Firestore, we'll use a simple document list for now
    // In a full implementation, you'd parse the query and build the appropriate request
    return makeRequest(url, "GET", headers);
}

nlohmann::json GoogleCloudManager::executeSQLQuery(const nlohmann::json& params) {
    std::string query = params.value("query", "");
    std::string database = params.value("database", "");
    
    if (query.empty() || database.empty()) {
        return {{"error", "Missing required parameters: query, database"}};
    }
    
    std::string url = buildUrl("sql", "/projects/" + config_.project_id + "/instances/" + config_.sql_instance + "/databases/" + database + "/query");
    
    std::map<std::string, std::string> headers = getDefaultHeaders();
    headers["Content-Type"] = "application/json";
    
    nlohmann::json request_body = {
        {"query", query},
        {"params", params.value("parameters", nlohmann::json::array())}
    };
    
    return makeRequest(url, "POST", headers, request_body.dump());
}

nlohmann::json GoogleCloudManager::executeTransaction(const std::string& service, const std::vector<nlohmann::json>& actions) {
    if (!isConnected()) {
        return {{"error", "Not connected to Google Cloud"}};
    }
    
    try {
        if (service == "firestore") {
            return executeFirestoreTransaction(actions);
        } else if (service == "sql") {
            return executeSQLTransaction(actions);
        } else {
            return {{"error", "Unsupported service: " + service}};
        }
    } catch (const std::exception& e) {
        spdlog::error("[GoogleCloud] Transaction execution failed: {}", e.what());
        return {{"error", e.what()}};
    }
}

nlohmann::json GoogleCloudManager::executeFirestoreTransaction(const std::vector<nlohmann::json>& actions) {
    std::string url = buildUrl("firestore", "/projects/" + config_.project_id + "/databases/(default)/documents:commit");
    
    std::map<std::string, std::string> headers = getDefaultHeaders();
    headers["Content-Type"] = "application/json";
    
    nlohmann::json writes = nlohmann::json::array();
    for (const auto& action : actions) {
        writes.push_back({
            {"update", {
                {"name", "projects/" + config_.project_id + "/databases/(default)/documents/" + action["collection"].get<std::string>() + "/" + action["id"].get<std::string>()},
                {"fields", action["data"]}
            }}
        });
    }
    
    nlohmann::json request_body = {
        {"writes", writes}
    };
    
    return makeRequest(url, "POST", headers, request_body.dump());
}

nlohmann::json GoogleCloudManager::executeSQLTransaction(const std::vector<nlohmann::json>& actions) {
    std::string database = actions[0].value("database", "");
    std::string url = buildUrl("sql", "/projects/" + config_.project_id + "/instances/" + config_.sql_instance + "/databases/" + database + "/query");
    
    std::map<std::string, std::string> headers = getDefaultHeaders();
    headers["Content-Type"] = "application/json";
    
    std::string batch_query = "BEGIN TRANSACTION;\n";
    for (const auto& action : actions) {
        batch_query += action["query"].get<std::string>() + ";\n";
    }
    batch_query += "COMMIT;";
    
    nlohmann::json request_body = {
        {"query", batch_query},
        {"params", nlohmann::json::array()}
    };
    
    return makeRequest(url, "POST", headers, request_body.dump());
}

nlohmann::json GoogleCloudManager::uploadFile(const std::string& bucket, const std::string& path, const std::string& file_path) {
    if (!isConnected()) {
        return {{"error", "Not connected to Google Cloud"}};
    }
    
    try {
        std::ifstream file(file_path, std::ios::binary);
        if (!file.is_open()) {
            return {{"error", "Cannot open file: " + file_path}};
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string file_content = buffer.str();
        
        std::string url = "https://storage.googleapis.com/upload/storage/v1/b/" + bucket + "/o";
        url += "?name=" + url_encode(path);
        
        std::map<std::string, std::string> headers = getDefaultHeaders();
        headers["Content-Type"] = "application/octet-stream";
        headers["Content-Length"] = std::to_string(file_content.length());
        
        auto response = makeRequest(url, "POST", headers, file_content);
        
        if (response.contains("error")) {
            return response;
        }
        
        spdlog::info("[GoogleCloud] Successfully uploaded file: {} to bucket: {}, path: {}", file_path, bucket, path);
        return {{"status", "success"}, {"url", "gs://" + bucket + "/" + path}};
        
    } catch (const std::exception& e) {
        spdlog::error("[GoogleCloud] File upload failed: {}", e.what());
        return {{"error", e.what()}};
    }
}

nlohmann::json GoogleCloudManager::downloadFile(const std::string& bucket, const std::string& path, const std::string& local_path) {
    if (!isConnected()) {
        return {{"error", "Not connected to Google Cloud"}};
    }
    
    try {
        std::string url = "https://storage.googleapis.com/storage/v1/b/" + bucket + "/o/" + url_encode(path) + "?alt=media";
        
        std::map<std::string, std::string> headers = getDefaultHeaders();
        
        auto response = makeRequest(url, "GET", headers);
        
        if (response.contains("error")) {
            return response;
        }
        
        // Write file content to local file
        std::ofstream file(local_path, std::ios::binary);
        if (!file.is_open()) {
            return {{"error", "Cannot create local file: " + local_path}};
        }
        
        file.write(response["content"].get<std::string>().c_str(), response["content"].get<std::string>().length());
        file.close();
        
        spdlog::info("[GoogleCloud] Successfully downloaded file from bucket: {}, path: {} to: {}", bucket, path, local_path);
        return {{"status", "success"}, {"local_path", local_path}};
        
    } catch (const std::exception& e) {
        spdlog::error("[GoogleCloud] File download failed: {}", e.what());
        return {{"error", e.what()}};
    }
}

nlohmann::json GoogleCloudManager::invokeFunction(const std::string& function_name, const nlohmann::json& payload) {
    if (!isConnected()) {
        return {{"error", "Not connected to Google Cloud"}};
    }
    
    try {
        std::string url = "https://" + config_.region + "-" + config_.project_id + ".cloudfunctions.net/" + function_name;
        
        std::map<std::string, std::string> headers = getDefaultHeaders();
        headers["Content-Type"] = "application/json";
        
        return makeRequest(url, "POST", headers, payload.dump());
        
    } catch (const std::exception& e) {
        spdlog::error("[GoogleCloud] Function invocation failed: {}", e.what());
        return {{"error", e.what()}};
    }
}

nlohmann::json GoogleCloudManager::getStatus() const {
    return {
        {"connected", isConnected()},
        {"project_id", config_.project_id},
        {"region", config_.region},
        {"storage_bucket", config_.storage_bucket},
        {"sql_instance", config_.sql_instance},
        {"firestore_collection", config_.firestore_collection},
        {"function_name", config_.function_name}
    };
}

void GoogleCloudManager::disconnect() {
    connected_ = false;
    access_token_.clear();
    token_expiry_ = 0;
    
    if (curl_) {
        curl_easy_cleanup(curl_);
        curl_ = nullptr;
    }
    
    curl_global_cleanup();
    spdlog::info("[GoogleCloud] Disconnected");
}

GoogleCloudManager::~GoogleCloudManager() {
    disconnect();
}

std::string GoogleCloudManager::buildUrl(const std::string& service, const std::string& resource) {
    if (service == "firestore") {
        return "https://firestore.googleapis.com/v1" + resource;
    } else if (service == "sql") {
        return "https://sqladmin.googleapis.com/v1" + resource;
    } else if (service == "storage") {
        return "https://storage.googleapis.com/storage/v1" + resource;
    }
    return "";
}

std::map<std::string, std::string> GoogleCloudManager::getDefaultHeaders() {
    return {
        {"Authorization", "Bearer " + getAccessToken()},
        {"Content-Type", "application/json"},
        {"Accept", "application/json"}
    };
}

nlohmann::json GoogleCloudManager::makeRequest(const std::string& url, const std::string& method, 
                                              const std::map<std::string, std::string>& headers, 
                                              const std::string& body) {
    if (!curl_) {
        return {{"error", "CURL not initialized"}};
    }
    
    std::string response_data;
    std::string request_body = body;
    
    curl_easy_reset(curl_);
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response_data);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, config_.timeout_seconds);
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, config_.enable_ssl_verification ? 1L : 0L);
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, config_.enable_ssl_verification ? 2L : 0L);
    
    // Set method
    if (method == "POST") {
        curl_easy_setopt(curl_, CURLOPT_POST, 1L);
        if (!request_body.empty()) {
            curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, request_body.c_str());
            curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, request_body.length());
        }
    } else if (method == "PUT") {
        curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "PUT");
        if (!request_body.empty()) {
            curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, request_body.c_str());
            curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, request_body.length());
        }
    } else if (method == "DELETE") {
        curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "DELETE");
    }
    
    // Set headers
    struct curl_slist* header_list = nullptr;
    for (const auto& header : headers) {
        std::string header_line = header.first + ": " + header.second;
        header_list = curl_slist_append(header_list, header_line.c_str());
    }
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, header_list);
    
    // Execute request
    CURLcode res = curl_easy_perform(curl_);
    
    long http_code = 0;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &http_code);
    
    curl_slist_free_all(header_list);
    
    if (res != CURLE_OK) {
        spdlog::error("[GoogleCloud] CURL error: {}", curl_easy_strerror(res));
        return {{"error", "CURL error: " + std::string(curl_easy_strerror(res))}};
    }
    
    try {
        if (http_code >= 200 && http_code < 300) {
            if (response_data.empty()) {
                return {{"status", "success"}};
            }
            return nlohmann::json::parse(response_data);
        } else {
            spdlog::error("[GoogleCloud] HTTP error {}: {}", http_code, response_data);
            return {{"error", "HTTP " + std::to_string(http_code)}, {"response", response_data}};
        }
    } catch (const nlohmann::json::exception& e) {
        // If response is not JSON, return as content
        return {{"content", response_data}};
    }
}

} // namespace cloud
} // namespace core
} // namespace satox
