#include "satox/quantum/hybrid_encryption.hpp"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <sodium.h>
#include <stdexcept>
#include <cstring>
#include <vector>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace satox {
namespace quantum {

HybridEncryption::HybridEncryption()
    : initialized_(false)
    , algorithm_("CRYSTALS-Kyber + AES-256-GCM")
    , version_("1.0.0") {
}

HybridEncryption::~HybridEncryption() {
    shutdown();
}

bool HybridEncryption::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) {
        return true;
    }

    if (!initializeEncryptionSystem()) {
        return false;
    }

    initialized_ = true;
    return true;
}

void HybridEncryption::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        return;
    }

    cleanupEncryptionSystem();
    initialized_ = false;
}

bool HybridEncryption::encrypt(const std::string& data,
                             const std::string& quantumPublicKey,
                             const std::string& classicalPublicKey,
                             std::string& encryptedData) {
    if (!initialized_) {
        return false;
    }

    // Generate a session key
    std::string sessionKey;
    if (!generateSessionKey(sessionKey)) {
        return false;
    }

    // Encrypt the data with the session key
    std::string encryptedSessionData;
    if (!encryptWithSessionKey(data, sessionKey, encryptedSessionData)) {
        return false;
    }

    // Encrypt the session key with both quantum and classical keys
    std::string encryptedSessionKey;
    if (!encryptWithSessionKey(sessionKey, quantumPublicKey + classicalPublicKey, encryptedSessionKey)) {
        return false;
    }

    // Combine the encrypted session key and data
    encryptedData = encryptedSessionKey + ":" + encryptedSessionData;
    return true;
}

bool HybridEncryption::decrypt(const std::string& encryptedData,
                             const std::string& quantumPrivateKey,
                             const std::string& classicalPrivateKey,
                             std::string& decryptedData) {
    if (!initialized_) {
        return false;
    }

    // Split the encrypted data into session key and data
    size_t separatorPos = encryptedData.find(':');
    if (separatorPos == std::string::npos) {
        return false;
    }

    std::string encryptedSessionKey = encryptedData.substr(0, separatorPos);
    std::string encryptedSessionData = encryptedData.substr(separatorPos + 1);

    // Decrypt the session key
    std::string sessionKey;
    if (!decryptWithSessionKey(encryptedSessionKey, quantumPrivateKey + classicalPrivateKey, sessionKey)) {
        return false;
    }

    // Decrypt the data with the session key
    return decryptWithSessionKey(encryptedSessionData, sessionKey, decryptedData);
}

bool HybridEncryption::rotateKeys(const std::string& oldQuantumKey,
                                const std::string& oldClassicalKey,
                                std::string& newQuantumKey,
                                std::string& newClassicalKey) {
    if (!initialized_) {
        return false;
    }

    // Generate new keys
    if (!generateSessionKey(newQuantumKey) || !generateSessionKey(newClassicalKey)) {
        return false;
    }

    return true;
}

bool HybridEncryption::reencrypt(const std::string& encryptedData,
                               const std::string& oldQuantumKey,
                               const std::string& oldClassicalKey,
                               const std::string& newQuantumKey,
                               const std::string& newClassicalKey,
                               std::string& reencryptedData) {
    if (!initialized_) {
        return false;
    }

    // Decrypt with old keys
    std::string decryptedData;
    if (!decrypt(encryptedData, oldQuantumKey, oldClassicalKey, decryptedData)) {
        return false;
    }

    // Encrypt with new keys
    return encrypt(decryptedData, newQuantumKey, newClassicalKey, reencryptedData);
}

bool HybridEncryption::storeKeys(const std::string& quantumKey,
                               const std::string& classicalKey,
                               const std::string& identifier) {
    if (!initialized_) {
        return false;
    }

    try {
        // Generate a master key for encryption
        unsigned char masterKey[crypto_secretbox_KEYBYTES];
        randombytes_buf(masterKey, sizeof masterKey);

        // Combine keys with identifier for storage
        nlohmann::json keyData;
        keyData["quantum"] = quantumKey;
        keyData["classical"] = classicalKey;
        keyData["identifier"] = identifier;
        std::string keyDataStr = keyData.dump();

        // Encrypt the combined data
        unsigned char nonce[crypto_secretbox_NONCEBYTES];
        randombytes_buf(nonce, sizeof nonce);

        std::vector<unsigned char> ciphertext(keyDataStr.size() + crypto_secretbox_MACBYTES);
        if (crypto_secretbox_easy(ciphertext.data(),
                                reinterpret_cast<const unsigned char*>(keyDataStr.c_str()),
                                keyDataStr.size(),
                                nonce,
                                masterKey) != 0) {
            return false;
        }

        // Store encrypted data and nonce
        std::string storagePath = getStoragePath(identifier);
        std::filesystem::create_directories(std::filesystem::path(storagePath).parent_path());

        nlohmann::json storageData;
        storageData["nonce"] = base64Encode(nonce, sizeof nonce);
        storageData["data"] = base64Encode(ciphertext.data(), ciphertext.size());
        storageData["master_key"] = base64Encode(masterKey, sizeof masterKey);

        std::ofstream file(storagePath);
        file << storageData.dump(4);
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool HybridEncryption::retrieveKeys(const std::string& identifier,
                                  std::string& quantumKey,
                                  std::string& classicalKey) {
    if (!initialized_) {
        return false;
    }

    try {
        std::string storagePath = getStoragePath(identifier);
        if (!std::filesystem::exists(storagePath)) {
            return false;
        }

        // Read storage data
        std::ifstream file(storagePath);
        nlohmann::json storageData = nlohmann::json::parse(file);

        // Decode stored data
        std::vector<unsigned char> nonce = base64Decode(storageData["nonce"].get<std::string>());
        std::vector<unsigned char> ciphertext = base64Decode(storageData["data"].get<std::string>());
        std::vector<unsigned char> masterKey = base64Decode(storageData["master_key"].get<std::string>());

        // Decrypt the data
        std::vector<unsigned char> plaintext(ciphertext.size() - crypto_secretbox_MACBYTES);
        if (crypto_secretbox_open_easy(plaintext.data(),
                                     ciphertext.data(),
                                     ciphertext.size(),
                                     nonce.data(),
                                     masterKey.data()) != 0) {
            return false;
        }

        // Parse the decrypted data
        nlohmann::json keyData = nlohmann::json::parse(plaintext);
        quantumKey = keyData["quantum"].get<std::string>();
        classicalKey = keyData["classical"].get<std::string>();

        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool HybridEncryption::validateKeys(const std::string& quantumKey, const std::string& classicalKey) {
    if (!initialized_) {
        return false;
    }

    try {
        // Validate key lengths
        if (quantumKey.empty() || classicalKey.empty()) {
            return false;
        }

        // Validate key format (base64 encoded)
        std::vector<unsigned char> quantumKeyBytes = base64Decode(quantumKey);
        std::vector<unsigned char> classicalKeyBytes = base64Decode(classicalKey);

        if (quantumKeyBytes.empty() || classicalKeyBytes.empty()) {
            return false;
        }

        // Validate key sizes
        if (quantumKeyBytes.size() != crypto_secretbox_KEYBYTES || 
            classicalKeyBytes.size() != crypto_secretbox_KEYBYTES) {
            return false;
        }

        // Test encryption/decryption with a small test message
        std::string testMessage = "test";
        std::string encrypted, decrypted;
        
        if (!encryptWithSessionKey(testMessage, quantumKey, encrypted)) {
            return false;
        }
        
        if (!decryptWithSessionKey(encrypted, quantumKey, decrypted)) {
            return false;
        }

        if (testMessage != decrypted) {
            return false;
        }

        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

std::string HybridEncryption::getStoragePath(const std::string& identifier) const {
    std::string homeDir = std::getenv("HOME") ? std::getenv("HOME") : ".";
    return homeDir + "/.satox/keys/" + identifier + ".json";
}

bool HybridEncryption::isInitialized() const {
    return initialized_;
}

std::string HybridEncryption::getAlgorithm() const {
    return algorithm_;
}

std::string HybridEncryption::getVersion() const {
    return version_;
}

bool HybridEncryption::initializeEncryptionSystem() {
    if (sodium_init() < 0) {
        return false;
    }
    return true;
}

void HybridEncryption::cleanupEncryptionSystem() {
    // No cleanup needed for libsodium
}

bool HybridEncryption::generateSessionKey(std::string& sessionKey) {
    if (!initialized_) {
        return false;
    }

    unsigned char key[crypto_secretbox_KEYBYTES];
    randombytes_buf(key, sizeof key);
    sessionKey = base64Encode(key, sizeof key);
    return true;
}

bool HybridEncryption::encryptWithSessionKey(const std::string& data, const std::string& sessionKey, std::string& encryptedData) {
    if (!initialized_) {
        return false;
    }

    std::vector<unsigned char> key = base64Decode(sessionKey);
    if (key.size() != crypto_secretbox_KEYBYTES) {
        return false;
    }

    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    randombytes_buf(nonce, sizeof nonce);

    std::vector<unsigned char> ciphertext(data.size() + crypto_secretbox_MACBYTES);
    if (crypto_secretbox_easy(ciphertext.data(),
                            reinterpret_cast<const unsigned char*>(data.c_str()),
                            data.size(),
                            nonce,
                            key.data()) != 0) {
        return false;
    }

    std::vector<unsigned char> combined(nonce, nonce + sizeof nonce);
    combined.insert(combined.end(), ciphertext.begin(), ciphertext.end());
    encryptedData = base64Encode(combined.data(), combined.size());
    return true;
}

bool HybridEncryption::decryptWithSessionKey(const std::string& encryptedData, const std::string& sessionKey, std::string& decryptedData) {
    if (!initialized_) {
        return false;
    }

    std::vector<unsigned char> key = base64Decode(sessionKey);
    if (key.size() != crypto_secretbox_KEYBYTES) {
        return false;
    }

    std::vector<unsigned char> combined = base64Decode(encryptedData);
    if (combined.size() < crypto_secretbox_NONCEBYTES + crypto_secretbox_MACBYTES) {
        return false;
    }

    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    std::copy(combined.begin(), combined.begin() + crypto_secretbox_NONCEBYTES, nonce);

    std::vector<unsigned char> ciphertext(combined.begin() + crypto_secretbox_NONCEBYTES, combined.end());
    std::vector<unsigned char> plaintext(ciphertext.size() - crypto_secretbox_MACBYTES);

    if (crypto_secretbox_open_easy(plaintext.data(),
                                 ciphertext.data(),
                                 ciphertext.size(),
                                 nonce,
                                 key.data()) != 0) {
        return false;
    }

    decryptedData = std::string(plaintext.begin(), plaintext.end());
    return true;
}

std::string HybridEncryption::base64Encode(const unsigned char* data, size_t length) {
    std::string encoded;
    encoded.resize(sodium_base64_encoded_len(length, sodium_base64_VARIANT_ORIGINAL));
    sodium_bin2base64(const_cast<char*>(encoded.data()),
                      encoded.size(),
                      data,
                      length,
                      sodium_base64_VARIANT_ORIGINAL);
    return encoded;
}

std::vector<unsigned char> HybridEncryption::base64Decode(const std::string& encoded) {
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