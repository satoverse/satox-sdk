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

class MockAssetManager {
public:
    MOCK_METHOD(bool, initialize, (), ());
    MOCK_METHOD(bool, shutdown, (), ());
    
    // Asset operations
    MOCK_METHOD(bool, createAsset, (const std::string& name, const std::string& symbol, uint64_t amount, const std::string& owner, std::string& assetId), ());
    MOCK_METHOD(bool, getAsset, (const std::string& assetId, nlohmann::json& asset), ());
    MOCK_METHOD(bool, listAssets, (std::vector<nlohmann::json>& assets), ());
    MOCK_METHOD(bool, updateAsset, (const std::string& assetId, const nlohmann::json& updates), ());
    MOCK_METHOD(bool, deleteAsset, (const std::string& assetId), ());
    
    // Asset transfer operations
    MOCK_METHOD(bool, transferAsset, (const std::string& assetId, const std::string& from, const std::string& to, uint64_t amount, std::string& txid), ());
    MOCK_METHOD(bool, getTransferHistory, (const std::string& assetId, std::vector<nlohmann::json>& history), ());
    MOCK_METHOD(bool, getTransferStatus, (const std::string& txid, nlohmann::json& status), ());
    
    // Asset balance operations
    MOCK_METHOD(bool, getBalance, (const std::string& assetId, const std::string& address, uint64_t& balance), ());
    MOCK_METHOD(bool, getTotalSupply, (const std::string& assetId, uint64_t& supply), ());
    MOCK_METHOD(bool, getHolders, (const std::string& assetId, std::vector<nlohmann::json>& holders), ());
    
    // Asset metadata operations
    MOCK_METHOD(bool, setMetadata, (const std::string& assetId, const nlohmann::json& metadata), ());
    MOCK_METHOD(bool, getMetadata, (const std::string& assetId, nlohmann::json& metadata), ());
    MOCK_METHOD(bool, updateMetadata, (const std::string& assetId, const nlohmann::json& updates), ());
    
    // Asset permission operations
    MOCK_METHOD(bool, setPermissions, (const std::string& assetId, const nlohmann::json& permissions), ());
    MOCK_METHOD(bool, getPermissions, (const std::string& assetId, nlohmann::json& permissions), ());
    MOCK_METHOD(bool, checkPermission, (const std::string& assetId, const std::string& address, const std::string& permission, bool& allowed), ());
    
    // Helper methods for test setup
    void setMockAsset(const std::string& assetId, const nlohmann::json& asset) {
        ON_CALL(*this, getAsset(assetId, testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<1>(asset),
                testing::Return(true)
            ));
    }
    
    void setMockCreateAsset(const std::string& name, const std::string& symbol, uint64_t amount, const std::string& owner, const std::string& assetId) {
        ON_CALL(*this, createAsset(name, symbol, amount, owner, testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<4>(assetId),
                testing::Return(true)
            ));
    }
    
    void setMockTransferAsset(const std::string& assetId, const std::string& from, const std::string& to, uint64_t amount, const std::string& txid) {
        ON_CALL(*this, transferAsset(assetId, from, to, amount, testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<4>(txid),
                testing::Return(true)
            ));
    }
    
    void setMockBalance(const std::string& assetId, const std::string& address, uint64_t balance) {
        ON_CALL(*this, getBalance(assetId, address, testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<2>(balance),
                testing::Return(true)
            ));
    }
    
    void setMockMetadata(const std::string& assetId, const nlohmann::json& metadata) {
        ON_CALL(*this, getMetadata(assetId, testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<1>(metadata),
                testing::Return(true)
            ));
    }
    
    void setMockPermissions(const std::string& assetId, const nlohmann::json& permissions) {
        ON_CALL(*this, getPermissions(assetId, testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<1>(permissions),
                testing::Return(true)
            ));
    }
};

} // namespace test
} // namespace satox::wallet 