/**
 * @file supabase_manager.cpp
 * @brief Supabase database manager implementation
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

#include "satox/core/cloud/supabase_manager.hpp"
#include <curl/curl.h>
#include <spdlog/spdlog.h>
#include <sstream>
#include <thread>
#include <chrono>

namespace satox {
namespace core {

// Forward declarations
struct SupabaseConnection {
    std::string id;
    SupabaseConfig config;
    CURL* curl_handle;
    bool connected;
    std::chrono::system_clock::time_point last_activity;
    std::unordered_map<std::string, std::function<void(const nlohmann::json&)>> subscriptions;
};

struct SupabaseManager::Impl {
    std::mutex mutex_;
    bool initialized_;
    std::unordered_map<std::string, std::unique_ptr<SupabaseConnection>> connections_;
    std::string last_error_;
    SupabaseConfig config_;
    
    Impl() : initialized_(false) {
        curl_global_init(CURL_GLOBAL_ALL);
    }
    
    ~Impl() {
        curl_global_cleanup();
    }
    
    // HTTP response callback
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        userp->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
    
    // HTTP request helper
    bool makeHttpRequest(const std::string& url, 
                        const std::string& method,
                        const nlohmann::json& headers,
                        const nlohmann::json& data,
                        nlohmann::json& response) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            last_error_ = "Failed to initialize CURL";
            return false;
        }
        
        std::string response_data;
        struct curl_slist* header_list = nullptr;
        
        // Set headers
        for (const auto& [key, value] : headers.items()) {
            std::string header = key + ": " + value.get<std::string>();
            header_list = curl_slist_append(header_list, header.c_str());
        }
        
        // Set CURL options
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, config_.connection_timeout);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, config_.enable_ssl ? 1L : 0L);
        
        if (method == "POST" || method == "PUT" || method == "PATCH") {
            std::string json_data = data.dump();
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
            
            if (method == "PUT") {
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            } else if (method == "PATCH") {
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
            }
        } else if (method == "DELETE") {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        }
        
        // Execute request
        CURLcode res = curl_easy_perform(curl);
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        
        // Cleanup
        curl_slist_free_all(header_list);
        curl_easy_cleanup(curl);
        
        if (res != CURLE_OK) {
            last_error_ = "CURL error: " + std::string(curl_easy_strerror(res));
            return false;
        }
        
        if (http_code >= 400) {
            last_error_ = "HTTP error: " + std::to_string(http_code);
            return false;
        }
        
        try {
            response = nlohmann::json::parse(response_data);
            return true;
        } catch (const std::exception& e) {
            last_error_ = "JSON parse error: " + std::string(e.what());
            return false;
        }
    }
};

SupabaseManager::SupabaseManager() : impl_(std::make_unique<Impl>()) {}

SupabaseManager::~SupabaseManager() = default;

bool SupabaseManager::initialize(const SupabaseConfig& config) {
    std::lock_guard<std::mutex> lock(impl_->mutex_);
    
    if (impl_->initialized_) {
        impl_->last_error_ = "SupabaseManager already initialized";
        return false;
    }
    
    if (!config.isValid()) {
        impl_->last_error_ = "Invalid Supabase configuration";
        return false;
    }
    
    impl_->config_ = config;
    impl_->initialized_ = true;
    
    if (config.enable_logging) {
        spdlog::info("SupabaseManager initialized with URL: {}", config.url);
    }
    
    return true;
}

std::string SupabaseManager::connect() {
    std::lock_guard<std::mutex> lock(impl_->mutex_);
    
    if (!impl_->initialized_) {
        impl_->last_error_ = "SupabaseManager not initialized";
        return "";
    }
    
    // Generate connection ID
    std::string connection_id = "supabase_" + std::to_string(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count()
    );
    
    // Create connection
    auto connection = std::make_unique<SupabaseConnection>();
    connection->id = connection_id;
    connection->config = impl_->config_;
    connection->curl_handle = curl_easy_init();
    connection->connected = true;
    connection->last_activity = std::chrono::system_clock::now();
    
    if (!connection->curl_handle) {
        impl_->last_error_ = "Failed to initialize CURL handle";
        return "";
    }
    
    impl_->connections_[connection_id] = std::move(connection);
    
    if (impl_->config_.enable_logging) {
        spdlog::info("Supabase connection established: {}", connection_id);
    }
    
    return connection_id;
}

bool SupabaseManager::disconnect(const std::string& connection_id) {
    std::lock_guard<std::mutex> lock(impl_->mutex_);
    
    auto it = impl_->connections_.find(connection_id);
    if (it == impl_->connections_.end()) {
        impl_->last_error_ = "Connection not found: " + connection_id;
        return false;
    }
    
    if (it->second->curl_handle) {
        curl_easy_cleanup(it->second->curl_handle);
    }
    
    impl_->connections_.erase(it);
    
    if (impl_->config_.enable_logging) {
        spdlog::info("Supabase connection closed: {}", connection_id);
    }
    
    return true;
}

bool SupabaseManager::executeQuery(const std::string& connection_id,
                                  const std::string& query,
                                  const nlohmann::json& params,
                                  nlohmann::json& result) {
    std::lock_guard<std::mutex> lock(impl_->mutex_);
    
    auto it = impl_->connections_.find(connection_id);
    if (it == impl_->connections_.end()) {
        impl_->last_error_ = "Connection not found: " + connection_id;
        return false;
    }
    
    if (!it->second->connected) {
        impl_->last_error_ = "Connection not active: " + connection_id;
        return false;
    }
    
    // Build request URL and headers
    std::string url = impl_->config_.url + "/rest/v1/rpc/exec_sql";
    nlohmann::json headers = {
        {"apikey", impl_->config_.anon_key},
        {"Authorization", "Bearer " + impl_->config_.anon_key},
        {"Content-Type", "application/json"},
        {"Prefer", "return=representation"}
    };
    
    nlohmann::json request_data = {
        {"query", query},
        {"params", params}
    };
    
    // Execute request
    bool success = impl_->makeHttpRequest(url, "POST", headers, request_data, result);
    
    if (success) {
        it->second->last_activity = std::chrono::system_clock::now();
    }
    
    return success;
}

bool SupabaseManager::executeTransaction(const std::string& connection_id,
                                        const std::vector<std::string>& queries,
                                        nlohmann::json& result) {
    std::lock_guard<std::mutex> lock(impl_->mutex_);
    
    auto it = impl_->connections_.find(connection_id);
    if (it == impl_->connections_.end()) {
        impl_->last_error_ = "Connection not found: " + connection_id;
        return false;
    }
    
    if (!it->second->connected) {
        impl_->last_error_ = "Connection not active: " + connection_id;
        return false;
    }
    
    // Build request URL and headers
    std::string url = impl_->config_.url + "/rest/v1/rpc/exec_transaction";
    nlohmann::json headers = {
        {"apikey", impl_->config_.anon_key},
        {"Authorization", "Bearer " + impl_->config_.anon_key},
        {"Content-Type", "application/json"},
        {"Prefer", "return=representation"}
    };
    
    nlohmann::json request_data = {
        {"queries", queries}
    };
    
    // Execute request
    bool success = impl_->makeHttpRequest(url, "POST", headers, request_data, result);
    
    if (success) {
        it->second->last_activity = std::chrono::system_clock::now();
    }
    
    return success;
}

std::string SupabaseManager::subscribe(const std::string& connection_id,
                                      const std::string& table,
                                      std::function<void(const nlohmann::json&)> callback) {
    std::lock_guard<std::mutex> lock(impl_->mutex_);
    
    auto it = impl_->connections_.find(connection_id);
    if (it == impl_->connections_.end()) {
        impl_->last_error_ = "Connection not found: " + connection_id;
        return "";
    }
    
    if (!impl_->config_.enable_realtime) {
        impl_->last_error_ = "Real-time subscriptions not enabled";
        return "";
    }
    
    // Generate subscription ID
    std::string subscription_id = "sub_" + std::to_string(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count()
    );
    
    // Store callback
    it->second->subscriptions[subscription_id] = callback;
    
    if (impl_->config_.enable_logging) {
        spdlog::info("Supabase subscription created: {} for table: {}", subscription_id, table);
    }
    
    return subscription_id;
}

bool SupabaseManager::unsubscribe(const std::string& connection_id, const std::string& subscription_id) {
    std::lock_guard<std::mutex> lock(impl_->mutex_);
    
    auto it = impl_->connections_.find(connection_id);
    if (it == impl_->connections_.end()) {
        impl_->last_error_ = "Connection not found: " + connection_id;
        return false;
    }
    
    auto sub_it = it->second->subscriptions.find(subscription_id);
    if (sub_it == it->second->subscriptions.end()) {
        impl_->last_error_ = "Subscription not found: " + subscription_id;
        return false;
    }
    
    it->second->subscriptions.erase(sub_it);
    
    if (impl_->config_.enable_logging) {
        spdlog::info("Supabase subscription removed: {}", subscription_id);
    }
    
    return true;
}

bool SupabaseManager::isConnected(const std::string& connection_id) const {
    std::lock_guard<std::mutex> lock(impl_->mutex_);
    
    auto it = impl_->connections_.find(connection_id);
    return it != impl_->connections_.end() && it->second->connected;
}

std::string SupabaseManager::getLastError() const {
    std::lock_guard<std::mutex> lock(impl_->mutex_);
    return impl_->last_error_;
}

void SupabaseManager::clearError() {
    std::lock_guard<std::mutex> lock(impl_->mutex_);
    impl_->last_error_.clear();
}

void SupabaseManager::shutdown() {
    std::lock_guard<std::mutex> lock(impl_->mutex_);
    
    for (auto& [id, connection] : impl_->connections_) {
        if (connection->curl_handle) {
            curl_easy_cleanup(connection->curl_handle);
        }
    }
    
    impl_->connections_.clear();
    impl_->initialized_ = false;
    
    if (impl_->config_.enable_logging) {
        spdlog::info("SupabaseManager shutdown complete");
    }
}

} // namespace core
} // namespace satox
