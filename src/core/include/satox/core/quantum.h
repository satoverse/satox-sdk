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
#include <utility>

namespace satox {
namespace core {

class QuantumManager {
public:
    QuantumManager();
    ~QuantumManager();

    // Prevent copying
    QuantumManager(const QuantumManager&) = delete;
    QuantumManager& operator=(const QuantumManager&) = delete;

    // Allow moving
    QuantumManager(QuantumManager&&) noexcept = default;
    QuantumManager& operator=(QuantumManager&&) noexcept = default;

    // Core functionality
    void initialize(const std::string& algorithm);
    std::pair<std::string, std::string> generateKeyPair();
    std::string encrypt(const std::string& publicKey, const std::string& data);
    std::string decrypt(const std::string& privateKey, const std::string& encryptedData);
    std::string serializeKey(const std::string& key);
    std::string deserializeKey(const std::string& serialized);

private:
    void cleanup();

    bool initialized_;
    std::string algorithm_;
};

} // namespace core
} // namespace satox 