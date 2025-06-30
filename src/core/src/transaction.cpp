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

#include "satox/core/transaction.h"
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/ec.h>
#include <openssl/bn.h>

namespace satox {
namespace core {

TransactionManager::TransactionManager() : initialized_(false) {}

TransactionManager::~TransactionManager() {
    if (initialized_) {
        cleanup();
    }
}

void TransactionManager::initialize() {
    if (initialized_) {
        throw std::runtime_error("TransactionManager already initialized");
    }
    initialized_ = true;
}

Transaction TransactionManager::createTransaction(
    const std::string& from,
    const std::string& to,
    const std::string& amount,
    const std::string& data) {
    
    if (!initialized_) {
        throw std::runtime_error("TransactionManager not initialized");
    }

    if (from.empty() || to.empty() || amount.empty()) {
        throw std::runtime_error("From, to, and amount are required");
    }

    Transaction transaction;
    transaction.from = from;
    transaction.to = to;
    transaction.amount = amount;
    transaction.data = data;
    transaction.timestamp = std::chrono::system_clock::now();
    transaction.status = "pending";

    // Generate transaction ID
    transaction.id = generateTransactionId(transaction);

    return transaction;
}

bool TransactionManager::signTransaction(Transaction& transaction, const std::string& privateKey) {
    if (!initialized_) {
        throw std::runtime_error("TransactionManager not initialized");
    }

    if (privateKey.empty()) {
        throw std::runtime_error("Private key is required");
    }

    try {
        // Create signature data
        std::stringstream ss;
        ss << transaction.from << transaction.to << transaction.amount 
           << transaction.data << transaction.timestamp.time_since_epoch().count();
        
        std::string dataToSign = ss.str();
        
        // Create signature using ECDSA
        EVP_PKEY* pkey = nullptr;
        EVP_PKEY_CTX* ctx = nullptr;
        
        // Convert private key from hex string to EVP_PKEY
        std::vector<unsigned char> privateKeyBytes;
        if (!hexStringToBytes(privateKey, privateKeyBytes)) {
            throw std::runtime_error("Invalid private key format");
        }
        
        // Create EC key from private key bytes
        EC_KEY* ecKey = EC_KEY_new_by_curve_name(NID_secp256k1);
        if (!ecKey) {
            throw std::runtime_error("Failed to create EC key");
        }
        
        BIGNUM* privBN = BN_bin2bn(privateKeyBytes.data(), privateKeyBytes.size(), nullptr);
        if (!privBN) {
            EC_KEY_free(ecKey);
            throw std::runtime_error("Failed to create BIGNUM from private key");
        }
        
        if (EC_KEY_set_private_key(ecKey, privBN) != 1) {
            BN_free(privBN);
            EC_KEY_free(ecKey);
            throw std::runtime_error("Failed to set private key");
        }
        
        // Generate public key from private key
        if (EC_KEY_generate_key(ecKey) != 1) {
            BN_free(privBN);
            EC_KEY_free(ecKey);
            throw std::runtime_error("Failed to generate public key");
        }
        
        pkey = EVP_PKEY_new();
        if (!pkey) {
            BN_free(privBN);
            EC_KEY_free(ecKey);
            throw std::runtime_error("Failed to create EVP_PKEY");
        }
        
        if (EVP_PKEY_assign_EC_KEY(pkey, ecKey) != 1) {
            EVP_PKEY_free(pkey);
            BN_free(privBN);
            throw std::runtime_error("Failed to assign EC key to EVP_PKEY");
        }
        
        // Create signature context
        ctx = EVP_PKEY_CTX_new(pkey, nullptr);
        if (!ctx) {
            EVP_PKEY_free(pkey);
            BN_free(privBN);
            throw std::runtime_error("Failed to create signature context");
        }
        
        // Initialize signing
        if (EVP_PKEY_sign_init(ctx) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            BN_free(privBN);
            throw std::runtime_error("Failed to initialize signing");
        }
        
        // Set digest algorithm
        if (EVP_PKEY_CTX_set_signature_md(ctx, EVP_sha256()) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            BN_free(privBN);
            throw std::runtime_error("Failed to set signature digest");
        }
        
        // Calculate signature length
        size_t sigLen;
        if (EVP_PKEY_sign(ctx, nullptr, &sigLen, 
                         reinterpret_cast<const unsigned char*>(dataToSign.c_str()), 
                         dataToSign.length()) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            BN_free(privBN);
            throw std::runtime_error("Failed to calculate signature length");
        }
        
        // Create signature
        std::vector<unsigned char> signature(sigLen);
        if (EVP_PKEY_sign(ctx, signature.data(), &sigLen,
                         reinterpret_cast<const unsigned char*>(dataToSign.c_str()),
                         dataToSign.length()) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            BN_free(privBN);
            throw std::runtime_error("Failed to create signature");
        }
        
        // Convert signature to hex string
        transaction.signature = bytesToHexString(signature);
        
        // Cleanup
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        BN_free(privBN);
        
        return true;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to sign transaction: " + std::string(e.what()));
    }
}

bool TransactionManager::verifyTransaction(const Transaction& transaction) {
    if (!initialized_) {
        throw std::runtime_error("TransactionManager not initialized");
    }

    if (!validateTransaction(transaction)) {
        return false;
    }

    try {
        // Create signature data (same as in signing)
        std::stringstream ss;
        ss << transaction.from << transaction.to << transaction.amount 
           << transaction.data << transaction.timestamp.time_since_epoch().count();
        
        std::string dataToVerify = ss.str();
        
        // Convert signature from hex string to bytes
        std::vector<unsigned char> signatureBytes;
        if (!hexStringToBytes(transaction.signature, signatureBytes)) {
            return false;
        }
        
        // Derive public key from transaction data
        // In a real implementation, you would get the public key from the transaction
        // For now, we'll use a placeholder approach
        std::string publicKey = derivePublicKeyFromTransaction(transaction);
        
        // Create EVP_PKEY from public key
        EVP_PKEY* pkey = nullptr;
        EVP_PKEY_CTX* ctx = nullptr;
        
        // Convert public key from hex string to EVP_PKEY
        std::vector<unsigned char> publicKeyBytes;
        if (!hexStringToBytes(publicKey, publicKeyBytes)) {
            return false;
        }
        
        // Create EC key from public key bytes
        EC_KEY* ecKey = EC_KEY_new_by_curve_name(NID_secp256k1);
        if (!ecKey) {
            return false;
        }
        
        EC_POINT* pubPoint = EC_POINT_new(EC_KEY_get0_group(ecKey));
        if (!pubPoint) {
            EC_KEY_free(ecKey);
            return false;
        }
        
        if (EC_POINT_oct2point(EC_KEY_get0_group(ecKey), pubPoint,
                              publicKeyBytes.data(), publicKeyBytes.size(), nullptr) != 1) {
            EC_POINT_free(pubPoint);
            EC_KEY_free(ecKey);
            return false;
        }
        
        if (EC_KEY_set_public_key(ecKey, pubPoint) != 1) {
            EC_POINT_free(pubPoint);
            EC_KEY_free(ecKey);
            return false;
        }
        
        pkey = EVP_PKEY_new();
        if (!pkey) {
            EC_POINT_free(pubPoint);
            EC_KEY_free(ecKey);
            return false;
        }
        
        if (EVP_PKEY_assign_EC_KEY(pkey, ecKey) != 1) {
            EVP_PKEY_free(pkey);
            EC_POINT_free(pubPoint);
            EC_KEY_free(ecKey);
            return false;
        }
        
        // Create verification context
        ctx = EVP_PKEY_CTX_new(pkey, nullptr);
        if (!ctx) {
            EVP_PKEY_free(pkey);
            EC_POINT_free(pubPoint);
            return false;
        }
        
        // Initialize verification
        if (EVP_PKEY_verify_init(ctx) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            EC_POINT_free(pubPoint);
            return false;
        }
        
        // Set digest algorithm
        if (EVP_PKEY_CTX_set_signature_md(ctx, EVP_sha256()) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            EC_POINT_free(pubPoint);
            return false;
        }
        
        // Verify signature
        int result = EVP_PKEY_verify(ctx, signatureBytes.data(), signatureBytes.size(),
                                    reinterpret_cast<const unsigned char*>(dataToVerify.c_str()),
                                    dataToVerify.length());
        
        // Cleanup
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        EC_POINT_free(pubPoint);
        
        return result == 1;
    } catch (const std::exception& e) {
        return false;
    }
}

bool TransactionManager::broadcastTransaction(const Transaction& transaction) {
    if (!initialized_) {
        throw std::runtime_error("TransactionManager not initialized");
    }

    if (!verifyTransaction(transaction)) {
        return false;
    }

    // Add to pending transactions
    pendingTransactions_.push_back(transaction);

    // Broadcast to network
    // This is a placeholder implementation
    // Replace with actual broadcasting logic
    return true;
}

std::string TransactionManager::getTransactionStatus(const std::string& transactionId) {
    if (!initialized_) {
        throw std::runtime_error("TransactionManager not initialized");
    }

    if (transactionId.empty()) {
        throw std::runtime_error("Transaction ID is required");
    }

    // Check pending transactions
    for (const auto& tx : pendingTransactions_) {
        if (tx.id == transactionId) {
            return tx.status;
        }
    }

    // Check transaction history
    for (const auto& tx : transactionHistory_) {
        if (tx.id == transactionId) {
            return tx.status;
        }
    }

    return "not_found";
}

std::vector<Transaction> TransactionManager::getPendingTransactions() {
    if (!initialized_) {
        throw std::runtime_error("TransactionManager not initialized");
    }

    return pendingTransactions_;
}

std::vector<Transaction> TransactionManager::getTransactionHistory(const std::string& address) {
    if (!initialized_) {
        throw std::runtime_error("TransactionManager not initialized");
    }

    if (address.empty()) {
        throw std::runtime_error("Address is required");
    }

    std::vector<Transaction> history;
    for (const auto& tx : transactionHistory_) {
        if (tx.from == address || tx.to == address) {
            history.push_back(tx);
        }
    }

    return history;
}

void TransactionManager::cleanup() {
    if (!initialized_) {
        return;
    }

    pendingTransactions_.clear();
    transactionHistory_.clear();
    initialized_ = false;
}

std::string TransactionManager::generateTransactionId(const Transaction& transaction) {
    std::stringstream ss;
    ss << transaction.from << transaction.to << transaction.amount 
       << transaction.data << transaction.timestamp.time_since_epoch().count();

    // Generate SHA-256 hash
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, ss.str().c_str(), ss.str().size());
    SHA256_Final(hash, &sha256);

    // Convert to hex string
    std::stringstream hexStream;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        hexStream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }

    return hexStream.str();
}

bool TransactionManager::validateTransaction(const Transaction& transaction) {
    if (transaction.from.empty() || transaction.to.empty() || 
        transaction.amount.empty() || transaction.id.empty()) {
        return false;
    }

    // Add more validation as needed
    return true;
}

// Helper methods
bool TransactionManager::hexStringToBytes(const std::string& hex, std::vector<unsigned char>& bytes) {
    if (hex.length() % 2 != 0) {
        return false;
    }
    
    bytes.clear();
    bytes.reserve(hex.length() / 2);
    
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        try {
            unsigned char byte = static_cast<unsigned char>(std::stoi(byteString, nullptr, 16));
            bytes.push_back(byte);
        } catch (const std::exception&) {
            return false;
        }
    }
    
    return true;
}

std::string TransactionManager::bytesToHexString(const std::vector<unsigned char>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char byte : bytes) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

std::string TransactionManager::derivePublicKeyFromTransaction(const Transaction& transaction) {
    // In a real implementation, this would derive the public key from the transaction data
    // For now, we'll return a placeholder public key
    // This should be replaced with actual public key derivation logic
    
    // Create a deterministic public key from transaction data
    std::stringstream ss;
    ss << transaction.from << transaction.to << transaction.amount;
    std::string data = ss.str();
    
    // Hash the data to create a deterministic public key
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data.c_str(), data.length());
    SHA256_Final(hash, &sha256);
    
    // Convert to hex string (this is a simplified approach)
    std::stringstream hexStream;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        hexStream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    
    return hexStream.str();
}

} // namespace core
} // namespace satox 