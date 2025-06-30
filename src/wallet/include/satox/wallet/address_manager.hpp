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
#include <memory>
#include <mutex>
#include <map>
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>
#include <openssl/ripemd.h>
#include <openssl/sha.h>
#include <nlohmann/json.hpp>

namespace satox {
namespace wallet {

class AddressManager {
public:
    static AddressManager& getInstance();

    // Prevent copying
    AddressManager(const AddressManager&) = delete;
    AddressManager& operator=(const AddressManager&) = delete;

    // Initialization and cleanup
    bool initialize();
    void shutdown();

    // Address generation
    std::string generateAddress(const std::vector<uint8_t>& publicKey);
    std::string generateSegWitAddress(const std::vector<uint8_t>& publicKey);
    std::string generateMultiSigAddress(const std::vector<std::vector<uint8_t>>& publicKeys, int requiredSignatures);

    // Address validation
    bool validateAddress(const std::string& address);
    bool validateSegWitAddress(const std::string& address);
    bool validateMultiSigAddress(const std::string& address);

    // Address conversion
    std::string toLegacyAddress(const std::string& address);
    std::string toSegWitAddress(const std::string& address);
    std::string toMultiSigAddress(const std::string& address);

    // Address information
    struct AddressInfo {
        std::string address;
        std::string type;  // "legacy", "segwit", "multisig"
        std::vector<uint8_t> scriptHash;
        std::vector<uint8_t> redeemScript;  // For multi-sig
        int requiredSignatures;  // For multi-sig
    };
    AddressInfo getAddressInfo(const std::string& address);

    // Address derivation
    std::string deriveAddress(const std::vector<uint8_t>& publicKey, uint32_t purpose, uint32_t coinType, uint32_t account, uint32_t change, uint32_t addressIndex);
    std::string deriveSegWitAddress(const std::vector<uint8_t>& publicKey, uint32_t purpose, uint32_t coinType, uint32_t account, uint32_t change, uint32_t addressIndex);
    std::string deriveMultiSigAddress(const std::vector<std::vector<uint8_t>>& publicKeys, int requiredSignatures, uint32_t purpose, uint32_t coinType, uint32_t account, uint32_t change, uint32_t addressIndex);

    // Address caching
    void cacheAddress(const std::string& address, const AddressInfo& info);
    void removeCachedAddress(const std::string& address);
    AddressInfo getCachedAddress(const std::string& address);
    void clearAddressCache();

    // Error handling
    struct Error {
        int code;
        std::string message;
    };
    Error getLastError() const;
    void clearLastError();

private:
    AddressManager() = default;
    ~AddressManager() = default;

    // Internal helper methods
    std::vector<uint8_t> hash160(const std::vector<uint8_t>& data);
    std::vector<uint8_t> sha256(const std::vector<uint8_t>& data);
    std::vector<uint8_t> ripemd160(const std::vector<uint8_t>& data);
    std::vector<uint8_t> doubleSha256(const std::vector<uint8_t>& data);
    std::string base58Encode(const std::vector<uint8_t>& data);
    std::vector<uint8_t> base58Decode(const std::string& str);
    std::string bech32Encode(const std::vector<uint8_t>& data, const std::string& hrp);
    std::vector<uint8_t> bech32Decode(const std::string& str, std::string& hrp);
    std::vector<uint8_t> createRedeemScript(const std::vector<std::vector<uint8_t>>& publicKeys, int requiredSignatures);

    // Member variables
    std::mutex mutex_;
    std::map<std::string, AddressInfo> addressCache_;
    Error lastError_;
    bool initialized_ = false;
};

} // namespace wallet
} // namespace satox 