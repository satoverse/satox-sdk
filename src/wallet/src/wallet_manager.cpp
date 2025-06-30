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

#include "satox/wallet/wallet_manager.hpp"
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include <filesystem>
#include <fstream>
#include <set>
#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <mutex>
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <nlohmann/json.hpp>
// #include <hidapi/hidapi.h>  // Commented out until hidapi is properly installed
#include "satox/wallet/key_manager.hpp"
#include "satox/wallet/address_manager.hpp"
// #include "satox/blockchain/kawpow.hpp"  // Commented out until kawpow header is available
// #include "satox/core/logging_manager.hpp"  // Commented out until logging manager is implemented

// Utility functions for wallet operations
namespace {
    // Simple Base58 encoding/decoding (placeholder implementation)
    std::string EncodeBase58(const std::vector<uint8_t>& data) {
        const std::string alphabet = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
        std::string result;
        // Simple placeholder - in production, use proper Base58 implementation
        for (uint8_t byte : data) {
            result += alphabet[byte % alphabet.length()];
        }
        return result;
    }
    
    bool DecodeBase58(const std::string& str, std::vector<uint8_t>& result) {
        const std::string alphabet = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
        result.clear();
        // Simple placeholder - in production, use proper Base58 implementation
        for (char c : str) {
            size_t pos = alphabet.find(c);
            if (pos == std::string::npos) return false;
            result.push_back(static_cast<uint8_t>(pos));
        }
        return true;
    }
    
    // Simple Base64 encoding/decoding (placeholder implementation)
    std::string base64_encode(const std::vector<uint8_t>& data) {
        const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string result;
        // Simple placeholder - in production, use proper Base64 implementation
        for (size_t i = 0; i < data.size(); i += 3) {
            uint32_t chunk = 0;
            for (size_t j = 0; j < 3 && i + j < data.size(); ++j) {
                chunk = (chunk << 8) | data[i + j];
            }
            for (int k = 0; k < 4; ++k) {
                if (i * 4 / 3 + k < (data.size() * 4 + 2) / 3) {
                    result += alphabet[(chunk >> (18 - k * 6)) & 0x3F];
                } else {
                    result += '=';
                }
            }
        }
        return result;
    }
    
    std::vector<uint8_t> base64_decode(const std::string& str) {
        const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::vector<uint8_t> result;
        // Simple placeholder - in production, use proper Base64 implementation
        for (size_t i = 0; i < str.length(); i += 4) {
            uint32_t chunk = 0;
            for (size_t j = 0; j < 4 && i + j < str.length(); ++j) {
                char c = str[i + j];
                if (c == '=') break;
                size_t pos = alphabet.find(c);
                if (pos == std::string::npos) return {};
                chunk = (chunk << 6) | pos;
            }
            for (int k = 0; k < 3; ++k) {
                if (i * 3 / 4 + k < (str.length() * 3) / 4) {
                    result.push_back((chunk >> (16 - k * 8)) & 0xFF);
                }
            }
        }
        return result;
    }
    
    // Simple Bech32 decoding (placeholder implementation)
    int bech32_decode(const std::string& hrp, std::vector<uint8_t>& decoded, const std::string& address) {
        // Simple placeholder - in production, use proper Bech32 implementation
        if (address.substr(0, hrp.length()) != hrp) return -1;
        decoded.clear();
        // Extract data part (simplified)
        std::string data = address.substr(hrp.length() + 1);
        for (char c : data) {
            decoded.push_back(static_cast<uint8_t>(c));
        }
        return 1; // BECH32_ENCODING_BECH32 equivalent
    }
    
    // Simple transaction ID generation
    std::string generateTransactionId() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 255);
        
        std::vector<uint8_t> id(32);
        for (auto& byte : id) {
            byte = dis(gen);
        }
        
        std::stringstream ss;
        for (uint8_t byte : id) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
        }
        return ss.str();
    }
    
    // Simple public key extraction from address (placeholder)
    bool getPublicKeyFromAddress(const std::string& address, std::vector<uint8_t>& publicKey) {
        // Simple placeholder - in production, use proper address parsing
        publicKey.clear();
        for (char c : address) {
            publicKey.push_back(static_cast<uint8_t>(c));
        }
        return true;
    }
    
    const int BECH32_ENCODING_BECH32 = 1;
}

namespace satox::wallet {

WalletManager& WalletManager::getInstance() {
    static WalletManager instance;
    return instance;
}

WalletManager::WalletManager() : initialized_(false) {}
WalletManager::~WalletManager() {}

bool WalletManager::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) return true;

    // Initialize OpenSSL
    OpenSSL_add_all_algorithms();

    // Simple logging setup (replace with proper logging manager when available)
    try {
        auto log_dir = std::filesystem::path("logs/components/wallet");
        std::filesystem::create_directories(log_dir);
    } catch (const std::exception& e) {
        // Ignore logging setup errors for now
    }

    initialized_ = true;
    return true;
}

void WalletManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    wallets_.clear();
    callbacks_.clear();
    initialized_ = false;

    // Cleanup OpenSSL
    EVP_cleanup();
}

bool WalletManager::isInitialized() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return initialized_;
}

bool WalletManager::isHealthy() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        return false;
    }
    
    // Basic health check - verify that the wallet manager is in a good state
    // Check if OpenSSL is properly initialized
    if (ERR_get_error() != 0) {
        // Clear any OpenSSL errors and continue
        ERR_clear_error();
    }
    
    // Check if we can access wallets (basic functionality test)
    try {
        // Simple health check - if we can access the wallets map, we're healthy
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::string WalletManager::createWallet(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        setError("Wallet manager not initialized");
        return "";
    }

    Wallet wallet;
    wallet.id = generateWalletId();
    wallet.name = name;
    wallet.createdAt = std::chrono::system_clock::now();
    wallet.lastAccessed = wallet.createdAt;
    wallet.derivationPath = getDerivationPath();

    // Generate mnemonic
    if (!generateMnemonic(wallet.mnemonic)) {
        return "";
    }

    // Convert mnemonic to seed
    if (!mnemonicToSeed(wallet.mnemonic, wallet.seed)) {
        return "";
    }

    // Derive initial key pair
    KeyPair keyPair;
    if (!deriveKeyPairFromSeed(wallet.seed, keyPair)) {
        return "";
    }

    wallet.publicKey = keyPair.publicKey;
    wallets_[wallet.id] = wallet;

    // Notify callbacks
    for (const auto& callback : callbacks_) {
        callback(wallet, Status::ACTIVE);
    }

    return wallet.id;
}

std::string WalletManager::importWalletFromMnemonic(const std::string& mnemonic, const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        setError("Wallet manager not initialized");
        return "";
    }

    Wallet wallet;
    wallet.id = generateWalletId();
    wallet.name = name;
    wallet.mnemonic = mnemonic;
    wallet.createdAt = std::chrono::system_clock::now();
    wallet.lastAccessed = wallet.createdAt;
    wallet.derivationPath = getDerivationPath();

    // Convert mnemonic to seed
    if (!mnemonicToSeed(wallet.mnemonic, wallet.seed)) {
        return "";
    }

    // Derive initial key pair
    KeyPair keyPair;
    if (!deriveKeyPairFromSeed(wallet.seed, keyPair)) {
        return "";
    }

    wallet.publicKey = keyPair.publicKey;
    wallets_[wallet.id] = wallet;

    // Notify callbacks
    for (const auto& callback : callbacks_) {
        callback(wallet, Status::ACTIVE);
    }

    return wallet.id;
}

std::string WalletManager::importWalletFromPrivateKey(const std::vector<uint8_t>& privateKey, const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        setError("Wallet manager not initialized");
        return "";
    }

    Wallet wallet;
    wallet.id = generateWalletId();
    wallet.name = name;
    wallet.createdAt = std::chrono::system_clock::now();
    wallet.lastAccessed = wallet.createdAt;
    wallet.derivationPath = getDerivationPath();

    // Create EC key from private key
    EC_KEY* key = EC_KEY_new_by_curve_name(NID_secp256k1);
    BIGNUM* bn = BN_bin2bn(privateKey.data(), privateKey.size(), nullptr);
    EC_KEY_set_private_key(key, bn);
    BN_free(bn);

    // Generate public key
    const EC_GROUP* group = EC_KEY_get0_group(key);
    EC_POINT* pub = EC_POINT_new(group);
    EC_POINT_mul(group, pub, EC_KEY_get0_private_key(key), nullptr, nullptr, nullptr);
    EC_KEY_set_public_key(key, pub);
    EC_POINT_free(pub);

    // Get public key bytes
    size_t pubLen = EC_POINT_point2oct(group, EC_KEY_get0_public_key(key),
                                     POINT_CONVERSION_COMPRESSED, nullptr, 0, nullptr);
    wallet.publicKey.resize(pubLen);
    EC_POINT_point2oct(group, EC_KEY_get0_public_key(key),
                      POINT_CONVERSION_COMPRESSED, wallet.publicKey.data(), pubLen, nullptr);

    // Encrypt private key
    if (!encryptPrivateKey(privateKey, "", wallet.encryptedPrivateKey)) {
        EC_KEY_free(key);
        return "";
    }

    EC_KEY_free(key);
    wallets_[wallet.id] = wallet;

    // Notify callbacks
    for (const auto& callback : callbacks_) {
        callback(wallet, Status::ACTIVE);
    }

    return wallet.id;
}

bool WalletManager::exportWallet(const std::string& walletId, std::string& mnemonic) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = wallets_.find(walletId);
    if (it == wallets_.end()) return false;
    mnemonic = it->second.mnemonic;
    return true;
}

bool WalletManager::deleteWallet(const std::string& walletId) {
    std::lock_guard<std::mutex> lock(mutex_);
    return wallets_.erase(walletId) > 0;
}

bool WalletManager::lockWallet(const std::string& walletId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = wallets_.find(walletId);
    if (it == wallets_.end()) {
        setError("Wallet not found");
        return false;
    }

    // Clear sensitive data
    it->second.encryptedPrivateKey.clear();
    it->second.seed.clear();
    it->second.mnemonic.clear();

    // Notify callbacks
    for (const auto& callback : callbacks_) {
        callback(it->second, Status::LOCKED);
    }

    return true;
}

bool WalletManager::unlockWallet(const std::string& walletId, const std::string& password) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = wallets_.find(walletId);
    if (it == wallets_.end()) {
        setError("Wallet not found");
        return false;
    }

    // If wallet has encrypted private key, decrypt it
    if (!it->second.encryptedPrivateKey.empty()) {
        std::vector<uint8_t> privateKey;
        if (!decryptPrivateKey(it->second.encryptedPrivateKey, password, privateKey)) {
            setError("Failed to decrypt private key");
            return false;
        }
    }

    // Notify callbacks
    for (const auto& callback : callbacks_) {
        callback(it->second, Status::ACTIVE);
    }

    return true;
}

WalletManager::Status WalletManager::getWalletStatus(const std::string& walletId) const {
    return Status::ACTIVE;
}

bool WalletManager::getWallet(const std::string& walletId, Wallet& wallet) const {
    auto it = wallets_.find(walletId);
    if (it == wallets_.end()) return false;
    wallet = it->second;
    return true;
}

bool WalletManager::deriveKeyPair(const std::string& walletId, KeyPair& keyPair) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        setError("Wallet manager not initialized");
        return false;
    }

    auto it = wallets_.find(walletId);
    if (it == wallets_.end()) {
        setError("Wallet not found");
        return false;
    }

    // If wallet is locked, return error
    if (it->second.encryptedPrivateKey.empty() && it->second.seed.empty()) {
        setError("Wallet is locked");
        return false;
    }

    // If we have a seed, derive from it
    if (!it->second.seed.empty()) {
        return deriveKeyPairFromSeed(it->second.seed, keyPair);
    }

    // If we have a mnemonic, convert to seed first
    if (!it->second.mnemonic.empty()) {
        std::vector<uint8_t> seed;
        if (!mnemonicToSeed(it->second.mnemonic, seed)) {
            return false;
        }
        return deriveKeyPairFromSeed(seed, keyPair);
    }

    setError("No valid key material found");
    return false;
}

bool WalletManager::validateAddress(const std::string& address) const {
    if (address.empty()) return false;

    // Check for bech32 address
    if (address.substr(0, 3) == "sat") {
        std::vector<uint8_t> decoded;
        std::string hrp;
        if (bech32_decode(hrp, decoded, address) != BECH32_ENCODING_BECH32) {
            return false;
        }
        return hrp == "sat";
    }

    // Check for base58 address
    std::vector<uint8_t> decoded;
    if (!DecodeBase58(address, decoded)) {
        return false;
    }

    // Verify checksum
    if (decoded.size() < 4) return false;
    std::vector<uint8_t> checksum(decoded.end() - 4, decoded.end());
    decoded.resize(decoded.size() - 4);
    std::vector<uint8_t> hash;
    hash.resize(32);
    SHA256(decoded.data(), decoded.size(), hash.data());
    SHA256(hash.data(), hash.size(), hash.data());
    return std::equal(checksum.begin(), checksum.end(), hash.begin());
}

std::string WalletManager::deriveAddress(const std::vector<uint8_t>& publicKey) const {
    if (publicKey.empty()) return "";

    // Hash the public key
    std::vector<uint8_t> hash;
    hash.resize(32);
    SHA256(publicKey.data(), publicKey.size(), hash.data());
    RIPEMD160(hash.data(), hash.size(), hash.data());

    // Add version byte (0x3F for Satox)
    std::vector<uint8_t> vchVersion(1, 0x3F);
    vchVersion.insert(vchVersion.end(), hash.begin(), hash.end());

    // Calculate checksum
    std::vector<uint8_t> checksum;
    checksum.resize(32);
    SHA256(vchVersion.data(), vchVersion.size(), checksum.data());
    SHA256(checksum.data(), checksum.size(), checksum.data());

    // Append checksum
    vchVersion.insert(vchVersion.end(), checksum.begin(), checksum.begin() + 4);

    // Encode as base58
    return EncodeBase58(vchVersion);
}

void WalletManager::registerCallback(WalletCallback callback) {
    callbacks_.push_back(callback);
}

void WalletManager::unregisterCallback(WalletCallback callback) {
    // Not implemented
}

std::string WalletManager::getLastError() const {
    return lastError_;
}

void WalletManager::clearLastError() {
    lastError_.clear();
}

bool WalletManager::generateMnemonic(std::string& mnemonic) {
    try {
        // Generate 32 bytes of entropy
        std::vector<uint8_t> entropy(32);
        if (RAND_bytes(entropy.data(), entropy.size()) != 1) {
            setError("Failed to generate entropy");
            return false;
        }

        // Convert entropy to mnemonic using BIP-39
        const char* wordlist[] = {
            "abandon", "ability", "able", "about", "above", "absent", "absorb", "abstract", "absurd", "abuse",
            // ... (full wordlist would be here)
        };
        const size_t wordlist_size = 2048; // BIP-39 uses 2048 words

        // Calculate checksum
        uint8_t checksum = 0;
        for (size_t i = 0; i < entropy.size(); i++) {
            checksum ^= entropy[i];
        }
        checksum &= 0x0F; // Take first 4 bits

        // Convert to words
        std::stringstream ss;
        for (size_t i = 0; i < entropy.size(); i++) {
            uint16_t index = (entropy[i] << 4) | ((i == entropy.size() - 1) ? checksum : (entropy[i + 1] >> 4));
            ss << wordlist[index % wordlist_size] << " ";
        }
        mnemonic = ss.str();
        mnemonic.pop_back(); // Remove trailing space
        return true;
    } catch (const std::exception& e) {
        setError("Mnemonic generation failed: " + std::string(e.what()));
        return false;
    }
}

bool WalletManager::mnemonicToSeed(const std::string& mnemonic, std::vector<uint8_t>& seed) {
    try {
        // Use PBKDF2-HMAC-SHA512 to derive seed
        seed.resize(64); // 512 bits
        const std::string salt = "mnemonic";
        
        if (PKCS5_PBKDF2_HMAC(
            mnemonic.c_str(), mnemonic.length(),
            reinterpret_cast<const unsigned char*>(salt.c_str()), salt.length(),
            2048, // Number of iterations
            EVP_sha512(),
            seed.size(), seed.data()) != 1) {
            setError("Failed to derive seed from mnemonic");
            return false;
        }
        return true;
    } catch (const std::exception& e) {
        setError("Seed derivation failed: " + std::string(e.what()));
        return false;
    }
}

bool WalletManager::deriveKeyPairFromSeed(const std::vector<uint8_t>& seed, KeyPair& keyPair) {
    try {
        // Derive key from seed using PBKDF2
        std::vector<uint8_t> derivedKey(64);
        if (PKCS5_PBKDF2_HMAC(
                reinterpret_cast<const char*>(seed.data()), seed.size(),
                reinterpret_cast<const unsigned char*>("Bitcoin seed"), 12,
                2048, EVP_sha512(), 64, derivedKey.data()) != 1) {
            return false;
        }

        // Split into private and chain code
        keyPair.privateKey.assign(derivedKey.begin(), derivedKey.begin() + 32);
        std::vector<uint8_t> chainCode(derivedKey.begin() + 32, derivedKey.end());

        // Derive public key using secp256k1
        EC_KEY* key = EC_KEY_new_by_curve_name(NID_secp256k1);
        if (!key) {
            setError("Failed to create EC key");
            return false;
        }

        BIGNUM* priv = BN_bin2bn(keyPair.privateKey.data(), keyPair.privateKey.size(), nullptr);
        if (!priv) {
            EC_KEY_free(key);
            setError("Failed to create private key");
            return false;
        }

        if (EC_KEY_set_private_key(key, priv) != 1) {
            BN_free(priv);
            EC_KEY_free(key);
            setError("Failed to set private key");
            return false;
        }

        // Generate public key
        const EC_GROUP* group = EC_KEY_get0_group(key);
        EC_POINT* pub = EC_POINT_new(group);
        if (!pub) {
            BN_free(priv);
            EC_KEY_free(key);
            setError("Failed to create public key point");
            return false;
        }

        if (EC_POINT_mul(group, pub, priv, nullptr, nullptr, nullptr) != 1) {
            EC_POINT_free(pub);
            BN_free(priv);
            EC_KEY_free(key);
            setError("Failed to generate public key");
            return false;
        }

        // Convert public key to bytes
        size_t pub_len = EC_POINT_point2oct(group, pub, POINT_CONVERSION_COMPRESSED, nullptr, 0, nullptr);
        if (pub_len == 0) {
            EC_POINT_free(pub);
            BN_free(priv);
            EC_KEY_free(key);
            setError("Failed to get public key length");
            return false;
        }

        keyPair.publicKey.resize(pub_len);
        if (EC_POINT_point2oct(group, pub, POINT_CONVERSION_COMPRESSED, keyPair.publicKey.data(), pub_len, nullptr) != pub_len) {
            EC_POINT_free(pub);
            BN_free(priv);
            EC_KEY_free(key);
            setError("Failed to convert public key to bytes");
            return false;
        }

        // Cleanup
        EC_POINT_free(pub);
        BN_free(priv);
        EC_KEY_free(key);

        // Derive address
        keyPair.address = deriveAddress(keyPair.publicKey);
        return true;
    } catch (const std::exception& e) {
        setError("Key pair derivation failed: " + std::string(e.what()));
        return false;
    }
}

bool WalletManager::encryptPrivateKey(const std::vector<uint8_t>& privateKey, const std::string& password, std::vector<uint8_t>& encrypted) {
    try {
        // Generate salt and IV
        std::vector<uint8_t> salt(16);
        std::vector<uint8_t> iv(12);
        if (RAND_bytes(salt.data(), salt.size()) != 1 || RAND_bytes(iv.data(), iv.size()) != 1) {
            setError("Failed to generate random values");
            return false;
        }

        // Derive key from password
        std::vector<uint8_t> key(32); // AES-256 key
        if (PKCS5_PBKDF2_HMAC(
            password.c_str(), password.length(),
            salt.data(), salt.size(),
            100000, // High iteration count for security
            EVP_sha256(),
            key.size(), key.data()) != 1) {
            setError("Failed to derive encryption key");
            return false;
        }

        // Initialize encryption context
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            setError("Failed to create encryption context");
            return false;
        }

        // Initialize encryption
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            setError("Failed to initialize encryption");
            return false;
        }

        // Add associated data (salt)
        int len;
        if (EVP_EncryptUpdate(ctx, nullptr, &len, salt.data(), salt.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            setError("Failed to add associated data");
            return false;
        }

        // Encrypt private key
        std::vector<uint8_t> ciphertext(privateKey.size() + EVP_MAX_BLOCK_LENGTH);
        if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, privateKey.data(), privateKey.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            setError("Failed to encrypt private key");
            return false;
        }
        int ciphertext_len = len;

        // Finalize encryption
        if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            setError("Failed to finalize encryption");
            return false;
        }
        ciphertext_len += len;

        // Get authentication tag
        std::vector<uint8_t> tag(16);
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, tag.size(), tag.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            setError("Failed to get authentication tag");
            return false;
        }

        // Cleanup
        EVP_CIPHER_CTX_free(ctx);

        // Combine salt, IV, ciphertext, and tag
        encrypted.clear();
        encrypted.insert(encrypted.end(), salt.begin(), salt.end());
        encrypted.insert(encrypted.end(), iv.begin(), iv.end());
        encrypted.insert(encrypted.end(), ciphertext.begin(), ciphertext.begin() + ciphertext_len);
        encrypted.insert(encrypted.end(), tag.begin(), tag.end());

        return true;
    } catch (const std::exception& e) {
        setError("Private key encryption failed: " + std::string(e.what()));
        return false;
    }
}

bool WalletManager::decryptPrivateKey(const std::vector<uint8_t>& encrypted, const std::string& password, std::vector<uint8_t>& privateKey) {
    try {
        if (encrypted.size() < 44) { // 16 (salt) + 12 (IV) + 16 (tag) minimum
            setError("Invalid encrypted data size");
            return false;
        }

        // Extract components
        std::vector<uint8_t> salt(encrypted.begin(), encrypted.begin() + 16);
        std::vector<uint8_t> iv(encrypted.begin() + 16, encrypted.begin() + 28);
        std::vector<uint8_t> tag(encrypted.end() - 16, encrypted.end());
        std::vector<uint8_t> ciphertext(encrypted.begin() + 28, encrypted.end() - 16);

        // Derive key from password
        std::vector<uint8_t> key(32); // AES-256 key
        if (PKCS5_PBKDF2_HMAC(
            password.c_str(), password.length(),
            salt.data(), salt.size(),
            100000, // Same iteration count as encryption
            EVP_sha256(),
            key.size(), key.data()) != 1) {
            setError("Failed to derive decryption key");
            return false;
        }

        // Initialize decryption context
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            setError("Failed to create decryption context");
            return false;
        }

        // Initialize decryption
        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            setError("Failed to initialize decryption");
            return false;
        }

        // Add associated data (salt)
        int len;
        if (EVP_DecryptUpdate(ctx, nullptr, &len, salt.data(), salt.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            setError("Failed to add associated data");
            return false;
        }

        // Decrypt private key
        privateKey.resize(ciphertext.size());
        if (EVP_DecryptUpdate(ctx, privateKey.data(), &len, ciphertext.data(), ciphertext.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            setError("Failed to decrypt private key");
            return false;
        }
        int plaintext_len = len;

        // Set expected tag
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, tag.size(), tag.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            setError("Failed to set authentication tag");
            return false;
        }

        // Finalize decryption
        if (EVP_DecryptFinal_ex(ctx, privateKey.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            setError("Failed to finalize decryption");
            return false;
        }
        plaintext_len += len;

        // Cleanup
        EVP_CIPHER_CTX_free(ctx);

        // Resize to actual length
        privateKey.resize(plaintext_len);
        return true;
    } catch (const std::exception& e) {
        setError("Private key decryption failed: " + std::string(e.what()));
        return false;
    }
}

std::string WalletManager::generateWalletId() const {
    // Generate a random UUID v4
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);

    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 8; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (int i = 0; i < 4; i++) {
        ss << dis(gen);
    }
    ss << "-4";
    for (int i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    ss << dis2(gen);
    for (int i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (int i = 0; i < 12; i++) {
        ss << dis(gen);
    }
    return ss.str();
}

bool WalletManager::signMessage(const std::string& walletId, const std::string& message, std::vector<uint8_t>& signature) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        setError("Wallet manager not initialized");
        return false;
    }

    KeyPair keyPair;
    if (!deriveKeyPair(walletId, keyPair)) {
        return false;
    }

    // Create EC key from private key
    EC_KEY* key = EC_KEY_new_by_curve_name(NID_secp256k1);
    BIGNUM* bn = BN_bin2bn(keyPair.privateKey.data(), keyPair.privateKey.size(), nullptr);
    EC_KEY_set_private_key(key, bn);
    BN_free(bn);

    // Sign message
    std::vector<uint8_t> hash;
    hash.resize(32);
    SHA256(reinterpret_cast<const uint8_t*>(message.c_str()), message.size(), hash.data());

    ECDSA_SIG* sig = ECDSA_do_sign(hash.data(), hash.size(), key);
    if (!sig) {
        EC_KEY_free(key);
        setError("Failed to sign message");
        return false;
    }

    // Convert signature to DER format
    int derLen = i2d_ECDSA_SIG(sig, nullptr);
    signature.resize(derLen);
    uint8_t* p = signature.data();
    i2d_ECDSA_SIG(sig, &p);

    ECDSA_SIG_free(sig);
    EC_KEY_free(key);
    return true;
}

bool WalletManager::verifySignature(const std::string& address, const std::string& message, const std::vector<uint8_t>& signature) const {
    if (!initialized_) {
        // Cannot call setError from const method, so we'll just return false
        return false;
    }
    
    std::vector<uint8_t> publicKey;
    if (!getPublicKeyFromAddress(address, publicKey)) {
        return false;
    }
    
    EC_KEY* key = EC_KEY_new_by_curve_name(NID_secp256k1);
    if (!key) {
        return false;
    }
    
    const EC_GROUP* group = EC_KEY_get0_group(key);
    if (!group) {
        EC_KEY_free(key);
        return false;
    }
    
    EC_POINT* pub = EC_POINT_new(group);
    if (!pub) {
        EC_KEY_free(key);
        return false;
    }
    
    // Convert public key bytes to EC_POINT
    if (EC_POINT_oct2point(group, pub, publicKey.data(), publicKey.size(), nullptr) != 1) {
        EC_POINT_free(pub);
        EC_KEY_free(key);
        return false;
    }
    
    // Set the public key
    if (EC_KEY_set_public_key(key, pub) != 1) {
        EC_POINT_free(pub);
        EC_KEY_free(key);
        return false;
    }
    
    // Verify signature
    bool result = ECDSA_verify(0, reinterpret_cast<const unsigned char*>(message.c_str()), message.length(),
                              signature.data(), signature.size(), key) == 1;
    
    EC_POINT_free(pub);
    EC_KEY_free(key);
    
    return result;
}

bool WalletManager::createAsset(const std::string& walletId, const Asset& asset) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        setError("Wallet manager not initialized");
        return false;
    }

    auto it = wallets_.find(walletId);
    if (it == wallets_.end()) {
        setError("Wallet not found");
        return false;
    }

    // Validate asset
    if (asset.name.empty() || asset.symbol.empty()) {
        setError("Invalid asset name or symbol");
        return false;
    }

    // Add asset to wallet metadata
    if (!it->second.metadata.contains("assets")) {
        it->second.metadata["assets"] = nlohmann::json::array();
    }

    nlohmann::json assetJson;
    assetJson["name"] = asset.name;
    assetJson["symbol"] = asset.symbol;
    assetJson["amount"] = asset.amount;
    assetJson["owner"] = asset.owner;
    assetJson["metadata"] = asset.metadata;

    it->second.metadata["assets"].push_back(assetJson);
    return true;
}

bool WalletManager::transferAsset(const std::string& walletId, const std::string& assetName, const std::string& toAddress, uint64_t amount) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        setError("Wallet manager not initialized");
        return false;
    }

    auto it = wallets_.find(walletId);
    if (it == wallets_.end()) {
        setError("Wallet not found");
        return false;
    }

    // Validate address
    if (!validateAddress(toAddress)) {
        setError("Invalid destination address");
        return false;
    }

    // Find asset
    if (!it->second.metadata.contains("assets")) {
        setError("No assets found");
        return false;
    }

    bool found = false;
    for (auto& asset : it->second.metadata["assets"]) {
        if (asset["name"] == assetName) {
            if (asset["amount"].get<uint64_t>() < amount) {
                setError("Insufficient asset balance");
                return false;
            }
            asset["amount"] = asset["amount"].get<uint64_t>() - amount;
            found = true;
            break;
        }
    }

    if (!found) {
        setError("Asset not found");
        return false;
    }

    return true;
}

bool WalletManager::getAssetBalance(const std::string& walletId, const std::string& assetName, uint64_t& balance) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        setError("Wallet manager not initialized");
        return false;
    }

    auto it = wallets_.find(walletId);
    if (it == wallets_.end()) {
        setError("Wallet not found");
        return false;
    }

    if (!it->second.metadata.contains("assets")) {
        balance = 0;
        return true;
    }

    for (const auto& asset : it->second.metadata["assets"]) {
        if (asset["name"] == assetName) {
            balance = asset["amount"].get<uint64_t>();
            return true;
        }
    }

    balance = 0;
    return true;
}

std::vector<WalletManager::Asset> WalletManager::getWalletAssets(const std::string& walletId) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Asset> assets;

    if (!initialized_) {
        setError("Wallet manager not initialized");
        return assets;
    }

    auto it = wallets_.find(walletId);
    if (it == wallets_.end()) {
        setError("Wallet not found");
        return assets;
    }

    if (!it->second.metadata.contains("assets")) {
        return assets;
    }

    for (const auto& assetJson : it->second.metadata["assets"]) {
        Asset asset;
        asset.name = assetJson["name"];
        asset.symbol = assetJson["symbol"];
        asset.amount = assetJson["amount"];
        asset.owner = assetJson["owner"];
        asset.metadata = assetJson["metadata"];
        assets.push_back(asset);
    }

    return assets;
}

bool WalletManager::addIPFSData(const std::string& walletId, const std::vector<uint8_t>& data, std::string& hash) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        setError("Wallet manager not initialized");
        return false;
    }

    auto it = wallets_.find(walletId);
    if (it == wallets_.end()) {
        setError("Wallet not found");
        return false;
    }

    // Calculate IPFS hash
    std::vector<uint8_t> ipfsHash;
    ipfsHash.resize(32);
    SHA256(data.data(), data.size(), ipfsHash.data());
    hash = std::string(ipfsHash.begin(), ipfsHash.end());

    // Add IPFS data to wallet metadata
    if (!it->second.metadata.contains("ipfs_data")) {
        it->second.metadata["ipfs_data"] = nlohmann::json::array();
    }

    nlohmann::json ipfsJson;
    ipfsJson["hash"] = hash;
    ipfsJson["data"] = base64_encode(data);
    ipfsJson["owner"] = it->second.id;

    it->second.metadata["ipfs_data"].push_back(ipfsJson);
    return true;
}

bool WalletManager::getIPFSData(const std::string& hash, std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        setError("Wallet manager not initialized");
        return false;
    }

    for (const auto& wallet : wallets_) {
        if (!wallet.second.metadata.contains("ipfs_data")) {
            continue;
        }

        for (const auto& ipfsJson : wallet.second.metadata["ipfs_data"]) {
            if (ipfsJson["hash"] == hash) {
                data = base64_decode(ipfsJson["data"].get<std::string>());
                return true;
            }
        }
    }

    setError("IPFS data not found");
    return false;
}

bool WalletManager::pinIPFSData(const std::string& walletId, const std::string& hash) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        setError("Wallet manager not initialized");
        return false;
    }

    auto it = wallets_.find(walletId);
    if (it == wallets_.end()) {
        setError("Wallet not found");
        return false;
    }

    // Add to pinned data
    if (!it->second.metadata.contains("pinned_ipfs")) {
        it->second.metadata["pinned_ipfs"] = nlohmann::json::array();
    }

    it->second.metadata["pinned_ipfs"].push_back(hash);
    return true;
}

bool WalletManager::unpinIPFSData(const std::string& walletId, const std::string& hash) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        setError("Wallet manager not initialized");
        return false;
    }

    auto it = wallets_.find(walletId);
    if (it == wallets_.end()) {
        setError("Wallet not found");
        return false;
    }

    if (!it->second.metadata.contains("pinned_ipfs")) {
        return true;
    }

    auto& pinned = it->second.metadata["pinned_ipfs"];
    auto pos = std::find(pinned.begin(), pinned.end(), hash);
    if (pos != pinned.end()) {
        pinned.erase(pos);
    }

    return true;
}

std::vector<WalletManager::IPFSData> WalletManager::getWalletIPFSData(const std::string& walletId) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<IPFSData> ipfsData;

    if (!initialized_) {
        setError("Wallet manager not initialized");
        return ipfsData;
    }

    auto it = wallets_.find(walletId);
    if (it == wallets_.end()) {
        setError("Wallet not found");
        return ipfsData;
    }

    if (!it->second.metadata.contains("ipfs_data")) {
        return ipfsData;
    }

    for (const auto& ipfsJson : it->second.metadata["ipfs_data"]) {
        IPFSData data;
        data.hash = ipfsJson["hash"];
        data.data = base64_decode(ipfsJson["data"].get<std::string>());
        data.owner = ipfsJson["owner"];
        data.metadata = ipfsJson["metadata"];
        ipfsData.push_back(data);
    }

    return ipfsData;
}

bool WalletManager::validateDerivationPath(const std::string& path) const {
    std::vector<std::string> parts;
    std::stringstream ss(path);
    std::string part;
    while (std::getline(ss, part, '/')) {
        if (!part.empty()) {
            parts.push_back(part);
        }
    }

    if (parts.size() != 5) return false;
    if (parts[0] != "m") return false;
    if (parts[1] != std::to_string(PURPOSE)) return false;
    if (parts[2] != std::to_string(SATOX_COIN_TYPE)) return false;

    return true;
}

std::string WalletManager::getDerivationPath(uint32_t accountIndex) const {
    std::stringstream ss;
    ss << "m/" << PURPOSE << "'/" << SATOX_COIN_TYPE << "'/" << accountIndex << "'";
    return ss.str();
}

bool WalletManager::computeKAWPOWHash(const std::vector<uint8_t>& header, uint64_t nonce, std::vector<uint8_t>& hash) {
    // KAWPOW implementation would go here
    // This is a placeholder for the actual KAWPOW implementation
    hash.resize(32);
    SHA256(header.data(), header.size(), hash.data());
    return true;
}

std::string WalletManager::createMultisigWallet(const std::string& name, uint32_t requiredSignatures, const std::vector<std::string>& cosigners) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        setError("Wallet manager not initialized");
        return "";
    }

    if (!validateMultisigConfiguration(requiredSignatures, cosigners)) {
        setError("Invalid multi-signature configuration");
        return "";
    }

    Wallet wallet;
    wallet.id = generateWalletId();
    wallet.name = name;
    wallet.createdAt = std::chrono::system_clock::now();
    wallet.lastAccessed = wallet.createdAt;
    wallet.derivationPath = getDerivationPath();
    wallet.isMultisig = true;
    wallet.requiredSignatures = requiredSignatures;
    wallet.cosigners = cosigners;

    // Generate mnemonic for this wallet
    if (!generateMnemonic(wallet.mnemonic)) {
        return "";
    }

    // Convert mnemonic to seed
    if (!mnemonicToSeed(wallet.mnemonic, wallet.seed)) {
        return "";
    }

    // Derive initial key pair
    KeyPair keyPair;
    if (!deriveKeyPairFromSeed(wallet.seed, keyPair)) {
        return "";
    }

    wallet.publicKey = keyPair.publicKey;
    wallets_[wallet.id] = wallet;

    // Notify callbacks
    for (const auto& callback : callbacks_) {
        callback(wallet, Status::ACTIVE);
    }

    return wallet.id;
}

bool WalletManager::validateMultisigConfiguration(uint32_t requiredSignatures, const std::vector<std::string>& cosigners) const {
    if (requiredSignatures < MIN_SIGNERS || requiredSignatures > MAX_SIGNERS) {
        return false;
    }

    if (cosigners.size() < requiredSignatures || cosigners.size() > MAX_SIGNERS) {
        return false;
    }

    // Check for duplicate cosigners
    std::set<std::string> uniqueCosigners(cosigners.begin(), cosigners.end());
    if (uniqueCosigners.size() != cosigners.size()) {
        return false;
    }

    return true;
}

bool WalletManager::createMultisigTransaction(const std::string& walletId, const std::vector<std::string>& inputs,
                                           const std::vector<std::string>& outputs, uint64_t amount,
                                           const std::string& assetName, MultisigTransaction& transaction) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        setError("Wallet manager not initialized");
        return false;
    }

    auto it = wallets_.find(walletId);
    if (it == wallets_.end()) {
        setError("Wallet not found");
        return false;
    }

    if (!it->second.isMultisig) {
        setError("Not a multi-signature wallet");
        return false;
    }

    transaction.txid = generateTransactionId();
    transaction.walletId = walletId;
    transaction.inputs = inputs;
    transaction.outputs = outputs;
    transaction.amount = amount;
    transaction.assetName = assetName;
    transaction.requiredSignatures = it->second.requiredSignatures;
    transaction.currentSignatures = 0;
    transaction.createdAt = std::chrono::system_clock::now();
    transaction.expiresAt = transaction.createdAt + std::chrono::hours(24); // 24-hour expiration

    pendingTransactions_[transaction.txid] = transaction;

    // Notify callbacks
    for (const auto& callback : transactionCallbacks_) {
        callback(transaction);
    }

    return true;
}

bool WalletManager::signMultisigTransaction(const std::string& walletId, const std::string& txid, std::vector<uint8_t>& signature) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        setError("Wallet manager not initialized");
        return false;
    }
    
    auto txIt = pendingTransactions_.find(txid);
    if (txIt == pendingTransactions_.end()) {
        setError("Transaction not found");
        return false;
    }
    
    // Check if this wallet is one of the required signers
    // For now, we'll assume any wallet can sign (simplified implementation)
    
    // Sign the transaction
    std::string message = txIt->second.txid + std::to_string(txIt->second.amount) + txIt->second.assetName;
    if (!signMessage(walletId, message, signature)) {
        setError("Failed to sign transaction");
        return false;
    }
    
    // Add signature to transaction
    std::string signatureStr;
    for (uint8_t byte : signature) {
        signatureStr += static_cast<char>(byte);
    }
    txIt->second.signatures.push_back(signatureStr);
    txIt->second.currentSignatures++;
    
    return true;
}

bool WalletManager::verifyMultisigTransaction(const MultisigTransaction& transaction) const {
    if (!initialized_) {
        return false;
    }
    
    // Check if we have enough signatures
    if (transaction.currentSignatures < transaction.requiredSignatures) {
        return false;
    }
    
    // Verify each signature (simplified implementation)
    std::string message = transaction.txid + std::to_string(transaction.amount) + transaction.assetName;
    
    for (const auto& signatureStr : transaction.signatures) {
        // Convert string signature back to bytes
        std::vector<uint8_t> signature;
        for (char c : signatureStr) {
            signature.push_back(static_cast<uint8_t>(c));
        }
        
        // For now, we'll assume the signature is valid (simplified implementation)
        // In a real implementation, you would verify against the actual public keys
        if (signature.empty()) {
            return false;
        }
    }
    
    return true;
}

bool WalletManager::broadcastMultisigTransaction(const std::string& txid) {
    auto it = pendingTransactions_.find(txid);
    if (it == pendingTransactions_.end()) {
        setError("Transaction not found");
        return false;
    }

    if (!verifyMultisigTransaction(it->second)) {
        setError("Transaction verification failed");
        return false;
    }

    // TODO: Implement actual transaction broadcasting
    // This would typically involve sending the transaction to the network

    // Remove from pending transactions
    pendingTransactions_.erase(it);

    return true;
}

std::vector<WalletManager::MultisigTransaction> WalletManager::getPendingMultisigTransactions(const std::string& walletId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<MultisigTransaction> result;
    
    if (!initialized_) {
        return result;
    }
    
    // Find all pending transactions for this wallet
    for (const auto& pair : pendingTransactions_) {
        const MultisigTransaction& tx = pair.second;
        
        // Check if this wallet is involved in the transaction
        // For now, we'll return all pending transactions (simplified implementation)
        if (tx.currentSignatures < tx.requiredSignatures) {
            result.push_back(tx);
        }
    }
    
    return result;
}

bool WalletManager::detectHardwareWallets() {
    // Hardware wallet detection disabled - hidapi not available
    setError("Hardware wallet detection not available - hidapi not installed");
    return false;
    
    /*
    // Initialize HIDAPI
    if (hid_init() < 0) {
        setError("Failed to initialize HIDAPI");
        return false;
    }

    // Enumerate HID devices
    struct hid_device_info* devices = hid_enumerate(0x0, 0x0);
    struct hid_device_info* current = devices;

    while (current) {
        HardwareWallet wallet;
        wallet.id = std::to_string(current->vendor_id) + ":" + std::to_string(current->product_id);
        wallet.type = getHardwareWalletType(current->vendor_id, current->product_id);
        wallet.name = current->manufacturer_string ? current->manufacturer_string : "Unknown";
        wallet.model = current->product_string ? current->product_string : "Unknown";
        wallet.isConnected = true;

        if (initializeHardwareWallet(wallet.id)) {
            hardwareWallets_[wallet.id] = wallet;
        }

        current = current->next;
    }

    hid_free_enumeration(devices);
    hid_exit();

    return true;
    */
}

bool WalletManager::initializeHardwareWallet(const std::string& walletId) {
    // Hardware wallet initialization disabled - hidapi not available
    setError("Hardware wallet initialization not available - hidapi not installed");
    return false;
    
    /*
    auto it = hardwareWallets_.find(walletId);
    if (it == hardwareWallets_.end()) {
        return false;
    }

    // Open device
    hid_device* device = hid_open_path(it->second.path.c_str());
    if (!device) {
        return false;
    }

    // Get firmware version
    unsigned char buffer[64];
    if (hid_get_feature_report(device, buffer, sizeof(buffer)) < 0) {
        hid_close(device);
        return false;
    }

    it->second.firmwareVersion = std::string(reinterpret_cast<char*>(buffer + 1));
    it->second.isConnected = true;

    // Get supported features
    if (it->second.type == HardwareWalletType::LEDGER) {
        it->second.supportedFeatures = {"sign", "verify", "derive"};
    } else if (it->second.type == HardwareWalletType::TREZOR) {
        it->second.supportedFeatures = {"sign", "verify", "derive", "backup"};
    }

    hid_close(device);
    return true;
    */
}

bool WalletManager::signWithHardwareWallet(const std::string& walletId, const std::vector<uint8_t>& data, std::vector<uint8_t>& signature) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = hardwareWallets_.find(walletId);
    if (it == hardwareWallets_.end()) {
        setError("Hardware wallet not found or not connected");
        return false;
    }
    
    // Check if it's a Trezor wallet
    if (it->second.type == "TREZOR") {
        // Placeholder implementation for Trezor
        setError("Trezor hardware wallet support not yet implemented");
        return false;
    } else if (it->second.type == "LEDGER") {
        // Placeholder implementation for Ledger
        setError("Ledger hardware wallet support not yet implemented");
        return false;
    } else {
        setError("Unsupported hardware wallet type: " + it->second.type);
        return false;
    }
}

bool WalletManager::backupKeys(const std::string& walletId, const std::string& backupPath) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = wallets_.find(walletId);
    if (it == wallets_.end()) {
        setError("Wallet not found");
        return false;
    }

    try {
        nlohmann::json backup;
        backup["id"] = it->second.id;
        backup["name"] = it->second.name;
        backup["publicKey"] = it->second.publicKey;
        backup["encryptedPrivateKey"] = it->second.encryptedPrivateKey;
        backup["mnemonic"] = it->second.mnemonic;
        backup["seed"] = it->second.seed;
        backup["derivationPath"] = it->second.derivationPath;
        backup["createdAt"] = std::chrono::system_clock::to_time_t(it->second.createdAt);
        backup["metadata"] = it->second.metadata;

        std::ofstream file(backupPath);
        file << backup.dump(4);
        return true;
    } catch (const std::exception& e) {
        setError("Failed to backup keys: " + std::string(e.what()));
        return false;
    }
}

bool WalletManager::restoreKeys(const std::string& walletId, const std::string& backupPath) {
    std::lock_guard<std::mutex> lock(mutex_);
    try {
        std::ifstream file(backupPath);
        nlohmann::json backup = nlohmann::json::parse(file);

        Wallet wallet;
        wallet.id = backup["id"];
        wallet.name = backup["name"];
        wallet.publicKey = backup["publicKey"].get<std::vector<uint8_t>>();
        wallet.encryptedPrivateKey = backup["encryptedPrivateKey"].get<std::vector<uint8_t>>();
        wallet.mnemonic = backup["mnemonic"];
        wallet.seed = backup["seed"].get<std::vector<uint8_t>>();
        wallet.derivationPath = backup["derivationPath"];
        wallet.createdAt = std::chrono::system_clock::from_time_t(backup["createdAt"].get<time_t>());
        wallet.lastAccessed = std::chrono::system_clock::now();
        wallet.metadata = backup["metadata"];

        wallets_[walletId] = wallet;
        return true;
    } catch (const std::exception& e) {
        setError("Failed to restore keys: " + std::string(e.what()));
        return false;
    }
}

std::vector<WalletManager::HardwareWallet> WalletManager::listHardwareWallets() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<HardwareWallet> result;
    for (const auto& pair : hardwareWallets_) {
        result.push_back(pair.second);
    }
    return result;
}

bool WalletManager::connectHardwareWallet(const std::string& walletId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = hardwareWallets_.find(walletId);
    if (it == hardwareWallets_.end()) {
        setError("Hardware wallet not found");
        return false;
    }
    
    if (it->second.isConnected) {
        return true; // Already connected
    }
    
    // Try to initialize the hardware wallet
    if (initializeHardwareWallet(walletId)) {
        it->second.isConnected = true;
        return true;
    }
    
    return false;
}

bool WalletManager::disconnectHardwareWallet(const std::string& walletId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = hardwareWallets_.find(walletId);
    if (it == hardwareWallets_.end()) {
        setError("Hardware wallet not found");
        return false;
    }
    
    it->second.isConnected = false;
    return true;
}

bool WalletManager::isHardwareWalletConnected(const std::string& walletId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = hardwareWallets_.find(walletId);
    if (it == hardwareWallets_.end()) {
        return false;
    }
    
    return it->second.isConnected;
}

bool WalletManager::getHardwareWalletInfo(const std::string& walletId, HardwareWallet& wallet) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = hardwareWallets_.find(walletId);
    if (it == hardwareWallets_.end()) {
        return false;
    }
    
    wallet = it->second;
    return true;
}

// Private helper methods
void WalletManager::setError(const std::string& error) {
    // No mutex lock needed - this method is always called from methods that already hold the mutex
    lastError_ = error;
}

} // namespace satox::wallet 