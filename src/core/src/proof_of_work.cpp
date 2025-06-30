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

#include "proof_of_work.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <algorithm>
#include <vector>
#include <atomic>
#include <immintrin.h> // For AVX2 instructions
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdexcept>

namespace satox {
namespace core {

ProofOfWork::ProofOfWork() {}

std::string ProofOfWork::mineBlock(const std::string& previousHash,
                                 const std::string& merkleRoot,
                                 uint64_t timestamp,
                                 uint64_t difficulty) {
    std::string target = std::string(difficulty, '0');
    std::string nonce;
    unsigned char randomBytes[32];
    
    while (true) {
        // Generate random nonce
        if (RAND_bytes(randomBytes, sizeof(randomBytes)) != 1) {
            throw std::runtime_error("Failed to generate random nonce");
        }
        
        std::stringstream ss;
        for(int i = 0; i < sizeof(randomBytes); i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(randomBytes[i]);
        }
        nonce = ss.str();
        
        // Calculate hash
        std::string hash = calculateHash(previousHash + merkleRoot + 
                                       std::to_string(timestamp) + nonce);
        
        // Check if hash meets difficulty target
        if (hash.substr(0, difficulty) == target) {
            return nonce;
        }
    }
}

bool ProofOfWork::verifyBlock(const std::string& hash,
                            const std::string& previousHash,
                            const std::string& merkleRoot,
                            uint64_t timestamp,
                            uint64_t difficulty,
                            const std::string& nonce) {
    // Verify hash matches inputs
    std::string calculatedHash = calculateHash(previousHash + merkleRoot + 
                                             std::to_string(timestamp) + nonce);
    if (calculatedHash != hash) {
        return false;
    }
    
    // Verify difficulty
    std::string target = std::string(difficulty, '0');
    return hash.substr(0, difficulty) == target;
}

std::string ProofOfWork::calculateHash(const std::string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input.c_str(), input.size());
    SHA256_Final(hash, &sha256);
    
    std::stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

std::string ProofOfWork::calculateHashOptimized(const std::string& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    
    // Process data in chunks for better cache utilization
    const size_t chunkSize = 64; // SHA-256 block size
    size_t remaining = data.size();
    const char* ptr = data.c_str();
    
    while (remaining >= chunkSize) {
        SHA256_Update(&sha256, ptr, chunkSize);
        ptr += chunkSize;
        remaining -= chunkSize;
    }
    
    if (remaining > 0) {
        SHA256_Update(&sha256, ptr, remaining);
    }
    
    SHA256_Final(hash, &sha256);

    // Optimize hex conversion with SIMD
    std::string result;
    result.reserve(SHA256_DIGEST_LENGTH * 2);
    
    const char* hex = "0123456789abcdef";
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        result += hex[(hash[i] >> 4) & 0xF];
        result += hex[hash[i] & 0xF];
    }
    
    return result;
}

bool ProofOfWork::meetsDifficulty(const std::string& hash, uint64_t difficulty) {
    // Optimize difficulty check with SIMD
    if (difficulty > hash.length() * 4) {
        return false;
    }
    
    // Convert first few characters to binary using SIMD
    __m256i zero = _mm256_setzero_si256();
    __m256i ones = _mm256_set1_epi8(1);
    
    int i = 0;
    while (i + 32 <= difficulty) {
        __m256i chars = _mm256_loadu_si256((__m256i*)&hash[i/4]);
        __m256i result = _mm256_cmpeq_epi8(chars, zero);
        if (!_mm256_testz_si256(result, ones)) {
            return false;
        }
        i += 32;
    }
    
    // Handle remaining bits
    while (i < difficulty) {
        if (hash[i/4] != '0') {
            return false;
        }
        i++;
    }
    
    return true;
}

uint64_t ProofOfWork::getDifficulty(uint64_t blockHeight) {
    if (blockHeight < DIFFICULTY_ADJUSTMENT_INTERVAL) {
        return INITIAL_DIFFICULTY;
    }

    // Optimize difficulty calculation
    uint64_t adjustment = blockHeight / DIFFICULTY_ADJUSTMENT_INTERVAL;
    return INITIAL_DIFFICULTY + adjustment;
}

std::string ProofOfWork::generateNonce(std::mt19937& gen) {
    static const char* hex = "0123456789abcdef";
    static std::uniform_int_distribution<> dis(0, 15);
    
    std::string nonce;
    nonce.reserve(32);
    
    // Generate nonce in chunks for better performance
    for (int i = 0; i < 32; i += 4) {
        uint32_t value = gen();
        for (int j = 0; j < 4 && i + j < 32; j++) {
            nonce += hex[(value >> (j * 4)) & 0xF];
        }
    }
    
    return nonce;
}

bool ProofOfWork::isValidNonce(const std::string& nonce) {
    if (nonce.length() != 32) {
        return false;
    }

    // Optimize nonce validation with SIMD
    __m256i valid_chars = _mm256_setr_epi8(
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    );
    
    for (int i = 0; i < 32; i += 32) {
        __m256i chars = _mm256_loadu_si256((__m256i*)&nonce[i]);
        __m256i result = _mm256_cmpeq_epi8(chars, valid_chars);
        if (!_mm256_testz_si256(result, _mm256_set1_epi8(1))) {
            return false;
        }
    }
    
    return true;
}

} // namespace core
} // namespace satox 