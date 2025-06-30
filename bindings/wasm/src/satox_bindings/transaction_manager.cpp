#include <emscripten/bind.h>
#include <string>
#include <vector>
#include <map>

using namespace emscripten;

class TransactionManager {
public:
    TransactionManager() : initialized(false) {}

    void initialize() {
        if (initialized) throw std::runtime_error("TransactionManager already initialized");
        initialized = true;
    }

    bool isInitialized() const { return initialized; }

    std::string createTransaction(const std::string& fromAddress, const std::string& toAddress, double amount) {
        if (!initialized) throw std::runtime_error("TransactionManager not initialized");
        if (amount <= 0) throw std::runtime_error("Invalid amount");
        // In WASM, we'll return a mock transaction ID
        return "tx" + std::to_string(rand());
    }

    std::string createAssetTransaction(const std::string& fromAddress, const std::string& toAddress, 
                                     const std::string& assetName, double amount) {
        if (!initialized) throw std::runtime_error("TransactionManager not initialized");
        if (amount <= 0) throw std::runtime_error("Invalid amount");
        if (assetName.empty()) throw std::runtime_error("Invalid asset name");
        // In WASM, we'll return a mock transaction ID
        return "tx" + std::to_string(rand());
    }

    std::string createNFTTransaction(const std::string& fromAddress, const std::string& toAddress, 
                                   const std::string& nftId) {
        if (!initialized) throw std::runtime_error("TransactionManager not initialized");
        if (nftId.empty()) throw std::runtime_error("Invalid NFT ID");
        // In WASM, we'll return a mock transaction ID
        return "tx" + std::to_string(rand());
    }

    std::map<std::string, std::string> getTransactionStatus(const std::string& txId) const {
        if (!initialized) throw std::runtime_error("TransactionManager not initialized");
        return {
            {"txid", txId},
            {"status", "confirmed"},
            {"confirmations", "6"},
            {"time", "1234567890"}
        };
    }

    std::vector<std::map<std::string, std::string>> getTransactionHistory(const std::string& address) const {
        if (!initialized) throw std::runtime_error("TransactionManager not initialized");
        return {
            {
                {"txid", "tx1"},
                {"type", "send"},
                {"amount", "100.0"},
                {"time", "1234567890"}
            }
        };
    }

    double getTransactionFee(const std::string& txId) const {
        if (!initialized) throw std::runtime_error("TransactionManager not initialized");
        return 0.001; // Mock fee
    }

    std::vector<std::string> getUnconfirmedTransactions() const {
        if (!initialized) throw std::runtime_error("TransactionManager not initialized");
        return {"tx1", "tx2", "tx3"};
    }

    void broadcastTransaction(const std::string& txId) {
        if (!initialized) throw std::runtime_error("TransactionManager not initialized");
        // In WASM, we'll just simulate broadcasting
    }

private:
    bool initialized;
};

EMSCRIPTEN_BINDINGS(TransactionManager) {
    class_<TransactionManager>("TransactionManager")
        .constructor<>()
        .function("initialize", &TransactionManager::initialize)
        .function("isInitialized", &TransactionManager::isInitialized)
        .function("createTransaction", &TransactionManager::createTransaction)
        .function("createAssetTransaction", &TransactionManager::createAssetTransaction)
        .function("createNFTTransaction", &TransactionManager::createNFTTransaction)
        .function("getTransactionStatus", &TransactionManager::getTransactionStatus)
        .function("getTransactionHistory", &TransactionManager::getTransactionHistory)
        .function("getTransactionFee", &TransactionManager::getTransactionFee)
        .function("getUnconfirmedTransactions", &TransactionManager::getUnconfirmedTransactions)
        .function("broadcastTransaction", &TransactionManager::broadcastTransaction)
        ;
} 