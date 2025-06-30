#include "satox/transactions/transaction_manager.hpp"
#include <openssl/sha.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <openssl/rand.h>

namespace satox::transactions {

TransactionManager& TransactionManager::getInstance() {
    static TransactionManager instance;
    return instance;
}

bool TransactionManager::initialize(const nlohmann::json& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        lastError_ = "TransactionManager already initialized";
        return false;
    }

    try {
        config_ = config;
        initialized_ = true;
        return true;
    } catch (const std::exception& e) {
        lastError_ = std::string("Failed to initialize TransactionManager: ") + e.what();
        return false;
    }
}

void TransactionManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    transactions_.clear();
    callbacks_.clear();
    initialized_ = false;
}

bool TransactionManager::createTransaction(const std::string& from, const std::string& to,
                                         uint64_t amount, const std::string& assetId,
                                         const nlohmann::json& metadata, std::string& transactionId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "TransactionManager not initialized";
        return false;
    }

    try {
        Transaction transaction;
        transaction.id = generateTransactionId();
        transaction.from = from;
        transaction.to = to;
        transaction.amount = amount;
        transaction.assetId = assetId;
        transaction.metadata = metadata;
        transaction.status = Status::PENDING;
        transaction.timestamp = std::chrono::system_clock::now();

        if (!validateTransaction(transaction)) {
            return false;
        }

        transactions_[transaction.id] = transaction;
        transactionId = transaction.id;

        // Notify callbacks
        for (const auto& callback : callbacks_) {
            callback(transaction);
        }

        return true;
    } catch (const std::exception& e) {
        lastError_ = std::string("Failed to create transaction: ") + e.what();
        return false;
    }
}

bool TransactionManager::signTransaction(const std::string& transactionId, const std::string& privateKey) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "TransactionManager not initialized";
        return false;
    }

    auto it = transactions_.find(transactionId);
    if (it == transactions_.end()) {
        lastError_ = "Transaction not found";
        return false;
    }

    try {
        // Create a string representation of the transaction for signing
        std::stringstream ss;
        ss << it->second.from << it->second.to << it->second.amount 
           << it->second.assetId << it->second.timestamp.time_since_epoch().count();
        std::string data = ss.str();

        // Sign the data using OpenSSL
        EVP_PKEY* pkey = nullptr;
        EVP_PKEY_CTX* ctx = nullptr;
        size_t siglen;
        unsigned char* signature = nullptr;

        // Convert private key from hex to binary
        std::vector<unsigned char> keyBytes(privateKey.length() / 2);
        for (size_t i = 0; i < keyBytes.size(); ++i) {
            keyBytes[i] = std::stoi(privateKey.substr(i * 2, 2), nullptr, 16);
        }

        // Create private key
        pkey = EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519, nullptr, keyBytes.data(), keyBytes.size());
        if (!pkey) {
            lastError_ = "Failed to create private key";
            return false;
        }

        // Create signature context
        ctx = EVP_PKEY_CTX_new(pkey, nullptr);
        if (!ctx) {
            EVP_PKEY_free(pkey);
            lastError_ = "Failed to create signature context";
            return false;
        }

        // Initialize signing
        if (EVP_PKEY_sign_init(ctx) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            lastError_ = "Failed to initialize signing";
            return false;
        }

        // Get signature length
        if (EVP_PKEY_sign(ctx, nullptr, &siglen, 
                         reinterpret_cast<const unsigned char*>(data.c_str()), 
                         data.length()) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            lastError_ = "Failed to get signature length";
            return false;
        }

        // Allocate signature buffer
        signature = new unsigned char[siglen];

        // Perform signing
        if (EVP_PKEY_sign(ctx, signature, &siglen,
                         reinterpret_cast<const unsigned char*>(data.c_str()),
                         data.length()) <= 0) {
            delete[] signature;
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            lastError_ = "Failed to sign transaction";
            return false;
        }

        // Convert signature to hex string
        std::stringstream sigss;
        for (size_t i = 0; i < siglen; ++i) {
            sigss << std::hex << std::setw(2) << std::setfill('0') 
                  << static_cast<int>(signature[i]);
        }
        it->second.signature = sigss.str();

        // Cleanup
        delete[] signature;
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);

        return true;
    } catch (const std::exception& e) {
        lastError_ = std::string("Failed to sign transaction: ") + e.what();
        return false;
    }
}

bool TransactionManager::broadcastTransaction(const std::string& transactionId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "TransactionManager not initialized";
        return false;
    }

    auto it = transactions_.find(transactionId);
    if (it == transactions_.end()) {
        lastError_ = "Transaction not found";
        return false;
    }

    if (it->second.status != Status::PENDING) {
        lastError_ = "Transaction is not in pending state";
        return false;
    }

    if (it->second.signature.empty()) {
        lastError_ = "Transaction is not signed";
        return false;
    }

    try {
        // Here we would typically broadcast to the network
        // For now, we'll just update the status
        updateTransactionStatus(transactionId, Status::CONFIRMED);
        return true;
    } catch (const std::exception& e) {
        lastError_ = std::string("Failed to broadcast transaction: ") + e.what();
        return false;
    }
}

bool TransactionManager::cancelTransaction(const std::string& transactionId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "TransactionManager not initialized";
        return false;
    }

    auto it = transactions_.find(transactionId);
    if (it == transactions_.end()) {
        lastError_ = "Transaction not found";
        return false;
    }

    if (it->second.status != Status::PENDING) {
        lastError_ = "Can only cancel pending transactions";
        return false;
    }

    try {
        updateTransactionStatus(transactionId, Status::CANCELLED);
        return true;
    } catch (const std::exception& e) {
        lastError_ = std::string("Failed to cancel transaction: ") + e.what();
        return false;
    }
}

bool TransactionManager::getTransaction(const std::string& transactionId, Transaction& transaction) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "TransactionManager not initialized";
        return false;
    }

    auto it = transactions_.find(transactionId);
    if (it == transactions_.end()) {
        lastError_ = "Transaction not found";
        return false;
    }

    transaction = it->second;
    return true;
}

bool TransactionManager::getTransactionStatus(const std::string& transactionId, Status& status) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "TransactionManager not initialized";
        return false;
    }

    auto it = transactions_.find(transactionId);
    if (it == transactions_.end()) {
        lastError_ = "Transaction not found";
        return false;
    }

    status = it->second.status;
    return true;
}

bool TransactionManager::getTransactionHistory(const std::string& address, 
                                             std::vector<Transaction>& transactions) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "TransactionManager not initialized";
        return false;
    }

    try {
        transactions.clear();
        for (const auto& pair : transactions_) {
            if (pair.second.from == address || pair.second.to == address) {
                transactions.push_back(pair.second);
            }
        }

        // Sort by timestamp in descending order
        std::sort(transactions.begin(), transactions.end(),
                 [](const Transaction& a, const Transaction& b) {
                     return a.timestamp > b.timestamp;
                 });

        return true;
    } catch (const std::exception& e) {
        lastError_ = std::string("Failed to get transaction history: ") + e.what();
        return false;
    }
}

bool TransactionManager::validateTransaction(const Transaction& transaction) {
    if (transaction.from.empty() || transaction.to.empty()) {
        lastError_ = "Invalid addresses";
        return false;
    }

    if (transaction.amount == 0) {
        lastError_ = "Invalid amount";
        return false;
    }

    if (transaction.assetId.empty()) {
        lastError_ = "Invalid asset ID";
        return false;
    }

    return true;
}

void TransactionManager::registerTransactionCallback(TransactionCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_.push_back(callback);
}

void TransactionManager::unregisterTransactionCallback(TransactionCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_.erase(
        std::remove_if(callbacks_.begin(), callbacks_.end(),
                      [&callback](const TransactionCallback& cb) {
                          return cb.target_type() == callback.target_type();
                      }),
        callbacks_.end()
    );
}

std::string TransactionManager::getLastError() const {
    return lastError_;
}

void TransactionManager::clearLastError() {
    lastError_.clear();
}

std::string TransactionManager::generateTransactionId() {
    // Generate a unique transaction ID using timestamp and random data
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    
    std::stringstream ss;
    ss << std::hex << timestamp;
    
    // Add some random data
    unsigned char random[16];
    RAND_bytes(random, sizeof(random));
    for (size_t i = 0; i < sizeof(random); ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') 
           << static_cast<int>(random[i]);
    }
    
    return ss.str();
}

bool TransactionManager::verifySignature(const Transaction& transaction) {
    if (transaction.signature.empty()) {
        return false;
    }

    try {
        // Create a string representation of the transaction for verification
        std::stringstream ss;
        ss << transaction.from << transaction.to << transaction.amount 
           << transaction.assetId << transaction.timestamp.time_since_epoch().count();
        std::string data = ss.str();

        // Convert signature from hex to binary
        std::vector<unsigned char> sigBytes(transaction.signature.length() / 2);
        for (size_t i = 0; i < sigBytes.size(); ++i) {
            sigBytes[i] = std::stoi(transaction.signature.substr(i * 2, 2), nullptr, 16);
        }

        // Here we would typically verify the signature using the public key
        // For now, we'll just return true if the signature is not empty
        return true;
    } catch (const std::exception& e) {
        lastError_ = std::string("Failed to verify signature: ") + e.what();
        return false;
    }
}

void TransactionManager::updateTransactionStatus(const std::string& transactionId, 
                                               Status status, const std::string& error) {
    auto it = transactions_.find(transactionId);
    if (it != transactions_.end()) {
        it->second.status = status;
        it->second.error = error;

        // Notify callbacks
        for (const auto& callback : callbacks_) {
            callback(it->second);
        }
    }
}

bool TransactionManager::addUTXO(const UTXO& utxo) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "TransactionManager not initialized";
        return false;
    }

    try {
        std::string key = utxo.txId + ":" + std::to_string(utxo.outputIndex);
        if (utxoMap_.find(key) != utxoMap_.end()) {
            lastError_ = "UTXO already exists";
            return false;
        }

        utxoMap_[key] = utxo;
        utxos_[utxo.address].push_back(utxo);
        return true;
    } catch (const std::exception& e) {
        lastError_ = std::string("Failed to add UTXO: ") + e.what();
        return false;
    }
}

bool TransactionManager::spendUTXO(const std::string& txId, uint32_t outputIndex) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "TransactionManager not initialized";
        return false;
    }

    try {
        std::string key = txId + ":" + std::to_string(outputIndex);
        auto it = utxoMap_.find(key);
        if (it == utxoMap_.end()) {
            lastError_ = "UTXO not found";
            return false;
        }

        if (it->second.spent) {
            lastError_ = "UTXO already spent";
            return false;
        }

        it->second.spent = true;
        return true;
    } catch (const std::exception& e) {
        lastError_ = std::string("Failed to spend UTXO: ") + e.what();
        return false;
    }
}

bool TransactionManager::getUTXOs(const std::string& address, std::vector<UTXO>& utxos) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "TransactionManager not initialized";
        return false;
    }

    try {
        auto it = utxos_.find(address);
        if (it == utxos_.end()) {
            utxos.clear();
            return true;
        }

        utxos = it->second;
        return true;
    } catch (const std::exception& e) {
        lastError_ = std::string("Failed to get UTXOs: ") + e.what();
        return false;
    }
}

bool TransactionManager::getUTXOsForAmount(const std::string& address, uint64_t amount, std::vector<UTXO>& utxos) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "TransactionManager not initialized";
        return false;
    }

    try {
        auto it = utxos_.find(address);
        if (it == utxos_.end()) {
            lastError_ = "No UTXOs found for address";
            return false;
        }

        utxos.clear();
        uint64_t total = 0;

        // Sort UTXOs by amount in descending order
        std::vector<UTXO> sortedUtxos = it->second;
        std::sort(sortedUtxos.begin(), sortedUtxos.end(),
                 [](const UTXO& a, const UTXO& b) { return a.amount > b.amount; });

        for (const auto& utxo : sortedUtxos) {
            if (!utxo.spent) {
                utxos.push_back(utxo);
                total += utxo.amount;
                if (total >= amount) {
                    break;
                }
            }
        }

        if (total < amount) {
            lastError_ = "Insufficient funds";
            return false;
        }

        return true;
    } catch (const std::exception& e) {
        lastError_ = std::string("Failed to get UTXOs for amount: ") + e.what();
        return false;
    }
}

uint64_t TransactionManager::calculateFee(const Transaction& transaction) {
    // Calculate transaction size in bytes
    size_t size = 0;
    size += transaction.id.length();
    size += transaction.from.length();
    size += transaction.to.length();
    size += sizeof(transaction.amount);
    size += transaction.assetId.length();
    size += transaction.signature.length();
    size += sizeof(transaction.timestamp);
    size += transaction.metadata.dump().length();

    // Add size of inputs and outputs
    for (const auto& input : transaction.inputs) {
        size += input.address.length();
        size += sizeof(input.amount);
        size += input.assetId.length();
        size += input.txId.length();
        size += sizeof(input.outputIndex);
    }

    for (const auto& output : transaction.outputs) {
        size += output.address.length();
        size += sizeof(output.amount);
        size += output.assetId.length();
    }

    return size * feeRate_;
}

uint64_t TransactionManager::estimateFee(uint64_t inputCount, uint64_t outputCount) {
    // Estimate transaction size based on input and output counts
    size_t size = 0;
    size += 32;  // transaction ID
    size += 32;  // from address
    size += 32;  // to address
    size += 8;   // amount
    size += 32;  // asset ID
    size += 64;  // signature
    size += 8;   // timestamp
    size += 32;  // metadata

    // Add size of inputs
    size += inputCount * (32 + 8 + 32 + 32 + 4);  // address + amount + assetId + txId + outputIndex

    // Add size of outputs
    size += outputCount * (32 + 8 + 32);  // address + amount + assetId

    return size * feeRate_;
}

bool TransactionManager::setFeeRate(uint64_t satoshisPerByte) {
    std::lock_guard<std::mutex> lock(mutex_);
    feeRate_ = satoshisPerByte;
    return true;
}

bool TransactionManager::addToMempool(const Transaction& transaction) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "TransactionManager not initialized";
        return false;
    }

    try {
        if (mempool_.find(transaction.id) != mempool_.end()) {
            lastError_ = "Transaction already in mempool";
            return false;
        }

        if (!validateTransaction(transaction)) {
            return false;
        }

        mempool_[transaction.id] = transaction;
        return true;
    } catch (const std::exception& e) {
        lastError_ = std::string("Failed to add to mempool: ") + e.what();
        return false;
    }
}

bool TransactionManager::removeFromMempool(const std::string& transactionId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "TransactionManager not initialized";
        return false;
    }

    try {
        auto it = mempool_.find(transactionId);
        if (it == mempool_.end()) {
            lastError_ = "Transaction not found in mempool";
            return false;
        }

        mempool_.erase(it);
        return true;
    } catch (const std::exception& e) {
        lastError_ = std::string("Failed to remove from mempool: ") + e.what();
        return false;
    }
}

bool TransactionManager::getMempoolTransactions(std::vector<Transaction>& transactions) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "TransactionManager not initialized";
        return false;
    }

    try {
        transactions.clear();
        for (const auto& pair : mempool_) {
            transactions.push_back(pair.second);
        }
        return true;
    } catch (const std::exception& e) {
        lastError_ = std::string("Failed to get mempool transactions: ") + e.what();
        return false;
    }
}

bool TransactionManager::getMempoolSize(size_t& size) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "TransactionManager not initialized";
        return false;
    }

    try {
        size = mempool_.size();
        return true;
    } catch (const std::exception& e) {
        lastError_ = std::string("Failed to get mempool size: ") + e.what();
        return false;
    }
}

bool TransactionManager::validateUTXOs(const Transaction& transaction) {
    uint64_t totalInput = 0;
    uint64_t totalOutput = 0;

    // Validate inputs
    for (const auto& input : transaction.inputs) {
        std::string key = input.txId + ":" + std::to_string(input.outputIndex);
        auto it = utxoMap_.find(key);
        if (it == utxoMap_.end()) {
            lastError_ = "Input UTXO not found";
            return false;
        }

        if (it->second.spent) {
            lastError_ = "Input UTXO already spent";
            return false;
        }

        if (it->second.address != input.address) {
            lastError_ = "Input address mismatch";
            return false;
        }

        if (it->second.amount != input.amount) {
            lastError_ = "Input amount mismatch";
            return false;
        }

        if (it->second.assetId != input.assetId) {
            lastError_ = "Input asset ID mismatch";
            return false;
        }

        totalInput += input.amount;
    }

    // Validate outputs
    for (const auto& output : transaction.outputs) {
        if (output.amount == 0) {
            lastError_ = "Output amount cannot be zero";
            return false;
        }

        totalOutput += output.amount;
    }

    // Validate total amounts
    if (totalOutput + transaction.fee > totalInput) {
        lastError_ = "Insufficient funds";
        return false;
    }

    return true;
}

bool TransactionManager::updateUTXOs(const Transaction& transaction) {
    // Mark input UTXOs as spent
    for (const auto& input : transaction.inputs) {
        if (!spendUTXO(input.txId, input.outputIndex)) {
            return false;
        }
    }

    // Create new UTXOs for outputs
    for (size_t i = 0; i < transaction.outputs.size(); ++i) {
        const auto& output = transaction.outputs[i];
        UTXO utxo;
        utxo.txId = transaction.id;
        utxo.outputIndex = i;
        utxo.amount = output.amount;
        utxo.assetId = output.assetId;
        utxo.address = output.address;
        utxo.timestamp = transaction.timestamp;
        utxo.spent = false;

        if (!addUTXO(utxo)) {
            return false;
        }
    }

    return true;
}

bool TransactionManager::checkBalance(const std::string& address, uint64_t amount) {
    std::vector<UTXO> utxos;
    if (!getUTXOs(address, utxos)) {
        return false;
    }

    uint64_t total = 0;
    for (const auto& utxo : utxos) {
        if (!utxo.spent) {
            total += utxo.amount;
            if (total >= amount) {
                return true;
            }
        }
    }

    return false;
}

} // namespace satox::transactions 