#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <functional>
#include <chrono>
#include <nlohmann/json.hpp>

namespace satox::transactions {

class TransactionManager {
public:
    // Transaction status enum
    enum class Status {
        PENDING,
        CONFIRMED,
        FAILED,
        CANCELLED
    };

    // UTXO structure
    struct UTXO {
        std::string txId;
        uint32_t outputIndex;
        uint64_t amount;
        std::string assetId;
        std::string address;
        std::chrono::system_clock::time_point timestamp;
        bool spent;
    };

    // Transaction input/output structure
    struct TransactionIO {
        std::string address;
        uint64_t amount;
        std::string assetId;
        std::string txId;        // For inputs, this is the UTXO's txId
        uint32_t outputIndex;    // For inputs, this is the UTXO's outputIndex
    };

    // Transaction structure
    struct Transaction {
        std::string id;
        std::string from;
        std::string to;
        uint64_t amount;
        std::string assetId;
        std::string signature;
        Status status;
        std::string error;
        std::chrono::system_clock::time_point timestamp;
        nlohmann::json metadata;
        std::vector<TransactionIO> inputs;
        std::vector<TransactionIO> outputs;
        uint64_t fee;
        std::string type; // Transaction type (e.g., "default", "priority", "batch")
    };

    // Transaction callback type
    using TransactionCallback = std::function<void(const Transaction&)>;

    // Singleton instance
    static TransactionManager& getInstance();

    // Prevent copying
    TransactionManager(const TransactionManager&) = delete;
    TransactionManager& operator=(const TransactionManager&) = delete;

    // Initialization and shutdown
    bool initialize(const nlohmann::json& config);
    void shutdown();

    // Transaction operations
    bool createTransaction(const std::string& from, const std::string& to,
                          uint64_t amount, const std::string& assetId,
                          const nlohmann::json& metadata, std::string& transactionId);
    bool signTransaction(const std::string& transactionId, const std::string& privateKey);
    bool broadcastTransaction(const std::string& transactionId);
    bool cancelTransaction(const std::string& transactionId);
    bool getTransaction(const std::string& transactionId, Transaction& transaction);
    bool getTransactionStatus(const std::string& transactionId, Status& status);
    bool getTransactionHistory(const std::string& address, std::vector<Transaction>& transactions);
    bool validateTransaction(const Transaction& transaction);

    // UTXO operations
    bool addUTXO(const UTXO& utxo);
    bool spendUTXO(const std::string& txId, uint32_t outputIndex);
    bool getUTXOs(const std::string& address, std::vector<UTXO>& utxos);
    bool getUTXOsForAmount(const std::string& address, uint64_t amount, std::vector<UTXO>& utxos);

    // Fee operations
    uint64_t calculateFee(const Transaction& transaction);
    uint64_t estimateFee(uint64_t inputCount, uint64_t outputCount);
    bool setFeeRate(uint64_t satoshisPerByte);

    // Mempool operations
    bool addToMempool(const Transaction& transaction);
    bool removeFromMempool(const std::string& transactionId);
    bool getMempoolTransactions(std::vector<Transaction>& transactions);
    bool getMempoolSize(size_t& size);

    // Transaction monitoring
    void registerTransactionCallback(TransactionCallback callback);
    void unregisterTransactionCallback(TransactionCallback callback);

    // Error handling
    std::string getLastError() const;
    void clearLastError();

private:
    TransactionManager() = default;
    ~TransactionManager() = default;

    // Helper methods
    std::string generateTransactionId();
    bool verifySignature(const Transaction& transaction);
    void updateTransactionStatus(const std::string& transactionId, Status status, const std::string& error = "");
    bool validateUTXOs(const Transaction& transaction);
    bool updateUTXOs(const Transaction& transaction);
    bool checkBalance(const std::string& address, uint64_t amount);

    // Member variables
    bool initialized_ = false;
    std::mutex mutex_;
    std::unordered_map<std::string, Transaction> transactions_;
    std::unordered_map<std::string, std::vector<UTXO>> utxos_;  // address -> UTXOs
    std::unordered_map<std::string, UTXO> utxoMap_;  // txId:outputIndex -> UTXO
    std::vector<TransactionCallback> callbacks_;
    std::string lastError_;
    nlohmann::json config_;
    uint64_t feeRate_ = 1;  // satoshis per byte
    std::unordered_map<std::string, Transaction> mempool_;  // transactionId -> Transaction
};

} // namespace satox::transactions 