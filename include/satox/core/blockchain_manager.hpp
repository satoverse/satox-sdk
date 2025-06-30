/*
 * MIT License
 * Copyright(c) 2025 Satoxcoin Core Developer
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <functional>
#include <chrono>
#include <optional>
#include <nlohmann/json.hpp>
#include "types.hpp"
#include "logging_manager.hpp"

namespace satox::core {

class BlockchainManager {
public:
    // Blockchain states
    enum class BlockchainState {
        UNINITIALIZED,
        INITIALIZING,
        INITIALIZED,
        CONNECTING,
        CONNECTED,
        DISCONNECTED,
        SYNCING,
        SYNCED,
        ERROR,
        SHUTDOWN
    };

    // Block structure
    struct Block {
        std::string hash;
        std::string previousHash;
        std::string merkleRoot;
        uint64_t height;
        uint64_t timestamp;
        uint32_t bits;
        uint32_t nonce;
        std::vector<std::string> transactions;
        nlohmann::json metadata;
    };

    // Transaction structure
    struct Transaction {
        std::string id;
        std::string hash;
        uint64_t timestamp;
        std::vector<Input> inputs;
        std::vector<Output> outputs;
        uint64_t fee;
        uint32_t confirmations;
        std::string blockHash;
        uint64_t blockHeight;
        nlohmann::json metadata;
    };

    // Callback types
    using TransactionCallback = std::function<void(const Transaction&)>;
    using BlockCallback = std::function<void(const Block&)>;
    using StateCallback = std::function<void(BlockchainState)>;
    using ErrorCallback = std::function<void(const std::string&)>;

    BlockchainManager();
    ~BlockchainManager();

    // Singleton pattern
    static BlockchainManager& getInstance();

    // Initialization and shutdown
    bool initialize(const nlohmann::json& config = nlohmann::json::object());
    bool shutdown();
    std::string getLastError() const;
    void clearLastError();

    // Configuration
    struct Config {
        std::string network;  // "mainnet" or "testnet"
        std::string data_dir;
        uint16_t p2p_port = 60777;  // Default Satoxcoin P2P port
        uint16_t rpc_port = 7777;   // Default Satoxcoin RPC port
        bool enable_mining = false;
        size_t max_connections = 125;
        size_t max_outbound_connections = 8;
        size_t max_inbound_connections = 117;
        std::chrono::seconds connection_timeout{30};
        std::chrono::seconds handshake_timeout{10};
        std::chrono::seconds ping_timeout{5};
        size_t max_block_size = 2 * 1024 * 1024;  // 2MB
        size_t max_tx_size = 1 * 1024 * 1024;     // 1MB
        size_t max_script_size = 10 * 1024;       // 10KB
        size_t max_script_num_size = 4;           // 4 bytes
        size_t max_stack_size = 1000;             // 1000 items
        size_t max_sig_ops = 20000;               // 20000 sigops
        size_t max_orphan_txs = 100;              // 100 orphan transactions
        size_t max_mempool_size = 300 * 1024 * 1024; // 300MB
        std::chrono::seconds mempool_expiry{336 * 3600}; // 336 hours
        bool enable_rpc = true;
        bool enable_rest = false;
        std::string rpc_user;
        std::string rpc_password;
        std::vector<std::string> rpc_allowed_ips;
        bool enable_wallet = false;
        std::string wallet_dir;
        bool enable_assets = true;
        bool enable_ipfs = true;
        bool enable_nft = true;
    };

    bool updateConfig(const Config& config);

    // Blockchain operations
    bool connect();
    bool disconnect();
    bool isConnected() const;

    // Block operations
    std::optional<std::string> getBlockHash(uint64_t height);
    std::optional<uint64_t> getBlockHeight(const std::string& hash);
    std::optional<nlohmann::json> getBlock(const std::string& hash);
    std::optional<nlohmann::json> getBlockByHeight(uint64_t height);
    uint64_t getBestBlockHeight() const;
    std::string getBestBlockHash() const;
    bool getBlockByHeight(uint64_t height, Block& block) const;
    std::vector<Block> getBlocks(uint64_t startHeight, uint64_t endHeight) const;

    // Transaction operations
    std::optional<nlohmann::json> getTransaction(const std::string& txid);
    std::optional<std::string> sendTransaction(const std::string& hex);
    std::optional<std::string> createTransaction(const std::vector<Input>& inputs, 
                                                const std::vector<Output>& outputs);
    bool validateTransaction(const std::string& hex);
    bool getTransaction(const std::string& txid, Transaction& transaction) const;
    std::vector<Transaction> getTransactions(uint64_t startHeight, uint64_t endHeight) const;

    // Network operations
    size_t getConnectionCount() const;
    std::vector<nlohmann::json> getPeerInfo() const;
    bool addNode(const std::string& address, bool add_to_banlist = false);
    bool removeNode(const std::string& address);

    // Mining operations
    bool startMining();
    bool stopMining();
    bool isMining() const;
    std::optional<std::string> getMiningInfo();

    // Callbacks
    void registerTransactionCallback(TransactionCallback callback);
    void registerBlockCallback(BlockCallback callback);
    void registerStateCallback(StateCallback callback);
    void registerErrorCallback(ErrorCallback callback);
    void unregisterBlockCallback();
    void unregisterTransactionCallback();
    void unregisterStateCallback();
    void unregisterErrorCallback();

    // Statistics and monitoring
    struct Stats {
        uint64_t totalBlocks;
        uint64_t totalTransactions;
        size_t currentConnections;
        size_t totalConnections;
        std::chrono::system_clock::time_point lastBlock;
        std::chrono::system_clock::time_point lastActivity;
        double difficulty;
        uint64_t networkHashrate;
        size_t mempoolSize;
        size_t mempoolTransactions;
    };
    Stats getStats() const;
    bool enableStats(bool enable);

    // Health check
    bool isHealthy() const;

    BlockchainState getState() const;

    // Blockchain info structure
    struct BlockchainInfo {
        std::string network;
        uint64_t height;
        std::string bestBlockHash;
        double difficulty;
        uint64_t networkHashrate;
        size_t mempoolSize;
        size_t mempoolTransactions;
        std::chrono::system_clock::time_point lastBlockTime;
        nlohmann::json additionalInfo;
    };

    // Add missing method declarations
    BlockchainInfo getInfo() const;
    void resetStats();
    uint64_t getCurrentHeight() const;
    uint64_t getCurrentHeightInternal() const;
    uint64_t getDifficulty() const;
    uint64_t getNetworkHashRate() const;

private:
    BlockchainManager(const BlockchainManager&) = delete;
    BlockchainManager& operator=(const BlockchainManager&) = delete;

    mutable std::mutex mutex_;
    std::string lastError_;
    Config config_;
    bool initialized_;
    bool connected_;
    bool mining_;
    bool statsEnabled_;
    BlockchainState state_;
    Stats stats_;
    std::map<std::string, std::vector<BlockCallback>> blockCallbacks_;
    std::map<std::string, std::vector<TransactionCallback>> transactionCallbacks_;
    std::vector<StateCallback> stateCallbacks_;
    std::vector<ErrorCallback> errorCallbacks_;
    std::map<std::string, Block> blocks_;
    std::map<std::string, Transaction> transactions_;
    std::unique_ptr<LoggingManager> logger_;

    void setLastError(const std::string& error);
    void notifyTransactionCallback(const std::string& txid, const std::string& status);
    void notifyBlockCallback(const std::string& hash, uint64_t height);
    void notifyErrorCallback(const std::string& error);
    void updateStats();
    
    // Private helper methods
    bool validateConfig(const nlohmann::json& config) const;
    bool validateBlock(const Block& block) const;
    void updateState(BlockchainState state);
    void updateStats(const Block& block);
    void updateStats(const Transaction& transaction);
    void notifyBlockChange(const Block& block);
    void notifyTransactionChange(const Transaction& transaction);
    void notifyStateChange(BlockchainState state);
    void notifyError(const std::string& error);
    uint64_t getDifficultyInternal() const;
    uint64_t getNetworkHashRateInternal() const;
};

} // namespace satox::core 