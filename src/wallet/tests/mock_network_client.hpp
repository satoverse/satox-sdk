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
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>

namespace satox::wallet {
namespace test {

class MockNetworkClient {
public:
    MOCK_METHOD(bool, connect, (const std::string& url, int port), ());
    MOCK_METHOD(bool, disconnect, (), ());
    MOCK_METHOD(bool, isConnected, (), (const));
    
    MOCK_METHOD(bool, getBlockchainInfo, (nlohmann::json& info), ());
    MOCK_METHOD(bool, getNetworkInfo, (nlohmann::json& info), ());
    MOCK_METHOD(bool, getBlock, (const std::string& hash, nlohmann::json& block), ());
    MOCK_METHOD(bool, getBlockByHeight, (int64_t height, nlohmann::json& block), ());
    MOCK_METHOD(bool, getTransaction, (const std::string& txid, nlohmann::json& tx), ());
    
    MOCK_METHOD(bool, broadcastTransaction, (const std::string& txHex), ());
    MOCK_METHOD(bool, createAsset, (const nlohmann::json& asset), ());
    MOCK_METHOD(bool, transferAsset, (const nlohmann::json& transfer), ());
    
    MOCK_METHOD(bool, getBalance, (const std::string& address, uint64_t& balance), ());
    MOCK_METHOD(bool, getAssetBalance, (const std::string& address, const std::string& asset, uint64_t& balance), ());
    
    // Helper methods for test setup
    void setMockBlockchainInfo(const nlohmann::json& info) {
        ON_CALL(*this, getBlockchainInfo(testing::_))
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
    
    void setMockBlock(const std::string& hash, const nlohmann::json& block) {
        ON_CALL(*this, getBlock(hash, testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<1>(block),
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
    
    void setMockBalance(const std::string& address, uint64_t balance) {
        ON_CALL(*this, getBalance(address, testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<1>(balance),
                testing::Return(true)
            ));
    }
    
    void setMockAssetBalance(const std::string& address, const std::string& asset, uint64_t balance) {
        ON_CALL(*this, getAssetBalance(address, asset, testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<2>(balance),
                testing::Return(true)
            ));
    }
};

} // namespace test
} // namespace satox::wallet 