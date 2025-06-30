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

#include "satox/ipfs/content_storage.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <openssl/sha.h>

namespace satox {
namespace ipfs {

namespace {
    // Helper function to get current timestamp
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    // Helper function to calculate SHA-256 hash
    std::string calculateHash(const std::string& content) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, content.c_str(), content.length());
        SHA256_Final(hash, &sha256);

        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
        }
        return ss.str();
    }

    // Helper function to detect MIME type
    std::string detectMimeType(const std::string& content) {
        // Basic MIME type detection based on content
        if (content.empty()) return "application/octet-stream";
        
        // Check for text content
        bool isText = true;
        for (char c : content) {
            if (!std::isprint(c) && !std::isspace(c)) {
                isText = false;
                break;
            }
        }
        if (isText) return "text/plain";

        // Check for JSON
        if (content[0] == '{' || content[0] == '[') {
            try {
                nlohmann::json::parse(content);
                return "application/json";
            } catch (...) {}
        }

        // Default to binary
        return "application/octet-stream";
    }
}

ContentStorage& ContentStorage::getInstance() {
    static ContentStorage instance;
    return instance;
}

bool ContentStorage::initialize(const std::string& storagePath) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) {
        return true;
    }

    if (!validateStoragePath(storagePath)) {
        lastError_ = {1, "Invalid storage path"};
        return false;
    }

    storagePath_ = storagePath;
    maxCacheSize_ = 1024 * 1024 * 100; // 100MB default cache size
    cacheEnabled_ = true;
    initialized_ = true;

    return createStorageDirectory();
}

void ContentStorage::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    contentInfo_.clear();
    contentCache_.clear();
    tagIndex_.clear();
    mimeTypeIndex_.clear();
    initialized_ = false;
}

std::future<ContentStorage::ContentInfo> ContentStorage::storeContent(const std::string& content, const std::string& name) {
    return std::async(std::launch::async, [this, content, name]() {
        ContentInfo info;
        if (!initialized_) {
            lastError_ = {2, "Content Storage not initialized"};
            return info;
        }

        std::string hash = calculateHash(content);
        info.hash = hash;
        info.name = name.empty() ? hash : name;
        info.size = content.length();
        info.mimeType = detectMimeType(content);
        info.createdAt = getCurrentTimestamp();
        info.updatedAt = info.createdAt;
        info.isPinned = false;

        std::string filePath = storagePath_ + "/" + hash;
        if (!writeFileContent(filePath, content)) {
            lastError_ = {3, "Failed to write content to file"};
            return info;
        }

        if (cacheEnabled_) {
            updateCache(hash, content);
        }

        contentInfo_[hash] = info;
        return info;
    });
}

std::future<ContentStorage::ContentInfo> ContentStorage::storeFile(const std::string& filePath) {
    return std::async(std::launch::async, [this, filePath]() {
        ContentInfo info;
        if (!initialized_) {
            lastError_ = {2, "Content Storage not initialized"};
            return info;
        }

        std::string content = readFileContent(filePath);
        if (content.empty()) {
            lastError_ = {4, "Failed to read file content"};
            return info;
        }

        return storeContent(content, std::filesystem::path(filePath).filename().string()).get();
    });
}

std::future<std::vector<ContentStorage::ContentInfo>> ContentStorage::storeDirectory(const std::string& directoryPath) {
    return std::async(std::launch::async, [this, directoryPath]() {
        std::vector<ContentInfo> results;
        if (!initialized_) {
            lastError_ = {2, "Content Storage not initialized"};
            return results;
        }

        for (const auto& entry : std::filesystem::recursive_directory_iterator(directoryPath)) {
            if (entry.is_regular_file()) {
                auto info = storeFile(entry.path().string()).get();
                results.push_back(info);
            }
        }

        return results;
    });
}

std::future<std::string> ContentStorage::getContent(const std::string& hash) {
    return std::async(std::launch::async, [this, hash]() {
        std::string result;
        if (!initialized_) {
            lastError_ = {2, "Content Storage not initialized"};
            return result;
        }

        // Check cache first
        if (cacheEnabled_) {
            auto it = contentCache_.find(hash);
            if (it != contentCache_.end()) {
                return it->second;
            }
        }

        // Read from file
        std::string filePath = storagePath_ + "/" + hash;
        result = readFileContent(filePath);
        if (result.empty()) {
            lastError_ = {5, "Failed to read content from file"};
            return result;
        }

        // Update cache
        if (cacheEnabled_) {
            updateCache(hash, result);
        }

        return result;
    });
}

std::future<bool> ContentStorage::getFile(const std::string& hash, const std::string& outputPath) {
    return std::async(std::launch::async, [this, hash, outputPath]() {
        if (!initialized_) {
            lastError_ = {2, "Content Storage not initialized"};
            return false;
        }

        std::string content = getContent(hash).get();
        if (content.empty()) {
            return false;
        }

        return writeFileContent(outputPath, content);
    });
}

std::future<bool> ContentStorage::getDirectory(const std::string& hash, const std::string& outputPath) {
    return std::async(std::launch::async, [this, hash, outputPath]() {
        if (!initialized_) {
            lastError_ = {2, "Content Storage not initialized"};
            return false;
        }

        std::string content = getContent(hash).get();
        if (content.empty()) {
            return false;
        }

        try {
            auto json = nlohmann::json::parse(content);
            for (const auto& [path, fileHash] : json.items()) {
                std::string fullPath = outputPath + "/" + path;
                std::filesystem::create_directories(std::filesystem::path(fullPath).parent_path());
                if (!getFile(fileHash, fullPath).get()) {
                    return false;
                }
            }
            return true;
        } catch (const std::exception& e) {
            lastError_ = {6, std::string("Failed to parse directory content: ") + e.what()};
            return false;
        }
    });
}

bool ContentStorage::updateContent(const std::string& hash, const std::string& newContent) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = {2, "Content Storage not initialized"};
        return false;
    }

    auto it = contentInfo_.find(hash);
    if (it == contentInfo_.end()) {
        lastError_ = {7, "Content not found"};
        return false;
    }

    std::string newHash = calculateHash(newContent);
    if (newHash != hash) {
        lastError_ = {8, "Content hash mismatch"};
        return false;
    }

    std::string filePath = storagePath_ + "/" + hash;
    if (!writeFileContent(filePath, newContent)) {
        lastError_ = {9, "Failed to update content file"};
        return false;
    }

    it->second.updatedAt = getCurrentTimestamp();
    it->second.size = newContent.length();
    it->second.mimeType = detectMimeType(newContent);

    if (cacheEnabled_) {
        updateCache(hash, newContent);
    }

    return true;
}

bool ContentStorage::updateMetadata(const std::string& hash, const nlohmann::json& metadata) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = {2, "Content Storage not initialized"};
        return false;
    }

    auto it = contentInfo_.find(hash);
    if (it == contentInfo_.end()) {
        lastError_ = {7, "Content not found"};
        return false;
    }

    it->second.metadata = metadata;
    it->second.updatedAt = getCurrentTimestamp();
    return true;
}

bool ContentStorage::deleteContent(const std::string& hash) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = {2, "Content Storage not initialized"};
        return false;
    }

    auto it = contentInfo_.find(hash);
    if (it == contentInfo_.end()) {
        lastError_ = {7, "Content not found"};
        return false;
    }

    if (it->second.isPinned) {
        lastError_ = {10, "Cannot delete pinned content"};
        return false;
    }

    std::string filePath = storagePath_ + "/" + hash;
    if (!std::filesystem::remove(filePath)) {
        lastError_ = {11, "Failed to delete content file"};
        return false;
    }

    removeFromCache(hash);
    removeContentInfo(hash);
    return true;
}

bool ContentStorage::pinContent(const std::string& hash) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = {2, "Content Storage not initialized"};
        return false;
    }

    auto it = contentInfo_.find(hash);
    if (it == contentInfo_.end()) {
        lastError_ = {7, "Content not found"};
        return false;
    }

    it->second.isPinned = true;
    it->second.updatedAt = getCurrentTimestamp();
    return true;
}

bool ContentStorage::unpinContent(const std::string& hash) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = {2, "Content Storage not initialized"};
        return false;
    }

    auto it = contentInfo_.find(hash);
    if (it == contentInfo_.end()) {
        lastError_ = {7, "Content not found"};
        return false;
    }

    it->second.isPinned = false;
    it->second.updatedAt = getCurrentTimestamp();
    return true;
}

ContentStorage::ContentInfo ContentStorage::getContentInfo(const std::string& hash) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = contentInfo_.find(hash);
    if (it == contentInfo_.end()) {
        return ContentInfo{};
    }
    return it->second;
}

std::vector<ContentStorage::ContentInfo> ContentStorage::getContentByTag(const std::string& tag) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ContentInfo> results;
    auto it = tagIndex_.find(tag);
    if (it != tagIndex_.end()) {
        for (const auto& hash : it->second) {
            auto infoIt = contentInfo_.find(hash);
            if (infoIt != contentInfo_.end()) {
                results.push_back(infoIt->second);
            }
        }
    }
    return results;
}

std::vector<ContentStorage::ContentInfo> ContentStorage::getContentByMimeType(const std::string& mimeType) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ContentInfo> results;
    auto it = mimeTypeIndex_.find(mimeType);
    if (it != mimeTypeIndex_.end()) {
        for (const auto& hash : it->second) {
            auto infoIt = contentInfo_.find(hash);
            if (infoIt != contentInfo_.end()) {
                results.push_back(infoIt->second);
            }
        }
    }
    return results;
}

std::vector<ContentStorage::ContentInfo> ContentStorage::searchContent(const std::string& query) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ContentInfo> results;
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

    for (const auto& [hash, info] : contentInfo_) {
        std::string lowerName = info.name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        
        if (lowerName.find(lowerQuery) != std::string::npos) {
            results.push_back(info);
            continue;
        }

        for (const auto& tag : info.tags) {
            std::string lowerTag = tag;
            std::transform(lowerTag.begin(), lowerTag.end(), lowerTag.begin(), ::tolower);
            if (lowerTag.find(lowerQuery) != std::string::npos) {
                results.push_back(info);
                break;
            }
        }
    }

    return results;
}

std::vector<ContentStorage::ContentInfo> ContentStorage::getPinnedContent() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ContentInfo> results;
    for (const auto& [hash, info] : contentInfo_) {
        if (info.isPinned) {
            results.push_back(info);
        }
    }
    return results;
}

void ContentStorage::enableCache(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    cacheEnabled_ = enable;
    if (!enable) {
        clearCache();
    }
}

void ContentStorage::clearCache() {
    std::lock_guard<std::mutex> lock(mutex_);
    contentCache_.clear();
}

void ContentStorage::setCacheSize(size_t maxSize) {
    std::lock_guard<std::mutex> lock(mutex_);
    maxCacheSize_ = maxSize;
    if (getCacheUsage() > maxSize) {
        clearCache();
    }
}

size_t ContentStorage::getCacheSize() const {
    return maxCacheSize_;
}

size_t ContentStorage::getCacheUsage() const {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t usage = 0;
    for (const auto& [hash, content] : contentCache_) {
        usage += content.length();
    }
    return usage;
}

bool ContentStorage::createBackup(const std::string& backupPath) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = {2, "Content Storage not initialized"};
        return false;
    }

    try {
        std::filesystem::create_directories(backupPath);
        
        // Backup content files
        for (const auto& [hash, info] : contentInfo_) {
            std::string sourcePath = storagePath_ + "/" + hash;
            std::string destPath = backupPath + "/" + hash;
            std::filesystem::copy_file(sourcePath, destPath, std::filesystem::copy_options::overwrite_existing);
        }

        // Backup metadata
        nlohmann::json backupData = {
            {"contentInfo", contentInfo_},
            {"tagIndex", tagIndex_},
            {"mimeTypeIndex", mimeTypeIndex_}
        };
        std::string metadataPath = backupPath + "/metadata.json";
        writeFileContent(metadataPath, backupData.dump());

        return true;
    } catch (const std::exception& e) {
        lastError_ = {12, std::string("Backup failed: ") + e.what()};
        return false;
    }
}

bool ContentStorage::restoreFromBackup(const std::string& backupPath) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = {2, "Content Storage not initialized"};
        return false;
    }

    try {
        // Read metadata
        std::string metadataPath = backupPath + "/metadata.json";
        std::string metadataContent = readFileContent(metadataPath);
        if (metadataContent.empty()) {
            lastError_ = {13, "Failed to read backup metadata"};
            return false;
        }

        auto backupData = nlohmann::json::parse(metadataContent);
        
        // Restore content files
        for (const auto& [hash, info] : backupData["contentInfo"].items()) {
            std::string sourcePath = backupPath + "/" + hash;
            std::string destPath = storagePath_ + "/" + hash;
            std::filesystem::copy_file(sourcePath, destPath, std::filesystem::copy_options::overwrite_existing);
        }

        // Restore indices
        contentInfo_ = backupData["contentInfo"].get<std::unordered_map<std::string, ContentInfo>>();
        tagIndex_ = backupData["tagIndex"].get<std::unordered_map<std::string, std::vector<std::string>>>();
        mimeTypeIndex_ = backupData["mimeTypeIndex"].get<std::unordered_map<std::string, std::vector<std::string>>>();

        return true;
    } catch (const std::exception& e) {
        lastError_ = {14, std::string("Restore failed: ") + e.what()};
        return false;
    }
}

ContentStorage::Error ContentStorage::getLastError() const {
    return lastError_;
}

void ContentStorage::clearLastError() {
    lastError_ = {0, ""};
}

// Private helper methods
bool ContentStorage::validateStoragePath(const std::string& path) {
    return !path.empty() && std::filesystem::path(path).is_absolute();
}

bool ContentStorage::createStorageDirectory() {
    try {
        std::filesystem::create_directories(storagePath_);
        return true;
    } catch (const std::exception& e) {
        lastError_ = {15, std::string("Failed to create storage directory: ") + e.what()};
        return false;
    }
}

bool ContentStorage::validateContentHash(const std::string& hash) {
    return !hash.empty() && hash.length() == 64; // SHA-256 hash length
}

bool ContentStorage::updateContentInfo(const std::string& hash, const ContentInfo& info) {
    contentInfo_[hash] = info;
    return true;
}

bool ContentStorage::removeContentInfo(const std::string& hash) {
    contentInfo_.erase(hash);
    return true;
}

void ContentStorage::updateCache(const std::string& hash, const std::string& content) {
    size_t contentSize = content.length();
    if (contentSize > maxCacheSize_) {
        return; // Content too large to cache
    }

    // Remove old content if cache would exceed max size
    while (getCacheUsage() + contentSize > maxCacheSize_ && !contentCache_.empty()) {
        contentCache_.erase(contentCache_.begin());
    }

    contentCache_[hash] = content;
}

void ContentStorage::removeFromCache(const std::string& hash) {
    contentCache_.erase(hash);
}

bool ContentStorage::handleError(const std::string& operation, const std::string& error) {
    lastError_ = {16, operation + ": " + error};
    return false;
}

} // namespace ipfs
} // namespace satox
 