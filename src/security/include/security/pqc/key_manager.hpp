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
#include "hybrid.hpp"
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <optional>

namespace satox {
namespace security {
namespace pqc {

struct KeyMetadata {
    std::string keyId;
    std::chrono::system_clock::time_point creationTime;
    std::chrono::system_clock::time_point expirationTime;
    std::string algorithm;
    int securityLevel;
    bool isActive;
    std::string metadata;
};

class KeyManager {
public:
    KeyManager();
    ~KeyManager();

    // Key generation and storage
    bool generateAndStoreKey(const std::string& keyId,
                           const std::string& algorithm,
                           int securityLevel,
                           const std::string& metadata = "");

    // Key retrieval
    bool getKey(const std::string& keyId,
               std::vector<uint8_t>& key,
               KeyMetadata& metadata);

    // Key rotation
    bool rotateKey(const std::string& keyId,
                  const std::string& newMetadata = "");

    // Key deletion
    bool deleteKey(const std::string& keyId);

    // Key listing
    std::vector<std::string> listKeys() const;
    std::vector<KeyMetadata> listKeyMetadata() const;

    // Key status
    bool isKeyValid(const std::string& keyId) const;
    bool isKeyExpired(const std::string& keyId) const;

    // Key metadata management
    bool updateKeyMetadata(const std::string& keyId,
                          const std::string& metadata);
    bool setKeyExpiration(const std::string& keyId,
                         const std::chrono::system_clock::time_point& expiration);

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