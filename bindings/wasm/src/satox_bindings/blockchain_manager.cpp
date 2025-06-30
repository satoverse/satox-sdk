#include <emscripten/bind.h>
#include <string>
#include <vector>
#include <map>

using namespace emscripten;

class BlockchainManager {
public:
    BlockchainManager() : initialized(false), connected(false) {}

    void initialize() {
        if (initialized) throw std::runtime_error("BlockchainManager already initialized");
        initialized = true;
    }

    bool isInitialized() const { return initialized; }

    void connect(const std::string& network) {
        if (!initialized) throw std::runtime_error("BlockchainManager not initialized");
        if (connected) throw std::runtime_error("Already connected to blockchain");
        if (network != "mainnet") throw std::runtime_error("Only mainnet is supported");
        connected = true;
    }

    void disconnect() {
        if (!connected) throw std::runtime_error("Not connected to blockchain");
        connected = false;
    }

    bool isConnected() const { return connected; }

    std::string getNetwork() const {
        if (!connected) throw std::runtime_error("Not connected to blockchain");
        return "mainnet";
    }

    int getBlockHeight() const {
        if (!connected) throw std::runtime_error("Not connected to blockchain");
        return 1000000; // Mock height
    }

    std::string getBlockHash(int height) const {
        if (!connected) throw std::runtime_error("Not connected to blockchain");
        return "0000000000000000000000000000000000000000000000000000000000000000";
    }

    std::map<std::string, std::string> getBlockInfo(const std::string& blockHash) const {
        if (!connected) throw std::runtime_error("Not connected to blockchain");
        return {
            {"hash", blockHash},
            {"height", "1000000"},
            {"time", "1234567890"},
            {"size", "1000000"}
        };
    }

    std::vector<std::string> getBlockTransactions(const std::string& blockHash) const {
        if (!connected) throw std::runtime_error("Not connected to blockchain");
        return {"tx1", "tx2", "tx3"};
    }

    std::map<std::string, std::string> getTransactionInfo(const std::string& txId) const {
        if (!connected) throw std::runtime_error("Not connected to blockchain");
        return {
            {"txid", txId},
            {"blockhash", "0000000000000000000000000000000000000000000000000000000000000000"},
            {"time", "1234567890"},
            {"size", "1000"}
        };
    }

    double getBalance(const std::string& address) const {
        if (!connected) throw std::runtime_error("Not connected to blockchain");
        return 1000.0; // Mock balance
    }

    std::vector<std::map<std::string, std::string>> getAddressTransactions(const std::string& address) const {
        if (!connected) throw std::runtime_error("Not connected to blockchain");
        return {
            {
                {"txid", "tx1"},
                {"time", "1234567890"},
                {"amount", "100.0"}
            }
        };
    }

private:
    bool initialized;
    bool connected;
};

EMSCRIPTEN_BINDINGS(BlockchainManager) {
    class_<BlockchainManager>("BlockchainManager")
        .constructor<>()
        .function("initialize", &BlockchainManager::initialize)
        .function("isInitialized", &BlockchainManager::isInitialized)
        .function("connect", &BlockchainManager::connect)
        .function("disconnect", &BlockchainManager::disconnect)
        .function("isConnected", &BlockchainManager::isConnected)
        .function("getNetwork", &BlockchainManager::getNetwork)
        .function("getBlockHeight", &BlockchainManager::getBlockHeight)
        .function("getBlockHash", &BlockchainManager::getBlockHash)
        .function("getBlockInfo", &BlockchainManager::getBlockInfo)
        .function("getBlockTransactions", &BlockchainManager::getBlockTransactions)
        .function("getTransactionInfo", &BlockchainManager::getTransactionInfo)
        .function("getBalance", &BlockchainManager::getBalance)
        .function("getAddressTransactions", &BlockchainManager::getAddressTransactions)
        ;
} 