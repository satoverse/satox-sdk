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

namespace satox::wallet {
namespace test {

class MockIPFSClient {
public:
    MOCK_METHOD(bool, connect, (const std::string& url, int port), ());
    MOCK_METHOD(bool, disconnect, (), ());
    MOCK_METHOD(bool, isConnected, (), (const));
    
    MOCK_METHOD(bool, add, (const std::vector<uint8_t>& data, std::string& cid), ());
    MOCK_METHOD(bool, get, (const std::string& cid, std::vector<uint8_t>& data), ());
    MOCK_METHOD(bool, pin, (const std::string& cid), ());
    MOCK_METHOD(bool, unpin, (const std::string& cid), ());
    MOCK_METHOD(bool, isPinned, (const std::string& cid, bool& pinned), ());
    
    MOCK_METHOD(bool, addWithMetadata, (const std::vector<uint8_t>& data, const nlohmann::json& metadata, std::string& cid), ());
    MOCK_METHOD(bool, getMetadata, (const std::string& cid, nlohmann::json& metadata), ());
    
    // Helper methods for test setup
    void setMockAdd(const std::vector<uint8_t>& data, const std::string& cid) {
        ON_CALL(*this, add(data, testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<1>(cid),
                testing::Return(true)
            ));
    }
    
    void setMockGet(const std::string& cid, const std::vector<uint8_t>& data) {
        ON_CALL(*this, get(cid, testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<1>(data),
                testing::Return(true)
            ));
    }
    
    void setMockPin(const std::string& cid, bool success) {
        ON_CALL(*this, pin(cid))
            .WillByDefault(testing::Return(success));
    }
    
    void setMockUnpin(const std::string& cid, bool success) {
        ON_CALL(*this, unpin(cid))
            .WillByDefault(testing::Return(success));
    }
    
    void setMockIsPinned(const std::string& cid, bool pinned) {
        ON_CALL(*this, isPinned(cid, testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<1>(pinned),
                testing::Return(true)
            ));
    }
    
    void setMockAddWithMetadata(const std::vector<uint8_t>& data, const nlohmann::json& metadata, const std::string& cid) {
        ON_CALL(*this, addWithMetadata(data, metadata, testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<2>(cid),
                testing::Return(true)
            ));
    }
    
    void setMockGetMetadata(const std::string& cid, const nlohmann::json& metadata) {
        ON_CALL(*this, getMetadata(cid, testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<1>(metadata),
                testing::Return(true)
            ));
    }
};

} // namespace test
} // namespace satox::wallet 