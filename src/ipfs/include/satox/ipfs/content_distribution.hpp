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
#include <unordered_map>
#include <mutex>
#include <future>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>

namespace satox {
namespace ipfs {

class ContentDistribution {
public:
    // Singleton instance
    static ContentDistribution& getInstance();

    // Initialization and cleanup
    bool initialize(const std::string& configPath);
    void shutdown();

    // Distribution operations
    std::future<bool> distributeContent(const std::string& contentHash, const std::vector<std::string>& targetNodes);
    std::future<bool> distributeFile(const std::string& filePath, const std::vector<std::string>& targetNodes);
    std::future<bool> distributeDirectory(const std::string& directoryPath, const std::vector<std::string>& targetNodes);

    // Node management
    bool addNode(const std::string& nodeId, const std::string& address);
    bool removeNode(const std::string& nodeId);
    bool updateNode(const std::string& nodeId, const std::string& newAddress);
    std::vector<std::string> getAvailableNodes() const;
    bool isNodeAvailable(const std::string& nodeId) const;

    // Distribution status
    struct DistributionStatus {
        std::string contentHash;
        std::string status;  // "pending", "in_progress", "completed", "failed"
        size_t progress;     // 0-100 percentage
        std::string error;
        std::vector<std::string> completedNodes;
        std::vector<std::string> failedNodes;
        std::string startTime;
        std::string endTime;
    };

    DistributionStatus getDistributionStatus(const std::string& contentHash) const;
    std::vector<DistributionStatus> getActiveDistributions() const;
    bool cancelDistribution(const std::string& contentHash);

    // Bandwidth management
    void setBandwidthLimit(size_t bytesPerSecond);
    size_t getBandwidthLimit() const;
    size_t getCurrentBandwidthUsage() const;

    // Priority management
    void setDistributionPriority(const std::string& contentHash, int priority);
    int getDistributionPriority(const std::string& contentHash) const;

    // Event callbacks
    using StatusCallback = std::function<void(const DistributionStatus&)>;
    void setStatusCallback(StatusCallback callback);
    void clearStatusCallback();

    // Error handling
    struct Error {
        int code;
        std::string message;
    };
    Error getLastError() const;
    void clearLastError();

private:
    ContentDistribution() = default;
    ~ContentDistribution() = default;
    ContentDistribution(const ContentDistribution&) = delete;
    ContentDistribution& operator=(const ContentDistribution&) = delete;

    // Helper methods
    bool validateNode(const std::string& nodeId, const std::string& address);
    bool updateDistributionStatus(const std::string& contentHash, const DistributionStatus& status);
    void notifyStatusCallback(const DistributionStatus& status);
    bool handleError(const std::string& operation, const std::string& error);

    // Member variables
    mutable std::mutex mutex_;
    bool initialized_ = false;
    std::string configPath_;
    std::unordered_map<std::string, std::string> nodes_;  // nodeId -> address
    std::unordered_map<std::string, DistributionStatus> activeDistributions_;
    std::unordered_map<std::string, int> distributionPriorities_;
    size_t bandwidthLimit_ = 1024 * 1024;  // 1MB/s default
    StatusCallback statusCallback_;
    Error lastError_;
};

} // namespace ipfs
} // namespace satox 