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

#include "satox/blockchain/transaction.hpp"
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

namespace satox {
namespace blockchain {

bool Transaction::isValid() const {
    // Basic validation checks
    if (from_.empty() || to_.empty()) {
        return false;
    }
    
    if (value_ <= 0) {
        return false;
    }
    
    // Check if timestamp is not in the future
    auto now = std::chrono::system_clock::now();
    if (timestamp_ > now) {
        return false;
    }
    
    return true;
}

nlohmann::json Transaction::toJson() const {
    nlohmann::json j;
    j["hash"] = hash_;
    j["from"] = from_;
    j["to"] = to_;
    j["value"] = value_;
    j["timestamp"] = std::chrono::system_clock::to_time_t(timestamp_);
    j["status"] = status_;
    j["metadata"] = metadata_;
    
    // Convert binary data to hex string
    std::stringstream ss;
    for (const auto& byte : data_) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    j["data"] = ss.str();
    
    return j;
}

std::shared_ptr<Transaction> Transaction::fromJson(const nlohmann::json& json) {
    auto tx = std::make_shared<Transaction>();
    
    tx->setHash(json["hash"].get<std::string>());
    tx->setFrom(json["from"].get<std::string>());
    tx->setTo(json["to"].get<std::string>());
    tx->setValue(json["value"].get<double>());
    tx->setTimestamp(std::chrono::system_clock::from_time_t(json["timestamp"].get<time_t>()));
    tx->setStatus(json["status"].get<std::string>());
    tx->setMetadata(json["metadata"]);
    
    // Convert hex string to binary data
    std::string hex_data = json["data"].get<std::string>();
    std::vector<uint8_t> data;
    for (size_t i = 0; i < hex_data.length(); i += 2) {
        std::string byteString = hex_data.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
        data.push_back(byte);
    }
    tx->setData(data);
    
    return tx;
}

std::vector<uint8_t> Transaction::getRaw() const {
    // Create a JSON object with all transaction data
    nlohmann::json tx_json;
    tx_json["from"] = from_;
    tx_json["to"] = to_;
    tx_json["value"] = value_;
    tx_json["nonce"] = nonce_;
    tx_json["gasPrice"] = gas_price_;
    tx_json["gasLimit"] = gas_limit_;
    tx_json["data"] = data_;
    tx_json["timestamp"] = std::chrono::system_clock::to_time_t(timestamp_);
    tx_json["metadata"] = metadata_;

    // Convert JSON to string
    std::string json_str = tx_json.dump();

    // Convert string to binary
    std::vector<uint8_t> raw_data(json_str.begin(), json_str.end());
    return raw_data;
}

} // namespace blockchain
} // namespace satox 