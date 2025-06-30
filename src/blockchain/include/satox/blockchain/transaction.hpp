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
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace satox {
namespace blockchain {

class Transaction {
public:
    Transaction() = default;
    ~Transaction() = default;

    // Getters
    const std::string& getHash() const { return hash_; }
    const std::string& getFrom() const { return from_; }
    const std::string& getTo() const { return to_; }
    double getValue() const { return value_; }
    const std::vector<uint8_t>& getData() const { return data_; }
    std::chrono::system_clock::time_point getTimestamp() const { return timestamp_; }
    const std::string& getStatus() const { return status_; }
    const nlohmann::json& getMetadata() const { return metadata_; }
    uint64_t getNonce() const { return nonce_; }
    double getGasPrice() const { return gas_price_; }
    uint64_t getGasLimit() const { return gas_limit_; }
    std::vector<uint8_t> getRaw() const;

    // Setters
    void setHash(const std::string& hash) { hash_ = hash; }
    void setFrom(const std::string& from) { from_ = from; }
    void setTo(const std::string& to) { to_ = to; }
    void setValue(double value) { value_ = value; }
    void setData(const std::vector<uint8_t>& data) { data_ = data; }
    void setTimestamp(std::chrono::system_clock::time_point timestamp) { timestamp_ = timestamp; }
    void setStatus(const std::string& status) { status_ = status; }
    void setMetadata(const nlohmann::json& metadata) { metadata_ = metadata; }
    void setNonce(uint64_t nonce) { nonce_ = nonce; }
    void setGasPrice(double gas_price) { gas_price_ = gas_price; }
    void setGasLimit(uint64_t gas_limit) { gas_limit_ = gas_limit; }

    // Validation and serialization methods
    bool isValid() const;
    nlohmann::json toJson() const;
    static std::shared_ptr<Transaction> fromJson(const nlohmann::json& json);

private:
    std::string hash_;
    std::string from_;
    std::string to_;
    double value_{0.0};
    std::vector<uint8_t> data_;
    std::chrono::system_clock::time_point timestamp_;
    std::string status_;
    nlohmann::json metadata_;
    uint64_t nonce_{0};
    double gas_price_{0.0};
    uint64_t gas_limit_{0};
};

} // namespace blockchain
} // namespace satox 