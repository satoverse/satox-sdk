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

#include <vector>
#include <cstdint>
#include <string>

namespace satox {
namespace blockchain {

// KAWPOW constants
constexpr uint32_t KAWPOW_EPOCH_LENGTH = 7500;
constexpr uint32_t KAWPOW_PERIOD_LENGTH = 3;
constexpr uint32_t KAWPOW_CACHE_ROUNDS = 2048;
constexpr uint32_t KAWPOW_DATASET_PARENTS = 256;
constexpr uint32_t KAWPOW_CACHE_BYTES_INIT = 1073741824; // 2^30
constexpr uint32_t KAWPOW_CACHE_BYTES_GROWTH = 131072;   // 2^17
constexpr uint32_t KAWPOW_EPOCH_MIX_BYTES = 64;
constexpr uint32_t KAWPOW_HASH_BYTES = 32;
constexpr uint32_t KAWPOW_NONCE_BYTES = 8;
constexpr uint32_t KAWPOW_HEADER_BYTES = 32;
constexpr uint32_t KAWPOW_MIX_BYTES = 128;
constexpr uint32_t KAWPOW_ACCESSES = 64;

class Kawpow {
public:
    Kawpow();
    ~Kawpow();

    // Prevent copying
    Kawpow(const Kawpow&) = delete;
    Kawpow& operator=(const Kawpow&) = delete;

    // Allow moving
    Kawpow(Kawpow&&) noexcept = default;
    Kawpow& operator=(Kawpow&&) noexcept = default;

    // Main KAWPOW functions
    bool computeHash(const std::vector<uint8_t>& header,
                    uint64_t nonce,
                    std::vector<uint8_t>& hash,
                    std::vector<uint8_t>& mix_hash);

    bool verifyHash(const std::vector<uint8_t>& header,
                   uint64_t nonce,
                   const std::vector<uint8_t>& mix_hash,
                   const std::vector<uint8_t>& target);

    // Cache and dataset management
    bool initializeCache(uint64_t block_number);
    bool initializeDataset(uint64_t block_number);
    void clearCache();
    void clearDataset();

private:
    // Internal helper functions
    uint64_t getEpoch(uint64_t block_number) const;
    uint64_t getCacheSize(uint64_t block_number) const;
    uint64_t getDatasetSize(uint64_t block_number) const;
    bool generateCache(uint64_t block_number);
    bool generateDataset(uint64_t block_number);
    void hashHeader(const std::vector<uint8_t>& header, std::vector<uint8_t>& hash);
    void hashNonce(uint64_t nonce, std::vector<uint8_t>& hash);
    void mixHash(const std::vector<uint8_t>& seed, std::vector<uint8_t>& mix_hash);

    // Internal state
    std::vector<uint8_t> cache_;
    std::vector<uint8_t> dataset_;
    uint64_t current_epoch_;
    bool cache_initialized_;
    bool dataset_initialized_;
};

} // namespace blockchain
} // namespace satox 