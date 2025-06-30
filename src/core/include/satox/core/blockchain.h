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
#include <atomic>
#include <unordered_map>
#include <functional>
#include <filesystem>
#include "network_interface.h"
#include <deque>

namespace satox {
namespace core {

struct Block {
    std::string hash;
    std::string previousHash;
    uint64_t height;
    uint64_t timestamp;
    std::vector<std::string> transactions;
    std::string merkleRoot;
    uint64_t difficulty;
    std::string nonce;
};

struct ChainState {
    uint64_t currentHeight;
    std::string bestBlockHash;
    uint64_t totalDifficulty;
    bool isSyncing;
};

class BlockchainManager {
public:
    BlockchainManager();
    ~BlockchainManager();

    // Prevent copying
    BlockchainManager(const BlockchainManager&) = delete;
    BlockchainManager& operator=(const BlockchainManager&) = delete;

    // Allow moving
    BlockchainManager(BlockchainManager&&) noexcept = default;
    BlockchainManager& operator=(BlockchainManager&&) noexcept = default;

    // Core functionality
    bool initialize(const std::string& network = "mainnet");
    void cleanup();
    bool isChainValid() const;
    bool validateBlock(const Block& block) const;
    bool validateChain() const;
    void updateChainState(const Block& block);
    std::vector<Block> getBlockRange(uint64_t startHeight, uint64_t endHeight) const;
    ChainState getChainState() const;
    void loadChainState();
    std::string getStateDirectory() const;
    std::string getLatestBlock();
    std::string getBlockByHash(const std::string& hash);
    std::string getTransactionByHash(const std::string& hash);
    std::string getBalance(const std::string& address);
    std::string sendTransaction(const std::string& transaction);

    // Network and Synchronization
    void connectToNetwork(const std::string& nodeAddress, uint16_t port);
    void disconnectFromNetwork();
    bool isConnected() const;
    void startSync();
    void stopSync();
    bool isSyncing() const;
    void setSyncCallback(std::function<void(const Block&)> callback);
    std::vector<std::string> getPeers() const;

    // Block Validation
    bool validateTransaction(const std::string& transaction) const;

private:
    mutable std::mutex stateMutex_;
    mutable std::mutex peerMutex_;
    bool initialized_;
    bool connected_;
    ChainState chainState_;
    std::unordered_map<std::string, Block> blockCache_;
    std::vector<std::string> peers_;
    std::string networkName_;
    std::string nodeAddress_;
    std::atomic<bool> syncing_;
    std::function<void(const Block&)> syncCallback_;
    std::thread syncThread_;
    std::unique_ptr<NetworkInterface> networkInterface_;
    std::deque<std::string> transactionPool_;

    // Network and Synchronization
    void syncLoop();
    void requestSyncFromNetwork();
    void processNetworkMessages();
    void handleNetworkMessage(const NetworkMessage& message);
    void handleBlockMessage(const NetworkMessage& message);
    void handleTransactionMessage(const NetworkMessage& message);
    void handlePeerListMessage(const NetworkMessage& message);
    void handleNetworkError(const std::string& error);
    void requestBlockFromNetwork(const std::string& hash);
    void handleSyncResponse(const NetworkMessage& message);

    // Block and Chain Management
    void persistChainState();
    void updatePeerList(const std::vector<std::string>& newPeers);
    void sendBlockToNetwork(const Block& block);
    void saveBlock(const Block& block);
    void loadBlockCache();
    Block loadBlock(const std::filesystem::path& path);
    bool verifyBlockProof(const Block& block) const;
    bool verifyTransactionProof(const std::string& transaction) const;
    void updateMerkleTree(const Block& block);

    // File System Operations
    std::string getStateFilePath() const;
    std::string getBlocksDirectory() const;
    std::string getBlockFilePath(const std::string& hash) const;

    // Helper Functions
    static Block parseBlockFromMessage(const std::string& message);
    static std::vector<std::string> parsePeerList(const std::string& message);
    static std::string serializeBlock(const Block& block);
};

} // namespace core
} // namespace satox 