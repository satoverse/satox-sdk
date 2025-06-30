#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include <functional>
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include "satox/transactions/transaction_manager.hpp"

namespace satox::transactions {

using Transaction = TransactionManager::Transaction;

class TransactionSigner {
public:
    // Signature result structure
    struct SignatureResult {
        bool success;
        std::string signature;
        std::string error;
    };

    // Key pair structure
    struct KeyPair {
        std::string privateKey;
        std::string publicKey;
    };

    // Get singleton instance
    static TransactionSigner& getInstance();

    // Initialize the signer
    bool initialize();

    // Shutdown the signer
    void shutdown();

    // Generate a new key pair
    KeyPair generateKeyPair();

    // Sign a transaction
    SignatureResult signTransaction(const Transaction& transaction, const std::string& privateKey);

    // Verify a transaction signature
    bool verifySignature(const Transaction& transaction, const std::string& signature, const std::string& publicKey);

    // Sign multiple transactions
    std::vector<SignatureResult> signTransactions(const std::vector<Transaction>& transactions, const std::string& privateKey);

    // Get last error message
    std::string getLastError() const;

    // Clear last error
    void clearLastError();

private:
    // Private constructor for singleton
    TransactionSigner() = default;
    ~TransactionSigner() = default;

    // Prevent copying
    TransactionSigner(const TransactionSigner&) = delete;
    TransactionSigner& operator=(const TransactionSigner&) = delete;

    // Helper methods for cryptographic operations
    std::string hashTransaction(const Transaction& transaction);
    std::string signHash(const std::string& hash, const std::string& privateKey);
    bool verifyHash(const std::string& hash, const std::string& signature, const std::string& publicKey);
    std::string serializeTransaction(const Transaction& transaction);

    // Member variables
    bool initialized_ = false;
    std::string lastError_;
    mutable std::mutex mutex_;
};

} // namespace satox::transactions 