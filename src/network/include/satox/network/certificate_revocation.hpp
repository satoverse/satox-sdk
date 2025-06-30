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
#include <chrono>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <openssl/ssl.h>
#include <openssl/ocsp.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <spdlog/spdlog.h>

namespace satox {
namespace network {

class CertificateRevocation {
public:
    struct RevocationConfig {
        bool enable_ocsp = true;
        bool enable_crl = true;
        std::string ocsp_responder_url;
        std::string crl_url;
        std::chrono::seconds cache_duration{3600}; // 1 hour
        std::chrono::seconds timeout{5}; // 5 seconds
    };

    struct RevocationStatus {
        bool is_revoked = false;
        std::string reason;
        std::chrono::system_clock::time_point revocation_time;
        std::chrono::system_clock::time_point check_time;
    };

    explicit CertificateRevocation(const RevocationConfig& config = RevocationConfig());
    ~CertificateRevocation();

    // Check if a certificate is revoked
    RevocationStatus checkRevocation(X509* cert);

    // Update revocation information
    bool updateRevocationInfo();

    // Get the last error
    std::string getLastError() const;

    // Configure revocation checking
    void configure(const RevocationConfig& config);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace network
} // namespace satox 