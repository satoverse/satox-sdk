/*
 * MIT License
 * Copyright(c) 2025 Satoxcoin Core Developer
 */

#include "satox/blockchain/blockchain_manager.hpp"
#include "satox/blockchain/block.hpp"
#include "satox/blockchain/transaction.hpp"
#include "satox/blockchain/types.hpp"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <chrono>
#include <thread>
#include <algorithm>
#include <sstream>
#include <iomanip>

using json = nlohmann::json;

namespace satox::blockchain {

BlockchainManager& BlockchainManager::getInstance() {
    static BlockchainManager instance;
    return instance;
}

BlockchainManager::BlockchainManager() : initialized_(false), connected_(false), state_(BlockchainState::UNINITIALIZED), statsEnabled_(false) {
    // Create log directory
    std::filesystem::create_directories("logs/components/blockchain/");
    
    // Initialize logger
    try {
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v");
        spdlog::set_level(spdlog::level::info);
    } catch (const std::exception& e) {
        // Logger initialization failed, continue without logging
    }
    
    spdlog::info("BlockchainManager: Initialized");
}

BlockchainManager::~BlockchainManager() {
    shutdown();
}

bool BlockchainManager::initialize(const BlockchainConfig& config) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        if (initialized_) {
            setLastError("Already initialized");
            logOperation("initialize", false, 0.0);
            return false;
        }
        
        if (!validateConfig(config)) {
            setLastError("Invalid configuration");
            logOperation("initialize", false, 0.0);
            return false;
        }
        
        state_ = BlockchainState::INITIALIZING;
        spdlog::info("BlockchainManager: Initializing with config: {}", config.name);
        
        config_ = config;
        
        // Initialize statistics
        stats_ = BlockchainStats{};
        statsEnabled_ = config.enableLogging;
        
        // Set up logging
        if (config.enableLogging) {
            std::filesystem::create_directories(config.logPath);
            spdlog::info("BlockchainManager: Logging enabled at {}", config.logPath);
        }
        
        initialized_ = true;
        state_ = BlockchainState::INITIALIZED;
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        updateStats(true, duration.count());
        logOperation("initialize", true, duration.count());
        
        spdlog::info("BlockchainManager: Initialization completed successfully");
        return true;
        
    } catch (const std::exception& e) {
        setLastError("Initialization failed: " + std::string(e.what()));
        state_ = BlockchainState::ERROR;
        logOperation("initialize", false, 0.0);
        spdlog::error("BlockchainManager: Initialization failed: {}", e.what());
        return false;
    }
}

void BlockchainManager::shutdown() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    if (!initialized_) {
        return;
    }
    
    spdlog::info("BlockchainManager: Shutting down");
    
    if (connected_) {
        disconnect();
    }
    
    initialized_ = false;
    state_ = BlockchainState::SHUTDOWN;
    
    spdlog::info("BlockchainManager: Shutdown completed");
}

bool BlockchainManager::connect() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        if (!initialized_) {
            setLastError("Not initialized");
            logOperation("connect", false, 0.0);
            return false;
        }
        
        if (connected_) {
            logOperation("connect", true, 0.0);
            return true;
        }
        
        state_ = BlockchainState::CONNECTING;
        spdlog::info("BlockchainManager: Connecting to {}:{}", config_.networkConfig.host, config_.networkConfig.port);
        
        // TODO: Implement actual connection logic
        // For now, simulate connection
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        connected_ = true;
        state_ = BlockchainState::CONNECTED;
        stats_.activeConnections = 1;
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        updateStats(true, duration.count());
        logOperation("connect", true, duration.count());
        
        spdlog::info("BlockchainManager: Connected successfully");
        return true;
        
    } catch (const std::exception& e) {
        setLastError("Connection failed: " + std::string(e.what()));
        state_ = BlockchainState::ERROR;
        logOperation("connect", false, 0.0);
        spdlog::error("BlockchainManager: Connection failed: {}", e.what());
        return false;
    }
}

bool BlockchainManager::disconnect() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        if (!connected_) {
            logOperation("disconnect", true, 0.0);
            return true;
        }
        
        spdlog::info("BlockchainManager: Disconnecting");
        
        // TODO: Implement actual disconnection logic
        connected_ = false;
        state_ = BlockchainState::INITIALIZED;
        stats_.activeConnections = 0;
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        updateStats(true, duration.count());
        logOperation("disconnect", true, duration.count());
        
        spdlog::info("BlockchainManager: Disconnected successfully");
        return true;
        
    } catch (const std::exception& e) {
        setLastError("Disconnection failed: " + std::string(e.what()));
        logOperation("disconnect", false, 0.0);
        spdlog::error("BlockchainManager: Disconnection failed: {}", e.what());
        return false;
    }
}

bool BlockchainManager::isConnected() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return connected_;
}

bool BlockchainManager::isInitialized() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return initialized_;
}

std::shared_ptr<Block> BlockchainManager::getLatestBlock() {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        if (!validateState()) {
            logOperation("getLatestBlock", false, 0.0);
            return nullptr;
        }
        
        // TODO: Implement actual block retrieval
        auto block = std::make_shared<Block>();
        block->setHeight(stats_.currentHeight);
        block->setHash("latest_block_hash");
        block->setTimestamp(std::chrono::system_clock::now());
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        updateStats(true, duration.count());
        logOperation("getLatestBlock", true, duration.count());
        
        return block;
        
    } catch (const std::exception& e) {
        setLastError("Failed to get latest block: " + std::string(e.what()));
        logOperation("getLatestBlock", false, 0.0);
        spdlog::error("BlockchainManager: Failed to get latest block: {}", e.what());
        return nullptr;
    }
}

std::shared_ptr<Block> BlockchainManager::getBlockByHash(const std::string& hash) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        if (!validateState()) {
            logOperation("getBlockByHash", false, 0.0);
            return nullptr;
        }
        
        if (hash.empty()) {
            setLastError("Invalid block hash");
            logOperation("getBlockByHash", false, 0.0);
            return nullptr;
        }
        
        // TODO: Implement actual block retrieval by hash
        auto block = std::make_shared<Block>();
        block->setHash(hash);
        block->setTimestamp(std::chrono::system_clock::now());
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        updateStats(true, duration.count());
        logOperation("getBlockByHash", true, duration.count());
        
        return block;
        
    } catch (const std::exception& e) {
        setLastError("Failed to get block by hash: " + std::string(e.what()));
        logOperation("getBlockByHash", false, 0.0);
        spdlog::error("BlockchainManager: Failed to get block by hash: {}", e.what());
        return nullptr;
    }
}

std::shared_ptr<Block> BlockchainManager::getBlockByHeight(uint64_t height) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        if (!validateState()) {
            logOperation("getBlockByHeight", false, 0.0);
            return nullptr;
        }
        
        // TODO: Implement actual block retrieval by height
        auto block = std::make_shared<Block>();
        block->setHeight(height);
        block->setHash("block_hash_" + std::to_string(height));
        block->setTimestamp(std::chrono::system_clock::now());
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        updateStats(true, duration.count());
        logOperation("getBlockByHeight", true, duration.count());
        
        return block;
        
    } catch (const std::exception& e) {
        setLastError("Failed to get block by height: " + std::string(e.what()));
        logOperation("getBlockByHeight", false, 0.0);
        spdlog::error("BlockchainManager: Failed to get block by height: {}", e.what());
        return nullptr;
    }
}

bool BlockchainManager::validateBlock(const std::shared_ptr<Block>& block) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        if (!validateState()) {
            logOperation("validateBlock", false, 0.0);
            return false;
        }
        
        if (!block) {
            setLastError("Invalid block");
            logOperation("validateBlock", false, 0.0);
            return false;
        }
        
        // TODO: Implement actual block validation
        bool isValid = block->isValid();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        updateStats(isValid, duration.count());
        logOperation("validateBlock", isValid, duration.count());
        
        return isValid;
        
    } catch (const std::exception& e) {
        setLastError("Block validation failed: " + std::string(e.what()));
        logOperation("validateBlock", false, 0.0);
        spdlog::error("BlockchainManager: Block validation failed: {}", e.what());
        return false;
    }
}

uint64_t BlockchainManager::getBlockCount() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return stats_.totalBlocks;
}

std::shared_ptr<Transaction> BlockchainManager::createTransaction(
    const std::string& from,
    const std::string& to,
    double value,
    const std::vector<uint8_t>& data) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        if (!validateState()) {
            logOperation("createTransaction", false, 0.0);
            return nullptr;
        }
        
        if (from.empty() || to.empty() || value <= 0) {
            setLastError("Invalid transaction parameters");
            logOperation("createTransaction", false, 0.0);
            return nullptr;
        }
        
        // TODO: Implement actual transaction creation
        auto tx = std::make_shared<Transaction>();
        tx->setFrom(from);
        tx->setTo(to);
        tx->setValue(value);
        tx->setData(data);
        tx->setTimestamp(std::chrono::system_clock::now());
        tx->setStatus("pending");
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        updateStats(true, duration.count());
        logOperation("createTransaction", true, duration.count());
        
        return tx;
        
    } catch (const std::exception& e) {
        setLastError("Failed to create transaction: " + std::string(e.what()));
        logOperation("createTransaction", false, 0.0);
        spdlog::error("BlockchainManager: Failed to create transaction: {}", e.what());
        return nullptr;
    }
}

bool BlockchainManager::broadcastTransaction(const std::shared_ptr<Transaction>& tx) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        if (!validateState()) {
            logOperation("broadcastTransaction", false, 0.0);
            return false;
        }
        
        if (!tx) {
            setLastError("Invalid transaction");
            logOperation("broadcastTransaction", false, 0.0);
            return false;
        }
        
        // TODO: Implement actual transaction broadcasting
        tx->setStatus("broadcasted");
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        updateStats(true, duration.count());
        logOperation("broadcastTransaction", true, duration.count());
        
        notifyTransactionEvent(tx);
        return true;
        
    } catch (const std::exception& e) {
        setLastError("Failed to broadcast transaction: " + std::string(e.what()));
        logOperation("broadcastTransaction", false, 0.0);
        spdlog::error("BlockchainManager: Failed to broadcast transaction: {}", e.what());
        return false;
    }
}

std::shared_ptr<Transaction> BlockchainManager::getTransaction(const std::string& tx_hash) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        if (!validateState()) {
            logOperation("getTransaction", false, 0.0);
            return nullptr;
        }
        
        if (tx_hash.empty()) {
            setLastError("Invalid transaction hash");
            logOperation("getTransaction", false, 0.0);
            return nullptr;
        }
        
        // TODO: Implement actual transaction retrieval
        auto tx = std::make_shared<Transaction>();
        tx->setHash(tx_hash);
        tx->setStatus("confirmed");
        tx->setTimestamp(std::chrono::system_clock::now());
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        updateStats(true, duration.count());
        logOperation("getTransaction", true, duration.count());
        
        return tx;
        
    } catch (const std::exception& e) {
        setLastError("Failed to get transaction: " + std::string(e.what()));
        logOperation("getTransaction", false, 0.0);
        spdlog::error("BlockchainManager: Failed to get transaction: {}", e.what());
        return nullptr;
    }
}

std::string BlockchainManager::getTransactionStatus(const std::string& tx_hash) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        if (!validateState()) {
            logOperation("getTransactionStatus", false, 0.0);
            return "unknown";
        }
        
        if (tx_hash.empty()) {
            setLastError("Invalid transaction hash");
            logOperation("getTransactionStatus", false, 0.0);
            return "unknown";
        }
        
        // TODO: Implement actual status retrieval
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        updateStats(true, duration.count());
        logOperation("getTransactionStatus", true, duration.count());
        
        return "confirmed";
        
    } catch (const std::exception& e) {
        setLastError("Failed to get transaction status: " + std::string(e.what()));
        logOperation("getTransactionStatus", false, 0.0);
        spdlog::error("BlockchainManager: Failed to get transaction status: {}", e.what());
        return "unknown";
    }
}

bool BlockchainManager::validateTransaction(const std::shared_ptr<Transaction>& tx) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        if (!validateState()) {
            logOperation("validateTransaction", false, 0.0);
            return false;
        }
        
        if (!tx) {
            setLastError("Invalid transaction");
            logOperation("validateTransaction", false, 0.0);
            return false;
        }
        
        // TODO: Implement actual transaction validation
        bool isValid = tx->isValid();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        updateStats(isValid, duration.count());
        logOperation("validateTransaction", isValid, duration.count());
        
        return isValid;
        
    } catch (const std::exception& e) {
        setLastError("Transaction validation failed: " + std::string(e.what()));
        logOperation("validateTransaction", false, 0.0);
        spdlog::error("BlockchainManager: Transaction validation failed: {}", e.what());
        return false;
    }
}

uint64_t BlockchainManager::getChainHeight() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return stats_.currentHeight;
}

double BlockchainManager::getBalance(const std::string& address) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        if (!validateState()) {
            logOperation("getBalance", false, 0.0);
            return 0.0;
        }
        
        if (address.empty()) {
            setLastError("Invalid address");
            logOperation("getBalance", false, 0.0);
            return 0.0;
        }
        
        // TODO: Implement actual balance retrieval
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        updateStats(true, duration.count());
        logOperation("getBalance", true, duration.count());
        
        return 1000.0; // Mock balance
        
    } catch (const std::exception& e) {
        setLastError("Failed to get balance: " + std::string(e.what()));
        logOperation("getBalance", false, 0.0);
        spdlog::error("BlockchainManager: Failed to get balance: {}", e.what());
        return 0.0;
    }
}

uint64_t BlockchainManager::getNonce(const std::string& address) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        if (!validateState()) {
            logOperation("getNonce", false, 0.0);
            return 0;
        }
        
        if (address.empty()) {
            setLastError("Invalid address");
            logOperation("getNonce", false, 0.0);
            return 0;
        }
        
        // TODO: Implement actual nonce retrieval
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        updateStats(true, duration.count());
        logOperation("getNonce", true, duration.count());
        
        return 1; // Mock nonce
        
    } catch (const std::exception& e) {
        setLastError("Failed to get nonce: " + std::string(e.what()));
        logOperation("getNonce", false, 0.0);
        spdlog::error("BlockchainManager: Failed to get nonce: {}", e.what());
        return 0;
    }
}

std::string BlockchainManager::getChainState() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    json state = {
        {"height", stats_.currentHeight},
        {"connected", connected_},
        {"synced", isChainSynced()},
        {"total_blocks", stats_.totalBlocks},
        {"total_transactions", stats_.totalTransactions}
    };
    
    return state.dump();
}

bool BlockchainManager::isChainSynced() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    // TODO: Implement actual sync status check
    return connected_ && stats_.currentHeight > 0;
}

std::vector<std::string> BlockchainManager::getConnectedPeers() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    // TODO: Implement actual peer list retrieval
    return {"127.0.0.1:60777"};
}

bool BlockchainManager::addPeer(const std::string& peer_address) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        if (!validateState()) {
            logOperation("addPeer", false, 0.0);
            return false;
        }
        
        if (peer_address.empty()) {
            setLastError("Invalid peer address");
            logOperation("addPeer", false, 0.0);
            return false;
        }
        
        // TODO: Implement actual peer addition
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        updateStats(true, duration.count());
        logOperation("addPeer", true, duration.count());
        
        return true;
        
    } catch (const std::exception& e) {
        setLastError("Failed to add peer: " + std::string(e.what()));
        logOperation("addPeer", false, 0.0);
        spdlog::error("BlockchainManager: Failed to add peer: {}", e.what());
        return false;
    }
}

bool BlockchainManager::removePeer(const std::string& peer_address) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        if (!validateState()) {
            logOperation("removePeer", false, 0.0);
            return false;
        }
        
        if (peer_address.empty()) {
            setLastError("Invalid peer address");
            logOperation("removePeer", false, 0.0);
            return false;
        }
        
        // TODO: Implement actual peer removal
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        updateStats(true, duration.count());
        logOperation("removePeer", true, duration.count());
        
        return true;
        
    } catch (const std::exception& e) {
        setLastError("Failed to remove peer: " + std::string(e.what()));
        logOperation("removePeer", false, 0.0);
        spdlog::error("BlockchainManager: Failed to remove peer: {}", e.what());
        return false;
    }
}

BlockchainConfig BlockchainManager::getConfig() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return config_;
}

bool BlockchainManager::updateConfig(const BlockchainConfig& config) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        if (!validateConfig(config)) {
            setLastError("Invalid configuration");
            logOperation("updateConfig", false, 0.0);
            return false;
        }
        
        config_ = config;
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        updateStats(true, duration.count());
        logOperation("updateConfig", true, duration.count());
        
        return true;
        
    } catch (const std::exception& e) {
        setLastError("Failed to update config: " + std::string(e.what()));
        logOperation("updateConfig", false, 0.0);
        spdlog::error("BlockchainManager: Failed to update config: {}", e.what());
        return false;
    }
}

bool BlockchainManager::validateConfig(const BlockchainConfig& config) {
    if (config.name.empty()) {
        return false;
    }
    
    if (config.networkConfig.port <= 0 || config.networkConfig.port > 65535) {
        return false;
    }
    
    if (config.timeout <= 0) {
        return false;
    }
    
    return true;
}

NetworkConfig BlockchainManager::getNetworkConfig() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return config_.networkConfig;
}

bool BlockchainManager::updateNetworkConfig(const NetworkConfig& config) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        if (config.port <= 0 || config.port > 65535) {
            setLastError("Invalid port number");
            logOperation("updateNetworkConfig", false, 0.0);
            return false;
        }
        
        config_.networkConfig = config;
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        updateStats(true, duration.count());
        logOperation("updateNetworkConfig", true, duration.count());
        
        return true;
        
    } catch (const std::exception& e) {
        setLastError("Failed to update network config: " + std::string(e.what()));
        logOperation("updateNetworkConfig", false, 0.0);
        spdlog::error("BlockchainManager: Failed to update network config: {}", e.what());
        return false;
    }
}

BlockchainStats BlockchainManager::getStats() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return stats_;
}

void BlockchainManager::resetStats() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    stats_ = BlockchainStats{};
    spdlog::info("BlockchainManager: Statistics reset");
}

bool BlockchainManager::enableStats(bool enable) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    statsEnabled_ = enable;
    spdlog::info("BlockchainManager: Statistics {}", enable ? "enabled" : "disabled");
    return true;
}

void BlockchainManager::registerBlockCallback(BlockCallback callback) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    block_callback_ = callback;
    spdlog::info("BlockchainManager: Block callback registered");
}

void BlockchainManager::registerTransactionCallback(TransactionCallback callback) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    transaction_callback_ = callback;
    spdlog::info("BlockchainManager: Transaction callback registered");
}

void BlockchainManager::registerErrorCallback(ErrorCallback callback) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    error_callback_ = callback;
    spdlog::info("BlockchainManager: Error callback registered");
}

void BlockchainManager::unregisterBlockCallback() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    block_callback_ = nullptr;
    spdlog::info("BlockchainManager: Block callback unregistered");
}

void BlockchainManager::unregisterTransactionCallback() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    transaction_callback_ = nullptr;
    spdlog::info("BlockchainManager: Transaction callback unregistered");
}

void BlockchainManager::unregisterErrorCallback() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    error_callback_ = nullptr;
    spdlog::info("BlockchainManager: Error callback unregistered");
}

std::string BlockchainManager::getLastError() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return lastError_;
}

void BlockchainManager::clearLastError() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    lastError_.clear();
}

bool BlockchainManager::healthCheck() {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        bool healthy = initialized_ && (connected_ || state_ == BlockchainState::INITIALIZED);
        
        if (!healthy) {
            consecutiveFailures_++;
        } else {
            consecutiveFailures_ = 0;
        }
        
        lastHealthCheck_ = std::chrono::system_clock::now();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        updateStats(healthy, duration.count());
        logOperation("healthCheck", healthy, duration.count());
        
        return healthy;
        
    } catch (const std::exception& e) {
        setLastError("Health check failed: " + std::string(e.what()));
        consecutiveFailures_++;
        logOperation("healthCheck", false, 0.0);
        spdlog::error("BlockchainManager: Health check failed: {}", e.what());
        return false;
    }
}

json BlockchainManager::getHealthStatus() {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    json status = {
        {"healthy", isHealthy()},
        {"state", static_cast<int>(state_)},
        {"initialized", initialized_},
        {"connected", connected_},
        {"consecutive_failures", consecutiveFailures_},
        {"last_health_check", lastHealthCheck_.time_since_epoch().count()},
        {"stats", {
            {"total_operations", stats_.totalOperations},
            {"successful_operations", stats_.successfulOperations},
            {"failed_operations", stats_.failedOperations}
        }}
    };
    
    return status;
}

BlockchainState BlockchainManager::getState() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return state_;
}

bool BlockchainManager::isHealthy() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return initialized_ && consecutiveFailures_ < 5;
}

// Private helper methods
void BlockchainManager::setLastError(const std::string& error) {
    lastError_ = error;
    if (error_callback_) {
        error_callback_("blockchain", error);
    }
    spdlog::error("BlockchainManager: {}", error);
}

void BlockchainManager::notifyBlockEvent(const std::shared_ptr<Block>& block) {
    if (block_callback_ && block) {
        block_callback_(block);
    }
}

void BlockchainManager::notifyTransactionEvent(const std::shared_ptr<Transaction>& tx) {
    if (transaction_callback_ && tx) {
        transaction_callback_(tx);
    }
}

void BlockchainManager::notifyErrorEvent(const std::string& operation, const std::string& error) {
    if (error_callback_) {
        error_callback_(operation, error);
    }
}

void BlockchainManager::updateStats(bool success, double operationTime) {
    if (!statsEnabled_) {
        return;
    }
    
    stats_.totalOperations++;
    if (success) {
        stats_.successfulOperations++;
    } else {
        stats_.failedOperations++;
    }
    
    stats_.lastOperation = std::chrono::system_clock::now();
    
    // Update average operation time
    if (stats_.totalOperations > 1) {
        stats_.averageOperationTime = (stats_.averageOperationTime * (stats_.totalOperations - 1) + operationTime) / stats_.totalOperations;
    } else {
        stats_.averageOperationTime = operationTime;
    }
}

bool BlockchainManager::validateState() {
    if (!initialized_) {
        setLastError("Not initialized");
        return false;
    }
    
    if (state_ == BlockchainState::ERROR) {
        setLastError("Component in error state");
        return false;
    }
    
    return true;
}

void BlockchainManager::logOperation(const std::string& operation, bool success, double duration) {
    if (!statsEnabled_) {
        return;
    }
    
    std::string level = success ? "info" : "error";
    std::string message = "Operation '" + operation + "' " + (success ? "completed" : "failed") + 
                          " in " + std::to_string(duration) + "ms";
    
    if (success) {
        spdlog::info("BlockchainManager: {}", message);
    } else {
        spdlog::error("BlockchainManager: {}", message);
    }
}

} // namespace satox::blockchain 