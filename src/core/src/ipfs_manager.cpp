/**
 * @file $(basename "$1")
 * @brief $(basename "$1" | sed 's/\./_/g' | tr '[:lower:]' '[:upper:]')
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "satox/core/ipfs_manager.hpp"
#include "satox/core/logging_manager.hpp"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <thread>
#include <chrono>
#include <algorithm>
#include <random>

namespace satox::core {

using ContentInfo = IPFSManager::ContentInfo;
using ContentType = IPFSManager::ContentType;
using IPFSStats = IPFSManager::IPFSStats;

namespace {
    size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        userp->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    std::string calculateCID(const std::vector<uint8_t>& data) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, data.data(), data.size());
        SHA256_Final(hash, &sha256);

        std::stringstream ss;
        ss << "Qm";
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
        }
        return ss.str();
    }
}

IPFSManager& IPFSManager::getInstance() {
    static IPFSManager instance;
    return instance;
}

bool IPFSManager::initialize(const nlohmann::json& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        lastError_ = "IPFS Manager already initialized";
        return false;
    }

    if (!validateConfig(config)) {
        lastError_ = "Invalid configuration";
        return false;
    }

    initialized_ = true;
    statsEnabled_ = config.value("enableStats", false);
    return true;
}

void IPFSManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return;
    }

    nodes_.clear();
    content_.clear();
    contentData_.clear();
    contentTags_.clear();
    nodeCallbacks_.clear();
    contentCallbacks_.clear();
    errorCallbacks_.clear();
    initialized_ = false;
}

bool IPFSManager::connectNode(const std::string& address) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "IPFS Manager not initialized";
        return false;
    }

    if (!validateNodeAddress(address)) {
        lastError_ = "Invalid node address";
        return false;
    }

    if (!checkNodeLimit()) {
        lastError_ = "Node limit reached";
        return false;
    }

    IPFSManager::NodeInfo node{
        generateNodeId(),
        address,
        {address},
        {"ipfs/1.0.0"},
        "1.0.0",
        "satox-ipfs/1.0.0",
        IPFSManager::NodeState::CONNECTING,
        std::chrono::system_clock::now(),
        nlohmann::json::object()
    };

    nodes_[address] = node;
    updateNodeState(address, IPFSManager::NodeState::CONNECTED);
    updateIPFSStats();
    notifyNodeChange(address, IPFSManager::NodeState::CONNECTED);
    return true;
}

bool IPFSManager::disconnectNode(const std::string& address) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "IPFS Manager not initialized";
        return false;
    }

    auto it = nodes_.find(address);
    if (it == nodes_.end()) {
        lastError_ = "Node not found";
        return false;
    }

    updateNodeState(address, IPFSManager::NodeState::DISCONNECTED);
    nodes_.erase(it);
    updateIPFSStats();
    notifyNodeChange(address, IPFSManager::NodeState::DISCONNECTED);
    return true;
}

bool IPFSManager::isNodeConnected(const std::string& address) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = nodes_.find(address);
    if (it == nodes_.end()) {
        return false;
    }
    return it->second.state == IPFSManager::NodeState::CONNECTED;
}

IPFSManager::NodeInfo IPFSManager::getNodeInfo(const std::string& address) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = nodes_.find(address);
    if (it == nodes_.end()) {
        return IPFSManager::NodeInfo{};
    }
    return it->second;
}

std::vector<IPFSManager::NodeInfo> IPFSManager::getAllNodes() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<IPFSManager::NodeInfo> result;
    for (const auto& [address, node] : nodes_) {
        result.push_back(node);
    }
    return result;
}

std::vector<IPFSManager::NodeInfo> IPFSManager::getNodesByState(IPFSManager::NodeState state) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<IPFSManager::NodeInfo> result;
    for (const auto& [address, node] : nodes_) {
        if (node.state == state) {
            result.push_back(node);
        }
    }
    return result;
}

bool IPFSManager::addContent(const std::string& path, ContentType type,
                           const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "IPFS Manager not initialized";
        return false;
    }

    if (data.empty()) {
        lastError_ = "Empty content data";
        return false;
    }

    std::string hash = calculateContentHash(data);
    ContentInfo info{
        hash,
        std::filesystem::path(path).filename().string(),
        type,
        data.size(),
        getMimeType(type),
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        {},
        nlohmann::json::object()
    };

    content_[hash] = info;
    contentData_[hash] = data;
    updateIPFSStats();
    notifyContentChange(hash, info);
    return true;
}

bool IPFSManager::addContentFromFile(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "IPFS Manager not initialized";
        return false;
    }

    if (!std::filesystem::exists(filePath)) {
        lastError_ = "File not found";
        return false;
    }

    std::vector<uint8_t> data;
    if (!readFile(filePath, data)) {
        lastError_ = "Failed to read file";
        return false;
    }

    ContentType type = getContentTypeFromFile(filePath);
    return addContent(filePath, type, data);
}

bool IPFSManager::addContentFromDirectory(const std::string& dirPath) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "IPFS Manager not initialized";
        return false;
    }

    if (!std::filesystem::exists(dirPath)) {
        lastError_ = "Directory not found";
        return false;
    }

    bool success = true;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(dirPath)) {
        if (entry.is_regular_file()) {
            if (!addContentFromFile(entry.path().string())) {
                success = false;
            }
        }
    }
    return success;
}

bool IPFSManager::getContent(const std::string& hash, std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "IPFS Manager not initialized";
        return false;
    }

    auto it = contentData_.find(hash);
    if (it == contentData_.end()) {
        lastError_ = "Content not found";
        return false;
    }

    data = it->second;
    return true;
}

bool IPFSManager::getContentToFile(const std::string& hash, const std::string& filePath) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "IPFS Manager not initialized";
        return false;
    }

    std::vector<uint8_t> data;
    if (!getContent(hash, data)) {
        return false;
    }

    return writeFile(filePath, data);
}

bool IPFSManager::pinContent(const std::string& hash) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "IPFS Manager not initialized";
        return false;
    }

    auto it = content_.find(hash);
    if (it == content_.end()) {
        lastError_ = "Content not found";
        return false;
    }

    updateContentState(hash, true);
    return true;
}

bool IPFSManager::unpinContent(const std::string& hash) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "IPFS Manager not initialized";
        return false;
    }

    auto it = content_.find(hash);
    if (it == content_.end()) {
        lastError_ = "Content not found";
        return false;
    }

    updateContentState(hash, false);
    return true;
}

bool IPFSManager::isContentPinned(const std::string& hash) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = content_.find(hash);
    if (it == content_.end()) {
        return false;
    }
    return it->second.metadata.value("pinned", false);
}

ContentInfo IPFSManager::getContentInfo(const std::string& hash) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = content_.find(hash);
    if (it == content_.end()) {
        return ContentInfo{};
    }
    return it->second;
}

std::vector<ContentInfo> IPFSManager::getContentByType(ContentType type) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ContentInfo> result;
    for (const auto& [hash, info] : content_) {
        if (info.type == type) {
            result.push_back(info);
        }
    }
    return result;
}

std::vector<ContentInfo> IPFSManager::getContentByTag(const std::string& tag) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ContentInfo> result;
    auto it = contentTags_.find(tag);
    if (it != contentTags_.end()) {
        for (const auto& hash : it->second) {
            auto contentIt = content_.find(hash);
            if (contentIt != content_.end()) {
                result.push_back(contentIt->second);
            }
        }
    }
    return result;
}

bool IPFSManager::removeContent(const std::string& hash) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "IPFS Manager not initialized";
        return false;
    }

    auto it = content_.find(hash);
    if (it == content_.end()) {
        lastError_ = "Content not found";
        return false;
    }

    content_.erase(it);
    contentData_.erase(hash);
    for (auto& [tag, hashes] : contentTags_) {
        hashes.erase(std::remove(hashes.begin(), hashes.end(), hash), hashes.end());
    }
    updateIPFSStats();
    return true;
}

std::vector<ContentInfo> IPFSManager::searchContent(const std::string& query) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ContentInfo> result;
    for (const auto& [hash, info] : content_) {
        if (info.name.find(query) != std::string::npos ||
            info.mimeType.find(query) != std::string::npos) {
            result.push_back(info);
        }
    }
    return result;
}

std::vector<ContentInfo> IPFSManager::findContentByHash(const std::string& hash) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ContentInfo> result;
    auto it = content_.find(hash);
    if (it != content_.end()) {
        result.push_back(it->second);
    }
    return result;
}

std::vector<ContentInfo> IPFSManager::findContentByName(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ContentInfo> result;
    for (const auto& [hash, info] : content_) {
        if (info.name == name) {
            result.push_back(info);
        }
    }
    return result;
}

std::vector<ContentInfo> IPFSManager::findContentByMimeType(const std::string& mimeType) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ContentInfo> result;
    for (const auto& [hash, info] : content_) {
        if (info.mimeType == mimeType) {
            result.push_back(info);
        }
    }
    return result;
}

bool IPFSManager::updateContentMetadata(const std::string& hash,
                                      const nlohmann::json& metadata) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "IPFS Manager not initialized";
        return false;
    }

    auto it = content_.find(hash);
    if (it == content_.end()) {
        lastError_ = "Content not found";
        return false;
    }

    it->second.metadata = metadata;
    it->second.lastModified = std::chrono::system_clock::now();
    notifyContentChange(hash, it->second);
    return true;
}

bool IPFSManager::addContentTag(const std::string& hash, const std::string& tag) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "IPFS Manager not initialized";
        return false;
    }

    auto it = content_.find(hash);
    if (it == content_.end()) {
        lastError_ = "Content not found";
        return false;
    }

    contentTags_[tag].push_back(hash);
    it->second.tags.push_back(tag);
    it->second.lastModified = std::chrono::system_clock::now();
    notifyContentChange(hash, it->second);
    return true;
}

bool IPFSManager::removeContentTag(const std::string& hash, const std::string& tag) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "IPFS Manager not initialized";
        return false;
    }

    auto it = content_.find(hash);
    if (it == content_.end()) {
        lastError_ = "Content not found";
        return false;
    }

    auto& tags = contentTags_[tag];
    tags.erase(std::remove(tags.begin(), tags.end(), hash), tags.end());
    
    auto& contentTags = it->second.tags;
    contentTags.erase(std::remove(contentTags.begin(), contentTags.end(), tag),
                     contentTags.end());
    
    it->second.lastModified = std::chrono::system_clock::now();
    notifyContentChange(hash, it->second);
    return true;
}

bool IPFSManager::updateContentName(const std::string& hash, const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "IPFS Manager not initialized";
        return false;
    }

    auto it = content_.find(hash);
    if (it == content_.end()) {
        lastError_ = "Content not found";
        return false;
    }

    it->second.name = name;
    it->second.lastModified = std::chrono::system_clock::now();
    notifyContentChange(hash, it->second);
    return true;
}

bool IPFSManager::updateContentMimeType(const std::string& hash,
                                      const std::string& mimeType) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "IPFS Manager not initialized";
        return false;
    }

    auto it = content_.find(hash);
    if (it == content_.end()) {
        lastError_ = "Content not found";
        return false;
    }

    it->second.mimeType = mimeType;
    it->second.lastModified = std::chrono::system_clock::now();
    notifyContentChange(hash, it->second);
    return true;
}

IPFSStats IPFSManager::getStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

void IPFSManager::resetStats() {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_ = IPFSStats{};
}

bool IPFSManager::enableStats(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    statsEnabled_ = enable;
    return true;
}

void IPFSManager::registerNodeCallback(NodeCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    nodeCallbacks_.push_back(callback);
}

void IPFSManager::registerContentCallback(ContentCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    contentCallbacks_.push_back(callback);
}

void IPFSManager::registerErrorCallback(ErrorCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    errorCallbacks_.push_back(callback);
}

void IPFSManager::unregisterNodeCallback() {
    std::lock_guard<std::mutex> lock(mutex_);
    nodeCallbacks_.clear();
}

void IPFSManager::unregisterContentCallback() {
    std::lock_guard<std::mutex> lock(mutex_);
    contentCallbacks_.clear();
}

void IPFSManager::unregisterErrorCallback() {
    std::lock_guard<std::mutex> lock(mutex_);
    errorCallbacks_.clear();
}

std::string IPFSManager::getLastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastError_;
}

void IPFSManager::clearLastError() {
    std::lock_guard<std::mutex> lock(mutex_);
    lastError_.clear();
}

// Private helper methods
bool IPFSManager::validateConfig(const nlohmann::json& config) const {
    return true; // Add validation logic as needed
}

bool IPFSManager::validateNodeAddress(const std::string& address) const {
    return !address.empty() && address.find("/ip4/") != std::string::npos;
}

bool IPFSManager::validateContentHash(const std::string& hash) const {
    return !hash.empty() && hash.length() == 46 && hash.substr(0, 2) == "Qm";
}

bool IPFSManager::checkNodeLimit() const {
    return nodes_.size() < 1000; // Add configurable limit
}

void IPFSManager::updateNodeState(const std::string& address, NodeState state) {
    auto it = nodes_.find(address);
    if (it != nodes_.end()) {
        it->second.state = state;
        it->second.lastSeen = std::chrono::system_clock::now();
    }
}

void IPFSManager::updateContentState(const std::string& hash, bool pinned) {
    auto it = content_.find(hash);
    if (it != content_.end()) {
        it->second.metadata["pinned"] = pinned;
        it->second.lastModified = std::chrono::system_clock::now();
    }
}

void IPFSManager::updateIPFSStats() {
    if (!statsEnabled_) {
        return;
    }

    stats_.totalNodes = nodes_.size();
    stats_.connectedNodes = std::count_if(nodes_.begin(), nodes_.end(),
        [](const auto& pair) { return pair.second.state == IPFSManager::NodeState::CONNECTED; });
    
    stats_.totalContent = content_.size();
    stats_.totalSize = 0;
    stats_.contentTypeDistribution.clear();
    stats_.mimeTypeDistribution.clear();

    for (const auto& [hash, info] : content_) {
        stats_.totalSize += info.size;
        stats_.contentTypeDistribution[info.type]++;
        stats_.mimeTypeDistribution[info.mimeType]++;
    }

    stats_.lastUpdated = std::chrono::system_clock::now();
}

void IPFSManager::notifyNodeChange(const std::string& address, IPFSManager::NodeState state) {
    for (const auto& callback : nodeCallbacks_) {
        try {
            callback(address, state);
        } catch (const std::exception& e) {
            notifyError(address, e.what());
        }
    }
}

void IPFSManager::notifyContentChange(const std::string& hash,
                                    const ContentInfo& info) {
    for (const auto& callback : contentCallbacks_) {
        try {
            callback(hash, info);
        } catch (const std::exception& e) {
            notifyError(hash, e.what());
        }
    }
}

void IPFSManager::notifyError(const std::string& id, const std::string& error) {
    for (const auto& callback : errorCallbacks_) {
        try {
            callback(id, error);
        } catch (...) {
            // Ignore errors in error callbacks
        }
    }
}

// Helper functions for content handling
std::string IPFSManager::generateNodeId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static const char* hex = "0123456789abcdef";

    std::string id;
    id.reserve(46);
    id += "Qm";

    for (int i = 0; i < 44; ++i) {
        id += hex[dis(gen)];
    }

    return id;
}

std::string IPFSManager::calculateContentHash(const std::vector<uint8_t>& data) {
    // Implement proper IPFS CID calculation using SHA-256
    try {
        if (data.empty()) {
            return "";
        }
        
        // Calculate SHA-256 hash
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, data.data(), data.size());
        SHA256_Final(hash, &sha256);
        
        // Convert to base58 (IPFS CID format)
        // For simplicity, we'll use a basic base58 encoding
        // In a real implementation, you'd use a proper base58 library
        std::string base58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
        std::string result = "Qm"; // IPFS CIDv0 prefix
        
        // Convert hash to base58
        std::vector<uint8_t> hashVec(hash, hash + SHA256_DIGEST_LENGTH);
        std::string base58Result = encodeBase58(hashVec);
        
        // Ensure it's 44 characters (IPFS CIDv0 length)
        if (base58Result.length() > 44) {
            base58Result = base58Result.substr(0, 44);
        } else while (base58Result.length() < 44) {
            base58Result = "1" + base58Result;
        }
        
        return result + base58Result;
    } catch (const std::exception& e) {
        return "";
    }
}

// Helper method for base58 encoding
std::string IPFSManager::encodeBase58(const std::vector<uint8_t>& data) {
    const std::string base58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
    std::string result;
    
    // Convert to base58
    std::vector<uint8_t> digits((data.size() * 138 / 100) + 1);
    size_t digitslen = 1;
    
    for (size_t i = 0; i < data.size(); i++) {
        uint32_t carry = data[i];
        for (size_t j = 0; j < digitslen; j++) {
            carry += static_cast<uint32_t>(digits[j]) << 8;
            digits[j] = carry % 58;
            carry /= 58;
        }
        while (carry > 0) {
            digits[digitslen++] = carry % 58;
            carry /= 58;
        }
    }
    
    // Convert to string
    for (size_t i = 0; i < digitslen; i++) {
        result += base58[digits[digitslen - 1 - i]];
    }
    
    // Remove leading zeros
    while (result.length() > 0 && result[0] == '1') {
        result = result.substr(1);
    }
    
    return result;
}

std::string IPFSManager::getMimeType(ContentType type) {
    switch (type) {
        case ContentType::FILE:
            return "application/octet-stream";
        case ContentType::DIRECTORY:
            return "application/x-directory";
        case ContentType::SYMLINK:
            return "application/x-symlink";
        case ContentType::RAW:
            return "application/octet-stream";
        case ContentType::JSON:
            return "application/json";
        case ContentType::TEXT:
            return "text/plain";
        case ContentType::BINARY:
            return "application/octet-stream";
        default:
            return "application/octet-stream";
    }
}

ContentType IPFSManager::getContentTypeFromFile(const std::string& filePath) {
    std::string extension = std::filesystem::path(filePath).extension().string();
    if (extension == ".json") {
        return ContentType::JSON;
    } else if (extension == ".txt") {
        return ContentType::TEXT;
    } else {
        return ContentType::FILE;
    }
}

bool IPFSManager::readFile(const std::string& filePath, std::vector<uint8_t>& data) {
    try {
        std::ifstream file(filePath, std::ios::binary);
        if (!file) {
            return false;
        }
        data = std::vector<uint8_t>(
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
        );
        return true;
    } catch (...) {
        return false;
    }
}

bool IPFSManager::writeFile(const std::string& filePath,
                          const std::vector<uint8_t>& data) {
    try {
        std::ofstream file(filePath, std::ios::binary);
        if (!file) {
            return false;
        }
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace satox::core 