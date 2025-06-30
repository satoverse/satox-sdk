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

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/evp.h>

namespace satox {
namespace wallet {

class KeyManager {
public:
    // Key pair structure
    struct KeyPair {
        std::vector<uint8_t> privateKey;
        std::vector<uint8_t> publicKey;
        std::vector<uint8_t> chainCode;  // For HD wallet derivation
        std::string address;
    };

    // Constructor and destructor
    KeyManager();
    ~KeyManager();

    // Initialization and cleanup
    bool initialize();
    void cleanup();

    // Key generation and derivation
    bool generateKeyPair(KeyPair& keyPair);
    bool deriveKeyPairFromSeed(const std::vector<uint8_t>& seed, KeyPair& keyPair);
    bool deriveKeyPairFromPrivateKey(const std::vector<uint8_t>& privateKey, KeyPair& keyPair);
    bool deriveChildKeyPair(const KeyPair& parent, uint32_t index, KeyPair& child);

    // Key validation
    bool validatePrivateKey(const std::vector<uint8_t>& privateKey) const;
    bool validatePublicKey(const std::vector<uint8_t>& publicKey) const;
    bool validateKeyPair(const KeyPair& keyPair) const;

    // Key conversion
    bool privateKeyToPublicKey(const std::vector<uint8_t>& privateKey, std::vector<uint8_t>& publicKey) const;
    bool publicKeyToAddress(const std::vector<uint8_t>& publicKey, std::string& address) const;

    // Key encryption
    bool encryptPrivateKey(const std::vector<uint8_t>& privateKey, const std::string& password, std::vector<uint8_t>& encrypted) const;
    bool decryptPrivateKey(const std::vector<uint8_t>& encrypted, const std::string& password, std::vector<uint8_t>& privateKey) const;

    // Error handling
    std::string getLastError() const;
    void clearLastError();

private:
    // Internal helper methods
    bool initializeOpenSSL();
    void cleanupOpenSSL();
    bool generateRandomBytes(std::vector<uint8_t>& bytes, size_t length) const;
    bool computeSHA256(const std::vector<uint8_t>& data, std::vector<uint8_t>& hash) const;
    bool computeRIPEMD160(const std::vector<uint8_t>& data, std::vector<uint8_t>& hash) const;
    bool computeDoubleSHA256(const std::vector<uint8_t>& data, std::vector<uint8_t>& hash) const;
    bool deriveKeyFromPassword(const std::string& password, const std::vector<uint8_t>& salt, std::vector<uint8_t>& key) const;
    bool computeHMACSHA512(const std::vector<uint8_t>& data, const std::vector<uint8_t>& key, std::vector<uint8_t>& hmac) const;
    std::string bytesToHex(const std::vector<uint8_t>& bytes) const;
    bool hexToBytes(const std::string& hex, std::vector<uint8_t>& bytes) const;
    std::string deriveAddress(const std::vector<uint8_t>& publicKey) const;

    // Member variables
    mutable std::mutex mutex_;
    mutable std::string lastError_;
    bool initialized_;
    EC_KEY* key_;
    const EC_GROUP* group_;
    BIGNUM* order_;
};

} // namespace wallet
} // namespace satox 