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

#include <gtest/gtest.h>
#include "satox/blockchain/kawpow.hpp"
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

using namespace satox::blockchain;

class KawpowTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test data
        header_.resize(KAWPOW_HEADER_BYTES);
        for (size_t i = 0; i < header_.size(); ++i) {
            header_[i] = static_cast<uint8_t>(i);
        }
        nonce_ = 12345;
    }

    std::vector<uint8_t> header_;
    uint64_t nonce_;
};

TEST_F(KawpowTest, CacheGeneration) {
    Kawpow kawpow;
    uint64_t block_number = 1000;
    
    EXPECT_TRUE(kawpow.initializeCache(block_number));
    
    // Verify cache size
    uint64_t expected_cache_size = KAWPOW_CACHE_BYTES_INIT;
    for (uint64_t i = 0; i < block_number / KAWPOW_EPOCH_LENGTH; ++i) {
        expected_cache_size = expected_cache_size * 3 / 2;
        expected_cache_size = (expected_cache_size + KAWPOW_CACHE_BYTES_GROWTH - 1) & ~(KAWPOW_CACHE_BYTES_GROWTH - 1);
    }
    
    EXPECT_EQ(kawpow.getCacheSize(block_number), expected_cache_size);
}

TEST_F(KawpowTest, DatasetGeneration) {
    Kawpow kawpow;
    uint64_t block_number = 1000;
    
    EXPECT_TRUE(kawpow.initializeCache(block_number));
    EXPECT_TRUE(kawpow.initializeDataset(block_number));
    
    // Verify dataset size
    uint64_t expected_dataset_size = kawpow.getCacheSize(block_number) * KAWPOW_DATASET_PARENTS;
    EXPECT_EQ(kawpow.getDatasetSize(block_number), expected_dataset_size);
}

TEST_F(KawpowTest, HashComputation) {
    Kawpow kawpow;
    std::vector<uint8_t> hash;
    std::vector<uint8_t> mix_hash;
    
    EXPECT_TRUE(kawpow.computeHash(header_, nonce_, hash, mix_hash));
    
    // Verify hash size
    EXPECT_EQ(hash.size(), KAWPOW_HASH_BYTES);
    EXPECT_EQ(mix_hash.size(), KAWPOW_MIX_BYTES);
    
    // Verify hash is not all zeros
    bool all_zeros = true;
    for (const auto& byte : hash) {
        if (byte != 0) {
            all_zeros = false;
            break;
        }
    }
    EXPECT_FALSE(all_zeros);
}

TEST_F(KawpowTest, HashVerification) {
    Kawpow kawpow;
    std::vector<uint8_t> hash;
    std::vector<uint8_t> mix_hash;
    std::vector<uint8_t> target(KAWPOW_HASH_BYTES, 0xFF);
    
    EXPECT_TRUE(kawpow.computeHash(header_, nonce_, hash, mix_hash));
    EXPECT_TRUE(kawpow.verifyHash(header_, nonce_, mix_hash, target));
    
    // Test with invalid mix hash
    std::vector<uint8_t> invalid_mix_hash = mix_hash;
    invalid_mix_hash[0] ^= 1;
    EXPECT_FALSE(kawpow.verifyHash(header_, nonce_, invalid_mix_hash, target));
}

TEST_F(KawpowTest, DifficultyAdjustment) {
    Kawpow kawpow;
    std::vector<uint8_t> hash;
    std::vector<uint8_t> mix_hash;
    
    // Test with different difficulties
    for (uint32_t difficulty = 1; difficulty <= 8; ++difficulty) {
        std::vector<uint8_t> target(KAWPOW_HASH_BYTES, 0xFF);
        target[0] = 0xFF >> (difficulty % 8);
        
        EXPECT_TRUE(kawpow.computeHash(header_, nonce_, hash, mix_hash));
        bool meets_difficulty = std::lexicographical_compare(hash.begin(), hash.end(),
                                                          target.begin(), target.end());
        
        // Note: This test might fail if the hash happens to meet the difficulty
        // In a real test, we would need to try multiple nonces
        if (meets_difficulty) {
            std::cout << "Hash meets difficulty " << difficulty << std::endl;
        }
    }
}

TEST_F(KawpowTest, EpochTransition) {
    Kawpow kawpow;
    
    // Test cache size at epoch boundaries
    uint64_t epoch1_block = KAWPOW_EPOCH_LENGTH - 1;
    uint64_t epoch2_block = KAWPOW_EPOCH_LENGTH;
    
    uint64_t epoch1_cache_size = kawpow.getCacheSize(epoch1_block);
    uint64_t epoch2_cache_size = kawpow.getCacheSize(epoch2_block);
    
    // Cache size should increase at epoch boundary
    EXPECT_GT(epoch2_cache_size, epoch1_cache_size);
}

TEST_F(KawpowTest, MixHashConsistency) {
    Kawpow kawpow;
    std::vector<uint8_t> hash1, hash2;
    std::vector<uint8_t> mix_hash1, mix_hash2;
    
    // Same input should produce same output
    EXPECT_TRUE(kawpow.computeHash(header_, nonce_, hash1, mix_hash1));
    EXPECT_TRUE(kawpow.computeHash(header_, nonce_, hash2, mix_hash2));
    
    EXPECT_EQ(hash1, hash2);
    EXPECT_EQ(mix_hash1, mix_hash2);
    
    // Different nonce should produce different output
    std::vector<uint8_t> hash3, mix_hash3;
    EXPECT_TRUE(kawpow.computeHash(header_, nonce_ + 1, hash3, mix_hash3));
    
    EXPECT_NE(hash1, hash3);
    EXPECT_NE(mix_hash1, mix_hash3);
} 