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

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <map>
#include <unordered_map>
#include <functional>
#include <future>
#include <nlohmann/json.hpp>

namespace satox {
namespace ipfs {

class ContentStorage {
public:
    static ContentStorage& getInstance();

    // Prevent copying
    ContentStorage(const ContentStorage&) = delete;
    ContentStorage& operator=(const ContentStorage&) = delete;

    // Initialization and cleanup
    bool initialize(const std::string& storagePath);
    void shutdown();

    // Content Operations
    struct ContentInfo {
        std::string hash;
        std::string name;
        size_t size;
        std::string mimeType;
        std::string createdAt;
        std::string updatedAt;
        bool isPinned;
        std::vector<std::string> tags;
        nlohmann::json metadata;
    };

    // Store content
    std::future<ContentInfo> storeContent(const std::string& content, const std::string& name = "");
    std::future<ContentInfo> storeFile(const std::string& filePath);
    std::future<std::vector<ContentInfo>> storeDirectory(const std::string& directoryPath);

    // Retrieve content
    std::future<std::string> getContent(const std::string& hash);
    std::future<bool> getFile(const std::string& hash, const std::string& outputPath);
    std::future<bool> getDirectory(const std::string& hash, const std::string& outputPath);

    // Content management
    bool updateContent(const std::string& hash, const std::string& newContent);
    bool updateMetadata(const std::string& hash, const nlohmann::json& metadata);
    bool deleteContent(const std::string& hash);
    bool pinContent(const std::string& hash);
    bool unpinContent(const std::string& hash);

    // Content queries
    ContentInfo getContentInfo(const std::string& hash) const;
    std::vector<ContentInfo> getContentByTag(const std::string& tag) const;
    std::vector<ContentInfo> getContentByMimeType(const std::string& mimeType) const;
    std::vector<ContentInfo> searchContent(const std::string& query) const;
    std::vector<ContentInfo> getPinnedContent() const;

    // Cache management
    void enableCache(bool enable);
    void clearCache();
    void setCacheSize(size_t maxSize);
    size_t getCacheSize() const;
    size_t getCacheUsage() const;

    // Backup and restore
    bool createBackup(const std::string& backupPath);
    bool restoreFromBackup(const std::string& backupPath);

    // Error handling
    struct Error {
        int code;
        std::string message;
    };
    Error getLastError() const;
    void clearLastError();

private:
    ContentStorage() = default;
    ~ContentStorage() = default;

    // Internal helper methods
    bool validateStoragePath(const std::string& path);
    bool createStorageDirectory();
    bool validateContentHash(const std::string& hash);
    bool updateContentInfo(const std::string& hash, const ContentInfo& info);
    bool removeContentInfo(const std::string& hash);
    void updateCache(const std::string& hash, const std::string& content);
    void removeFromCache(const std::string& hash);
    bool handleError(const std::string& operation, const std::string& error);

    // Member variables
    std::mutex mutex_;
    std::string storagePath_;
    std::unordered_map<std::string, ContentInfo> contentInfo_;
    std::unordered_map<std::string, std::string> contentCache_;
    std::unordered_map<std::string, std::vector<std::string>> tagIndex_;
    std::unordered_map<std::string, std::vector<std::string>> mimeTypeIndex_;
    size_t maxCacheSize_;
    bool cacheEnabled_;
    Error lastError_;
    bool initialized_;
};

} // namespace ipfs
} // namespace satox 