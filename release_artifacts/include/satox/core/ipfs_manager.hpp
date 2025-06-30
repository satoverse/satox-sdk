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
#include <map>
#include <memory>
#include <mutex>
#include <functional>
#include <nlohmann/json.hpp>
#include <chrono>

namespace satox::core {

class IPFSManager {
public:
    // IPFS node states
    enum class NodeState {
        UNINITIALIZED,
        INITIALIZING,
        CONNECTING,
        CONNECTED,
        DISCONNECTED,
        ERROR
    };

    // IPFS content types
    enum class ContentType {
        FILE,
        DIRECTORY,
        SYMLINK,
        RAW,
        JSON,
        TEXT,
        BINARY
    };

    // IPFS content information structure
    struct ContentInfo {
        std::string hash;
        std::string name;
        ContentType type;
        size_t size;
        std::string mimeType;
        std::chrono::system_clock::time_point createdAt;
        std::chrono::system_clock::time_point lastModified;
        std::vector<std::string> tags;
        nlohmann::json metadata;
    };

    // IPFS node information structure
    struct NodeInfo {
        std::string id;
        std::string address;
        std::vector<std::string> addresses;
        std::vector<std::string> protocols;
        std::string version;
        std::string agent;
        NodeState state;
        std::chrono::system_clock::time_point lastSeen;
        nlohmann::json metadata;
    };

    // IPFS statistics structure
    struct IPFSStats {
        size_t totalNodes;
        size_t connectedNodes;
        size_t totalContent;
        size_t totalSize;
        std::map<ContentType, size_t> contentTypeDistribution;
        std::map<std::string, size_t> mimeTypeDistribution;
        std::chrono::system_clock::time_point lastUpdated;
    };

    // Callback types
    using NodeCallback = std::function<void(const std::string&, NodeState)>;
    using ContentCallback = std::function<void(const std::string&, const ContentInfo&)>;
    using ErrorCallback = std::function<void(const std::string&, const std::string&)>;

    // Singleton instance
    static IPFSManager& getInstance();

    // Initialization and shutdown
    bool initialize(const nlohmann::json& config);
    void shutdown();

    // Node management
    bool connectNode(const std::string& address);
    bool disconnectNode(const std::string& address);
    bool isNodeConnected(const std::string& address) const;
    NodeInfo getNodeInfo(const std::string& address) const;
    std::vector<NodeInfo> getAllNodes() const;
    std::vector<NodeInfo> getNodesByState(NodeState state) const;

    // Content management
    bool addContent(const std::string& path, ContentType type,
                   const std::vector<uint8_t>& data);
    bool addContentFromFile(const std::string& filePath);
    bool addContentFromDirectory(const std::string& dirPath);
    bool getContent(const std::string& hash, std::vector<uint8_t>& data);
    bool getContentToFile(const std::string& hash, const std::string& filePath);
    bool pinContent(const std::string& hash);
    bool unpinContent(const std::string& hash);
    bool isContentPinned(const std::string& hash) const;
    ContentInfo getContentInfo(const std::string& hash) const;
    std::vector<ContentInfo> getContentByType(ContentType type) const;
    std::vector<ContentInfo> getContentByTag(const std::string& tag) const;
    bool removeContent(const std::string& hash);

    // Content search and discovery
    std::vector<ContentInfo> searchContent(const std::string& query) const;
    std::vector<ContentInfo> findContentByHash(const std::string& hash) const;
    std::vector<ContentInfo> findContentByName(const std::string& name) const;
    std::vector<ContentInfo> findContentByMimeType(const std::string& mimeType) const;

    // Content metadata management
    bool updateContentMetadata(const std::string& hash, const nlohmann::json& metadata);
    bool addContentTag(const std::string& hash, const std::string& tag);
    bool removeContentTag(const std::string& hash, const std::string& tag);
    bool updateContentName(const std::string& hash, const std::string& name);
    bool updateContentMimeType(const std::string& hash, const std::string& mimeType);

    // Statistics
    IPFSStats getStats() const;
    void resetStats();
    bool enableStats(bool enable);

    // Callback registration
    void registerNodeCallback(NodeCallback callback);
    void registerContentCallback(ContentCallback callback);
    void registerErrorCallback(ErrorCallback callback);
    void unregisterNodeCallback();
    void unregisterContentCallback();
    void unregisterErrorCallback();

    // Error handling
    std::string getLastError() const;
    void clearLastError();

private:
    IPFSManager() = default;
    ~IPFSManager() = default;
    IPFSManager(const IPFSManager&) = delete;
    IPFSManager& operator=(const IPFSManager&) = delete;

    // Helper methods
    bool validateConfig(const nlohmann::json& config) const;
    bool validateNodeAddress(const std::string& address) const;
    bool validateContentHash(const std::string& hash) const;
    bool checkNodeLimit() const;
    void updateNodeState(const std::string& address, NodeState state);
    void updateContentState(const std::string& hash, bool pinned);
    void updateIPFSStats();
    void notifyNodeChange(const std::string& address, NodeState state);
    void notifyContentChange(const std::string& hash, const ContentInfo& info);
    void notifyError(const std::string& id, const std::string& error);

    // Content handling helper methods
    std::string generateNodeId();
    std::string calculateContentHash(const std::vector<uint8_t>& data);
    std::string encodeBase58(const std::vector<uint8_t>& data);
    std::string getMimeType(ContentType type);
    ContentType getContentTypeFromFile(const std::string& filePath);
    bool readFile(const std::string& filePath, std::vector<uint8_t>& data);
    bool writeFile(const std::string& filePath, const std::vector<uint8_t>& data);

    // Member variables
    mutable std::mutex mutex_;
    bool initialized_ = false;
    bool statsEnabled_ = false;
    std::string lastError_;
    std::map<std::string, NodeInfo> nodes_;
    std::map<std::string, ContentInfo> content_;
    std::map<std::string, std::vector<uint8_t>> contentData_;
    std::map<std::string, std::vector<std::string>> contentTags_;
    IPFSStats stats_;
    std::vector<NodeCallback> nodeCallbacks_;
    std::vector<ContentCallback> contentCallbacks_;
    std::vector<ErrorCallback> errorCallbacks_;
};

} // namespace satox::core 