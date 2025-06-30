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
#include <chrono>
#include <memory>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace satox {
namespace blockchain {

class Transaction;

class Block {
public:
    Block() = default;
    ~Block() = default;

    // Getters
    const std::string& getHash() const { return hash_; }
    const std::string& getPreviousHash() const { return previous_hash_; }
    std::chrono::system_clock::time_point getTimestamp() const { return timestamp_; }
    const std::vector<std::shared_ptr<Transaction>>& getTransactions() const { return transactions_; }
    int64_t getNonce() const { return nonce_; }
    const nlohmann::json& getMetadata() const { return metadata_; }
    uint32_t getDifficulty() const { return difficulty_; }
    const std::string& getMerkleRoot() const { return merkle_root_; }
    uint32_t getVersion() const { return version_; }
    uint64_t getHeight() const { return height_; }
    const std::string& getMinerAddress() const { return miner_address_; }
    uint64_t getBlockReward() const { return block_reward_; }
    const std::string& getKawpowMixHash() const { return kawpow_mix_hash_; }
    const std::string& getKawpowHeaderHash() const { return kawpow_header_hash_; }

    // Setters
    void setHash(const std::string& hash) { hash_ = hash; }
    void setPreviousHash(const std::string& hash) { previous_hash_ = hash; }
    void setTimestamp(std::chrono::system_clock::time_point timestamp) { timestamp_ = timestamp; }
    void setTransactions(const std::vector<std::shared_ptr<Transaction>>& txs) { transactions_ = txs; }
    void setNonce(int64_t nonce) { nonce_ = nonce; }
    void setMetadata(const nlohmann::json& metadata) { metadata_ = metadata; }
    void setDifficulty(uint32_t difficulty) { difficulty_ = difficulty; }
    void setMerkleRoot(const std::string& root) { merkle_root_ = root; }
    void setVersion(uint32_t version) { version_ = version; }
    void setHeight(uint64_t height) { height_ = height; }
    void setMinerAddress(const std::string& address) { miner_address_ = address; }
    void setBlockReward(uint64_t reward) { block_reward_ = reward; }
    void setKawpowMixHash(const std::string& mix_hash) { kawpow_mix_hash_ = mix_hash; }
    void setKawpowHeaderHash(const std::string& header_hash) { kawpow_header_hash_ = header_hash; }

    // Kawpow-specific methods
    bool validateKawpowProof() const;
    std::string calculateKawpowHeaderHash() const;
    std::string calculateKawpowMixHash() const;
    bool verifyKawpowDifficulty() const;

    // Block validation methods
    bool isValid() const;
    bool verifyMerkleRoot() const;
    bool verifyTransactions() const;
    bool verifyTimestamp() const;

    // Serialization
    nlohmann::json toJson() const;
    static std::shared_ptr<Block> fromJson(const nlohmann::json& json);

private:
    std::string hash_;
    std::string previous_hash_;
    std::chrono::system_clock::time_point timestamp_;
    std::vector<std::shared_ptr<Transaction>> transactions_;
    int64_t nonce_{0};
    nlohmann::json metadata_;
    
    // Kawpow-specific fields
    uint32_t difficulty_{0};
    std::string merkle_root_;
    uint32_t version_{1};
    uint64_t height_{0};
    std::string miner_address_;
    uint64_t block_reward_{0};
    std::string kawpow_mix_hash_;
    std::string kawpow_header_hash_;
};

} // namespace blockchain
} // namespace satox 