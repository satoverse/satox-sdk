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

#include "security/pqc/key_manager.hpp"
#include "security/pqc/hybrid.hpp"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <sodium.h>
#include <stdexcept>
#include <cstring>
#include <chrono>
#include <mutex>
#include <unordered_map>

namespace satox {
namespace security {
namespace pqc {

class KeyManager::Impl {
public:
    Impl() {
        if (sodium_init() < 0) {
            throw std::runtime_error("Failed to initialize libsodium");
        }
    }

    bool generateAndStoreKey(const std::string& keyId,
                           MLKEM::SecurityLevel kemLevel,
                           MLDSA::SecurityLevel dsaLevel,
                           const std::chrono::seconds& expirationTime) {
        try {
            std::lock_guard<std::mutex> lock(mutex);

            // Check if key already exists
            if (keys.find(keyId) != keys.end()) {
                throw std::runtime_error("Key with ID " + keyId + " already exists");
            }

            // Generate new key pair
            HybridCrypto crypto(kemLevel, dsaLevel);
            std::vector<uint8_t> publicKey, privateKey;
            if (!crypto.generateHybridKeyPair(publicKey, privateKey)) {
                throw std::runtime_error("Failed to generate key pair: " + 
                                       crypto.getLastError());
            }

            // Create key metadata
            KeyMetadata metadata;
            metadata.keyId = keyId;
            metadata.creationTime = std::chrono::system_clock::now();
            metadata.expirationTime = metadata.creationTime + expirationTime;
            metadata.algorithm = "ML-KEM-" + std::to_string(static_cast<int>(kemLevel)) + 
                               "+ML-DSA-" + std::to_string(static_cast<int>(dsaLevel));
            metadata.securityLevel = std::max(static_cast<int>(kemLevel), 
                                           static_cast<int>(dsaLevel));
            metadata.isActive = true;

            // Store key and metadata
            keys[keyId] = {publicKey, privateKey, metadata};

            return true;
        } catch (const std::exception& e) {
            lastError = e.what();
            return false;
        }
    }

    bool retrieveKey(const std::string& keyId,
                    std::vector<uint8_t>& publicKey,
                    std::vector<uint8_t>& privateKey,
                    KeyMetadata& metadata) {
        try {
            std::lock_guard<std::mutex> lock(mutex);

            auto it = keys.find(keyId);
            if (it == keys.end()) {
                throw std::runtime_error("Key with ID " + keyId + " not found");
            }

            // Check if key is expired
            if (std::chrono::system_clock::now() > it->second.metadata.expirationTime) {
                throw std::runtime_error("Key with ID " + keyId + " has expired");
            }

            // Check if key is active
            if (!it->second.metadata.isActive) {
                throw std::runtime_error("Key with ID " + keyId + " is not active");
            }

            publicKey = it->second.publicKey;
            privateKey = it->second.privateKey;
            metadata = it->second.metadata;

            return true;
        } catch (const std::exception& e) {
            lastError = e.what();
            return false;
        }
    }

    bool rotateKey(const std::string& keyId,
                  MLKEM::SecurityLevel newKemLevel,
                  MLDSA::SecurityLevel newDsaLevel,
                  const std::chrono::seconds& newExpirationTime) {
        try {
            std::lock_guard<std::mutex> lock(mutex);

            auto it = keys.find(keyId);
            if (it == keys.end()) {
                throw std::runtime_error("Key with ID " + keyId + " not found");
            }

            // Generate new key pair
            HybridCrypto crypto(newKemLevel, newDsaLevel);
            std::vector<uint8_t> newPublicKey, newPrivateKey;
            if (!crypto.rotateKeys(it->second.privateKey, newPublicKey, newPrivateKey)) {
                throw std::runtime_error("Failed to rotate key: " + 
                                       crypto.getLastError());
            }

            // Update key and metadata
            it->second.publicKey = newPublicKey;
            it->second.privateKey = newPrivateKey;
            it->second.metadata.creationTime = std::chrono::system_clock::now();
            it->second.metadata.expirationTime = it->second.metadata.creationTime + 
                                               newExpirationTime;
            it->second.metadata.algorithm = "ML-KEM-" + 
                                          std::to_string(static_cast<int>(newKemLevel)) + 
                                          "+ML-DSA-" + 
                                          std::to_string(static_cast<int>(newDsaLevel));
            it->second.metadata.securityLevel = std::max(static_cast<int>(newKemLevel), 
                                                      static_cast<int>(newDsaLevel));

            return true;
        } catch (const std::exception& e) {
            lastError = e.what();
            return false;
        }
    }

    bool deleteKey(const std::string& keyId) {
        try {
            std::lock_guard<std::mutex> lock(mutex);

            auto it = keys.find(keyId);
            if (it == keys.end()) {
                throw std::runtime_error("Key with ID " + keyId + " not found");
            }

            keys.erase(it);
            return true;
        } catch (const std::exception& e) {
            lastError = e.what();
            return false;
        }
    }

    std::vector<KeyMetadata> listKeys() const {
        std::lock_guard<std::mutex> lock(mutex);
        std::vector<KeyMetadata> result;
        result.reserve(keys.size());
        
        for (const auto& [keyId, keyData] : keys) {
            result.push_back(keyData.metadata);
        }
        
        return result;
    }

    bool isKeyActive(const std::string& keyId) const {
        try {
            std::lock_guard<std::mutex> lock(mutex);

            auto it = keys.find(keyId);
            if (it == keys.end()) {
                throw std::runtime_error("Key with ID " + keyId + " not found");
            }

            return it->second.metadata.isActive && 
                   std::chrono::system_clock::now() <= 
                   it->second.metadata.expirationTime;
        } catch (const std::exception& e) {
            lastError = e.what();
            return false;
        }
    }

    bool updateKeyMetadata(const std::string& keyId,
                          const std::unordered_map<std::string, std::string>& metadata) {
        try {
            std::lock_guard<std::mutex> lock(mutex);

            auto it = keys.find(keyId);
            if (it == keys.end()) {
                throw std::runtime_error("Key with ID " + keyId + " not found");
            }

            it->second.metadata.additionalMetadata = metadata;
            return true;
        } catch (const std::exception& e) {
            lastError = e.what();
            return false;
        }
    }

    std::string getLastError() const {
        return lastError;
    }

    void clearLastError() {
        lastError.clear();
    }

private:
    struct KeyData {
        std::vector<uint8_t> publicKey;
        std::vector<uint8_t> privateKey;
        KeyMetadata metadata;
    };

    mutable std::mutex mutex;
    std::unordered_map<std::string, KeyData> keys;
    mutable std::string lastError;
};

// KeyManager implementation
KeyManager::KeyManager() : pImpl(std::make_unique<Impl>()) {}

KeyManager::~KeyManager() = default;

bool KeyManager::generateAndStoreKey(const std::string& keyId,
                                   MLKEM::SecurityLevel kemLevel,
                                   MLDSA::SecurityLevel dsaLevel,
                                   const std::chrono::seconds& expirationTime) {
    return pImpl->generateAndStoreKey(keyId, kemLevel, dsaLevel, expirationTime);
}

bool KeyManager::retrieveKey(const std::string& keyId,
                           std::vector<uint8_t>& publicKey,
                           std::vector<uint8_t>& privateKey,
                           KeyMetadata& metadata) {
    return pImpl->retrieveKey(keyId, publicKey, privateKey, metadata);
}

bool KeyManager::rotateKey(const std::string& keyId,
                         MLKEM::SecurityLevel newKemLevel,
                         MLDSA::SecurityLevel newDsaLevel,
                         const std::chrono::seconds& newExpirationTime) {
    return pImpl->rotateKey(keyId, newKemLevel, newDsaLevel, newExpirationTime);
}

bool KeyManager::deleteKey(const std::string& keyId) {
    return pImpl->deleteKey(keyId);
}

std::vector<KeyMetadata> KeyManager::listKeys() const {
    return pImpl->listKeys();
}

bool KeyManager::isKeyActive(const std::string& keyId) const {
    return pImpl->isKeyActive(keyId);
}

bool KeyManager::updateKeyMetadata(const std::string& keyId,
                                 const std::unordered_map<std::string, std::string>& metadata) {
    return pImpl->updateKeyMetadata(keyId, metadata);
}

std::string KeyManager::getLastError() const {
    return pImpl->getLastError();
}

void KeyManager::clearLastError() {
    pImpl->clearLastError();
}

} // namespace pqc
} // namespace security
} // namespace satox 