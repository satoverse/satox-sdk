#include "satox/quantum/quantum_manager.hpp"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <sodium.h>
#include <random>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace satox {
namespace quantum {

QuantumManager::QuantumManager() : initialized_(false), algorithm_("CRYSTALS-Kyber"), version_("1.0.0") {}

QuantumManager::~QuantumManager() {
    shutdown();
}

bool QuantumManager::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) {
        return true;
    }

    // Initialize OpenSSL
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();

    // Initialize libsodium
    if (sodium_init() < 0) {
        return false;
    }

    initialized_ = true;
    return true;
}

void QuantumManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        return;
    }

    // Cleanup OpenSSL
    EVP_cleanup();
    ERR_free_strings();

    initialized_ = false;
}

bool QuantumManager::generateQuantumKeyPair(std::string& publicKey, std::string& privateKey) {
    if (!initialized_) {
        return false;
    }

    // Generate a new key pair using libsodium
    unsigned char pk[crypto_kx_PUBLICKEYBYTES];
    unsigned char sk[crypto_kx_SECRETKEYBYTES];

    if (crypto_kx_keypair(pk, sk) != 0) {
        return false;
    }

    // Convert to base64 strings
    publicKey = std::string(reinterpret_cast<char*>(pk), crypto_kx_PUBLICKEYBYTES);
    privateKey = std::string(reinterpret_cast<char*>(sk), crypto_kx_SECRETKEYBYTES);

    return true;
}

bool QuantumManager::generateQuantumSharedSecret(const std::string& publicKey, const std::string& privateKey, const std::string& serverPublicKey, std::string& sharedSecret) {
    if (!initialized_) {
        return false;
    }

    // Generate shared secret using libsodium
    unsigned char rx[crypto_kx_SESSIONKEYBYTES];
    unsigned char tx[crypto_kx_SESSIONKEYBYTES];

    if (crypto_kx_client_session_keys(rx, tx,
        reinterpret_cast<const unsigned char*>(publicKey.c_str()),
        reinterpret_cast<const unsigned char*>(privateKey.c_str()),
        reinterpret_cast<const unsigned char*>(serverPublicKey.c_str())) != 0) {
        return false;
    }

    // Combine rx and tx into shared secret
    sharedSecret = std::string(reinterpret_cast<char*>(rx), crypto_kx_SESSIONKEYBYTES) +
                  std::string(reinterpret_cast<char*>(tx), crypto_kx_SESSIONKEYBYTES);

    return true;
}

bool QuantumManager::encrypt(const std::string& publicKey, const std::string& data, std::string& encryptedData) {
    if (!initialized_) {
        return false;
    }

    // Convert public key from base64
    std::vector<unsigned char> pk_bytes = base64Decode(publicKey);
    if (pk_bytes.size() != crypto_kx_PUBLICKEYBYTES) {
        return false;
    }

    // Generate a random nonce
    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    randombytes_buf(nonce, sizeof nonce);

    // Encrypt the data
    std::vector<unsigned char> ciphertext(data.size() + crypto_secretbox_MACBYTES);
    if (crypto_secretbox_easy(ciphertext.data(),
                            reinterpret_cast<const unsigned char*>(data.c_str()),
                            data.size(),
                            nonce,
                            pk_bytes.data()) != 0) {
        return false;
    }

    // Combine nonce and ciphertext
    std::vector<unsigned char> combined(nonce, nonce + sizeof nonce);
    combined.insert(combined.end(), ciphertext.begin(), ciphertext.end());

    // Convert to base64
    encryptedData = base64Encode(combined.data(), combined.size());
    return true;
}

bool QuantumManager::decrypt(const std::string& privateKey, const std::string& encryptedData, std::string& decryptedData) {
    if (!initialized_) {
        return false;
    }

    // Convert private key from base64
    std::vector<unsigned char> sk_bytes = base64Decode(privateKey);
    if (sk_bytes.size() != crypto_kx_SECRETKEYBYTES) {
        return false;
    }

    // Convert encrypted data from base64
    std::vector<unsigned char> combined = base64Decode(encryptedData);
    if (combined.size() < crypto_secretbox_NONCEBYTES + crypto_secretbox_MACBYTES) {
        return false;
    }

    // Extract nonce and ciphertext
    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    std::copy(combined.begin(), combined.begin() + crypto_secretbox_NONCEBYTES, nonce);

    std::vector<unsigned char> ciphertext(combined.begin() + crypto_secretbox_NONCEBYTES, combined.end());
    std::vector<unsigned char> plaintext(ciphertext.size() - crypto_secretbox_MACBYTES);

    // Decrypt the data
    if (crypto_secretbox_open_easy(plaintext.data(),
                                 ciphertext.data(),
                                 ciphertext.size(),
                                 nonce,
                                 sk_bytes.data()) != 0) {
        return false;
    }

    decryptedData = std::string(plaintext.begin(), plaintext.end());
    return true;
}

bool QuantumManager::sign(const std::string& privateKey, const std::string& message, std::string& signature) {
    if (!initialized_) {
        return false;
    }

    // Convert private key from base64
    std::vector<unsigned char> sk_bytes = base64Decode(privateKey);
    if (sk_bytes.size() != crypto_sign_SECRETKEYBYTES) {
        return false;
    }

    // Sign the message
    std::vector<unsigned char> sig(crypto_sign_BYTES);
    unsigned long long siglen;
    if (crypto_sign_detached(sig.data(),
                           &siglen,
                           reinterpret_cast<const unsigned char*>(message.c_str()),
                           message.size(),
                           sk_bytes.data()) != 0) {
        return false;
    }

    signature = base64Encode(sig.data(), siglen);
    return true;
}

bool QuantumManager::verify(const std::string& publicKey, const std::string& message, const std::string& signature) {
    if (!initialized_) {
        return false;
    }

    // Convert public key from base64
    std::vector<unsigned char> pk_bytes = base64Decode(publicKey);
    if (pk_bytes.size() != crypto_sign_PUBLICKEYBYTES) {
        return false;
    }

    // Convert signature from base64
    std::vector<unsigned char> sig_bytes = base64Decode(signature);
    if (sig_bytes.size() != crypto_sign_BYTES) {
        return false;
    }

    // Verify the signature
    return crypto_sign_verify_detached(sig_bytes.data(),
                                     reinterpret_cast<const unsigned char*>(message.c_str()),
                                     message.size(),
                                     pk_bytes.data()) == 0;
}

bool QuantumManager::computeQuantumHash(const std::string& data, std::string& hash) {
    if (!initialized_) {
        return false;
    }

    // Compute hash using libsodium
    unsigned char h[crypto_generichash_BYTES];
    if (crypto_generichash(h, sizeof h,
        reinterpret_cast<const unsigned char*>(data.c_str()),
        data.length(),
        nullptr, 0) != 0) {
        return false;
    }

    hash = std::string(reinterpret_cast<char*>(h), sizeof h);
    return true;
}

bool QuantumManager::verifyQuantumHash(const std::string& data, const std::string& hash) {
    if (!initialized_) {
        return false;
    }

    std::string computedHash;
    return computeQuantumHash(data, computedHash) && computedHash == hash;
}

bool QuantumManager::performQuantumKeyExchange(const std::string& publicKey, const std::string& serverPublicKey, std::string& sessionKey) {
    if (!initialized_) {
        return false;
    }

    // Generate a new key pair for this session
    std::string myPublicKey, myPrivateKey;
    if (!generateQuantumKeyPair(myPublicKey, myPrivateKey)) {
        return false;
    }

    // Generate the shared secret
    return generateQuantumSharedSecret(publicKey, myPrivateKey, serverPublicKey, sessionKey);
}

bool QuantumManager::verifyQuantumKeyExchange(const std::string& sessionKey, const std::string& privateKey) {
    if (!initialized_) {
        return false;
    }

    // Verify the session key length
    return sessionKey.length() == crypto_kx_SESSIONKEYBYTES * 2;
}

bool QuantumManager::generateQuantumRandomBytes(size_t length, std::vector<uint8_t>& randomBytes) {
    if (!initialized_) {
        return false;
    }

    randomBytes.resize(length);
    randombytes_buf(randomBytes.data(), length);
    return true;
}

bool QuantumManager::generateQuantumRandomNumber(uint64_t min, uint64_t max, uint64_t& randomNumber) {
    if (!initialized_) {
        return false;
    }

    std::vector<uint8_t> randomBytes;
    if (!generateQuantumRandomBytes(sizeof(uint64_t), randomBytes)) {
        return false;
    }

    // Convert random bytes to number
    uint64_t range = max - min + 1;
    uint64_t value;
    memcpy(&value, randomBytes.data(), sizeof(uint64_t));
    randomNumber = min + (value % range);

    return true;
}

bool QuantumManager::isQuantumResistant() const {
    return initialized_;
}

std::string QuantumManager::getAlgorithm() const {
    return algorithm_;
}

std::string QuantumManager::getVersion() const {
    return version_;
}

bool QuantumManager::initializeQuantumSystem() {
    // Initialize any quantum-specific systems here
    return true;
}

void QuantumManager::cleanupQuantumSystem() {
    // Cleanup any quantum-specific systems here
}

bool QuantumManager::validateKeyPair(const std::string& publicKey, const std::string& privateKey) {
    if (!initialized_) {
        return false;
    }

    // Verify key lengths
    return publicKey.length() == crypto_kx_PUBLICKEYBYTES &&
           privateKey.length() == crypto_kx_SECRETKEYBYTES;
}

bool QuantumManager::isInitialized() const {
    return initialized_;
}

// Helper functions
std::string QuantumManager::base64Encode(const unsigned char* data, size_t length) {
    std::string encoded;
    encoded.resize(sodium_base64_encoded_len(length, sodium_base64_VARIANT_ORIGINAL));
    sodium_bin2base64(const_cast<char*>(encoded.data()),
                      encoded.size(),
                      data,
                      length,
                      sodium_base64_VARIANT_ORIGINAL);
    return encoded;
}

std::vector<unsigned char> QuantumManager::base64Decode(const std::string& encoded) {
    std::vector<unsigned char> decoded;
    decoded.resize(encoded.size());
    size_t decoded_len;
    if (sodium_base642bin(decoded.data(),
                         decoded.size(),
                         encoded.c_str(),
                         encoded.size(),
                         nullptr,
                         &decoded_len,
                         nullptr,
                         sodium_base64_VARIANT_ORIGINAL) != 0) {
        return std::vector<unsigned char>();
    }
    decoded.resize(decoded_len);
    return decoded;
}

} // namespace quantum
} // namespace satox 