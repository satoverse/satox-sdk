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
#include <functional>
#include <mutex>
#include <unordered_map>
#include <chrono>
#include <optional>

namespace satox {
namespace ipfs {

struct IPFSConfig {
    std::string api_endpoint;
    std::string gateway_endpoint;
    std::string pinning_endpoint;
    std::string api_key;
    std::string secret_key;
    uint32_t timeout_ms;
    uint32_t max_retries;
    bool enable_pinning;
    bool enable_gateway;
};

struct IPFSContent {
    std::string cid;
    std::vector<uint8_t> data;
    std::string content_type;
    std::chrono::system_clock::time_point upload_time;
    std::chrono::system_clock::time_point last_access_time;
    bool is_pinned;
    std::unordered_map<std::string, std::string> metadata;
};

class IPFSManager {
public:
    using ContentCallback = std::function<void(const std::string&, const IPFSContent&)>;
    using ErrorCallback = std::function<void(const std::string&)>;
    using ProgressCallback = std::function<void(const std::string&, uint32_t)>;

    IPFSManager();
    ~IPFSManager();

    bool initialize(const IPFSConfig& config);
    void shutdown();
    std::string getLastError() const;

    // Content operations
    std::string addContent(const std::vector<uint8_t>& data, const std::string& content_type,
                          const std::unordered_map<std::string, std::string>& metadata = {});
    bool removeContent(const std::string& cid);
    std::optional<IPFSContent> getContent(const std::string& cid);
    std::vector<std::string> listContent() const;

    // Pinning operations
    bool pinContent(const std::string& cid);
    bool unpinContent(const std::string& cid);
    bool isContentPinned(const std::string& cid) const;
    std::vector<std::string> listPinnedContent() const;

    // Gateway operations
    std::string getGatewayUrl(const std::string& cid) const;
    bool isGatewayEnabled() const;

    // Batch operations
    bool batchAddContent(const std::vector<std::pair<std::vector<uint8_t>, std::string>>& contents);
    bool batchPinContent(const std::vector<std::string>& cids);
    bool batchUnpinContent(const std::vector<std::string>& cids);

    // Content management
    bool updateMetadata(const std::string& cid, const std::unordered_map<std::string, std::string>& metadata);
    std::optional<std::unordered_map<std::string, std::string>> getMetadata(const std::string& cid) const;
    bool setContentType(const std::string& cid, const std::string& content_type);
    std::optional<std::string> getContentType(const std::string& cid) const;

    // Statistics
    struct IPFSStatistics {
        uint64_t total_content;
        uint64_t total_size;
        uint64_t pinned_content;
        uint64_t pinned_size;
        std::chrono::system_clock::time_point last_sync_time;
    };
    std::optional<IPFSStatistics> getStatistics() const;

    // Cache management
    void clearCache();
    bool isCached(const std::string& cid) const;
    std::optional<IPFSContent> getFromCache(const std::string& cid) const;

    // Callback registration
    void registerContentCallback(ContentCallback callback);
    void registerErrorCallback(ErrorCallback callback);
    void registerProgressCallback(ProgressCallback callback);

private:
    struct CachedContent {
        IPFSContent content;
        std::chrono::system_clock::time_point cache_time;
    };

    bool validateConfig(const IPFSConfig& config) const;
    bool validateContent(const std::vector<uint8_t>& data) const;
    bool validateCID(const std::string& cid) const;
    void updateContentAccessTime(const std::string& cid);
    bool persistContent(const std::string& cid, const IPFSContent& content);
    bool loadContent(const std::string& cid, IPFSContent& content);
    void notifyContentUpdate(const std::string& cid, const IPFSContent& content);
    void notifyError(const std::string& error);
    void notifyProgress(const std::string& cid, uint32_t progress);

    IPFSConfig config_;
    std::unordered_map<std::string, IPFSContent> content_;
    std::unordered_map<std::string, CachedContent> cache_;
    mutable std::mutex mutex_;
    std::string lastError_;

    std::vector<ContentCallback> contentCallbacks_;
    std::vector<ErrorCallback> errorCallbacks_;
    std::vector<ProgressCallback> progressCallbacks_;
};

} // namespace ipfs
} // namespace satox 