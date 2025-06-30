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

class MockHardwareWallet {
public:
    MOCK_METHOD(bool, connect, (), ());
    MOCK_METHOD(bool, disconnect, (), ());
    MOCK_METHOD(bool, isConnected, (), (const));
    
    MOCK_METHOD(bool, getPublicKey, (const std::string& path, std::vector<uint8_t>& publicKey), ());
    MOCK_METHOD(bool, signTransaction, (const std::vector<uint8_t>& txHash, const std::string& path, std::vector<uint8_t>& signature), ());
    MOCK_METHOD(bool, signMessage, (const std::vector<uint8_t>& message, const std::string& path, std::vector<uint8_t>& signature), ());
    
    MOCK_METHOD(bool, getAddress, (const std::string& path, std::string& address), ());
    MOCK_METHOD(bool, getXPub, (const std::string& path, std::string& xpub), ());
    
    MOCK_METHOD(bool, verifyPin, (const std::string& pin), ());
    MOCK_METHOD(bool, changePin, (const std::string& oldPin, const std::string& newPin), ());
    
    MOCK_METHOD(bool, getDeviceInfo, (nlohmann::json& info), ());
    MOCK_METHOD(bool, getFirmwareVersion, (std::string& version), ());
    
    // Helper methods for test setup
    void setMockGetPublicKey(const std::string& path, const std::vector<uint8_t>& publicKey) {
        ON_CALL(*this, getPublicKey(path, testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<1>(publicKey),
                testing::Return(true)
            ));
    }
    
    void setMockSignTransaction(const std::vector<uint8_t>& txHash, const std::string& path, const std::vector<uint8_t>& signature) {
        ON_CALL(*this, signTransaction(txHash, path, testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<2>(signature),
                testing::Return(true)
            ));
    }
    
    void setMockSignMessage(const std::vector<uint8_t>& message, const std::string& path, const std::vector<uint8_t>& signature) {
        ON_CALL(*this, signMessage(message, path, testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<2>(signature),
                testing::Return(true)
            ));
    }
    
    void setMockGetAddress(const std::string& path, const std::string& address) {
        ON_CALL(*this, getAddress(path, testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<1>(address),
                testing::Return(true)
            ));
    }
    
    void setMockGetXPub(const std::string& path, const std::string& xpub) {
        ON_CALL(*this, getXPub(path, testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<1>(xpub),
                testing::Return(true)
            ));
    }
    
    void setMockVerifyPin(const std::string& pin, bool success) {
        ON_CALL(*this, verifyPin(pin))
            .WillByDefault(testing::Return(success));
    }
    
    void setMockChangePin(const std::string& oldPin, const std::string& newPin, bool success) {
        ON_CALL(*this, changePin(oldPin, newPin))
            .WillByDefault(testing::Return(success));
    }
    
    void setMockGetDeviceInfo(const nlohmann::json& info) {
        ON_CALL(*this, getDeviceInfo(testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<0>(info),
                testing::Return(true)
            ));
    }
    
    void setMockGetFirmwareVersion(const std::string& version) {
        ON_CALL(*this, getFirmwareVersion(testing::_))
            .WillByDefault(testing::DoAll(
                testing::SetArgReferee<0>(version),
                testing::Return(true)
            ));
    }
};

} // namespace test
} // namespace satox::wallet 