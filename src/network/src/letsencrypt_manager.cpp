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

#include "satox/network/letsencrypt_manager.hpp"
#include <spdlog/spdlog.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <curl/curl.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <sys/stat.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <queue>
#include <future>

namespace satox {
namespace network {

class LetsEncryptManager::Impl {
public:
    Impl() : is_initialized(false), is_running(false), renewal_thread_running(false) {
        curl_global_init(CURL_GLOBAL_ALL);
        OpenSSL_add_all_algorithms();
        ERR_load_crypto_strings();
    }

    ~Impl() {
        shutdown();
        curl_global_cleanup();
        EVP_cleanup();
        ERR_free_strings();
    }

    bool initialize(const Config& config) {
        std::lock_guard<std::mutex> lock(mutex);
        
        if (is_initialized) {
            last_error = "Already initialized";
            return false;
        }

        // Validate configuration
        if (config.domain.empty() || config.email.empty() || config.webroot_path.empty() || config.cert_path.empty()) {
            last_error = "Invalid configuration: missing required fields";
            return false;
        }

        // Create necessary directories
        try {
            std::filesystem::create_directories(config.cert_path);
            std::filesystem::create_directories(config.webroot_path);
            
            // Set proper permissions
            chmod(config.cert_path.c_str(), 0700);
            chmod(config.webroot_path.c_str(), 0755);
        } catch (const std::exception& e) {
            last_error = std::string("Failed to create directories: ") + e.what();
            return false;
        }

        this->config = config;
        is_initialized = true;

        // Check for existing certificate
        std::string cert_path = config.cert_path + "/cert.pem";
        if (std::filesystem::exists(cert_path)) {
            current_cert.cert_path = cert_path;
            current_cert.key_path = config.cert_path + "/privkey.pem";
            current_cert.chain_path = config.cert_path + "/chain.pem";
            current_cert.domain = config.domain;
            current_cert.expiry = getCertificateExpiry(cert_path);
            
            // If certificate is expired or near expiry, trigger renewal
            auto now = std::chrono::system_clock::now();
            if (current_cert.expiry - now < config.renewal_threshold) {
                spdlog::warn("Existing certificate is expired or near expiry, triggering renewal");
                std::async(std::launch::async, [this]() {
                    renewCertificate();
                });
            }
        }

        // Start renewal thread if auto-renewal is enabled
        if (config.auto_renew) {
            startRenewalThread();
        }

        return true;
    }

    void shutdown() {
        std::lock_guard<std::mutex> lock(mutex);
        
        if (!is_initialized) {
            return;
        }

        is_running = false;
        renewal_thread_running = false;
        renewal_cv.notify_all();
        
        if (renewal_thread.joinable()) {
            renewal_thread.join();
        }

        is_initialized = false;
    }

    Certificate getCurrentCertificate() const {
        std::lock_guard<std::mutex> lock(mutex);
        return current_cert;
    }

    bool renewCertificate(const std::string& domain = "") {
        std::lock_guard<std::mutex> lock(mutex);
        
        if (!is_initialized) {
            last_error = "Not initialized";
            return false;
        }

        // If domain is provided, use it instead of config domain
        std::string target_domain = domain.empty() ? config.domain : domain;

        try {
            spdlog::info("Starting certificate renewal for domain: {}", target_domain);

            // Generate new private key
            std::string key_path = config.cert_path + "/privkey.pem";
            if (!generatePrivateKey(key_path)) {
                spdlog::error("Failed to generate private key: {}", last_error);
                return false;
            }

            // Generate CSR
            std::string csr_path = config.cert_path + "/domain.csr";
            if (!generateCSR(csr_path, key_path)) {
                spdlog::error("Failed to generate CSR: {}", last_error);
                return false;
            }

            // Request certificate from Let's Encrypt
            std::string cert_path = config.cert_path + "/cert.pem";
            std::string chain_path = config.cert_path + "/chain.pem";
            if (!requestCertificate(csr_path, cert_path, chain_path)) {
                spdlog::error("Failed to request certificate: {}", last_error);
                return false;
            }

            // Verify the new certificate
            if (!verifyCertificate(cert_path, key_path)) {
                spdlog::error("Failed to verify new certificate: {}", last_error);
                return false;
            }

            // Backup old certificate if it exists
            if (std::filesystem::exists(current_cert.cert_path)) {
                std::string backup_path = current_cert.cert_path + ".bak";
                std::filesystem::rename(current_cert.cert_path, backup_path);
            }

            // Update current certificate
            current_cert.cert_path = cert_path;
            current_cert.key_path = key_path;
            current_cert.chain_path = chain_path;
            current_cert.domain = target_domain;
            current_cert.expiry = getCertificateExpiry(cert_path);

            spdlog::info("Certificate renewed successfully for domain: {}", target_domain);
            spdlog::info("Certificate expires at: {}", std::chrono::system_clock::to_time_t(current_cert.expiry));

            // Notify callback
            if (update_callback) {
                update_callback(current_cert);
            }

            return true;
        } catch (const std::exception& e) {
            last_error = std::string("Certificate renewal failed: ") + e.what();
            spdlog::error("Certificate renewal failed: {}", last_error);
            return false;
        }
    }

    void registerUpdateCallback(CertificateUpdateCallback callback) {
        std::lock_guard<std::mutex> lock(mutex);
        update_callback = callback;
    }

    bool isConfigured() const {
        std::lock_guard<std::mutex> lock(mutex);
        return is_initialized;
    }

    std::string getLastError() const {
        std::lock_guard<std::mutex> lock(mutex);
        return last_error;
    }

private:
    bool generatePrivateKey(const std::string& path) {
        try {
            // Generate RSA key
            RSA* rsa = RSA_generate_key(2048, RSA_F4, nullptr, nullptr);
            if (!rsa) {
                last_error = "Failed to generate RSA key";
                return false;
            }

            // Write private key to file
            FILE* fp = fopen(path.c_str(), "w");
            if (!fp) {
                RSA_free(rsa);
                last_error = "Failed to open private key file for writing";
                return false;
            }

            if (!PEM_write_RSAPrivateKey(fp, rsa, nullptr, nullptr, 0, nullptr, nullptr)) {
                fclose(fp);
                RSA_free(rsa);
                last_error = "Failed to write private key";
                return false;
            }

            fclose(fp);
            RSA_free(rsa);
            return true;
        } catch (const std::exception& e) {
            last_error = std::string("Error generating private key: ") + e.what();
            return false;
        }
    }

    bool generateCSR(const std::string& path, const std::string& key_path) {
        try {
            // Read private key
            FILE* fp = fopen(key_path.c_str(), "r");
            if (!fp) {
                last_error = "Failed to open private key file";
                return false;
            }

            RSA* rsa = PEM_read_RSAPrivateKey(fp, nullptr, nullptr, nullptr);
            fclose(fp);

            if (!rsa) {
                last_error = "Failed to read private key";
                return false;
            }

            // Create CSR
            X509_REQ* req = X509_REQ_new();
            if (!req) {
                RSA_free(rsa);
                last_error = "Failed to create CSR";
                return false;
            }

            // Set subject
            X509_NAME* name = X509_REQ_get_subject_name(req);
            X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, 
                                     (unsigned char*)config.domain.c_str(), -1, -1, 0);

            // Set public key
            EVP_PKEY* pkey = EVP_PKEY_new();
            EVP_PKEY_assign_RSA(pkey, rsa);
            X509_REQ_set_pubkey(req, pkey);

            // Sign CSR
            if (!X509_REQ_sign(req, pkey, EVP_sha256())) {
                X509_REQ_free(req);
                EVP_PKEY_free(pkey);
                last_error = "Failed to sign CSR";
                return false;
            }

            // Write CSR to file
            fp = fopen(path.c_str(), "w");
            if (!fp) {
                X509_REQ_free(req);
                EVP_PKEY_free(pkey);
                last_error = "Failed to open CSR file for writing";
                return false;
            }

            PEM_write_X509_REQ(fp, req);
            fclose(fp);
            X509_REQ_free(req);
            EVP_PKEY_free(pkey);

            return true;
        } catch (const std::exception& e) {
            last_error = std::string("Error generating CSR: ") + e.what();
            return false;
        }
    }

    bool requestCertificate(const std::string& csr_path, 
                           const std::string& cert_path,
                           const std::string& chain_path) {
        // Implementation for ACME protocol
        // This is a placeholder - actual implementation would use ACME protocol
        // to request and validate certificates from Let's Encrypt
        return true;
    }

    std::chrono::system_clock::time_point getCertificateExpiry(const std::string& cert_path) {
        try {
            FILE* fp = fopen(cert_path.c_str(), "r");
            if (!fp) {
                last_error = "Failed to open certificate file";
                return std::chrono::system_clock::now();
            }

            X509* cert = PEM_read_X509(fp, nullptr, nullptr, nullptr);
            fclose(fp);

            if (!cert) {
                last_error = "Failed to read certificate";
                return std::chrono::system_clock::now();
            }

            ASN1_TIME* notAfter = X509_get_notAfter(cert);
            if (!notAfter) {
                X509_free(cert);
                last_error = "Failed to get certificate expiry";
                return std::chrono::system_clock::now();
            }

            // Convert ASN1_TIME to system_clock::time_point
            struct tm tm;
            ASN1_TIME_to_tm(notAfter, &tm);
            auto time = std::mktime(&tm);
            auto expiry = std::chrono::system_clock::from_time_t(time);

            X509_free(cert);
            return expiry;
        } catch (const std::exception& e) {
            last_error = std::string("Error getting certificate expiry: ") + e.what();
            return std::chrono::system_clock::now();
        }
    }

    bool verifyCertificate(const std::string& cert_path, const std::string& key_path) {
        try {
            // Read certificate
            FILE* fp = fopen(cert_path.c_str(), "r");
            if (!fp) {
                last_error = "Failed to open certificate file";
                return false;
            }

            X509* cert = PEM_read_X509(fp, nullptr, nullptr, nullptr);
            fclose(fp);

            if (!cert) {
                last_error = "Failed to read certificate";
                return false;
            }

            // Read private key
            fp = fopen(key_path.c_str(), "r");
            if (!fp) {
                X509_free(cert);
                last_error = "Failed to open private key file";
                return false;
            }

            EVP_PKEY* pkey = PEM_read_PrivateKey(fp, nullptr, nullptr, nullptr);
            fclose(fp);

            if (!pkey) {
                X509_free(cert);
                last_error = "Failed to read private key";
                return false;
            }

            // Verify certificate signature
            bool valid = X509_verify(cert, pkey) == 1;
            
            X509_free(cert);
            EVP_PKEY_free(pkey);

            if (!valid) {
                last_error = "Certificate signature verification failed";
                return false;
            }

            return true;
        } catch (const std::exception& e) {
            last_error = std::string("Error verifying certificate: ") + e.what();
            return false;
        }
    }

    void startRenewalThread() {
        if (renewal_thread_running) {
            return;
        }

        renewal_thread_running = true;
        renewal_thread = std::thread([this]() {
            while (renewal_thread_running) {
                try {
                    auto now = std::chrono::system_clock::now();
                    auto expiry = current_cert.expiry;
                    auto threshold = config.renewal_threshold;

                    // Check if certificate needs renewal
                    if (expiry - now < threshold) {
                        spdlog::info("Certificate approaching expiry, initiating renewal");
                        if (!renewCertificate()) {
                            spdlog::error("Automatic renewal failed: {}", last_error);
                            // If renewal failed, try again in 5 minutes
                            std::this_thread::sleep_for(std::chrono::minutes(5));
                            continue;
                        }
                    }

                    // Calculate next check time
                    auto next_check = std::min(
                        std::chrono::hours(1),  // Check at least every hour
                        std::chrono::duration_cast<std::chrono::hours>(expiry - now - threshold) / 2  // Or halfway to renewal threshold
                    );

                    // Wait for next check or shutdown signal
                    std::unique_lock<std::mutex> lock(mutex);
                    renewal_cv.wait_for(lock, next_check, [this]() {
                        return !renewal_thread_running;
                    });
                } catch (const std::exception& e) {
                    spdlog::error("Error in renewal thread: {}", e.what());
                    std::this_thread::sleep_for(std::chrono::minutes(5));  // Wait before retrying
                }
            }
        });
    }

    Config config;
    Certificate current_cert;
    CertificateUpdateCallback update_callback;
    mutable std::mutex mutex;
    std::condition_variable renewal_cv;
    std::thread renewal_thread;
    std::atomic<bool> is_initialized;
    std::atomic<bool> is_running;
    std::atomic<bool> renewal_thread_running;
    std::string last_error;
};

// Implementation of public interface
LetsEncryptManager::LetsEncryptManager() : pimpl(std::make_unique<Impl>()) {}
LetsEncryptManager::~LetsEncryptManager() = default;

bool LetsEncryptManager::initialize(const Config& config) {
    return pimpl->initialize(config);
}

LetsEncryptManager::Certificate LetsEncryptManager::getCurrentCertificate() const {
    return pimpl->getCurrentCertificate();
}

bool LetsEncryptManager::renewCertificate() {
    return pimpl->renewCertificate();
}

void LetsEncryptManager::registerUpdateCallback(CertificateUpdateCallback callback) {
    pimpl->registerUpdateCallback(callback);
}

bool LetsEncryptManager::isConfigured() const {
    return pimpl->isConfigured();
}

std::string LetsEncryptManager::getLastError() const {
    return pimpl->getLastError();
}

void LetsEncryptManager::shutdown() {
    pimpl->shutdown();
}

} // namespace network
} // namespace satox 