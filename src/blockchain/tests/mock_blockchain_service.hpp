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
#include <memory>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "satox/blockchain/block.hpp"
#include "satox/blockchain/transaction.hpp"

namespace satox::blockchain::testing {

class MockBlockchainService {
public:
    static MockBlockchainService& getInstance() {
        static MockBlockchainService instance;
        return instance;
    }

    nlohmann::json handleRPCCall(const std::string& method, const std::vector<std::string>& params) {
        if (method == "getblockcount") {
            return "1";
        } else if (method == "getblock") {
            return createMockBlock();
        } else if (method == "getblockhash") {
            return "0x123";
        } else if (method == "getrawtransaction") {
            return createMockTransaction();
        } else if (method == "getbalance") {
            return "1000.00000000"; // 1000 SATOX
        } else if (method == "gettransactioncount") {
            return "1";
        } else if (method == "addnode") {
            return true;
        } else if (method == "getpeerinfo") {
            return nlohmann::json::array({
                {{"addr", params[0]}}
            });
        } else if (method == "disconnectnode") {
            return true;
        } else if (method == "getnetworkinfo") {
            return {{"version", 1}};
        } else if (method == "getblockchaininfo") {
            return {{"chain", "main"}};
        } else if (method == "estimatesmartfee") {
            return {{"feerate", 0.0001}};
        }
        return nullptr;
    }

private:
    MockBlockchainService() = default;
    ~MockBlockchainService() = default;
    MockBlockchainService(const MockBlockchainService&) = delete;
    MockBlockchainService& operator=(const MockBlockchainService&) = delete;

    nlohmann::json createMockBlock() {
        nlohmann::json block;
        block["hash"] = "0x123";
        block["previousblockhash"] = "0x456";
        block["time"] = std::time(nullptr);
        block["nonce"] = 1;
        block["transactions"] = nlohmann::json::array();
        block["size"] = 100;
        block["weight"] = 400;
        block["strippedsize"] = 100;
        block["merkleroot"] = "0x789";
        block["version"] = 1;
        block["bits"] = "1d00ffff";
        block["difficulty"] = 1.0;
        return block;
    }

    nlohmann::json createMockTransaction() {
        nlohmann::json tx;
        tx["txid"] = "0xabc";
        tx["vin"] = nlohmann::json::array();
        tx["vout"] = nlohmann::json::array();
        tx["amount"] = 1000.0;
        tx["fee"] = 0.0001;
        tx["confirmations"] = 1;
        tx["time"] = std::time(nullptr);
        tx["blockhash"] = "0x123";
        tx["blocktime"] = std::time(nullptr);
        tx["version"] = 1;
        tx["size"] = 100;
        tx["weight"] = 400;
        return tx;
    }
};

} // namespace satox::blockchain::testing 