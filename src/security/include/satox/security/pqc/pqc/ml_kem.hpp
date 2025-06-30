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
#include <optional>

namespace satox {
namespace security {
namespace pqc {

class MLKEM {
public:
    // Security levels as per NIST FIPS 203
    enum class SecurityLevel {
        Level1 = 512,  // 128-bit security
        Level3 = 768,  // 192-bit security
        Level5 = 1024  // 256-bit security
    };

    // Constructor with security level
    explicit MLKEM(SecurityLevel level = SecurityLevel::Level3);
    ~MLKEM();

    // Key generation
    bool generateKeyPair(std::vector<uint8_t>& publicKey, 
                        std::vector<uint8_t>& privateKey);

    // Encapsulation (encryption)
    bool encapsulate(const std::vector<uint8_t>& publicKey,
                    std::vector<uint8_t>& ciphertext,
                    std::vector<uint8_t>& sharedSecret);

    // Decapsulation (decryption)
    bool decapsulate(const std::vector<uint8_t>& privateKey,
                    const std::vector<uint8_t>& ciphertext,
                    std::vector<uint8_t>& sharedSecret);

    // Get algorithm parameters
    size_t getPublicKeySize() const;
    size_t getPrivateKeySize() const;
    size_t getCiphertextSize() const;
    size_t getSharedSecretSize() const;
    SecurityLevel getSecurityLevel() const;

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