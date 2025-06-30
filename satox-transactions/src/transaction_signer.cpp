#include "satox/transactions/transaction_signer.hpp"
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <openssl/obj_mac.h>
#include <openssl/ecdsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>

namespace satox::transactions {

TransactionSigner& TransactionSigner::getInstance() {
    static TransactionSigner instance;
    return instance;
}

bool TransactionSigner::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (initialized_) {
        lastError_ = "TransactionSigner already initialized";
        return false;
    }

    // Initialize OpenSSL
    OpenSSL_add_all_algorithms();
    // ERR_load_crypto_strings(); // Removed for OpenSSL 3.x

    initialized_ = true;
    return true;
}

void TransactionSigner::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    initialized_ = false;
    lastError_.clear();
    EVP_cleanup();
    // ERR_free_strings(); // Removed for OpenSSL 3.x
}

TransactionSigner::KeyPair TransactionSigner::generateKeyPair() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_) {
        lastError_ = "TransactionSigner not initialized";
        return KeyPair{"", ""};
    }

    KeyPair keyPair;
    EC_KEY* key = EC_KEY_new_by_curve_name(NID_secp256k1);
    if (!key) {
        lastError_ = "Failed to create EC key";
        return keyPair;
    }

    if (!EC_KEY_generate_key(key)) {
        EC_KEY_free(key);
        lastError_ = "Failed to generate key pair";
        return keyPair;
    }

    // Convert private key to PEM format
    BIO* bio = BIO_new(BIO_s_mem());
    if (!bio) {
        EC_KEY_free(key);
        lastError_ = "Failed to create BIO";
        return keyPair;
    }

    if (!PEM_write_bio_ECPrivateKey(bio, key, nullptr, nullptr, 0, nullptr, nullptr)) {
        BIO_free(bio);
        EC_KEY_free(key);
        lastError_ = "Failed to write private key";
        return keyPair;
    }

    BUF_MEM* bptr;
    BIO_get_mem_ptr(bio, &bptr);
    keyPair.privateKey = std::string(bptr->data, bptr->length);
    BIO_free(bio);

    // Convert public key to PEM format
    bio = BIO_new(BIO_s_mem());
    if (!bio) {
        EC_KEY_free(key);
        lastError_ = "Failed to create BIO";
        return keyPair;
    }

    if (!PEM_write_bio_EC_PUBKEY(bio, key)) {
        BIO_free(bio);
        EC_KEY_free(key);
        lastError_ = "Failed to write public key";
        return keyPair;
    }

    BIO_get_mem_ptr(bio, &bptr);
    keyPair.publicKey = std::string(bptr->data, bptr->length);
    BIO_free(bio);
    EC_KEY_free(key);

    return keyPair;
}

TransactionSigner::SignatureResult TransactionSigner::signTransaction(
    const Transaction& transaction, const std::string& privateKey) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_) {
        lastError_ = "TransactionSigner not initialized";
        return SignatureResult{false, "", "Not initialized"};
    }

    // Hash the transaction
    std::string hash = hashTransaction(transaction);
    if (hash.empty()) {
        return SignatureResult{false, "", lastError_};
    }

    // Sign the hash
    std::string signature = signHash(hash, privateKey);
    if (signature.empty()) {
        return SignatureResult{false, "", lastError_};
    }

    return SignatureResult{true, signature, ""};
}

bool TransactionSigner::verifySignature(
    const Transaction& transaction, const std::string& signature, const std::string& publicKey) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_) {
        lastError_ = "TransactionSigner not initialized";
        return false;
    }

    // Hash the transaction
    std::string hash = hashTransaction(transaction);
    if (hash.empty()) {
        return false;
    }

    // Verify the signature
    return verifyHash(hash, signature, publicKey);
}

std::vector<TransactionSigner::SignatureResult> TransactionSigner::signTransactions(
    const std::vector<Transaction>& transactions, const std::string& privateKey) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_) {
        lastError_ = "TransactionSigner not initialized";
        return std::vector<SignatureResult>();
    }

    std::vector<SignatureResult> results;
    results.reserve(transactions.size());

    for (const auto& transaction : transactions) {
        results.push_back(signTransaction(transaction, privateKey));
    }

    return results;
}

std::string TransactionSigner::getLastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastError_;
}

void TransactionSigner::clearLastError() {
    std::lock_guard<std::mutex> lock(mutex_);
    lastError_.clear();
}

std::string TransactionSigner::hashTransaction(const Transaction& transaction) {
    std::string serialized = serializeTransaction(transaction);
    if (serialized.empty()) {
        return "";
    }

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, serialized.c_str(), serialized.size());
    SHA256_Final(hash, &sha256);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

std::string TransactionSigner::signHash(const std::string& hash, const std::string& privateKey) {
    // Load private key
    BIO* bio = BIO_new_mem_buf(privateKey.c_str(), privateKey.length());
    if (!bio) {
        lastError_ = "Failed to create BIO";
        return "";
    }

    EC_KEY* key = PEM_read_bio_ECPrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!key) {
        lastError_ = "Failed to read private key";
        return "";
    }

    // Sign the hash
    ECDSA_SIG* sig = ECDSA_do_sign(
        reinterpret_cast<const unsigned char*>(hash.c_str()),
        hash.length(),
        key);
    EC_KEY_free(key);

    if (!sig) {
        lastError_ = "Failed to sign hash";
        return "";
    }

    // Convert signature to DER format
    unsigned char* der = nullptr;
    int derLen = i2d_ECDSA_SIG(sig, &der);
    ECDSA_SIG_free(sig);

    if (derLen <= 0) {
        lastError_ = "Failed to convert signature to DER";
        return "";
    }

    std::string signature(reinterpret_cast<char*>(der), derLen);
    OPENSSL_free(der);

    return signature;
}

bool TransactionSigner::verifyHash(
    const std::string& hash, const std::string& signature, const std::string& publicKey) {
    // Load public key
    BIO* bio = BIO_new_mem_buf(publicKey.c_str(), publicKey.length());
    if (!bio) {
        lastError_ = "Failed to create BIO";
        return false;
    }

    EC_KEY* key = PEM_read_bio_EC_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!key) {
        lastError_ = "Failed to read public key";
        return false;
    }

    // Convert signature from DER format
    const unsigned char* der = reinterpret_cast<const unsigned char*>(signature.c_str());
    ECDSA_SIG* sig = d2i_ECDSA_SIG(nullptr, &der, signature.length());
    if (!sig) {
        EC_KEY_free(key);
        lastError_ = "Failed to convert signature from DER";
        return false;
    }

    // Verify the signature
    int result = ECDSA_do_verify(
        reinterpret_cast<const unsigned char*>(hash.c_str()),
        hash.length(),
        sig,
        key);

    ECDSA_SIG_free(sig);
    EC_KEY_free(key);

    return result == 1;
}

std::string TransactionSigner::serializeTransaction(const Transaction& transaction) {
    std::stringstream ss;
    ss << transaction.from
       << transaction.to
       << transaction.amount
       << transaction.assetId
       << transaction.timestamp.time_since_epoch().count()
       << transaction.type;
    return ss.str();
}

} // namespace satox::transactions 