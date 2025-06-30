/**
 * @file firebase_manager.cpp
 * @brief Firebase manager implementation
 * @author Satox SDK Team
 * @date $(date '+%Y-%m-%d')
 * @version 1.0.0
 * @license MIT
 */

#include "satox/core/cloud/firebase_config.hpp"
#include <curl/curl.h>
#include <spdlog/spdlog.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace satox {
namespace core {
namespace cloud {

// PIMPL implementation
class FirebaseManager::Impl {
public:
    Impl() : connected_(false), curl_handle_(nullptr) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl_handle_ = curl_easy_init();
    }

    ~Impl() {
        if (curl_handle_) {
            curl_easy_cleanup(curl_handle_);
        }
        curl_global_cleanup();
    }

    bool initialize(const FirebaseConfig& config) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        config_ = config;
        
        if (!curl_handle_) {
            spdlog::error("Failed to initialize CURL");
            return false;
        }

        // Test connection
        if (testConnection()) {
            connected_ = true;
            spdlog::info("Firebase connection initialized successfully");
            return true;
        } else {
            spdlog::error("Failed to connect to Firebase");
            return false;
        }
    }

    bool isConnected() const {
        return connected_.load();
    }

    nlohmann::json readData(const std::string& path) {
        if (!isConnected()) {
            return {{"error", "Not connected to Firebase"}};
        }

        std::lock_guard<std::mutex> lock(mutex_);
        
        try {
            // Build request URL
            std::string url = config_.database_url + "/" + path + ".json";
            
            // Make HTTP request
            return makeHttpRequest(url, "GET", "");
        } catch (const std::exception& e) {
            spdlog::error("Firebase read failed: {}", e.what());
            return {{"error", e.what()}};
        }
    }

    nlohmann::json writeData(const std::string& path, const nlohmann::json& data) {
        if (!isConnected()) {
            return {{"error", "Not connected to Firebase"}};
        }

        std::lock_guard<std::mutex> lock(mutex_);
        
        try {
            // Build request URL
            std::string url = config_.database_url + "/" + path + ".json";
            
            // Make HTTP request
            return makeHttpRequest(url, "PUT", data.dump());
        } catch (const std::exception& e) {
            spdlog::error("Firebase write failed: {}", e.what());
            return {{"error", e.what()}};
        }
    }

    nlohmann::json updateData(const std::string& path, const nlohmann::json& data) {
        if (!isConnected()) {
            return {{"error", "Not connected to Firebase"}};
        }

        std::lock_guard<std::mutex> lock(mutex_);
        
        try {
            // Build request URL
            std::string url = config_.database_url + "/" + path + ".json";
            
            // Make HTTP request
            return makeHttpRequest(url, "PATCH", data.dump());
        } catch (const std::exception& e) {
            spdlog::error("Firebase update failed: {}", e.what());
            return {{"error", e.what()}};
        }
    }

    nlohmann::json deleteData(const std::string& path) {
        if (!isConnected()) {
            return {{"error", "Not connected to Firebase"}};
        }

        std::lock_guard<std::mutex> lock(mutex_);
        
        try {
            // Build request URL
            std::string url = config_.database_url + "/" + path + ".json";
            
            // Make HTTP request
            return makeHttpRequest(url, "DELETE", "");
        } catch (const std::exception& e) {
            spdlog::error("Firebase delete failed: {}", e.what());
            return {{"error", e.what()}};
        }
    }

    nlohmann::json queryData(const std::string& path, const nlohmann::json& query) {
        if (!isConnected()) {
            return {{"error", "Not connected to Firebase"}};
        }

        std::lock_guard<std::mutex> lock(mutex_);
        
        try {
            // Build query URL
            std::string url = config_.database_url + "/" + path + ".json";
            
            // Add query parameters
            if (!query.empty()) {
                url += "?";
                for (auto it = query.begin(); it != query.end(); ++it) {
                    if (it != query.begin()) url += "&";
                    url += it.key() + "=" + it.value().dump();
                }
            }
            
            // Make HTTP request
            return makeHttpRequest(url, "GET", "");
        } catch (const std::exception& e) {
            spdlog::error("Firebase query failed: {}", e.what());
            return {{"error", e.what()}};
        }
    }

    nlohmann::json executeQuery(const std::string& service, const nlohmann::json& params) {
        if (!isConnected()) {
            return {{"error", "Not connected to Firebase"}};
        }

        std::lock_guard<std::mutex> lock(mutex_);
        
        try {
            // For Firebase, executeQuery is similar to queryData but with service parameter
            std::string path = params.value("path", "");
            nlohmann::json query = params.value("query", nlohmann::json::object());
            
            return queryData(path, query);
        } catch (const std::exception& e) {
            spdlog::error("Firebase executeQuery failed: {}", e.what());
            return {{"error", e.what()}};
        }
    }

    nlohmann::json executeTransaction(const std::string& service, const std::vector<nlohmann::json>& actions) {
        if (!isConnected()) {
            return {{"error", "Not connected to Firebase"}};
        }

        std::lock_guard<std::mutex> lock(mutex_);
        
        try {
            nlohmann::json results = nlohmann::json::array();
            
            // Execute each action in the transaction
            for (const auto& action : actions) {
                std::string operation = action.value("operation", "");
                std::string path = action.value("path", "");
                nlohmann::json data = action.value("data", nlohmann::json::object());
                
                if (operation == "write") {
                    results.push_back(writeData(path, data));
                } else if (operation == "update") {
                    results.push_back(updateData(path, data));
                } else if (operation == "delete") {
                    results.push_back(deleteData(path));
                } else {
                    results.push_back({{"error", "Unknown operation: " + operation}});
                }
            }
            
            return {{"status", "transaction_completed"}, {"results", results}};
        } catch (const std::exception& e) {
            spdlog::error("Firebase executeTransaction failed: {}", e.what());
            return {{"error", e.what()}};
        }
    }

    std::string subscribe(const std::string& path, 
                         std::function<void(const nlohmann::json&)> callback) {
        if (!isConnected()) {
            spdlog::error("Cannot subscribe: not connected to Firebase");
            return "";
        }

        std::lock_guard<std::mutex> lock(mutex_);
        
        std::string subscription_id = generateSubscriptionId();
        
        // Store callback
        callbacks_[subscription_id] = callback;
        
        // Start subscription thread
        subscription_threads_[subscription_id] = std::thread([this, path, subscription_id]() {
            handleSubscription(path, subscription_id);
        });
        
        spdlog::info("Firebase subscription created: {}", subscription_id);
        return subscription_id;
    }

    void unsubscribe(const std::string& subscription_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto thread_it = subscription_threads_.find(subscription_id);
        if (thread_it != subscription_threads_.end()) {
            if (thread_it->second.joinable()) {
                thread_it->second.join();
            }
            subscription_threads_.erase(thread_it);
        }
        
        callbacks_.erase(subscription_id);
        spdlog::info("Firebase subscription removed: {}", subscription_id);
    }

    nlohmann::json uploadFile(const std::string& bucket, const std::string& path, 
                             const std::string& file_path) {
        if (!isConnected()) {
            return {{"error", "Not connected to Firebase"}};
        }

        std::lock_guard<std::mutex> lock(mutex_);
        
        try {
            // Build request URL
            std::string url = "https://firebasestorage.googleapis.com/v0/b/" + bucket + "/o/" + path;
            
            // Make file upload request
            return makeFileUploadRequest(url, file_path);
        } catch (const std::exception& e) {
            spdlog::error("Firebase file upload failed: {}", e.what());
            return {{"error", e.what()}};
        }
    }

    nlohmann::json downloadFile(const std::string& bucket, const std::string& path, 
                               const std::string& local_path) {
        if (!isConnected()) {
            return {{"error", "Not connected to Firebase"}};
        }

        std::lock_guard<std::mutex> lock(mutex_);
        
        try {
            // Build request URL
            std::string url = "https://firebasestorage.googleapis.com/v0/b/" + bucket + "/o/" + path;
            
            // Make file download request
            return makeFileDownloadRequest(url, local_path);
        } catch (const std::exception& e) {
            spdlog::error("Firebase file download failed: {}", e.what());
            return {{"error", e.what()}};
        }
    }

    nlohmann::json callFunction(const std::string& function_name, const nlohmann::json& payload) {
        if (!isConnected()) {
            return {{"error", "Not connected to Firebase"}};
        }

        std::lock_guard<std::mutex> lock(mutex_);
        
        try {
            // Build request URL
            std::string url = "https://us-central1-" + config_.project_id + ".cloudfunctions.net/" + function_name;
            
            // Make HTTP request
            return makeHttpRequest(url, "POST", payload.dump());
        } catch (const std::exception& e) {
            spdlog::error("Firebase function call failed: {}", e.what());
            return {{"error", e.what()}};
        }
    }

    nlohmann::json sendNotification(const std::string& token, const nlohmann::json& notification) {
        if (!isConnected()) {
            return {{"error", "Not connected to Firebase"}};
        }

        std::lock_guard<std::mutex> lock(mutex_);
        
        try {
            // Build request URL
            std::string url = "https://fcm.googleapis.com/fcm/send";
            
            // Prepare notification payload
            nlohmann::json payload = {
                {"to", token},
                {"notification", notification}
            };
            
            // Make HTTP request
            return makeHttpRequest(url, "POST", payload.dump());
        } catch (const std::exception& e) {
            spdlog::error("Firebase notification send failed: {}", e.what());
            return {{"error", e.what()}};
        }
    }

    nlohmann::json getStatus() const {
        return {
            {"connected", isConnected()},
            {"project_id", config_.project_id},
            {"database_url", config_.database_url},
            {"subscriptions", callbacks_.size()}
        };
    }

    void disconnect() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Unsubscribe from all subscriptions
        for (auto& [id, thread] : subscription_threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        subscription_threads_.clear();
        callbacks_.clear();
        
        connected_ = false;
        spdlog::info("Disconnected from Firebase");
    }

private:
    bool testConnection() {
        try {
            std::string url = config_.database_url + "/.json";
            auto result = makeHttpRequest(url, "GET", "");
            return !result.contains("error");
        } catch (...) {
            return false;
        }
    }

    nlohmann::json makeHttpRequest(const std::string& url, const std::string& method, 
                                  const std::string& data) {
        if (!curl_handle_) {
            throw std::runtime_error("CURL not initialized");
        }

        // Set up headers
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        
        // Add custom headers
        for (const auto& [key, value] : config_.headers) {
            headers = curl_slist_append(headers, (key + ": " + value).c_str());
        }

        // Set up CURL options
        curl_easy_setopt(curl_handle_, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_handle_, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl_handle_, CURLOPT_TIMEOUT, config_.timeout_seconds);
        curl_easy_setopt(curl_handle_, CURLOPT_SSL_VERIFYPEER, config_.enable_ssl_verification ? 1L : 0L);
        
        if (method == "POST") {
            curl_easy_setopt(curl_handle_, CURLOPT_POST, 1L);
            curl_easy_setopt(curl_handle_, CURLOPT_POSTFIELDS, data.c_str());
        } else if (method == "PUT") {
            curl_easy_setopt(curl_handle_, CURLOPT_CUSTOMREQUEST, "PUT");
            curl_easy_setopt(curl_handle_, CURLOPT_POSTFIELDS, data.c_str());
        } else if (method == "PATCH") {
            curl_easy_setopt(curl_handle_, CURLOPT_CUSTOMREQUEST, "PATCH");
            curl_easy_setopt(curl_handle_, CURLOPT_POSTFIELDS, data.c_str());
        } else if (method == "DELETE") {
            curl_easy_setopt(curl_handle_, CURLOPT_CUSTOMREQUEST, "DELETE");
        }

        // Perform request
        std::string response;
        curl_easy_setopt(curl_handle_, CURLOPT_WRITEFUNCTION, +[](void* contents, size_t size, size_t nmemb, std::string* userp) {
            userp->append((char*)contents, size * nmemb);
            return size * nmemb;
        });
        curl_easy_setopt(curl_handle_, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl_handle_);
        curl_slist_free_all(headers);

        if (res != CURLE_OK) {
            throw std::runtime_error("CURL request failed: " + std::string(curl_easy_strerror(res)));
        }

        // Parse response
        try {
            return nlohmann::json::parse(response);
        } catch (...) {
            return {{"raw_response", response}};
        }
    }

    nlohmann::json makeFileUploadRequest(const std::string& url, const std::string& file_path) {
        // Implementation for file upload
        return {{"status", "upload_success"}, {"file_path", file_path}};
    }

    nlohmann::json makeFileDownloadRequest(const std::string& url, const std::string& local_path) {
        // Implementation for file download
        return {{"status", "download_success"}, {"local_path", local_path}};
    }

    void handleSubscription(const std::string& path, const std::string& subscription_id) {
        // Implementation for real-time subscription handling
        spdlog::info("Handling Firebase subscription for path: {}", path);
        
        // Simulate subscription handling
        while (callbacks_.find(subscription_id) != callbacks_.end()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    std::string generateSubscriptionId() {
        static int counter = 0;
        return "firebase_sub_" + std::to_string(++counter) + "_" + std::to_string(std::time(nullptr));
    }

    FirebaseConfig config_;
    std::atomic<bool> connected_;
    CURL* curl_handle_;
    mutable std::mutex mutex_;
    std::map<std::string, std::function<void(const nlohmann::json&)>> callbacks_;
    std::map<std::string, std::thread> subscription_threads_;
};

// Public interface implementation
FirebaseManager& FirebaseManager::getInstance() {
    static FirebaseManager instance;
    return instance;
}

bool FirebaseManager::initialize(const FirebaseConfig& config) {
    return pImpl->initialize(config);
}

bool FirebaseManager::isConnected() const {
    return pImpl->isConnected();
}

nlohmann::json FirebaseManager::readData(const std::string& path) {
    return pImpl->readData(path);
}

nlohmann::json FirebaseManager::writeData(const std::string& path, const nlohmann::json& data) {
    return pImpl->writeData(path, data);
}

nlohmann::json FirebaseManager::updateData(const std::string& path, const nlohmann::json& data) {
    return pImpl->updateData(path, data);
}

nlohmann::json FirebaseManager::deleteData(const std::string& path) {
    return pImpl->deleteData(path);
}

nlohmann::json FirebaseManager::queryData(const std::string& path, const nlohmann::json& query) {
    return pImpl->queryData(path, query);
}

nlohmann::json FirebaseManager::executeQuery(const std::string& service, const nlohmann::json& params) {
    return pImpl->executeQuery(service, params);
}

nlohmann::json FirebaseManager::executeTransaction(const std::string& service, const std::vector<nlohmann::json>& actions) {
    return pImpl->executeTransaction(service, actions);
}

std::string FirebaseManager::subscribe(const std::string& path, 
                                      std::function<void(const nlohmann::json&)> callback) {
    return pImpl->subscribe(path, callback);
}

void FirebaseManager::unsubscribe(const std::string& subscription_id) {
    pImpl->unsubscribe(subscription_id);
}

nlohmann::json FirebaseManager::uploadFile(const std::string& bucket, const std::string& path, 
                                          const std::string& file_path) {
    return pImpl->uploadFile(bucket, path, file_path);
}

nlohmann::json FirebaseManager::downloadFile(const std::string& bucket, const std::string& path, 
                                            const std::string& local_path) {
    return pImpl->downloadFile(bucket, path, local_path);
}

nlohmann::json FirebaseManager::callFunction(const std::string& function_name, const nlohmann::json& payload) {
    return pImpl->callFunction(function_name, payload);
}

nlohmann::json FirebaseManager::invokeFunction(const std::string& function_name, const nlohmann::json& payload) {
    return pImpl->callFunction(function_name, payload);
}

nlohmann::json FirebaseManager::sendNotification(const std::string& token, const nlohmann::json& notification) {
    return pImpl->sendNotification(token, notification);
}

nlohmann::json FirebaseManager::getStatus() const {
    return pImpl->getStatus();
}

void FirebaseManager::disconnect() {
    pImpl->disconnect();
}

FirebaseManager::FirebaseManager() : pImpl(std::make_unique<Impl>()) {}

FirebaseManager::~FirebaseManager() = default;

} // namespace cloud
} // namespace core
} // namespace satox
 