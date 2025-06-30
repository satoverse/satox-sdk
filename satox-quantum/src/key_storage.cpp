#include "satox/quantum/key_storage.hpp"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <sodium.h>
#include <stdexcept>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace satox {
namespace quantum {

KeyStorage::KeyStorage() : initialized_(false), algorithm_("AES-256-GCM"), version_("1.0.0") {}

KeyStorage::~KeyStorage() {
    shutdown();
}

bool KeyStorage::initialize() {
    std::lock_guard<std::mutex> lock(this->mutex_);
    if (this->initialized_) {
        return true;
    }

    if (!initializeStorageSystem()) {
        return false;
    }

    this->initialized_ = true;
    return true;
}

void KeyStorage::shutdown() {
    std::lock_guard<std::mutex> lock(this->mutex_);
    if (!this->initialized_) {
        return;
    }

    cleanupStorageSystem();
    this->initialized_ = false;
}

bool KeyStorage::storeKey(const std::string& identifier,
                         const std::string& key,
                         const std::string& metadata) {
    if (!this->initialized_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(this->mutex_);

    // Validate key format
    if (!validateKeyFormat(key)) {
        return false;
    }

    // Validate metadata
    if (!validateKeyMetadata(metadata)) {
        return false;
    }

    // Encrypt the key
    std::string encryptedKey;
    if (!encryptKey(key, encryptedKey)) {
        return false;
    }

    // Create key entry
    KeyEntry entry;
    entry.key = encryptedKey;
    entry.metadata = metadata;
    entry.expiration = std::chrono::system_clock::time_point::max();
    entry.lastAccess = std::chrono::system_clock::now();
    entry.accessCount = 0;

    // Store the key
    this->keys_[identifier] = std::move(entry);
    return true;
}

bool KeyStorage::retrieveKey(const std::string& identifier,
                           std::string& key,
                           std::string& metadata) {
    if (!this->initialized_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(this->mutex_);

    auto it = this->keys_.find(identifier);
    if (it == this->keys_.end()) {
        return false;
    }

    // Check if key is expired
    if (isKeyExpired(identifier)) {
        return false;
    }

    // Decrypt the key
    if (!decryptKey(it->second.key, key)) {
        return false;
    }

    // Update access information
    updateKeyAccess(it->second);
    metadata = it->second.metadata;

    return true;
}

bool KeyStorage::deleteKey(const std::string& identifier) {
    if (!this->initialized_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(this->mutex_);
    return this->keys_.erase(identifier) > 0;
}

bool KeyStorage::updateKey(const std::string& key,
                          const std::string& identifier,
                          const std::string& metadata) {
    if (!this->initialized_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(this->mutex_);

    // Validate key format
    if (!validateKeyFormat(key)) {
        return false;
    }

    // Validate metadata
    if (!validateKeyMetadata(metadata)) {
        return false;
    }

    auto it = this->keys_.find(identifier);
    if (it == this->keys_.end()) {
        return false;
    }

    // Encrypt the new key
    std::string encryptedKey;
    if (!encryptKey(key, encryptedKey)) {
        return false;
    }

    // Update the key entry
    it->second.key = encryptedKey;
    it->second.metadata = metadata;
    updateKeyAccess(it->second);

    return true;
}

bool KeyStorage::setKeyMetadata(const std::string& identifier,
                               const std::string& metadata) {
    if (!this->initialized_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(this->mutex_);

    // Validate metadata
    if (!validateKeyMetadata(metadata)) {
        return false;
    }

    auto it = this->keys_.find(identifier);
    if (it == this->keys_.end()) {
        return false;
    }

    it->second.metadata = metadata;
    return true;
}

bool KeyStorage::getKeyMetadata(const std::string& identifier,
                               std::string& metadata) {
    if (!this->initialized_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(this->mutex_);

    auto it = this->keys_.find(identifier);
    if (it == this->keys_.end()) {
        return false;
    }

    metadata = it->second.metadata;
    return true;
}

bool KeyStorage::rotateKey(const std::string& identifier,
                          const std::string& newKey,
                          const std::string& metadata) {
    if (!this->initialized_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(this->mutex_);

    // Validate new key format
    if (!validateKeyFormat(newKey)) {
        return false;
    }

    auto it = this->keys_.find(identifier);
    if (it == this->keys_.end()) {
        return false;
    }

    // Encrypt the new key
    std::string encryptedKey;
    if (!encryptKey(newKey, encryptedKey)) {
        return false;
    }

    // Update the key entry
    it->second.key = encryptedKey;
    if (!metadata.empty()) {
        it->second.metadata = metadata;
    }
    updateKeyAccess(it->second);

    return true;
}

bool KeyStorage::reencryptKey(const std::string& identifier,
                             const std::string& oldKey,
                             const std::string& newKey) {
    if (!this->initialized_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(this->mutex_);

    auto it = this->keys_.find(identifier);
    if (it == this->keys_.end()) {
        return false;
    }

    // Decrypt with old key
    std::string decryptedKey;
    if (!decryptKey(it->second.key, decryptedKey)) {
        return false;
    }

    // Encrypt with new key
    std::string encryptedKey;
    if (!encryptKey(decryptedKey, encryptedKey)) {
        return false;
    }

    // Update the key entry
    it->second.key = encryptedKey;
    updateKeyAccess(it->second);

    return true;
}

bool KeyStorage::validateKey(const std::string& key,
                            const std::string& identifier) {
    if (!this->initialized_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(this->mutex_);

    // Validate key format
    if (!validateKeyFormat(key)) {
        return false;
    }

    auto it = this->keys_.find(identifier);
    if (it == this->keys_.end()) {
        return false;
    }

    // Decrypt stored key
    std::string storedKey;
    if (!decryptKey(it->second.key, storedKey)) {
        return false;
    }

    // Compare keys
    return key == storedKey;
}

bool KeyStorage::validateAllKeys() {
    if (!this->initialized_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(this->mutex_);

    for (const auto& pair : this->keys_) {
        std::string key;
        if (!decryptKey(pair.second.key, key)) {
            return false;
        }
        if (!validateKeyFormat(key)) {
            return false;
        }
    }

    return true;
}

bool KeyStorage::setKeyExpiration(const std::string& identifier,
                                 const std::chrono::system_clock::time_point& expiration) {
    if (!this->initialized_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(this->mutex_);

    auto it = this->keys_.find(identifier);
    if (it == this->keys_.end()) {
        return false;
    }

    it->second.expiration = expiration;
    return true;
}

bool KeyStorage::getKeyExpiration(const std::string& identifier,
                                 std::chrono::system_clock::time_point& expiration) {
    if (!this->initialized_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(this->mutex_);

    auto it = this->keys_.find(identifier);
    if (it == this->keys_.end()) {
        return false;
    }

    expiration = it->second.expiration;
    return true;
}

bool KeyStorage::isKeyExpired(const std::string& identifier) {
    if (!this->initialized_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(this->mutex_);

    auto it = this->keys_.find(identifier);
    if (it == this->keys_.end()) {
        return false;
    }

    return std::chrono::system_clock::now() > it->second.expiration;
}

bool KeyStorage::setKeyAccess(const std::string& identifier,
                             const std::vector<std::string>& allowedUsers) {
    if (!this->initialized_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(this->mutex_);

    auto it = this->keys_.find(identifier);
    if (it == this->keys_.end()) {
        return false;
    }

    it->second.allowedUsers = allowedUsers;
    return true;
}

bool KeyStorage::getKeyAccess(const std::string& identifier,
                             std::vector<std::string>& allowedUsers) {
    if (!this->initialized_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(this->mutex_);

    auto it = this->keys_.find(identifier);
    if (it == this->keys_.end()) {
        return false;
    }

    allowedUsers = it->second.allowedUsers;
    return true;
}

bool KeyStorage::checkKeyAccess(const std::string& identifier,
                               const std::string& user) {
    if (!this->initialized_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(this->mutex_);

    auto it = this->keys_.find(identifier);
    if (it == this->keys_.end()) {
        return false;
    }

    return validateKeyAccess(it->second, user);
}

bool KeyStorage::isInitialized() const {
    return this->initialized_;
}

std::string KeyStorage::getAlgorithm() const {
    return this->algorithm_;
}

std::string KeyStorage::getVersion() const {
    return this->version_;
}

size_t KeyStorage::getKeyCount() const {
    std::lock_guard<std::mutex> lock(this->mutex_);
    return this->keys_.size();
}

std::vector<std::string> KeyStorage::getAllKeyIdentifiers() const {
    std::lock_guard<std::mutex> lock(this->mutex_);
    std::vector<std::string> identifiers;
    identifiers.reserve(this->keys_.size());
    for (const auto& pair : this->keys_) {
        identifiers.push_back(pair.first);
    }
    return identifiers;
}

bool KeyStorage::initializeStorageSystem() {
    // Initialize OpenSSL
    if (sodium_init() < 0) {
        return false;
    }

    return true;
}

void KeyStorage::cleanupStorageSystem() {
    // Cleanup OpenSSL
    EVP_cleanup();
    ERR_free_strings();
}

bool KeyStorage::encryptKey(const std::string& key, std::string& encryptedKey) {
    // Generate a random nonce
    unsigned char nonce[crypto_aead_aes256gcm_NPUBBYTES];
    randombytes_buf(nonce, sizeof(nonce));

    // Prepare the output buffer
    encryptedKey.resize(key.size() + crypto_aead_aes256gcm_ABYTES);
    unsigned long long encryptedLength;

    // Encrypt the key
    if (crypto_aead_aes256gcm_encrypt(
            reinterpret_cast<unsigned char*>(&encryptedKey[0]), &encryptedLength,
            reinterpret_cast<const unsigned char*>(key.c_str()), key.size(),
            nullptr, 0, nullptr, nonce,
            reinterpret_cast<const unsigned char*>(key.c_str())) != 0) {
        return false;
    }

    // Prepend the nonce
    encryptedKey = std::string(reinterpret_cast<char*>(nonce), sizeof(nonce)) + encryptedKey;
    return true;
}

bool KeyStorage::decryptKey(const std::string& encryptedKey, std::string& key) {
    if (encryptedKey.size() < crypto_aead_aes256gcm_NPUBBYTES) {
        return false;
    }

    // Extract the nonce
    unsigned char nonce[crypto_aead_aes256gcm_NPUBBYTES];
    std::memcpy(nonce, encryptedKey.c_str(), sizeof(nonce));

    // Prepare the output buffer
    key.resize(encryptedKey.size() - crypto_aead_aes256gcm_NPUBBYTES);
    unsigned long long decryptedLength;

    // Decrypt the key
    if (crypto_aead_aes256gcm_decrypt(
            reinterpret_cast<unsigned char*>(&key[0]), &decryptedLength,
            nullptr,
            reinterpret_cast<const unsigned char*>(encryptedKey.c_str() + crypto_aead_aes256gcm_NPUBBYTES),
            encryptedKey.size() - crypto_aead_aes256gcm_NPUBBYTES,
            nullptr, 0,
            nonce,
            reinterpret_cast<const unsigned char*>(key.c_str())) != 0) {
        return false;
    }

    key.resize(decryptedLength);
    return true;
}

bool KeyStorage::validateKeyFormat(const std::string& key) {
    // Check key length
    if (key.size() != 32) { // 256 bits
        return false;
    }

    // Check for null bytes
    if (std::find(key.begin(), key.end(), '\0') != key.end()) {
        return false;
    }

    return true;
}

bool KeyStorage::validateKeyMetadata(const std::string& metadata) {
    // Check metadata length
    if (metadata.size() > 1024) { // 1KB limit
        return false;
    }

    // Check for null bytes
    if (std::find(metadata.begin(), metadata.end(), '\0') != metadata.end()) {
        return false;
    }

    return true;
}

bool KeyStorage::validateKeyAccess(const KeyEntry& entry, const std::string& user) {
    // Check if user is in allowed users list
    return std::find(entry.allowedUsers.begin(), entry.allowedUsers.end(), user) != entry.allowedUsers.end();
}

void KeyStorage::updateKeyAccess(KeyEntry& entry) {
    entry.lastAccess = std::chrono::system_clock::now();
    entry.accessCount++;
}

bool KeyStorage::cleanupExpiredKeys() {
    if (!this->initialized_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(this->mutex_);

    auto now = std::chrono::system_clock::now();
    for (auto it = this->keys_.begin(); it != this->keys_.end();) {
        if (now > it->second.expiration) {
            it = this->keys_.erase(it);
        } else {
            ++it;
        }
    }

    return true;
}

} // namespace quantum
} // namespace satox 