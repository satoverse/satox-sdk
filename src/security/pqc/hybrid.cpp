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

#include "security/pqc/hybrid.hpp"
#include "security/pqc/ml_kem.hpp"
#include "security/pqc/ml_dsa.hpp"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <sodium.h>
#include <stdexcept>
#include <cstring>

namespace satox {
namespace security {
namespace pqc {

class HybridCrypto::Impl {
public:
    Impl(MLKEM::SecurityLevel kemLevel, MLDSA::SecurityLevel dsaLevel)
        : kemLevel(kemLevel), dsaLevel(dsaLevel) {
        if (sodium_init() < 0) {
            throw std::runtime_error("Failed to initialize libsodium");
        }
    }

    bool generateHybridKeyPair(std::vector<uint8_t>& publicKey, 
                             std::vector<uint8_t>& privateKey) {
        try {
            // Generate ML-KEM key pair
            MLKEM kem(kemLevel);
            std::vector<uint8_t> kemPublicKey, kemPrivateKey;
            if (!kem.generateKeyPair(kemPublicKey, kemPrivateKey)) {
                throw std::runtime_error("Failed to generate ML-KEM key pair: " + 
                                       kem.getLastError());
            }

            // Generate ML-DSA key pair
            MLDSA dsa(dsaLevel);
            std::vector<uint8_t> dsaPublicKey, dsaPrivateKey;
            if (!dsa.generateKeyPair(dsaPublicKey, dsaPrivateKey)) {
                throw std::runtime_error("Failed to generate ML-DSA key pair: " + 
                                       dsa.getLastError());
            }

            // Combine keys
            publicKey = combineKeys(kemPublicKey, dsaPublicKey);
            privateKey = combineKeys(kemPrivateKey, dsaPrivateKey);

            return true;
        } catch (const std::exception& e) {
            lastError = e.what();
            return false;
        }
    }

    bool hybridEncrypt(const std::vector<uint8_t>& publicKey,
                      const std::vector<uint8_t>& message,
                      std::vector<uint8_t>& ciphertext) {
        try {
            // Split public key
            auto [kemPublicKey, dsaPublicKey] = splitKey(publicKey);

            // Generate ephemeral key pair
            MLKEM kem(kemLevel);
            std::vector<uint8_t> ephemeralPublicKey, ephemeralPrivateKey;
            if (!kem.generateKeyPair(ephemeralPublicKey, ephemeralPrivateKey)) {
                throw std::runtime_error("Failed to generate ephemeral key pair: " + 
                                       kem.getLastError());
            }

            // Encapsulate shared secret
            std::vector<uint8_t> kemCiphertext, sharedSecret;
            if (!kem.encapsulate(kemPublicKey, kemCiphertext, sharedSecret)) {
                throw std::runtime_error("Failed to encapsulate: " + 
                                       kem.getLastError());
            }

            // Encrypt message with shared secret
            std::vector<uint8_t> encryptedMessage;
            if (!encryptMessage(message, sharedSecret, encryptedMessage)) {
                throw std::runtime_error("Failed to encrypt message");
            }

            // Sign ephemeral public key
            MLDSA dsa(dsaLevel);
            std::vector<uint8_t> signature;
            if (!dsa.sign(dsaPublicKey, ephemeralPublicKey, signature)) {
                throw std::runtime_error("Failed to sign: " + 
                                       dsa.getLastError());
            }

            // Combine components
            ciphertext = combineComponents(kemCiphertext, encryptedMessage, 
                                         ephemeralPublicKey, signature);

            return true;
        } catch (const std::exception& e) {
            lastError = e.what();
            return false;
        }
    }

    bool hybridDecrypt(const std::vector<uint8_t>& privateKey,
                      const std::vector<uint8_t>& ciphertext,
                      std::vector<uint8_t>& message) {
        try {
            // Split private key
            auto [kemPrivateKey, dsaPrivateKey] = splitKey(privateKey);

            // Split ciphertext components
            auto [kemCiphertext, encryptedMessage, ephemeralPublicKey, signature] = 
                splitComponents(ciphertext);

            // Verify signature
            MLDSA dsa(dsaLevel);
            if (!dsa.verify(dsaPrivateKey, ephemeralPublicKey, signature)) {
                throw std::runtime_error("Signature verification failed: " + 
                                       dsa.getLastError());
            }

            // Decapsulate shared secret
            MLKEM kem(kemLevel);
            std::vector<uint8_t> sharedSecret;
            if (!kem.decapsulate(kemPrivateKey, kemCiphertext, sharedSecret)) {
                throw std::runtime_error("Failed to decapsulate: " + 
                                       kem.getLastError());
            }

            // Decrypt message
            if (!decryptMessage(encryptedMessage, sharedSecret, message)) {
                throw std::runtime_error("Failed to decrypt message");
            }

            return true;
        } catch (const std::exception& e) {
            lastError = e.what();
            return false;
        }
    }

    bool hybridSign(const std::vector<uint8_t>& privateKey,
                   const std::vector<uint8_t>& message,
                   std::vector<uint8_t>& signature) {
        try {
            // Split private key
            auto [kemPrivateKey, dsaPrivateKey] = splitKey(privateKey);

            // Sign with ML-DSA
            MLDSA dsa(dsaLevel);
            if (!dsa.sign(dsaPrivateKey, message, signature)) {
                throw std::runtime_error("Failed to sign: " + 
                                       dsa.getLastError());
            }

            return true;
        } catch (const std::exception& e) {
            lastError = e.what();
            return false;
        }
    }

    bool hybridVerify(const std::vector<uint8_t>& publicKey,
                     const std::vector<uint8_t>& message,
                     const std::vector<uint8_t>& signature) {
        try {
            // Split public key
            auto [kemPublicKey, dsaPublicKey] = splitKey(publicKey);

            // Verify with ML-DSA
            MLDSA dsa(dsaLevel);
            if (!dsa.verify(dsaPublicKey, message, signature)) {
                throw std::runtime_error("Signature verification failed: " + 
                                       dsa.getLastError());
            }

            return true;
        } catch (const std::exception& e) {
            lastError = e.what();
            return false;
        }
    }

    bool rotateKeys(const std::vector<uint8_t>& oldPrivateKey,
                   std::vector<uint8_t>& newPublicKey,
                   std::vector<uint8_t>& newPrivateKey) {
        try {
            // Generate new key pair
            if (!generateHybridKeyPair(newPublicKey, newPrivateKey)) {
                throw std::runtime_error("Failed to generate new key pair");
            }

            // TODO: Implement key rotation logic
            return true;
        } catch (const std::exception& e) {
            lastError = e.what();
            return false;
        }
    }

    // Getters
    size_t getPublicKeySize() const {
        MLKEM kem(kemLevel);
        MLDSA dsa(dsaLevel);
        return kem.getPublicKeySize() + dsa.getPublicKeySize();
    }

    size_t getPrivateKeySize() const {
        MLKEM kem(kemLevel);
        MLDSA dsa(dsaLevel);
        return kem.getPrivateKeySize() + dsa.getPrivateKeySize();
    }

    size_t getCiphertextSize() const {
        MLKEM kem(kemLevel);
        MLDSA dsa(dsaLevel);
        return kem.getCiphertextSize() + dsa.getSignatureSize() + 
               kem.getPublicKeySize() + 32; // 32 bytes for encrypted message
    }

    size_t getSignatureSize() const {
        MLDSA dsa(dsaLevel);
        return dsa.getSignatureSize();
    }

    MLKEM::SecurityLevel getKEMSecurityLevel() const {
        return kemLevel;
    }

    MLDSA::SecurityLevel getDSASecurityLevel() const {
        return dsaLevel;
    }

    std::string getLastError() const {
        return lastError;
    }

    void clearLastError() {
        lastError.clear();
    }

private:
    MLKEM::SecurityLevel kemLevel;
    MLDSA::SecurityLevel dsaLevel;
    std::string lastError;

    std::pair<std::vector<uint8_t>, std::vector<uint8_t>> 
    splitKey(const std::vector<uint8_t>& combinedKey) {
        MLKEM kem(kemLevel);
        MLDSA dsa(dsaLevel);
        
        size_t kemSize = kem.getPublicKeySize();
        std::vector<uint8_t> kemKey(combinedKey.begin(), 
                                  combinedKey.begin() + kemSize);
        std::vector<uint8_t> dsaKey(combinedKey.begin() + kemSize, 
                                  combinedKey.end());
        
        return {kemKey, dsaKey};
    }

    std::vector<uint8_t> combineKeys(const std::vector<uint8_t>& kemKey,
                                   const std::vector<uint8_t>& dsaKey) {
        std::vector<uint8_t> combined;
        combined.reserve(kemKey.size() + dsaKey.size());
        combined.insert(combined.end(), kemKey.begin(), kemKey.end());
        combined.insert(combined.end(), dsaKey.begin(), dsaKey.end());
        return combined;
    }

    bool encryptMessage(const std::vector<uint8_t>& message,
                       const std::vector<uint8_t>& key,
                       std::vector<uint8_t>& encrypted) {
        // TODO: Implement message encryption
        return true;
    }

    bool decryptMessage(const std::vector<uint8_t>& encrypted,
                       const std::vector<uint8_t>& key,
                       std::vector<uint8_t>& message) {
        // TODO: Implement message decryption
        return true;
    }

    std::tuple<std::vector<uint8_t>, std::vector<uint8_t>, 
               std::vector<uint8_t>, std::vector<uint8_t>>
    splitComponents(const std::vector<uint8_t>& combined) {
        MLKEM kem(kemLevel);
        MLDSA dsa(dsaLevel);
        
        size_t kemCiphertextSize = kem.getCiphertextSize();
        size_t dsaSignatureSize = dsa.getSignatureSize();
        size_t kemPublicKeySize = kem.getPublicKeySize();
        
        auto it = combined.begin();
        std::vector<uint8_t> kemCiphertext(it, it + kemCiphertextSize);
        it += kemCiphertextSize;
        
        std::vector<uint8_t> encryptedMessage(it, it + 32);
        it += 32;
        
        std::vector<uint8_t> ephemeralPublicKey(it, it + kemPublicKeySize);
        it += kemPublicKeySize;
        
        std::vector<uint8_t> signature(it, it + dsaSignatureSize);
        
        return {kemCiphertext, encryptedMessage, ephemeralPublicKey, signature};
    }

    std::vector<uint8_t> combineComponents(
        const std::vector<uint8_t>& kemCiphertext,
        const std::vector<uint8_t>& encryptedMessage,
        const std::vector<uint8_t>& ephemeralPublicKey,
        const std::vector<uint8_t>& signature) {
        
        std::vector<uint8_t> combined;
        combined.reserve(kemCiphertext.size() + encryptedMessage.size() + 
                        ephemeralPublicKey.size() + signature.size());
        
        combined.insert(combined.end(), kemCiphertext.begin(), kemCiphertext.end());
        combined.insert(combined.end(), encryptedMessage.begin(), encryptedMessage.end());
        combined.insert(combined.end(), ephemeralPublicKey.begin(), ephemeralPublicKey.end());
        combined.insert(combined.end(), signature.begin(), signature.end());
        
        return combined;
    }
};

// HybridCrypto implementation
HybridCrypto::HybridCrypto(MLKEM::SecurityLevel kemLevel, 
                          MLDSA::SecurityLevel dsaLevel)
    : pImpl(std::make_unique<Impl>(kemLevel, dsaLevel)) {}

HybridCrypto::~HybridCrypto() = default;

bool HybridCrypto::generateHybridKeyPair(std::vector<uint8_t>& publicKey, 
                                       std::vector<uint8_t>& privateKey) {
    return pImpl->generateHybridKeyPair(publicKey, privateKey);
}

bool HybridCrypto::hybridEncrypt(const std::vector<uint8_t>& publicKey,
                               const std::vector<uint8_t>& message,
                               std::vector<uint8_t>& ciphertext) {
    return pImpl->hybridEncrypt(publicKey, message, ciphertext);
}

bool HybridCrypto::hybridDecrypt(const std::vector<uint8_t>& privateKey,
                               const std::vector<uint8_t>& ciphertext,
                               std::vector<uint8_t>& message) {
    return pImpl->hybridDecrypt(privateKey, ciphertext, message);
}

bool HybridCrypto::hybridSign(const std::vector<uint8_t>& privateKey,
                            const std::vector<uint8_t>& message,
                            std::vector<uint8_t>& signature) {
    return pImpl->hybridSign(privateKey, message, signature);
}

bool HybridCrypto::hybridVerify(const std::vector<uint8_t>& publicKey,
                              const std::vector<uint8_t>& message,
                              const std::vector<uint8_t>& signature) {
    return pImpl->hybridVerify(publicKey, message, signature);
}

bool HybridCrypto::rotateKeys(const std::vector<uint8_t>& oldPrivateKey,
                            std::vector<uint8_t>& newPublicKey,
                            std::vector<uint8_t>& newPrivateKey) {
    return pImpl->rotateKeys(oldPrivateKey, newPublicKey, newPrivateKey);
}

size_t HybridCrypto::getPublicKeySize() const {
    return pImpl->getPublicKeySize();
}

size_t HybridCrypto::getPrivateKeySize() const {
    return pImpl->getPrivateKeySize();
}

size_t HybridCrypto::getCiphertextSize() const {
    return pImpl->getCiphertextSize();
}

size_t HybridCrypto::getSignatureSize() const {
    return pImpl->getSignatureSize();
}

MLKEM::SecurityLevel HybridCrypto::getKEMSecurityLevel() const {
    return pImpl->getKEMSecurityLevel();
}

MLDSA::SecurityLevel HybridCrypto::getDSASecurityLevel() const {
    return pImpl->getDSASecurityLevel();
}

std::string HybridCrypto::getLastError() const {
    return pImpl->getLastError();
}

void HybridCrypto::clearLastError() {
    pImpl->clearLastError();
}

} // namespace pqc
} // namespace security
} // namespace satox 