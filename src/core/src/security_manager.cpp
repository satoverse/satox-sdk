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

#include "satox/core/security_manager.hpp"
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <sstream>
#include <iomanip>
#include <random>
#include <chrono>
#include <algorithm>

namespace satox::core {

SecurityManager::SecurityManager() : initialized_(false) {
}

SecurityManager::~SecurityManager() {
    shutdown();
}

bool SecurityManager::initialize(const SecurityConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        setLastError("Security Manager already initialized");
        return false;
    }

    try {
        // Initialize OpenSSL
        OpenSSL_add_all_algorithms();
        ERR_load_crypto_strings();
        
        // Store configuration
        config_ = config;
        initialized_ = true;
        return true;
    } catch (const std::exception& e) {
        setLastError("Failed to initialize Security Manager: " + std::string(e.what()));
        return false;
    }
}

void SecurityManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return;
    }

    try {
        // Cleanup OpenSSL
        EVP_cleanup();
        ERR_free_strings();
        
        initialized_ = false;
    } catch (const std::exception& e) {
        setLastError("Failed to shutdown Security Manager: " + std::string(e.what()));
    }
}

std::vector<uint8_t> SecurityManager::encrypt(const std::vector<uint8_t>& data, const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        setLastError("Security Manager not initialized");
        return {};
    }

    try {
        std::vector<unsigned char> iv(EVP_MAX_IV_LENGTH);
        if (RAND_bytes(iv.data(), iv.size()) != 1) {
            setLastError("Failed to generate IV");
            return {};
        }

        const EVP_CIPHER* cipher = EVP_aes_256_cbc();
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            setLastError("Failed to create encryption context");
            return {};
        }

        std::vector<unsigned char> keyBytes(key.begin(), key.end());
        if (EVP_EncryptInit_ex(ctx, cipher, nullptr, keyBytes.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            setLastError("Failed to initialize encryption");
            return {};
        }

        std::vector<unsigned char> ciphertext(data.size() + EVP_MAX_BLOCK_LENGTH);
        int len1, len2;

        if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len1, data.data(), data.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            setLastError("Failed to encrypt data");
            return {};
        }

        if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len1, &len2) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            setLastError("Failed to finalize encryption");
            return {};
        }

        EVP_CIPHER_CTX_free(ctx);

        // Combine IV and ciphertext
        std::vector<uint8_t> result(iv.begin(), iv.end());
        result.insert(result.end(), ciphertext.begin(), ciphertext.begin() + len1 + len2);

        stats_.totalEncryptions++;
        return result;
    } catch (const std::exception& e) {
        setLastError("Failed to encrypt data: " + std::string(e.what()));
        return {};
    }
}

std::vector<uint8_t> SecurityManager::decrypt(const std::vector<uint8_t>& data, const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        setLastError("Security Manager not initialized");
        return {};
    }

    try {
        if (data.size() < EVP_MAX_IV_LENGTH) {
            setLastError("Invalid encrypted data");
            return {};
        }

        std::vector<unsigned char> iv(data.begin(), data.begin() + EVP_MAX_IV_LENGTH);
        std::vector<unsigned char> ciphertext(data.begin() + EVP_MAX_IV_LENGTH, data.end());

        const EVP_CIPHER* cipher = EVP_aes_256_cbc();
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            setLastError("Failed to create decryption context");
            return {};
        }

        std::vector<unsigned char> keyBytes(key.begin(), key.end());
        if (EVP_DecryptInit_ex(ctx, cipher, nullptr, keyBytes.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            setLastError("Failed to initialize decryption");
            return {};
        }

        std::vector<unsigned char> plaintext(ciphertext.size());
        int len1, len2;

        if (EVP_DecryptUpdate(ctx, plaintext.data(), &len1, ciphertext.data(), ciphertext.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            setLastError("Failed to decrypt data");
            return {};
        }

        if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len1, &len2) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            setLastError("Failed to finalize decryption");
            return {};
        }

        EVP_CIPHER_CTX_free(ctx);

        std::vector<uint8_t> result(plaintext.begin(), plaintext.begin() + len1 + len2);
        stats_.totalDecryptions++;
        return result;
    } catch (const std::exception& e) {
        setLastError("Failed to decrypt data: " + std::string(e.what()));
        return {};
    }
}

std::string SecurityManager::encryptString(const std::string& data, const std::string& key) {
    std::vector<uint8_t> dataVec(data.begin(), data.end());
    std::vector<uint8_t> encrypted = encrypt(dataVec, key);
    
    std::stringstream ss;
    for (uint8_t byte : encrypted) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return ss.str();
}

std::string SecurityManager::decryptString(const std::string& data, const std::string& key) {
    if (data.length() % 2 != 0) {
        setLastError("Invalid encrypted string");
        return "";
    }

    std::vector<uint8_t> encrypted;
    for (size_t i = 0; i < data.length(); i += 2) {
        std::string byteStr = data.substr(i, 2);
        encrypted.push_back(static_cast<uint8_t>(std::stoi(byteStr, nullptr, 16)));
    }

    std::vector<uint8_t> decrypted = decrypt(encrypted, key);
    return std::string(decrypted.begin(), decrypted.end());
}

std::string SecurityManager::generateKey(int length) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        setLastError("Security Manager not initialized");
        return "";
    }

    try {
        std::vector<unsigned char> key(length);
        if (RAND_bytes(key.data(), length) != 1) {
            setLastError("Failed to generate random key");
            return "";
        }

        std::stringstream ss;
        for (size_t i = 0; i < length; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(key[i]);
        }
        return ss.str();
    } catch (const std::exception& e) {
        setLastError("Failed to generate key: " + std::string(e.what()));
        return "";
    }
}

bool SecurityManager::validateKey(const std::string& key) {
    return !key.empty() && key.length() >= 16;
}

bool SecurityManager::storeKey(const std::string& keyId, const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        setLastError("Security Manager not initialized");
        return false;
    }

    if (!validateKey(key)) {
        setLastError("Invalid key");
        return false;
    }

    keys_[keyId] = key;
    return true;
}

std::string SecurityManager::getKey(const std::string& keyId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = keys_.find(keyId);
    if (it == keys_.end()) {
        setLastError("Key not found");
        return "";
    }
    return it->second;
}

bool SecurityManager::authenticate(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        setLastError("Security Manager not initialized");
        return false;
    }

    // Simple authentication - in real implementation, this would check against a database
    if (username == "admin" && password == "password") {
        stats_.successfulLogins++;
        return true;
    }

    stats_.failedAttempts++;
    setLastError("Invalid credentials");
    return false;
}

bool SecurityManager::validateSession(const std::string& sessionId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = sessions_.find(sessionId);
    if (it == sessions_.end()) {
        return false;
    }

    if (isSessionExpired(sessionId)) {
        sessions_.erase(it);
        return false;
    }

    return true;
}

std::string SecurityManager::createSession(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string sessionId = generateKey(32);
    sessions_[sessionId] = username;
    sessionTimes_[sessionId] = std::chrono::system_clock::now();
    return sessionId;
}

void SecurityManager::invalidateSession(const std::string& sessionId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    sessions_.erase(sessionId);
    sessionTimes_.erase(sessionId);
}

bool SecurityManager::validateInput(const std::string& input) {
    // Basic input validation - check for common injection patterns
    std::string lowerInput = input;
    std::transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(), ::tolower);
    
    std::vector<std::string> dangerousPatterns = {
        "script", "javascript", "vbscript", "onload", "onerror", "onclick",
        "union select", "drop table", "delete from", "insert into"
    };

    for (const auto& pattern : dangerousPatterns) {
        if (lowerInput.find(pattern) != std::string::npos) {
            return false;
        }
    }

    return true;
}

bool SecurityManager::checkPermission(const std::string& user, const std::string& resource) {
    // Simple permission check - in real implementation, this would check against a permission system
    return user == "admin" || resource.find("public") != std::string::npos;
}

SecurityManager::SecurityLevel SecurityManager::getSecurityLevel() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return security_level_;
}

void SecurityManager::setSecurityLevel(SecurityLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    security_level_ = level;
}

void SecurityManager::logEvent(const std::string& event, const std::string& details) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string logEntry = std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + 
                          " - " + event + " - " + details;
    auditLog_.push_back(logEntry);
    
    notifyAuditEvent(event, details);
}

std::vector<std::string> SecurityManager::getAuditLog() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return auditLog_;
}

void SecurityManager::clearAuditLog() {
    std::lock_guard<std::mutex> lock(mutex_);
    auditLog_.clear();
}

void SecurityManager::registerSecurityCallback(SecurityCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    securityCallbacks_.push_back(callback);
}

void SecurityManager::registerAuditCallback(AuditCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    auditCallbacks_.push_back(callback);
}

void SecurityManager::unregisterSecurityCallback() {
    std::lock_guard<std::mutex> lock(mutex_);
    securityCallbacks_.clear();
}

void SecurityManager::unregisterAuditCallback() {
    std::lock_guard<std::mutex> lock(mutex_);
    auditCallbacks_.clear();
}

SecurityManager::SecurityStats SecurityManager::getStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

void SecurityManager::resetStats() {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_ = SecurityStats{};
}

SecurityManager::SecurityConfig SecurityManager::getConfig() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_;
}

bool SecurityManager::updateConfig(const SecurityConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = config;
    return true;
}

std::string SecurityManager::getLastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return last_error_;
}

void SecurityManager::setLastError(const std::string& error) {
    std::lock_guard<std::mutex> lock(mutex_);
    last_error_ = error;
}

void SecurityManager::notifySecurityEvent(const std::string& event, SecurityLevel level) {
    for (const auto& callback : securityCallbacks_) {
        callback(event, level);
    }
}

void SecurityManager::notifyAuditEvent(const std::string& event, const std::string& details) {
    for (const auto& callback : auditCallbacks_) {
        callback(event, details);
    }
}

bool SecurityManager::isSessionExpired(const std::string& sessionId) {
    auto it = sessionTimes_.find(sessionId);
    if (it == sessionTimes_.end()) {
        return true;
    }

    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - it->second);
    return duration.count() > config_.sessionTimeout;
}

void SecurityManager::cleanupExpiredSessions() {
    std::vector<std::string> expiredSessions;
    
    for (const auto& [sessionId, _] : sessions_) {
        if (isSessionExpired(sessionId)) {
            expiredSessions.push_back(sessionId);
        }
    }

    for (const auto& sessionId : expiredSessions) {
        sessions_.erase(sessionId);
        sessionTimes_.erase(sessionId);
    }
}

} // namespace satox::core 