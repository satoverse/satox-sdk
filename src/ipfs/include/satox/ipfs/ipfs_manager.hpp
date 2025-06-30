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

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace satox {
namespace ipfs {

struct Config {
    std::string api_endpoint;
    std::string gateway_url;
    int timeout_seconds;
    bool enable_pinning;
    std::string pinning_service;
};

class IPFSManager {
public:
    static IPFSManager& getInstance();

    // Prevent copying
    IPFSManager(const IPFSManager&) = delete;
    IPFSManager& operator=(const IPFSManager&) = delete;

    // Allow moving
    IPFSManager(IPFSManager&&) = default;
    IPFSManager& operator=(IPFSManager&&) = default;

    // Core functionality
    bool initialize(const Config& config);
    void shutdown();
    bool isRunning() const;
    bool isInitialized() const;
    bool isHealthy() const;

    // IPFS operations
    bool addFile(const std::string& filePath, std::string& hash);
    bool addFileData(const std::string& data, std::string& hash);
    bool getFile(const std::string& hash, const std::string& outputPath);
    bool getFile(const std::string& hash, std::string& data);
    bool pinFile(const std::string& hash);
    bool unpinFile(const std::string& hash);
    bool getPinnedFiles(std::vector<std::string>& pinnedFiles);
    std::string getLastError() const;

    // Additional operations (to be implemented)
    std::string addData(const std::vector<uint8_t>& data);
    std::vector<uint8_t> getData(const std::string& ipfs_hash);
    bool pinHash(const std::string& ipfs_hash);
    bool unpinHash(const std::string& ipfs_hash);
    std::vector<std::string> listPinnedHashes();
    nlohmann::json getIPFSInfo(const std::string& ipfs_hash);
    std::vector<nlohmann::json> getIPFSHistory(const std::string& ipfs_hash);
    std::vector<nlohmann::json> getIPFSBalances(const std::string& ipfs_hash);
    std::vector<nlohmann::json> getIPFSTransactions(const std::string& ipfs_hash);
    std::vector<std::string> getIPFSAddresses(const std::string& ipfs_hash);
    std::vector<nlohmann::json> getIPFSAddressBalances(const std::string& ipfs_hash);
    std::vector<nlohmann::json> getIPFSAddressTransactions(const std::string& ipfs_hash);
    std::vector<nlohmann::json> getIPFSAddressHistory(const std::string& ipfs_hash);

private:
    IPFSManager();
    ~IPFSManager();

    class Impl {
    public:
        Impl();
        ~Impl();
        
        bool initialize(const Config& config);
        bool addFile(const std::string& filePath, std::string& hash);
        bool getFile(const std::string& hash, const std::string& outputPath);
        bool pinFile(const std::string& hash);
        bool unpinFile(const std::string& hash);
        std::string getLastError() const;
        bool isInitialized() const;
        std::string getApiEndpoint() const;
        bool getPinnedFiles(std::vector<std::string>& pinnedFiles);

    private:
        static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
        static size_t writeFileCallback(void* contents, size_t size, size_t nmemb, std::ofstream* file);

        Config config_;
        bool initialized_;
        std::string lastError_;
    };

    std::unique_ptr<Impl> pimpl_;
};

} // namespace ipfs
} // namespace satox 