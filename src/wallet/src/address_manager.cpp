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

#include <openssl/evp.h>
#include "satox/wallet/address_manager.hpp"
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>
#include <stdexcept>
#include <sstream>
#include <iomanip>

namespace satox {
namespace wallet {

AddressManager& AddressManager::getInstance() {
    static AddressManager instance;
    return instance;
}

bool AddressManager::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) {
        return true;
    }

    // Initialize OpenSSL
    OpenSSL_add_all_algorithms();

    initialized_ = true;
    return true;
}

void AddressManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        return;
    }

    // Cleanup OpenSSL
    EVP_cleanup();

    initialized_ = false;
}

std::string AddressManager::generateAddress(const std::vector<uint8_t>& publicKey) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = {1, "AddressManager not initialized"};
        return "";
    }

    try {
        // Hash the public key
        auto hash = hash160(publicKey);

        // Add version byte (0x00 for mainnet)
        std::vector<uint8_t> versionedHash = {0x00};
        versionedHash.insert(versionedHash.end(), hash.begin(), hash.end());

        // Double SHA256 for checksum
        auto checksum = doubleSha256(versionedHash);
        checksum.resize(4);  // Take first 4 bytes

        // Combine version + hash + checksum
        std::vector<uint8_t> finalData = versionedHash;
        finalData.insert(finalData.end(), checksum.begin(), checksum.end());

        // Base58 encode
        return base58Encode(finalData);
    } catch (const std::exception& e) {
        lastError_ = {2, std::string("Failed to generate address: ") + e.what()};
        return "";
    }
}

std::string AddressManager::generateSegWitAddress(const std::vector<uint8_t>& publicKey) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = {1, "AddressManager not initialized"};
        return "";
    }

    try {
        // Hash the public key
        auto hash = hash160(publicKey);

        // Create witness program (version 0 + hash)
        std::vector<uint8_t> witnessProgram = {0x00, static_cast<uint8_t>(hash.size())};
        witnessProgram.insert(witnessProgram.end(), hash.begin(), hash.end());

        // Bech32 encode
        return bech32Encode(witnessProgram, "bc");  // Use "bc" for mainnet
    } catch (const std::exception& e) {
        lastError_ = {3, std::string("Failed to generate SegWit address: ") + e.what()};
        return "";
    }
}

std::string AddressManager::generateMultiSigAddress(const std::vector<std::vector<uint8_t>>& publicKeys, int requiredSignatures) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = {1, "AddressManager not initialized"};
        return "";
    }

    try {
        // Create redeem script
        auto redeemScript = createRedeemScript(publicKeys, requiredSignatures);

        // Hash the redeem script
        auto hash = hash160(redeemScript);

        // Add version byte (0x05 for mainnet P2SH)
        std::vector<uint8_t> versionedHash = {0x05};
        versionedHash.insert(versionedHash.end(), hash.begin(), hash.end());

        // Double SHA256 for checksum
        auto checksum = doubleSha256(versionedHash);
        checksum.resize(4);  // Take first 4 bytes

        // Combine version + hash + checksum
        std::vector<uint8_t> finalData = versionedHash;
        finalData.insert(finalData.end(), checksum.begin(), checksum.end());

        // Base58 encode
        return base58Encode(finalData);
    } catch (const std::exception& e) {
        lastError_ = {4, std::string("Failed to generate multi-sig address: ") + e.what()};
        return "";
    }
}

bool AddressManager::validateAddress(const std::string& address) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = {1, "AddressManager not initialized"};
        return false;
    }

    try {
        // Decode base58
        auto decoded = base58Decode(address);
        if (decoded.size() != 25) {  // 1 byte version + 20 bytes hash + 4 bytes checksum
            return false;
        }

        // Verify checksum
        std::vector<uint8_t> data(decoded.begin(), decoded.end() - 4);
        auto checksum = doubleSha256(data);
        checksum.resize(4);
        return std::equal(checksum.begin(), checksum.end(), decoded.end() - 4);
    } catch (const std::exception& e) {
        lastError_ = {5, std::string("Failed to validate address: ") + e.what()};
        return false;
    }
}

bool AddressManager::validateSegWitAddress(const std::string& address) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = {1, "AddressManager not initialized"};
        return false;
    }

    try {
        std::string hrp;
        auto decoded = bech32Decode(address, hrp);
        return !decoded.empty() && hrp == "bc";  // Check for mainnet prefix
    } catch (const std::exception& e) {
        lastError_ = {6, std::string("Failed to validate SegWit address: ") + e.what()};
        return false;
    }
}

bool AddressManager::validateMultiSigAddress(const std::string& address) {
    return validateAddress(address);  // Multi-sig addresses use the same format as P2SH
}

std::string AddressManager::toLegacyAddress(const std::string& address) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = {1, "AddressManager not initialized"};
        return "";
    }

    try {
        if (validateSegWitAddress(address)) {
            std::string hrp;
            auto witnessProgram = bech32Decode(address, hrp);
            if (witnessProgram.size() < 2) {
                throw std::runtime_error("Invalid witness program");
            }

            // Extract the hash from the witness program
            std::vector<uint8_t> hash(witnessProgram.begin() + 2, witnessProgram.end());

            // Create legacy address
            std::vector<uint8_t> versionedHash = {0x00};  // Mainnet
            versionedHash.insert(versionedHash.end(), hash.begin(), hash.end());

            auto checksum = doubleSha256(versionedHash);
            checksum.resize(4);

            std::vector<uint8_t> finalData = versionedHash;
            finalData.insert(finalData.end(), checksum.begin(), checksum.end());

            return base58Encode(finalData);
        }
        return address;  // Already a legacy address
    } catch (const std::exception& e) {
        lastError_ = {7, std::string("Failed to convert to legacy address: ") + e.what()};
        return "";
    }
}

std::string AddressManager::toSegWitAddress(const std::string& address) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = {1, "AddressManager not initialized"};
        return "";
    }

    try {
        if (validateAddress(address)) {
            auto decoded = base58Decode(address);
            if (decoded.size() != 25) {
                throw std::runtime_error("Invalid address length");
            }

            // Extract the hash
            std::vector<uint8_t> hash(decoded.begin() + 1, decoded.begin() + 21);

            // Create witness program
            std::vector<uint8_t> witnessProgram = {0x00, static_cast<uint8_t>(hash.size())};
            witnessProgram.insert(witnessProgram.end(), hash.begin(), hash.end());

            return bech32Encode(witnessProgram, "bc");
        }
        return address;  // Already a SegWit address
    } catch (const std::exception& e) {
        lastError_ = {8, std::string("Failed to convert to SegWit address: ") + e.what()};
        return "";
    }
}

std::string AddressManager::toMultiSigAddress(const std::string& address) {
    // This would require the public keys and required signatures
    // For now, we'll just return the original address
    return address;
}

AddressManager::AddressInfo AddressManager::getAddressInfo(const std::string& address) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        lastError_ = {1, "AddressManager not initialized"};
        return AddressInfo{};
    }

    try {
        // Check cache first
        auto it = addressCache_.find(address);
        if (it != addressCache_.end()) {
            return it->second;
        }

        AddressInfo info;
        info.address = address;

        if (validateSegWitAddress(address)) {
            info.type = "segwit";
            std::string hrp;
            auto witnessProgram = bech32Decode(address, hrp);
            info.scriptHash = std::vector<uint8_t>(witnessProgram.begin() + 2, witnessProgram.end());
        } else if (validateAddress(address)) {
            auto decoded = base58Decode(address);
            if (decoded[0] == 0x05) {  // P2SH
                info.type = "multisig";
                info.scriptHash = std::vector<uint8_t>(decoded.begin() + 1, decoded.begin() + 21);
            } else {
                info.type = "legacy";
                info.scriptHash = std::vector<uint8_t>(decoded.begin() + 1, decoded.begin() + 21);
            }
        }

        // Cache the result
        cacheAddress(address, info);
        return info;
    } catch (const std::exception& e) {
        lastError_ = {9, std::string("Failed to get address info: ") + e.what()};
        return AddressInfo{};
    }
}

std::string AddressManager::deriveAddress(const std::vector<uint8_t>& publicKey, uint32_t purpose, uint32_t coinType, uint32_t account, uint32_t change, uint32_t addressIndex) {
    return generateAddress(publicKey);  // For now, just generate a regular address
}

std::string AddressManager::deriveSegWitAddress(const std::vector<uint8_t>& publicKey, uint32_t purpose, uint32_t coinType, uint32_t account, uint32_t change, uint32_t addressIndex) {
    return generateSegWitAddress(publicKey);  // For now, just generate a regular SegWit address
}

std::string AddressManager::deriveMultiSigAddress(const std::vector<std::vector<uint8_t>>& publicKeys, int requiredSignatures, uint32_t purpose, uint32_t coinType, uint32_t account, uint32_t change, uint32_t addressIndex) {
    return generateMultiSigAddress(publicKeys, requiredSignatures);  // For now, just generate a regular multi-sig address
}

void AddressManager::cacheAddress(const std::string& address, const AddressInfo& info) {
    std::lock_guard<std::mutex> lock(mutex_);
    addressCache_[address] = info;
}

void AddressManager::removeCachedAddress(const std::string& address) {
    std::lock_guard<std::mutex> lock(mutex_);
    addressCache_.erase(address);
}

AddressManager::AddressInfo AddressManager::getCachedAddress(const std::string& address) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = addressCache_.find(address);
    if (it != addressCache_.end()) {
        return it->second;
    }
    return AddressInfo{};
}

void AddressManager::clearAddressCache() {
    std::lock_guard<std::mutex> lock(mutex_);
    addressCache_.clear();
}

AddressManager::Error AddressManager::getLastError() const {
    return lastError_;
}

void AddressManager::clearLastError() {
    lastError_ = {0, ""};
}

// Private helper methods
std::vector<uint8_t> AddressManager::hash160(const std::vector<uint8_t>& data) {
    auto sha256Hash = sha256(data);
    return ripemd160(sha256Hash);
}

std::vector<uint8_t> AddressManager::sha256(const std::vector<uint8_t>& data) {
    std::vector<uint8_t> hash(32);
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) throw std::runtime_error("EVP_MD_CTX_new failed");
    
    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1 ||
        EVP_DigestUpdate(ctx, data.data(), data.size()) != 1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("EVP sha256 failed");
    }
    
    unsigned int len = 0;
    if (EVP_DigestFinal_ex(ctx, hash.data(), &len) != 1 || len != 32) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("EVP sha256 final failed");
    }
    
    EVP_MD_CTX_free(ctx);
    EVP_MD_CTX_free(ctx);
    return hash;
}

std::vector<uint8_t> AddressManager::ripemd160(const std::vector<uint8_t>& data) {
    std::vector<uint8_t> hash(20);
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) throw std::runtime_error("EVP_MD_CTX_new failed");
    
    if (EVP_DigestInit_ex(ctx, EVP_ripemd160(), nullptr) != 1 ||
        EVP_DigestUpdate(ctx, data.data(), data.size()) != 1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("EVP ripemd160 failed");
    }
    
    unsigned int len = 0;
    if (EVP_DigestFinal_ex(ctx, hash.data(), &len) != 1 || len != 20) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("EVP ripemd160 final failed");
    }
    
    EVP_MD_CTX_free(ctx);
    EVP_MD_CTX_free(ctx);
    return hash;
}

std::vector<uint8_t> AddressManager::doubleSha256(const std::vector<uint8_t>& data) {
    auto firstHash = sha256(data);
    return sha256(firstHash);
}

std::string AddressManager::base58Encode(const std::vector<uint8_t>& data) {
    static const char* base58Chars = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
    std::string result;
    result.reserve(data.size() * 138 / 100 + 1);  // Maximum possible size

    // Convert to base58
    std::vector<uint8_t> digits((data.size() * 138 / 100) + 1);
    size_t digitsLen = 0;
    for (size_t i = 0; i < data.size(); i++) {
        uint32_t carry = data[i];
        for (size_t j = 0; j < digitsLen; j++) {
            carry += static_cast<uint32_t>(digits[j]) << 8;
            digits[j] = carry % 58;
            carry /= 58;
        }
        while (carry > 0) {
            digits[digitsLen++] = carry % 58;
            carry /= 58;
        }
    }

    // Skip leading zero bytes
    size_t zeroCount = 0;
    while (zeroCount < data.size() && data[zeroCount] == 0) {
        zeroCount++;
    }

    // Convert to string
    result.resize(zeroCount + digitsLen);
    for (size_t i = 0; i < zeroCount; i++) {
        result[i] = '1';
    }
    for (size_t i = 0; i < digitsLen; i++) {
        result[zeroCount + i] = base58Chars[digits[digitsLen - 1 - i]];
    }

    return result;
}

std::vector<uint8_t> AddressManager::base58Decode(const std::string& str) {
    static const int8_t base58Map[256] = {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,0,1,2,3,4,5,6,7,8,-1,-1,-1,-1,-1,-1,
        -1,9,10,11,12,13,14,15,16,-1,17,18,19,20,21,-1,
        22,23,24,25,26,27,28,29,30,31,32,-1,-1,-1,-1,-1,
        -1,33,34,35,36,37,38,39,40,41,42,43,-1,44,45,46,
        47,48,49,50,51,52,53,54,55,56,57,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    };

    // Skip leading spaces
    const char* psz = str.c_str();
    while (*psz && isspace(*psz)) {
        psz++;
    }

    // Skip and count leading '1's
    int zeroes = 0;
    int length = 0;
    while (*psz == '1') {
        zeroes++;
        psz++;
    }

    // Allocate enough space in big-endian base256 representation
    int size = strlen(psz) * 733 / 1000 + 1;  // log(58) / log(256), rounded up
    std::vector<uint8_t> b256(size);

    // Process the characters
    while (*psz && !isspace(*psz)) {
        // Decode base58 character
        int carry = base58Map[*psz];
        if (carry == -1) {
            throw std::runtime_error("Invalid base58 character");
        }
        int i = 0;
        for (std::vector<uint8_t>::reverse_iterator it = b256.rbegin(); (carry != 0 || i < length) && (it != b256.rend()); ++it, ++i) {
            carry += 58 * (*it);
            *it = carry % 256;
            carry /= 256;
        }
        length = i;
        psz++;
    }

    // Skip trailing spaces
    while (isspace(*psz)) {
        psz++;
    }
    if (*psz != 0) {
        throw std::runtime_error("Extra characters at end of base58 string");
    }

    // Skip leading zeroes in b256
    std::vector<uint8_t>::iterator it = b256.begin() + (size - length);
    while (it != b256.end() && *it == 0) {
        it++;
    }

    // Copy result into output vector
    std::vector<uint8_t> vch(zeroes + (b256.end() - it));
    vch.assign(zeroes, 0x00);
    while (it != b256.end()) {
        vch.push_back(*(it++));
    }

    return vch;
}

std::string AddressManager::bech32Encode(const std::vector<uint8_t>& data, const std::string& hrp) {
    // This is a simplified version. A full implementation would need to handle
    // the Bech32 checksum and proper encoding of the witness program.
    std::stringstream ss;
    ss << hrp << "1";
    for (uint8_t b : data) {
        ss << "qpzry9x8gf2tvdw0s3jn54khce6mua7l"[b % 32];
    }
    return ss.str();
}

std::vector<uint8_t> AddressManager::bech32Decode(const std::string& str, std::string& hrp) {
    // This is a simplified version. A full implementation would need to handle
    // the Bech32 checksum and proper decoding of the witness program.
    size_t pos = str.find_last_of('1');
    if (pos == std::string::npos || pos == 0 || pos + 7 > str.size()) {
        return {};
    }

    hrp = str.substr(0, pos);
    std::vector<uint8_t> data;
    for (size_t i = pos + 1; i < str.size(); i++) {
        const char* p = strchr("qpzry9x8gf2tvdw0s3jn54khce6mua7l", str[i]);
        if (p == nullptr) {
            return {};
        }
        data.push_back(p - "qpzry9x8gf2tvdw0s3jn54khce6mua7l");
    }

    return data;
}

std::vector<uint8_t> AddressManager::createRedeemScript(const std::vector<std::vector<uint8_t>>& publicKeys, int requiredSignatures) {
    if (publicKeys.empty() || requiredSignatures <= 0 || requiredSignatures > publicKeys.size()) {
        throw std::runtime_error("Invalid public keys or required signatures");
    }

    std::vector<uint8_t> script;
    script.push_back(0x50 + requiredSignatures);  // OP_N + required signatures

    for (const auto& key : publicKeys) {
        script.push_back(key.size());  // Push length
        script.insert(script.end(), key.begin(), key.end());  // Push public key
    }

    script.push_back(0x50 + publicKeys.size());  // OP_N + total public keys
    script.push_back(0xae);  // OP_CHECKMULTISIG

    return script;
}

} // namespace wallet
} // namespace satox 