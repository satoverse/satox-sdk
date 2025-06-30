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
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <chrono>

namespace satox {
namespace core {

/**
 * @brief SecurityManager provides cryptographic and security operations for Satox SDK.
 */
class SecurityManager {
public:
    /** Security configuration structure */
    struct SecurityConfig {
        std::string network;
        int key_length = 32;
        int sessionTimeout = 3600; // seconds
        // Add more config fields as needed
    };

    /** Security level enumeration */
    enum class SecurityLevel {
        LOW,
        MEDIUM,
        HIGH
    };

    /** Security statistics structure */
    struct SecurityStats {
        size_t totalEncryptions = 0;
        size_t totalDecryptions = 0;
        size_t totalAuths = 0;
        size_t successfulLogins = 0;
        size_t failedAttempts = 0;
        // Add more stats as needed
    };

    /** Callback types */
    using SecurityCallback = std::function<void(const std::string&, SecurityLevel)>;
    using AuditCallback = std::function<void(const std::string&, const std::string&)>;

    SecurityManager();
    ~SecurityManager();

    /** Initialize the security manager with configuration */
    bool initialize(const SecurityConfig& config);
    void shutdown();

    /** Key management */
    bool generateKeyPair(const std::string& key_id);
    bool validateKey(const std::string& key);
    bool storeKey(const std::string& keyId, const std::string& key);
    std::string getKey(const std::string& keyId);
    std::string generateKey(int length);

    /** Signing and verification */
    std::optional<std::string> sign(const std::string& key_id, const std::string& data);
    bool verify(const std::string& key_id, const std::string& data, const std::string& signature);

    /** Encryption and decryption */
    std::optional<std::string> encrypt(const std::string& key_id, const std::string& data);
    std::optional<std::string> decrypt(const std::string& key_id, const std::string& encrypted_data);
    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data, const std::string& key);
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data, const std::string& key);
    std::string encryptString(const std::string& data, const std::string& key);
    std::string decryptString(const std::string& data, const std::string& key);

    /** Authentication and session management */
    bool authenticate(const std::string& username, const std::string& password);
    bool validateSession(const std::string& sessionId);
    std::string createSession(const std::string& username);
    void invalidateSession(const std::string& sessionId);
    bool isSessionExpired(const std::string& sessionId);
    void cleanupExpiredSessions();

    /** Input validation and permissions */
    bool validateInput(const std::string& input);
    bool checkPermission(const std::string& user, const std::string& resource);

    /** Security level management */
    SecurityLevel getSecurityLevel() const;
    void setSecurityLevel(SecurityLevel level);

    /** Event and audit logging */
    void logEvent(const std::string& event, const std::string& details);
    std::vector<std::string> getAuditLog() const;
    void clearAuditLog();
    void registerSecurityCallback(SecurityCallback callback);
    void registerAuditCallback(AuditCallback callback);
    void unregisterSecurityCallback();
    void unregisterAuditCallback();
    void notifySecurityEvent(const std::string& event, SecurityLevel level);
    void notifyAuditEvent(const std::string& event, const std::string& details);

    /** Statistics and config */
    SecurityStats getStats() const;
    void resetStats();
    SecurityConfig getConfig() const;
    bool updateConfig(const SecurityConfig& config);

    /** Error handling */
    std::string getLastError() const;
    void setLastError(const std::string& error);

private:
    bool initialized_ = false;
    SecurityConfig config_;
    SecurityStats stats_;
    std::string last_error_;
    std::unordered_map<std::string, std::string> keys_;
    std::unordered_map<std::string, std::string> sessions_;
    std::unordered_map<std::string, std::chrono::system_clock::time_point> sessionTimes_;
    mutable std::mutex mutex_;
    std::vector<std::string> auditLog_;
    std::vector<SecurityCallback> securityCallbacks_;
    std::vector<AuditCallback> auditCallbacks_;
    SecurityLevel security_level_ = SecurityLevel::MEDIUM;
};

} // namespace core
} // namespace satox 