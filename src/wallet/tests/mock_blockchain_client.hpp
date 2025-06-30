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

#include <gmock/gmock.h>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <nlohmann/json.hpp>

namespace satox::wallet {
namespace test {

class MockBlockchainClient {
public:
    MOCK_METHOD(bool, connect, (const std::string& url, int port), ());
    MOCK_METHOD(bool, disconnect, (), ());
    MOCK_METHOD(bool, isConnected, (), (const));
    
    // Block operations
    MOCK_METHOD(bool, getBlock, (const std::string& hash, nlohmann::json& block), ());
    MOCK_METHOD(bool, getBlockByHeight, (int64_t height, nlohmann::json& block), ());
    MOCK_METHOD(bool, getLatestBlock, (nlohmann::json& block), ());
    MOCK_METHOD(bool, getBlockCount, (int64_t& count), ());
    
    // Transaction operations
    MOCK_METHOD(bool, getTransaction, (const std::string& txid, nlohmann::json& tx), ());
    MOCK_METHOD(bool, sendTransaction, (const std::string& txHex, std::string& txid), ());
    MOCK_METHOD(bool, getTransactionStatus, (const std::string& txid, nlohmann::json& status), ());
    MOCK_METHOD(bool, getTransactionConfirmations, (const std::string& txid, int& confirmations), ());
    
    // Address operations
    MOCK_METHOD(bool, getBalance, (const std::string& address, uint64_t& balance), ());
    MOCK_METHOD(bool, getUnspentOutputs, (const std::string& address, std::vector<nlohmann::json>& outputs), ());
    MOCK_METHOD(bool, getAddressHistory, (const std::string& address, std::vector<nlohmann::json>& history), ());
    
    // Asset operations
    MOCK_METHOD(bool, getAsset, (const std::string& assetId, nlohmann::json& asset), ());
    MOCK_METHOD(bool, createAsset, (const nlohmann::json& asset, std::string& assetId), ());
    MOCK_METHOD(bool, transferAsset, (const nlohmann::json& transfer, std::string& txid), ());
    MOCK_METHOD(bool, getAssetBalance, (const std::string& address, const std::string& assetId, uint64_t& balance), ());
    
    // Mining operations
    MOCK_METHOD(bool, getMiningInfo, (nlohmann::json& info), ());
    MOCK_METHOD(bool, submitBlock, (const std::string& blockHex), ());
    MOCK_METHOD(bool, getBlockTemplate, (nlohmann::json& template), ());
    
    // Network operations
    MOCK_METHOD(bool, getNetworkInfo, (nlohmann::json& info), ());
    MOCK_METHOD(bool, getPeerInfo, (std::vector<nlohmann::json>& peers), ());
    MOCK_METHOD(bool, getMempoolInfo, (nlohmann::json& info), ());
    
    // Helper methods for test setup
    void setMockBlock(const std::string& hash, const nlohmann::json& block) {
        ON_CALL(*this, getBlock(hash, testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<1>(block),
                testing::Return(true)
            ));
    }
    
    void setMockBlockByHeight(int64_t height, const nlohmann::json& block) {
        ON_CALL(*this, getBlockByHeight(height, testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<1>(block),
                testing::Return(true)
            ));
    }
    
    void setMockLatestBlock(const nlohmann::json& block) {
        ON_CALL(*this, getLatestBlock(testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<0>(block),
                testing::Return(true)
            ));
    }
    
    void setMockTransaction(const std::string& txid, const nlohmann::json& tx) {
        ON_CALL(*this, getTransaction(txid, testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<1>(tx),
                testing::Return(true)
            ));
    }
    
    void setMockSendTransaction(const std::string& txHex, const std::string& txid) {
        ON_CALL(*this, sendTransaction(txHex, testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<1>(txid),
                testing::Return(true)
            ));
    }
    
    void setMockBalance(const std::string& address, uint64_t balance) {
        ON_CALL(*this, getBalance(address, testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<1>(balance),
                testing::Return(true)
            ));
    }
    
    void setMockAsset(const std::string& assetId, const nlohmann::json& asset) {
        ON_CALL(*this, getAsset(assetId, testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<1>(asset),
                testing::Return(true)
            ));
    }
    
    void setMockCreateAsset(const nlohmann::json& asset, const std::string& assetId) {
        ON_CALL(*this, createAsset(asset, testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<1>(assetId),
                testing::Return(true)
            ));
    }
    
    void setMockMiningInfo(const nlohmann::json& info) {
        ON_CALL(*this, getMiningInfo(testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<0>(info),
                testing::Return(true)
            ));
    }
    
    void setMockNetworkInfo(const nlohmann::json& info) {
        ON_CALL(*this, getNetworkInfo(testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<0>(info),
                testing::Return(true)
            ));
    }
};

} // namespace test
} // namespace satox::wallet 