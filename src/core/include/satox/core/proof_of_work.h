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
#include <cstdint>
#include <openssl/sha.h>

namespace satox {
namespace core {

class ProofOfWork {
public:
    ProofOfWork();
    ~ProofOfWork() = default;

    // Prevent copying
    ProofOfWork(const ProofOfWork&) = delete;
    ProofOfWork& operator=(const ProofOfWork&) = delete;

    // Allow moving
    ProofOfWork(ProofOfWork&&) noexcept = default;
    ProofOfWork& operator=(ProofOfWork&&) noexcept = default;

    // Mining functions
    std::string mineBlock(const std::string& previousHash,
                         const std::string& merkleRoot,
                         uint64_t timestamp,
                         uint64_t difficulty);
    
    // Verification functions
    bool verifyBlock(const std::string& hash,
                    const std::string& previousHash,
                    const std::string& merkleRoot,
                    uint64_t timestamp,
                    uint64_t difficulty,
                    const std::string& nonce);

    // Utility functions
    static std::string calculateHash(const std::string& data);
    static bool meetsDifficulty(const std::string& hash, uint64_t difficulty);
    static uint64_t getDifficulty(uint64_t blockHeight);

private:
    static constexpr uint64_t INITIAL_DIFFICULTY = 4;
    static constexpr uint64_t DIFFICULTY_ADJUSTMENT_INTERVAL = 2016;
    static constexpr uint64_t TARGET_BLOCK_TIME = 600; // 10 minutes in seconds

    std::string generateNonce();
    bool isValidNonce(const std::string& nonce);
};

} // namespace core
} // namespace satox 