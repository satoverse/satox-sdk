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
#include <unordered_map>
#include <chrono>
#include <nlohmann/json.hpp>

namespace satox {
namespace database {

// Base class for all data models
class DataModel {
public:
    virtual ~DataModel() = default;
    virtual nlohmann::json toJson() const = 0;
    virtual void fromJson(const nlohmann::json& json) = 0;
    virtual std::string getTableName() const = 0;
};

// Block model
class Block : public DataModel {
public:
    std::string hash;
    std::string previousHash;
    uint64_t height;
    uint64_t timestamp;
    std::string miner;
    uint64_t difficulty;
    std::string merkleRoot;
    std::vector<std::string> transactions;
    uint64_t size;
    uint64_t weight;
    std::string version;
    std::string bits;
    uint64_t nonce;
    std::string chainwork;
    bool isMainChain;

    nlohmann::json toJson() const override {
        nlohmann::json j;
        j["hash"] = hash;
        j["previousHash"] = previousHash;
        j["height"] = height;
        j["timestamp"] = timestamp;
        j["miner"] = miner;
        j["difficulty"] = difficulty;
        j["merkleRoot"] = merkleRoot;
        j["transactions"] = transactions;
        j["size"] = size;
        j["weight"] = weight;
        j["version"] = version;
        j["bits"] = bits;
        j["nonce"] = nonce;
        j["chainwork"] = chainwork;
        j["isMainChain"] = isMainChain;
        return j;
    }

    void fromJson(const nlohmann::json& j) override {
        hash = j["hash"];
        previousHash = j["previousHash"];
        height = j["height"];
        timestamp = j["timestamp"];
        miner = j["miner"];
        difficulty = j["difficulty"];
        merkleRoot = j["merkleRoot"];
        transactions = j["transactions"].get<std::vector<std::string>>();
        size = j["size"];
        weight = j["weight"];
        version = j["version"];
        bits = j["bits"];
        nonce = j["nonce"];
        chainwork = j["chainwork"];
        isMainChain = j["isMainChain"];
    }

    std::string getTableName() const override { return "blocks"; }
};

// Transaction model
class Transaction : public DataModel {
public:
    std::string txid;
    uint64_t version;
    uint64_t locktime;
    std::vector<nlohmann::json> inputs;
    std::vector<nlohmann::json> outputs;
    uint64_t size;
    uint64_t weight;
    uint64_t fee;
    std::string blockHash;
    uint64_t blockHeight;
    uint64_t timestamp;
    bool isCoinbase;
    std::string status;

    nlohmann::json toJson() const override {
        nlohmann::json j;
        j["txid"] = txid;
        j["version"] = version;
        j["locktime"] = locktime;
        j["inputs"] = inputs;
        j["outputs"] = outputs;
        j["size"] = size;
        j["weight"] = weight;
        j["fee"] = fee;
        j["blockHash"] = blockHash;
        j["blockHeight"] = blockHeight;
        j["timestamp"] = timestamp;
        j["isCoinbase"] = isCoinbase;
        j["status"] = status;
        return j;
    }

    void fromJson(const nlohmann::json& j) override {
        txid = j["txid"];
        version = j["version"];
        locktime = j["locktime"];
        inputs = j["inputs"].get<std::vector<nlohmann::json>>();
        outputs = j["outputs"].get<std::vector<nlohmann::json>>();
        size = j["size"];
        weight = j["weight"];
        fee = j["fee"];
        blockHash = j["blockHash"];
        blockHeight = j["blockHeight"];
        timestamp = j["timestamp"];
        isCoinbase = j["isCoinbase"];
        status = j["status"];
    }

    std::string getTableName() const override { return "transactions"; }
};

// Asset model
class Asset : public DataModel {
public:
    std::string id;
    std::string name;
    std::string symbol;
    std::string owner;
    uint64_t totalSupply;
    uint8_t decimals;
    std::string status;
    nlohmann::json metadata;
    uint64_t createdAt;
    uint64_t updatedAt;

    nlohmann::json toJson() const override {
        nlohmann::json j;
        j["id"] = id;
        j["name"] = name;
        j["symbol"] = symbol;
        j["owner"] = owner;
        j["totalSupply"] = totalSupply;
        j["decimals"] = decimals;
        j["status"] = status;
        j["metadata"] = metadata;
        j["createdAt"] = createdAt;
        j["updatedAt"] = updatedAt;
        return j;
    }

    void fromJson(const nlohmann::json& j) override {
        id = j["id"];
        name = j["name"];
        symbol = j["symbol"];
        owner = j["owner"];
        totalSupply = j["totalSupply"];
        decimals = j["decimals"];
        status = j["status"];
        metadata = j["metadata"];
        createdAt = j["createdAt"];
        updatedAt = j["updatedAt"];
    }

    std::string getTableName() const override { return "assets"; }
};

// NFT model
class NFT : public DataModel {
public:
    std::string id;
    std::string assetId;
    std::string owner;
    std::string creator;
    nlohmann::json metadata;
    std::string status;
    uint64_t createdAt;
    uint64_t updatedAt;
    std::string tokenURI;
    std::vector<std::string> history;

    nlohmann::json toJson() const override {
        nlohmann::json j;
        j["id"] = id;
        j["assetId"] = assetId;
        j["owner"] = owner;
        j["creator"] = creator;
        j["metadata"] = metadata;
        j["status"] = status;
        j["createdAt"] = createdAt;
        j["updatedAt"] = updatedAt;
        j["tokenURI"] = tokenURI;
        j["history"] = history;
        return j;
    }

    void fromJson(const nlohmann::json& j) override {
        id = j["id"];
        assetId = j["assetId"];
        owner = j["owner"];
        creator = j["creator"];
        metadata = j["metadata"];
        status = j["status"];
        createdAt = j["createdAt"];
        updatedAt = j["updatedAt"];
        tokenURI = j["tokenURI"];
        history = j["history"].get<std::vector<std::string>>();
    }

    std::string getTableName() const override { return "nfts"; }
};

// Wallet model
class Wallet : public DataModel {
public:
    std::string id;
    std::string address;
    std::string type;
    std::string status;
    nlohmann::json metadata;
    uint64_t createdAt;
    uint64_t updatedAt;
    std::vector<std::string> assets;
    std::vector<std::string> nfts;

    nlohmann::json toJson() const override {
        nlohmann::json j;
        j["id"] = id;
        j["address"] = address;
        j["type"] = type;
        j["status"] = status;
        j["metadata"] = metadata;
        j["createdAt"] = createdAt;
        j["updatedAt"] = updatedAt;
        j["assets"] = assets;
        j["nfts"] = nfts;
        return j;
    }

    void fromJson(const nlohmann::json& j) override {
        id = j["id"];
        address = j["address"];
        type = j["type"];
        status = j["status"];
        metadata = j["metadata"];
        createdAt = j["createdAt"];
        updatedAt = j["updatedAt"];
        assets = j["assets"].get<std::vector<std::string>>();
        nfts = j["nfts"].get<std::vector<std::string>>();
    }

    std::string getTableName() const override { return "wallets"; }
};

// IPFS file model
class IPFSFile : public DataModel {
public:
    std::string hash;
    std::string name;
    std::string mimeType;
    uint64_t size;
    std::string status;
    nlohmann::json metadata;
    uint64_t createdAt;
    uint64_t updatedAt;
    std::vector<std::string> pins;
    std::string content;

    nlohmann::json toJson() const override {
        nlohmann::json j;
        j["hash"] = hash;
        j["name"] = name;
        j["mimeType"] = mimeType;
        j["size"] = size;
        j["status"] = status;
        j["metadata"] = metadata;
        j["createdAt"] = createdAt;
        j["updatedAt"] = updatedAt;
        j["pins"] = pins;
        j["content"] = content;
        return j;
    }

    void fromJson(const nlohmann::json& j) override {
        hash = j["hash"];
        name = j["name"];
        mimeType = j["mimeType"];
        size = j["size"];
        status = j["status"];
        metadata = j["metadata"];
        createdAt = j["createdAt"];
        updatedAt = j["updatedAt"];
        pins = j["pins"].get<std::vector<std::string>>();
        content = j["content"];
    }

    std::string getTableName() const override { return "ipfs_files"; }
};

} // namespace database
} // namespace satox 