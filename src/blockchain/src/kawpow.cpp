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

#include "satox/blockchain/kawpow.hpp"
#include "KeccakF-1600-reference.h"
#include "KeccakSponge.h"
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <algorithm>
#include <stdexcept>
#include <cstring>

namespace satox {
namespace blockchain {

Kawpow::Kawpow()
    : current_epoch_(0)
    , cache_initialized_(false)
    , dataset_initialized_(false) {
}

Kawpow::~Kawpow() {
    clearCache();
    clearDataset();
}

bool Kawpow::computeHash(const std::vector<uint8_t>& header,
                        uint64_t nonce,
                        std::vector<uint8_t>& hash,
                        std::vector<uint8_t>& mix_hash) {
    if (header.size() != KAWPOW_HEADER_BYTES) {
        return false;
    }

    // Initialize cache and dataset if needed
    uint64_t block_number = 0; // TODO: Get from header
    if (!initializeCache(block_number) || !initializeDataset(block_number)) {
        return false;
    }

    // Calculate header hash
    std::vector<uint8_t> header_hash(KAWPOW_HASH_BYTES);
    hashHeader(header, header_hash);

    // Calculate nonce hash
    std::vector<uint8_t> nonce_hash(KAWPOW_HASH_BYTES);
    hashNonce(nonce, nonce_hash);

    // Combine header and nonce hashes
    std::vector<uint8_t> seed(KAWPOW_HASH_BYTES);
    for (size_t i = 0; i < KAWPOW_HASH_BYTES; ++i) {
        seed[i] = header_hash[i] ^ nonce_hash[i];
    }

    // Generate mix hash
    mix_hash.resize(KAWPOW_MIX_BYTES);
    mixHash(seed, mix_hash);

    // Calculate final hash
    hash.resize(KAWPOW_HASH_BYTES);
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, mix_hash.data(), mix_hash.size());
    SHA256_Final(hash.data(), &sha256);

    return true;
}

bool Kawpow::verifyHash(const std::vector<uint8_t>& header,
                       uint64_t nonce,
                       const std::vector<uint8_t>& mix_hash,
                       const std::vector<uint8_t>& target) {
    if (header.size() != KAWPOW_HEADER_BYTES || mix_hash.size() != KAWPOW_MIX_BYTES) {
        return false;
    }

    std::vector<uint8_t> hash;
    std::vector<uint8_t> computed_mix_hash;
    if (!computeHash(header, nonce, hash, computed_mix_hash)) {
        return false;
    }

    // Verify mix hash
    if (computed_mix_hash != mix_hash) {
        return false;
    }

    // Verify target
    return std::lexicographical_compare(hash.begin(), hash.end(),
                                      target.begin(), target.end());
}

bool Kawpow::initializeCache(uint64_t block_number) {
    uint64_t epoch = getEpoch(block_number);
    if (epoch == current_epoch_ && cache_initialized_) {
        return true;
    }

    uint64_t cache_size = getCacheSize(block_number);
    cache_.resize(cache_size);
    
    if (!generateCache(block_number)) {
        return false;
    }

    current_epoch_ = epoch;
    cache_initialized_ = true;
    return true;
}

bool Kawpow::initializeDataset(uint64_t block_number) {
    uint64_t epoch = getEpoch(block_number);
    if (epoch == current_epoch_ && dataset_initialized_) {
        return true;
    }

    uint64_t dataset_size = getDatasetSize(block_number);
    dataset_.resize(dataset_size);
    
    if (!generateDataset(block_number)) {
        return false;
    }

    dataset_initialized_ = true;
    return true;
}

void Kawpow::clearCache() {
    cache_.clear();
    cache_initialized_ = false;
}

void Kawpow::clearDataset() {
    dataset_.clear();
    dataset_initialized_ = false;
}

uint64_t Kawpow::getEpoch(uint64_t block_number) const {
    return block_number / KAWPOW_EPOCH_LENGTH;
}

uint64_t Kawpow::getCacheSize(uint64_t block_number) const {
    uint64_t epoch = getEpoch(block_number);
    uint64_t size = KAWPOW_CACHE_BYTES_INIT;
    for (uint64_t i = 0; i < epoch; ++i) {
        size = size * 3 / 2;
        size = (size + KAWPOW_CACHE_BYTES_GROWTH - 1) & ~(KAWPOW_CACHE_BYTES_GROWTH - 1);
    }
    return size;
}

uint64_t Kawpow::getDatasetSize(uint64_t block_number) const {
    return getCacheSize(block_number) * KAWPOW_DATASET_PARENTS;
}

bool Kawpow::generateCache(uint64_t block_number) {
    if (cache_.empty()) {
        return false;
    }

    // Initialize Keccak sponge
    KeccakSponge sponge;
    KeccakSponge_Initialize(&sponge, KECCAK_SPONGE_BITRATE, KECCAK_SPONGE_CAPACITY);

    // Generate seed
    std::vector<uint8_t> seed(32);
    for (size_t i = 0; i < 32; ++i) {
        seed[i] = static_cast<uint8_t>(block_number >> (i * 8));
    }

    // Generate cache using Keccak-512
    for (size_t i = 0; i < cache_.size(); i += 64) {
        KeccakSponge_Absorb(&sponge, seed.data(), seed.size());
        KeccakSponge_Squeeze(&sponge, cache_.data() + i, std::min(64UL, cache_.size() - i));
    }

    // Mix the cache
    for (size_t i = 0; i < KAWPOW_CACHE_ROUNDS; ++i) {
        for (size_t j = 0; j < cache_.size(); j += 64) {
            uint64_t v = 0;
            for (size_t k = 0; k < 8; ++k) {
                v ^= *reinterpret_cast<uint64_t*>(cache_.data() + ((j + k * 8) % cache_.size()));
            }
            for (size_t k = 0; k < 8; ++k) {
                *reinterpret_cast<uint64_t*>(cache_.data() + ((j + k * 8) % cache_.size())) ^= v;
            }
        }
    }

    return true;
}

bool Kawpow::generateDataset(uint64_t block_number) {
    if (dataset_.empty() || cache_.empty()) {
        return false;
    }

    // Generate dataset using cache
    for (size_t i = 0; i < dataset_.size(); i += 64) {
        uint64_t index = i / 64;
        std::vector<uint8_t> mix(64);
        
        // Initialize mix with cache data
        size_t cache_index = (index % (cache_.size() / 64)) * 64;
        std::memcpy(mix.data(), cache_.data() + cache_index, 64);

        // Mix with parent nodes
        for (size_t j = 0; j < KAWPOW_DATASET_PARENTS; ++j) {
            uint64_t parent_index = (index * KAWPOW_DATASET_PARENTS + j) % (cache_.size() / 64);
            size_t parent_offset = parent_index * 64;
            
            for (size_t k = 0; k < 64; ++k) {
                mix[k] ^= cache_[parent_offset + k];
            }
        }

        // Store mixed data
        std::memcpy(dataset_.data() + i, mix.data(), 64);
    }

    return true;
}

void Kawpow::hashHeader(const std::vector<uint8_t>& header, std::vector<uint8_t>& hash) {
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, header.data(), header.size());
    SHA256_Final(hash.data(), &sha256);
}

void Kawpow::hashNonce(uint64_t nonce, std::vector<uint8_t>& hash) {
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, &nonce, sizeof(nonce));
    SHA256_Final(hash.data(), &sha256);
}

void Kawpow::mixHash(const std::vector<uint8_t>& seed, std::vector<uint8_t>& mix_hash) {
    if (dataset_.empty()) {
        std::fill(mix_hash.begin(), mix_hash.end(), 0);
        return;
    }

    // Initialize mix with seed
    std::vector<uint8_t> mix(64);
    std::memcpy(mix.data(), seed.data(), std::min(seed.size(), size_t(64)));

    // Mix with dataset
    for (size_t i = 0; i < KAWPOW_ACCESSES; ++i) {
        uint64_t index = 0;
        for (size_t j = 0; j < 8; ++j) {
            index = (index << 8) | mix[j];
        }
        index %= dataset_.size() / 64;
        
        for (size_t j = 0; j < 64; ++j) {
            mix[j] ^= dataset_[index * 64 + j];
        }
    }

    // Store final mix
    std::memcpy(mix_hash.data(), mix.data(), std::min(mix.size(), mix_hash.size()));
}

} // namespace blockchain
} // namespace satox 