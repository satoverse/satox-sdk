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

#include "ml_kem.hpp"
#include "ml_dsa.hpp"
#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace satox {
namespace security {
namespace pqc {

class HybridCrypto {
public:
    // Constructor with security levels
    HybridCrypto(MLKEM::SecurityLevel kemLevel = MLKEM::SecurityLevel::Level3,
                 MLDSA::SecurityLevel dsaLevel = MLDSA::SecurityLevel::Level3);
    ~HybridCrypto();

    // Hybrid key generation
    bool generateHybridKeyPair(std::vector<uint8_t>& publicKey,
                             std::vector<uint8_t>& privateKey);

    // Hybrid encryption
    bool encrypt(const std::vector<uint8_t>& message,
                const std::vector<uint8_t>& publicKey,
                std::vector<uint8_t>& ciphertext);

    // Hybrid decryption
    bool decrypt(const std::vector<uint8_t>& ciphertext,
                const std::vector<uint8_t>& privateKey,
                std::vector<uint8_t>& message);

    // Hybrid signing
    bool sign(const std::vector<uint8_t>& message,
             const std::vector<uint8_t>& privateKey,
             std::vector<uint8_t>& signature);

    // Hybrid verification
    bool verify(const std::vector<uint8_t>& message,
               const std::vector<uint8_t>& signature,
               const std::vector<uint8_t>& publicKey);

    // Key rotation
    bool rotateKeys(const std::vector<uint8_t>& oldPrivateKey,
                   std::vector<uint8_t>& newPublicKey,
                   std::vector<uint8_t>& newPrivateKey);

    // Get algorithm parameters
    size_t getPublicKeySize() const;
    size_t getPrivateKeySize() const;
    size_t getCiphertextSize() const;
    size_t getSignatureSize() const;

    // Error handling
    std::string getLastError() const;
    void clearLastError();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace pqc
} // namespace security
} // namespace satox 