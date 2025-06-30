/**
 * @file azure_manager.cpp
 * @brief Azure manager implementation
 * @author Satox SDK Team
 * @date $(date '+%Y-%m-%d')
 * @version 1.0.0
 * @license MIT
 */

#include "satox/core/cloud/azure_manager.hpp"
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

static size_t ReadCallback(void* ptr, size_t size, size_t nmemb, std::string* userp) {
    if (userp->empty()) return 0;
    size_t len = std::min(size * nmemb, userp->length());
    memcpy(ptr, userp->c_str(), len);
    userp->erase(0, len);
    return len;
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

AzureManager& AzureManager::getInstance() {
    static AzureManager instance;
    return instance;
}

bool AzureManager::initialize(const AzureConfig& config) {
    config_ = config;
    
    // Initialize CURL
    curl_global_init(CURL_GLOBAL_ALL);
    curl_ = curl_easy_init();
    if (!curl_) {
        spdlog::error("[Azure] Failed to initialize CURL");
        return false;
    }
    
    // Test connection by getting access token
    try {
        access_token_ = getAccessToken();
        if (!access_token_.empty()) {
            connected_ = true;
            spdlog::info("[Azure] Successfully initialized with tenant: {}", config.tenant_id);
            return true;
        }
    } catch (const std::exception& e) {
        spdlog::error("[Azure] Initialization failed: {}", e.what());
    }
    
    connected_ = false;
    return false;
}

bool AzureManager::isConnected() const {
    return connected_;
}

std::string AzureManager::getAccessToken() {
    // Check if token is still valid (with 5 minute buffer)
    if (!access_token_.empty() && std::time(nullptr) < token_expiry_ - 300) {
        return access_token_;
    }
    
    // Get new token using client credentials flow
    std::string token_url = "https://login.microsoftonline.com/" + config_.tenant_id + "/oauth2/v2.0/token";
    
    std::map<std::string, std::string> headers = {
        {"Content-Type", "application/x-www-form-urlencoded"}
    };
    
    std::string body = "grant_type=client_credentials"
                      "&client_id=" + url_encode(config_.client_id) +
                      "&client_secret=" + url_encode(config_.client_secret) +
                      "&scope=" + url_encode("https://management.azure.com/.default");
    
    auto response = makeRequest(token_url, "POST", headers, body);
    
    if (response.contains("access_token")) {
        access_token_ = response["access_token"];
        token_expiry_ = std::time(nullptr) + response["expires_in"].get<int>();
        spdlog::debug("[Azure] Got new access token, expires in {} seconds", response["expires_in"].get<int>());
        return access_token_;
    }
    
    spdlog::error("[Azure] Failed to get access token: {}", response.dump());
    return "";
}

nlohmann::json AzureManager::executeQuery(const std::string& service, const nlohmann::json& params) {
    if (!isConnected()) {
        return {{"error", "Not connected to Azure"}};
    }
    
    try {
        if (service == "cosmosdb") {
            return executeCosmosDBQuery(params);
        } else if (service == "sql") {
            return executeSQLQuery(params);
        } else {
            return {{"error", "Unsupported service: " + service}};
        }
    } catch (const std::exception& e) {
        spdlog::error("[Azure] Query execution failed: {}", e.what());
        return {{"error", e.what()}};
    }
}

nlohmann::json AzureManager::executeCosmosDBQuery(const nlohmann::json& params) {
    std::string query = params.value("query", "");
    std::string database = params.value("database", "");
    std::string container = params.value("container", "");
    
    if (query.empty() || database.empty() || container.empty()) {
        return {{"error", "Missing required parameters: query, database, container"}};
    }
    
    std::string url = buildUrl("cosmosdb", "/dbs/" + database + "/colls/" + container + "/docs");
    
    std::map<std::string, std::string> headers = getDefaultHeaders();
    headers["x-ms-documentdb-isquery"] = "true";
    headers["x-ms-documentdb-query-enablecrosspartition"] = "true";
    
    nlohmann::json request_body = {
        {"query", query},
        {"parameters", params.value("parameters", nlohmann::json::array())}
    };
    
    return makeRequest(url, "POST", headers, request_body.dump());
}

nlohmann::json AzureManager::executeSQLQuery(const nlohmann::json& params) {
    std::string query = params.value("query", "");
    std::string database = params.value("database", "");
    
    if (query.empty() || database.empty()) {
        return {{"error", "Missing required parameters: query, database"}};
    }
    
    std::string url = buildUrl("sql", "/servers/" + config_.sql_server + "/databases/" + database + "/query");
    
    std::map<std::string, std::string> headers = getDefaultHeaders();
    headers["Content-Type"] = "application/json";
    
    nlohmann::json request_body = {
        {"query", query},
        {"parameters", params.value("parameters", nlohmann::json::array())}
    };
    
    return makeRequest(url, "POST", headers, request_body.dump());
}

nlohmann::json AzureManager::executeTransaction(const std::string& service, const std::vector<nlohmann::json>& actions) {
    if (!isConnected()) {
        return {{"error", "Not connected to Azure"}};
    }
    
    try {
        if (service == "cosmosdb") {
            return executeCosmosDBTransaction(actions);
        } else if (service == "sql") {
            return executeSQLTransaction(actions);
        } else {
            return {{"error", "Unsupported service: " + service}};
        }
    } catch (const std::exception& e) {
        spdlog::error("[Azure] Transaction execution failed: {}", e.what());
        return {{"error", e.what()}};
    }
}

nlohmann::json AzureManager::executeCosmosDBTransaction(const std::vector<nlohmann::json>& actions) {
    // Cosmos DB transactions require a batch request
    std::string url = buildUrl("cosmosdb", "/dbs/" + actions[0].value("database", "") + "/colls/" + actions[0].value("container", "") + "/docs");
    
    std::map<std::string, std::string> headers = getDefaultHeaders();
    headers["x-ms-documentdb-partitionkey"] = actions[0].value("partitionKey", "[]");
    headers["x-ms-documentdb-is-upsert"] = "true";
    
    nlohmann::json batch_operations = nlohmann::json::array();
    for (const auto& action : actions) {
        batch_operations.push_back({
            {"operationType", action["operation"]},
            {"id", action["id"]},
            {"resourceBody", action["data"]}
        });
    }
    
    return makeRequest(url, "POST", headers, batch_operations.dump());
}

nlohmann::json AzureManager::executeSQLTransaction(const std::vector<nlohmann::json>& actions) {
    // SQL transactions are executed as a single batch
    std::string database = actions[0].value("database", "");
    std::string url = buildUrl("sql", "/servers/" + config_.sql_server + "/databases/" + database + "/query");
    
    std::map<std::string, std::string> headers = getDefaultHeaders();
    headers["Content-Type"] = "application/json";
    
    std::string batch_query = "BEGIN TRANSACTION;\n";
    for (const auto& action : actions) {
        batch_query += action["query"].get<std::string>() + ";\n";
    }
    batch_query += "COMMIT;";
    
    nlohmann::json request_body = {
        {"query", batch_query},
        {"parameters", nlohmann::json::array()}
    };
    
    return makeRequest(url, "POST", headers, request_body.dump());
}

nlohmann::json AzureManager::uploadFile(const std::string& container, const std::string& path, const std::string& file_path) {
    if (!isConnected()) {
        return {{"error", "Not connected to Azure"}};
    }
    
    try {
        std::ifstream file(file_path, std::ios::binary);
        if (!file.is_open()) {
            return {{"error", "Cannot open file: " + file_path}};
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string file_content = buffer.str();
        
        std::string url = "https://" + config_.storage_account + ".blob.core.windows.net/" + container + "/" + path;
        
        std::map<std::string, std::string> headers = {
            {"x-ms-version", "2020-04-08"},
            {"x-ms-blob-type", "BlockBlob"},
            {"Content-Length", std::to_string(file_content.length())}
        };
        
        // Add authorization header
        std::string auth_header = "Bearer " + getAccessToken();
        headers["Authorization"] = auth_header;
        
        auto response = makeRequest(url, "PUT", headers, file_content);
        
        if (response.contains("error")) {
            return response;
        }
        
        spdlog::info("[Azure] Successfully uploaded file: {} to container: {}, path: {}", file_path, container, path);
        return {{"status", "success"}, {"url", url}};
        
    } catch (const std::exception& e) {
        spdlog::error("[Azure] File upload failed: {}", e.what());
        return {{"error", e.what()}};
    }
}

nlohmann::json AzureManager::downloadFile(const std::string& container, const std::string& path, const std::string& local_path) {
    if (!isConnected()) {
        return {{"error", "Not connected to Azure"}};
    }
    
    try {
        std::string url = "https://" + config_.storage_account + ".blob.core.windows.net/" + container + "/" + path;
        
        std::map<std::string, std::string> headers = {
            {"x-ms-version", "2020-04-08"}
        };
        
        // Add authorization header
        std::string auth_header = "Bearer " + getAccessToken();
        headers["Authorization"] = auth_header;
        
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
        
        spdlog::info("[Azure] Successfully downloaded file from container: {}, path: {} to: {}", container, path, local_path);
        return {{"status", "success"}, {"local_path", local_path}};
        
    } catch (const std::exception& e) {
        spdlog::error("[Azure] File download failed: {}", e.what());
        return {{"error", e.what()}};
    }
}

nlohmann::json AzureManager::invokeFunction(const std::string& function_name, const nlohmann::json& payload) {
    if (!isConnected()) {
        return {{"error", "Not connected to Azure"}};
    }
    
    try {
        std::string url = "https://" + config_.function_app + ".azurewebsites.net/api/" + function_name;
        
        std::map<std::string, std::string> headers = {
            {"Content-Type", "application/json"},
            {"x-functions-key", config_.client_secret} // Using client secret as function key for simplicity
        };
        
        return makeRequest(url, "POST", headers, payload.dump());
        
    } catch (const std::exception& e) {
        spdlog::error("[Azure] Function invocation failed: {}", e.what());
        return {{"error", e.what()}};
    }
}

nlohmann::json AzureManager::getStatus() const {
    return {
        {"connected", isConnected()},
        {"tenant_id", config_.tenant_id},
        {"subscription_id", config_.subscription_id},
        {"resource_group", config_.resource_group},
        {"cosmosdb_account", config_.cosmosdb_account},
        {"sql_server", config_.sql_server},
        {"storage_account", config_.storage_account},
        {"function_app", config_.function_app}
    };
}

void AzureManager::disconnect() {
    connected_ = false;
    access_token_.clear();
    token_expiry_ = 0;
    
    if (curl_) {
        curl_easy_cleanup(curl_);
        curl_ = nullptr;
    }
    
    curl_global_cleanup();
    spdlog::info("[Azure] Disconnected");
}

AzureManager::~AzureManager() {
    disconnect();
}

std::string AzureManager::buildUrl(const std::string& service, const std::string& resource) {
    if (service == "cosmosdb") {
        return "https://" + config_.cosmosdb_account + ".documents.azure.com" + resource;
    } else if (service == "sql") {
        return "https://management.azure.com/subscriptions/" + config_.subscription_id + 
               "/resourceGroups/" + config_.resource_group + 
               "/providers/Microsoft.Sql/servers/" + config_.sql_server + resource;
    }
    return "";
}

std::map<std::string, std::string> AzureManager::getDefaultHeaders() {
    return {
        {"Authorization", "Bearer " + getAccessToken()},
        {"Content-Type", "application/json"},
        {"Accept", "application/json"}
    };
}

nlohmann::json AzureManager::makeRequest(const std::string& url, const std::string& method, 
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
        spdlog::error("[Azure] CURL error: {}", curl_easy_strerror(res));
        return {{"error", "CURL error: " + std::string(curl_easy_strerror(res))}};
    }
    
    try {
        if (http_code >= 200 && http_code < 300) {
            if (response_data.empty()) {
                return {{"status", "success"}};
            }
            return nlohmann::json::parse(response_data);
        } else {
            spdlog::error("[Azure] HTTP error {}: {}", http_code, response_data);
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