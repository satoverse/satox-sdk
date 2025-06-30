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

#include <openssl/evp.h>
#include "satox/wallet/key_manager.hpp"
#include "satox/wallet/address_manager.hpp"
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/obj_mac.h>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <openssl/core_names.h>

namespace satox {
namespace wallet {

KeyManager::KeyManager() : initialized_(false), key_(nullptr), group_(nullptr), order_(nullptr) {}

KeyManager::~KeyManager() {
    cleanup();
}

bool KeyManager::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) {
        return true;
    }

    if (!initializeOpenSSL()) {
        return false;
    }

    initialized_ = true;
    return true;
}

void KeyManager::cleanup() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        return;
    }

    cleanupOpenSSL();
    initialized_ = false;
}

bool KeyManager::generateKeyPair(KeyPair& keyPair) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = "Key manager not initialized";
        return false;
    }

    // Generate random private key
    std::vector<uint8_t> privateKey(32);
    if (!generateRandomBytes(privateKey, 32)) {
        return false;
    }

    // Convert private key to public key
    if (!privateKeyToPublicKey(privateKey, keyPair.publicKey)) {
        return false;
    }

    keyPair.privateKey = privateKey;
    keyPair.address = deriveAddress(keyPair.publicKey);

    return true;
}

bool KeyManager::deriveKeyPairFromSeed(const std::vector<uint8_t>& seed, KeyPair& keyPair) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = "Key manager not initialized";
        return false;
    }

    // Use the seed as the private key (in production, use proper key derivation)
    keyPair.privateKey = seed;
    if (!privateKeyToPublicKey(seed, keyPair.publicKey)) {
        return false;
    }

    keyPair.address = deriveAddress(keyPair.publicKey);
    return true;
}

bool KeyManager::deriveKeyPairFromPrivateKey(const std::vector<uint8_t>& privateKey, KeyPair& keyPair) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = "Key manager not initialized";
        return false;
    }

    if (!validatePrivateKey(privateKey)) {
        return false;
    }

    keyPair.privateKey = privateKey;
    if (!privateKeyToPublicKey(privateKey, keyPair.publicKey)) {
        return false;
    }

    keyPair.address = deriveAddress(keyPair.publicKey);
    return true;
}

bool KeyManager::deriveChildKeyPair(const KeyPair& parent, uint32_t index, KeyPair& child) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = "Key manager not initialized";
        return false;
    }

    // BIP-32 child key derivation
    std::vector<uint8_t> data;
    if (index >= 0x80000000) {
        // Hardened derivation
        data.push_back(0x00); // padding
        data.insert(data.end(), parent.privateKey.begin(), parent.privateKey.end());
    } else {
        // Normal derivation
        data.insert(data.end(), parent.publicKey.begin(), parent.publicKey.end());
    }

    // Append index
    data.push_back(static_cast<uint8_t>(index >> 24));
    data.push_back(static_cast<uint8_t>(index >> 16));
    data.push_back(static_cast<uint8_t>(index >> 8));
    data.push_back(static_cast<uint8_t>(index));

    // Compute HMAC-SHA512
    std::vector<uint8_t> hmac;
    if (!computeHMACSHA512(data, parent.chainCode, hmac)) {
        lastError_ = "Failed to compute HMAC-SHA512";
        return false;
    }

    // Split HMAC into IL and IR
    std::vector<uint8_t> IL(hmac.begin(), hmac.begin() + 32);
    std::vector<uint8_t> IR(hmac.begin() + 32, hmac.end());

    // Convert IL to big number
    BIGNUM* bnIL = BN_bin2bn(IL.data(), IL.size(), nullptr);
    if (!bnIL) {
        lastError_ = "Failed to convert IL to big number";
        return false;
    }

    // Convert parent private key to big number
    BIGNUM* bnParent = BN_bin2bn(parent.privateKey.data(), parent.privateKey.size(), nullptr);
    if (!bnParent) {
        BN_free(bnIL);
        lastError_ = "Failed to convert parent private key to big number";
        return false;
    }

    // Add IL to parent private key
    BIGNUM* bnChild = BN_new();
    if (!BN_add(bnChild, bnIL, bnParent)) {
        BN_free(bnIL);
        BN_free(bnParent);
        lastError_ = "Failed to add IL to parent private key";
        return false;
    }

    // Take modulo n
    if (!BN_mod(bnChild, bnChild, order_, nullptr)) {
        BN_free(bnIL);
        BN_free(bnParent);
        BN_free(bnChild);
        lastError_ = "Failed to take modulo n";
        return false;
    }

    // Convert child private key to bytes
    child.privateKey.resize(32);
    if (BN_bn2bin(bnChild, child.privateKey.data()) != 32) {
        BN_free(bnIL);
        BN_free(bnParent);
        BN_free(bnChild);
        lastError_ = "Failed to convert child private key to bytes";
        return false;
    }

    // Set child chain code
    child.chainCode = IR;

    // Derive public key and address
    if (!privateKeyToPublicKey(child.privateKey, child.publicKey)) {
        BN_free(bnIL);
        BN_free(bnParent);
        BN_free(bnChild);
        lastError_ = "Failed to derive public key";
        return false;
    }

    child.address = deriveAddress(child.publicKey);

    BN_free(bnIL);
    BN_free(bnParent);
    BN_free(bnChild);
    return true;
}

bool KeyManager::validatePrivateKey(const std::vector<uint8_t>& privateKey) const {
    if (privateKey.size() != 32) {
        return false;
    }

    // Check if private key is in valid range
    BIGNUM* bn = BN_bin2bn(privateKey.data(), privateKey.size(), nullptr);
    if (!bn) {
        return false;
    }

    bool valid = BN_cmp(bn, order_) < 0 && !BN_is_zero(bn);
    BN_free(bn);
    return valid;
}

bool KeyManager::validatePublicKey(const std::vector<uint8_t>& publicKey) const {
    if (publicKey.size() != 33 && publicKey.size() != 65) {
        return false;
    }
    EVP_PKEY* pkey = EVP_PKEY_new_raw_public_key(EVP_PKEY_EC, nullptr, publicKey.data(), publicKey.size());
    bool valid = (pkey != nullptr);
    if (pkey) EVP_PKEY_free(pkey);
    return valid;
}

bool KeyManager::validateKeyPair(const KeyPair& keyPair) const {
    return validatePrivateKey(keyPair.privateKey) && 
           validatePublicKey(keyPair.publicKey) &&
           keyPair.address == deriveAddress(keyPair.publicKey);
}

bool KeyManager::privateKeyToPublicKey(const std::vector<uint8_t>& privateKey, std::vector<uint8_t>& publicKey) const {
    if (!validatePrivateKey(privateKey)) {
        return false;
    }
    EVP_PKEY* pkey = EVP_PKEY_new_raw_private_key(EVP_PKEY_EC, nullptr, privateKey.data(), privateKey.size());
    if (!pkey) {
        return false;
    }
    size_t pubkey_len = 0;
    // First, get the required size
    if (EVP_PKEY_get_raw_public_key(pkey, nullptr, &pubkey_len) != 1) {
        EVP_PKEY_free(pkey);
        return false;
    }
    publicKey.resize(pubkey_len);
    if (EVP_PKEY_get_raw_public_key(pkey, publicKey.data(), &pubkey_len) != 1) {
        EVP_PKEY_free(pkey);
        return false;
    }
    EVP_PKEY_free(pkey);
    return true;
}

bool KeyManager::publicKeyToAddress(const std::vector<uint8_t>& publicKey, std::string& address) const {
    if (!validatePublicKey(publicKey)) {
        return false;
    }

    // Compute SHA256 of public key
    std::vector<uint8_t> sha256;
    if (!computeSHA256(publicKey, sha256)) {
        return false;
    }

    // Compute RIPEMD160 of SHA256
    std::vector<uint8_t> ripemd160;
    if (!computeRIPEMD160(sha256, ripemd160)) {
        return false;
    }

    // Add version byte (0x00 for mainnet)
    std::vector<uint8_t> versioned(ripemd160.size() + 1);
    versioned[0] = 0x00;
    std::copy(ripemd160.begin(), ripemd160.end(), versioned.begin() + 1);

    // Compute double SHA256 for checksum
    std::vector<uint8_t> checksum;
    if (!computeDoubleSHA256(versioned, checksum)) {
        return false;
    }

    // Take first 4 bytes of checksum
    std::vector<uint8_t> final(versioned.size() + 4);
    std::copy(versioned.begin(), versioned.end(), final.begin());
    std::copy(checksum.begin(), checksum.begin() + 4, final.begin() + versioned.size());

    // Convert to base58
    address = bytesToHex(final);
    return true;
}

bool KeyManager::encryptPrivateKey(const std::vector<uint8_t>& privateKey, const std::string& password, std::vector<uint8_t>& encrypted) const {
    if (!validatePrivateKey(privateKey)) {
        lastError_ = "Invalid private key";
        return false;
    }

    // Generate salt
    std::vector<uint8_t> salt(16);
    if (!generateRandomBytes(salt, 16)) {
        lastError_ = "Failed to generate salt";
        return false;
    }

    // Derive key from password
    std::vector<uint8_t> key(32);
    if (!deriveKeyFromPassword(password, salt, key)) {
        lastError_ = "Failed to derive key from password";
        return false;
    }

    // Generate IV
    std::vector<uint8_t> iv(16);
    if (!generateRandomBytes(iv, 16)) {
        lastError_ = "Failed to generate IV";
        return false;
    }

    // Encrypt private key
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        lastError_ = "Failed to create cipher context";
        return false;
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        lastError_ = "Failed to initialize encryption";
        return false;
    }

    encrypted.resize(privateKey.size() + EVP_MAX_BLOCK_LENGTH);
    int len1 = 0, len2 = 0;

    if (EVP_EncryptUpdate(ctx, encrypted.data(), &len1, privateKey.data(), privateKey.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        lastError_ = "Failed to encrypt data";
        return false;
    }

    if (EVP_EncryptFinal_ex(ctx, encrypted.data() + len1, &len2) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        lastError_ = "Failed to finalize encryption";
        return false;
    }

    EVP_CIPHER_CTX_free(ctx);

    // Combine salt, IV, and encrypted data
    std::vector<uint8_t> combined;
    combined.insert(combined.end(), salt.begin(), salt.end());
    combined.insert(combined.end(), iv.begin(), iv.end());
    combined.insert(combined.end(), encrypted.begin(), encrypted.begin() + len1 + len2);

    encrypted = combined;
    return true;
}

bool KeyManager::decryptPrivateKey(const std::vector<uint8_t>& encrypted, const std::string& password, std::vector<uint8_t>& privateKey) const {
    if (encrypted.size() < 48) { // 16 (salt) + 16 (IV) + minimum encrypted data
        lastError_ = "Invalid encrypted data size";
        return false;
    }

    // Extract salt and IV
    std::vector<uint8_t> salt(encrypted.begin(), encrypted.begin() + 16);
    std::vector<uint8_t> iv(encrypted.begin() + 16, encrypted.begin() + 32);
    std::vector<uint8_t> encryptedData(encrypted.begin() + 32, encrypted.end());

    // Derive key from password
    std::vector<uint8_t> key(32);
    if (!deriveKeyFromPassword(password, salt, key)) {
        lastError_ = "Failed to derive key from password";
        return false;
    }

    // Decrypt private key
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        lastError_ = "Failed to create cipher context";
        return false;
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        lastError_ = "Failed to initialize decryption";
        return false;
    }

    privateKey.resize(encryptedData.size());
    int len1 = 0, len2 = 0;

    if (EVP_DecryptUpdate(ctx, privateKey.data(), &len1, encryptedData.data(), encryptedData.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        lastError_ = "Failed to decrypt data";
        return false;
    }

    if (EVP_DecryptFinal_ex(ctx, privateKey.data() + len1, &len2) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        lastError_ = "Failed to finalize decryption";
        return false;
    }

    EVP_CIPHER_CTX_free(ctx);
    privateKey.resize(len1 + len2);

    return validatePrivateKey(privateKey);
}

bool KeyManager::deriveKeyFromPassword(const std::string& password, const std::vector<uint8_t>& salt, std::vector<uint8_t>& key) const {
    return PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                            salt.data(), salt.size(),
                            100000, // Number of iterations
                            EVP_sha256(),
                            key.size(), key.data()) == 1;
}

bool KeyManager::computeHMACSHA512(const std::vector<uint8_t>& data, const std::vector<uint8_t>& key, std::vector<uint8_t>& hmac) const {
    hmac.resize(64);
    size_t len = 0;
    
    EVP_MAC* mac = EVP_MAC_fetch(nullptr, "HMAC", nullptr);
    if (!mac) return false;
    
    EVP_MAC_CTX* ctx = EVP_MAC_CTX_new(mac);
    if (!ctx) {
        EVP_MAC_free(mac);
        return false;
    }
    
    OSSL_PARAM params[] = {
        OSSL_PARAM_utf8_string(OSSL_MAC_PARAM_DIGEST, const_cast<char*>("SHA512"), 0),
        OSSL_PARAM_END
    };
    
    bool ok = EVP_MAC_init(ctx, key.data(), key.size(), params) == 1 &&
              EVP_MAC_update(ctx, data.data(), data.size()) == 1 &&
              EVP_MAC_final(ctx, hmac.data(), &len, hmac.size()) == 1;
    
    EVP_MAC_CTX_free(ctx);
    EVP_MAC_free(mac);
    return ok && len == 64;
}

std::string KeyManager::getLastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastError_;
}

void KeyManager::clearLastError() {
    std::lock_guard<std::mutex> lock(mutex_);
    lastError_.clear();
}

// Private helper methods

bool KeyManager::initializeOpenSSL() {
    if (!RAND_status()) {
        lastError_ = "Failed to initialize random number generator";
        return false;
    }
    if (order_) {
        BN_free(order_);
        order_ = nullptr;
    }
    static const char* secp256k1_order_hex = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141";
    order_ = BN_new();
    if (!BN_hex2bn(&order_, secp256k1_order_hex)) {
        lastError_ = "Failed to set secp256k1 order";
        BN_free(order_);
        order_ = nullptr;
        return false;
    }
    return true;
}

void KeyManager::cleanupOpenSSL() {
    if (order_) {
        BN_free(order_);
        order_ = nullptr;
    }
}

bool KeyManager::generateRandomBytes(std::vector<uint8_t>& bytes, size_t length) const {
    if (RAND_bytes(bytes.data(), length) != 1) {
        return false;
    }
    return true;
}

bool KeyManager::computeSHA256(const std::vector<uint8_t>& data, std::vector<uint8_t>& hash) const {
    hash.resize(32);
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) return false;
    
    bool ok = EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) == 1 &&
              EVP_DigestUpdate(ctx, data.data(), data.size()) == 1;
    unsigned int len = 0;
    ok = ok && EVP_DigestFinal_ex(ctx, hash.data(), &len) == 1;
    
    EVP_MD_CTX_free(ctx);
    if (!ok || len != 32) return false;
    return true;
}

bool KeyManager::computeRIPEMD160(const std::vector<uint8_t>& data, std::vector<uint8_t>& hash) const {
    hash.resize(20);
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) return false;
    
    bool ok = EVP_DigestInit_ex(ctx, EVP_ripemd160(), nullptr) == 1 &&
              EVP_DigestUpdate(ctx, data.data(), data.size()) == 1;
    unsigned int len = 0;
    ok = ok && EVP_DigestFinal_ex(ctx, hash.data(), &len) == 1;
    
    EVP_MD_CTX_free(ctx);
    if (!ok || len != 20) return false;
    return true;
}

bool KeyManager::computeDoubleSHA256(const std::vector<uint8_t>& data, std::vector<uint8_t>& hash) const {
    std::vector<uint8_t> firstHash;
    if (!computeSHA256(data, firstHash)) {
        return false;
    }
    return computeSHA256(firstHash, hash);
}

std::string KeyManager::bytesToHex(const std::vector<uint8_t>& bytes) const {
    std::stringstream ss;
    for (const auto& byte : bytes) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return ss.str();
}

bool KeyManager::hexToBytes(const std::string& hex, std::vector<uint8_t>& bytes) const {
    if (hex.length() % 2 != 0) {
        return false;
    }

    bytes.resize(hex.length() / 2);
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        try {
            bytes[i / 2] = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
        } catch (const std::exception&) {
            return false;
        }
    }
    return true;
}

std::string KeyManager::deriveAddress(const std::vector<uint8_t>& publicKey) const {
    return AddressManager::getInstance().generateAddress(publicKey);
}

} // namespace wallet
} // namespace satox 