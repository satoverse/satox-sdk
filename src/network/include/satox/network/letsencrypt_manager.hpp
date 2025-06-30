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
#include <memory>
#include <vector>
#include <functional>
#include <chrono>
#include <mutex>
#include <thread>
#include <atomic>
#include <nlohmann/json.hpp>

namespace satox::network {

class LetsEncryptManager {
public:
    struct Config {
        std::string domain;                    // Domain name for the certificate
        std::string email;                     // Email for Let's Encrypt notifications
        std::string webroot_path;              // Path for webroot challenge
        std::string cert_path;                 // Path to store certificates
        std::string staging;                   // Use Let's Encrypt staging server
        std::chrono::hours renewal_threshold;  // When to renew before expiry
        bool auto_renew;                       // Enable automatic renewal
    };

    struct Certificate {
        std::string cert_path;     // Path to certificate file
        std::string key_path;      // Path to private key file
        std::string chain_path;    // Path to certificate chain file
        std::chrono::system_clock::time_point expiry;
        std::string domain;
    };

    LetsEncryptManager();
    ~LetsEncryptManager();

    // Initialize the Let's Encrypt manager
    bool initialize(const Config& config);

    // Shutdown the manager
    void shutdown();

    // Get current certificate information
    Certificate getCurrentCertificate() const;

    // Force certificate renewal
    bool renewCertificate();

    // Register callback for certificate updates
    using CertificateUpdateCallback = std::function<void(const Certificate&)>;
    void registerUpdateCallback(CertificateUpdateCallback callback);

    // Check if Let's Encrypt is properly configured
    bool isConfigured() const;

    // Get last error message
    std::string getLastError() const;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};

} // namespace satox::network 