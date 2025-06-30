/*
 * MIT License
 * Copyright(c) 2025 Satoxcoin Core Developer
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <mutex>
#include <memory>
#include <chrono>
#include <variant>
#include <optional>
#include <shared_mutex>
#include <nlohmann/json.hpp>
#include "satox/blockchain/types.hpp"

// Forward declarations
namespace satox::blockchain {
    class Block;
    class Transaction;
    class BlockchainManager;
}

namespace satox::blockchain {

// Enums
enum class BlockchainState {
    UNINITIALIZED,
    INITIALIZING,
    INITIALIZED,
    CONNECTING,
    CONNECTED,
    ERROR,
    SHUTDOWN
};

enum class BlockchainType {
    MAINNET,
    TESTNET,
    REGTEST
};

// Callback types
using BlockchainCallback = std::function<void(const std::string&, bool)>;
using BlockCallback = std::function<void(const std::shared_ptr<Block>&)>;
using TransactionCallback = std::function<void(const std::shared_ptr<Transaction>&)>;
using ErrorCallback = std::function<void(const std::string&, const std::string&)>;

// Main manager class
class BlockchainManager {
public:
    // Singleton pattern
    static BlockchainManager& getInstance();

    // Constructor/Destructor
    BlockchainManager();
    ~BlockchainManager();

    // Initialization and shutdown
    bool initialize(const BlockchainConfig& config);
    void shutdown();

    // Core operations
    bool connect();
    bool disconnect();
    bool isConnected() const;
    bool isInitialized() const;

    // Block operations
    std::shared_ptr<Block> getLatestBlock();
    std::shared_ptr<Block> getBlockByHash(const std::string& hash);
    std::shared_ptr<Block> getBlockByHeight(uint64_t height);
    bool validateBlock(const std::shared_ptr<Block>& block);
    uint64_t getBlockCount() const;

    // Transaction operations
    std::shared_ptr<Transaction> createTransaction(
        const std::string& from,
        const std::string& to,
        double value,
        const std::vector<uint8_t>& data = {});
    bool broadcastTransaction(const std::shared_ptr<Transaction>& tx);
    std::shared_ptr<Transaction> getTransaction(const std::string& tx_hash);
    std::string getTransactionStatus(const std::string& tx_hash);
    bool validateTransaction(const std::shared_ptr<Transaction>& tx);

    // Chain state operations
    uint64_t getChainHeight() const;
    double getBalance(const std::string& address);
    uint64_t getNonce(const std::string& address);
    std::string getChainState() const;
    bool isChainSynced() const;

    // Network operations
    std::vector<std::string> getConnectedPeers() const;
    bool addPeer(const std::string& peer_address);
    bool removePeer(const std::string& peer_address);

    // Configuration management
    BlockchainConfig getConfig() const;
    bool updateConfig(const BlockchainConfig& config);
    bool validateConfig(const BlockchainConfig& config);
    NetworkConfig getNetworkConfig() const;
    bool updateNetworkConfig(const NetworkConfig& config);

    // Statistics and monitoring
    BlockchainStats getStats() const;
    void resetStats();
    bool enableStats(bool enable);

    // Callback registration
    void registerBlockCallback(BlockCallback callback);
    void registerTransactionCallback(TransactionCallback callback);
    void registerErrorCallback(ErrorCallback callback);
    void unregisterBlockCallback();
    void unregisterTransactionCallback();
    void unregisterErrorCallback();

    // Error handling
    std::string getLastError() const;
    void clearLastError();

    // Health check
    bool healthCheck();
    nlohmann::json getHealthStatus();

    // State management
    BlockchainState getState() const;
    bool isHealthy() const;

private:
    // Disable copy and assignment
    BlockchainManager(const BlockchainManager&) = delete;
    BlockchainManager& operator=(const BlockchainManager&) = delete;

    // Private helper methods
    void setLastError(const std::string& error);
    void notifyBlockEvent(const std::shared_ptr<Block>& block);
    void notifyTransactionEvent(const std::shared_ptr<Transaction>& tx);
    void notifyErrorEvent(const std::string& operation, const std::string& error);
    void updateStats(bool success, double operationTime);
    bool validateState();
    void logOperation(const std::string& operation, bool success, double duration);

    // Member variables
    mutable std::shared_mutex mutex_;
    bool initialized_ = false;
    bool connected_ = false;
    BlockchainState state_ = BlockchainState::UNINITIALIZED;
    BlockchainConfig config_;
    BlockchainStats stats_;
    mutable std::string lastError_;
    bool statsEnabled_ = false;

    // Callbacks
    BlockCallback block_callback_;
    TransactionCallback transaction_callback_;
    ErrorCallback error_callback_;

    // Internal state
    std::chrono::system_clock::time_point lastHealthCheck_;
    int consecutiveFailures_ = 0;
    nlohmann::json internalState_;
};

} // namespace satox::blockchain 