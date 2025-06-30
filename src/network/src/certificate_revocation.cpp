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

#include "satox/network/certificate_revocation.hpp"
#include <curl/curl.h>
#include <sstream>
#include <fstream>
#include <chrono>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <openssl/ssl.h>
#include <openssl/ocsp.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>
#include <spdlog/spdlog.h>

namespace satox {
namespace network {

class CertificateRevocation::Impl {
public:
    Impl(const RevocationConfig& config) : config(config) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    ~Impl() {
        curl_global_cleanup();
    }

    RevocationStatus checkRevocation(X509* cert) {
        std::lock_guard<std::mutex> lock(mutex);
        
        if (!cert) {
            last_error = "Invalid certificate";
            return RevocationStatus{true, "Invalid certificate"};
        }

        // Check cache first
        auto cache_key = getCertificateKey(cert);
        auto now = std::chrono::system_clock::now();
        
        auto it = revocation_cache.find(cache_key);
        if (it != revocation_cache.end()) {
            if (now - it->second.check_time < config.cache_duration) {
                return it->second;
            }
        }

        RevocationStatus status;
        status.check_time = now;

        // Check OCSP if enabled
        if (config.enable_ocsp) {
            if (checkOCSP(cert, status)) {
                updateCache(cache_key, status);
                return status;
            }
        }

        // Check CRL if enabled
        if (config.enable_crl) {
            if (checkCRL(cert, status)) {
                updateCache(cache_key, status);
                return status;
            }
        }

        // If both checks failed, assume not revoked
        status.is_revoked = false;
        updateCache(cache_key, status);
        return status;
    }

    bool updateRevocationInfo() {
        std::lock_guard<std::mutex> lock(mutex);
        
        if (config.enable_crl) {
            if (!updateCRL()) {
                return false;
            }
        }

        return true;
    }

    std::string getLastError() const {
        std::lock_guard<std::mutex> lock(mutex);
        return last_error;
    }

    void configure(const RevocationConfig& new_config) {
        std::lock_guard<std::mutex> lock(mutex);
        config = new_config;
    }

private:
    struct CertificateKey {
        std::string serial;
        std::string issuer;

        bool operator==(const CertificateKey& other) const {
            return serial == other.serial && issuer == other.issuer;
        }
    };

    struct CertificateKeyHash {
        std::size_t operator()(const CertificateKey& key) const {
            return std::hash<std::string>{}(key.serial) ^ 
                   (std::hash<std::string>{}(key.issuer) << 1);
        }
    };

    CertificateKey getCertificateKey(X509* cert) {
        CertificateKey key;
        
        // Get serial number
        ASN1_INTEGER* serial = X509_get_serialNumber(cert);
        if (serial) {
            BIGNUM* bn = ASN1_INTEGER_to_BN(serial, nullptr);
            if (bn) {
                char* hex = BN_bn2hex(bn);
                if (hex) {
                    key.serial = hex;
                    OPENSSL_free(hex);
                }
                BN_free(bn);
            }
        }

        // Get issuer
        X509_NAME* issuer = X509_get_issuer_name(cert);
        if (issuer) {
            char* name = X509_NAME_oneline(issuer, nullptr, 0);
            if (name) {
                key.issuer = name;
                OPENSSL_free(name);
            }
        }

        return key;
    }

    bool checkOCSP(X509* cert, RevocationStatus& status) {
        if (config.ocsp_responder_url.empty()) {
            // Try to get OCSP responder URL from certificate
            AUTHORITY_INFO_ACCESS* aia = (AUTHORITY_INFO_ACCESS*)X509_get_ext_d2i(cert, NID_info_access, nullptr, nullptr);
            if (!aia) {
                last_error = "No OCSP responder URL found";
                return false;
            }

            for (int i = 0; i < sk_ACCESS_DESCRIPTION_num(aia); i++) {
                ACCESS_DESCRIPTION* ad = sk_ACCESS_DESCRIPTION_value(aia, i);
                if (OBJ_obj2nid(ad->method) == NID_ad_OCSP) {
                    if (ad->location->type == GEN_URI) {
                        config.ocsp_responder_url = (char*)ASN1_STRING_get0_data(ad->location->d.uniformResourceIdentifier);
                        break;
                    }
                }
            }
            AUTHORITY_INFO_ACCESS_free(aia);
        }

        if (config.ocsp_responder_url.empty()) {
            last_error = "No OCSP responder URL available";
            return false;
        }

        // Create OCSP request
        OCSP_REQUEST* req = OCSP_REQUEST_new();
        if (!req) {
            last_error = "Failed to create OCSP request";
            return false;
        }

        OCSP_CERTID* certid = OCSP_cert_to_id(nullptr, cert, nullptr);
        if (!certid) {
            OCSP_REQUEST_free(req);
            last_error = "Failed to create OCSP cert ID";
            return false;
        }

        if (!OCSP_request_add0_id(req, certid)) {
            OCSP_REQUEST_free(req);
            last_error = "Failed to add cert ID to OCSP request";
            return false;
        }

        // Send OCSP request
        CURL* curl = curl_easy_init();
        if (!curl) {
            OCSP_REQUEST_free(req);
            last_error = "Failed to initialize CURL";
            return false;
        }

        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, config.ocsp_responder_url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, config.timeout.count());

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        OCSP_REQUEST_free(req);

        if (res != CURLE_OK) {
            last_error = "Failed to send OCSP request: " + std::string(curl_easy_strerror(res));
            return false;
        }

        // Parse OCSP response
        OCSP_RESPONSE* ocsp_resp = OCSP_response_create(OCSP_RESPONSE_STATUS_SUCCESSFUL, nullptr);
        if (!ocsp_resp) {
            last_error = "Failed to create OCSP response";
            return false;
        }

        int resp_status = OCSP_response_status(ocsp_resp);
        if (resp_status != OCSP_RESPONSE_STATUS_SUCCESSFUL) {
            OCSP_RESPONSE_free(ocsp_resp);
            last_error = "OCSP response status: " + std::to_string(resp_status);
            return false;
        }

        OCSP_BASICRESP* basic_resp = OCSP_response_get1_basic(ocsp_resp);
        if (!basic_resp) {
            OCSP_RESPONSE_free(ocsp_resp);
            last_error = "Failed to get basic OCSP response";
            return false;
        }

        int cert_status = OCSP_cert_status_t::V_OCSP_CERTSTATUS_UNKNOWN;
        ASN1_GENERALIZEDTIME* revtime = nullptr;
        ASN1_GENERALIZEDTIME* thisupd = nullptr;
        ASN1_GENERALIZEDTIME* nextupd = nullptr;

        if (!OCSP_resp_find_status(basic_resp, certid, &cert_status, &revtime, &thisupd, &nextupd)) {
            OCSP_BASICRESP_free(basic_resp);
            OCSP_RESPONSE_free(ocsp_resp);
            last_error = "Failed to find certificate status in OCSP response";
            return false;
        }

        status.is_revoked = (cert_status == OCSP_CERTSTATUS_REVOKED);
        if (status.is_revoked && revtime) {
            status.revocation_time = ASN1_TIME_to_tm(revtime);
            status.reason = "Certificate revoked";
        }

        OCSP_BASICRESP_free(basic_resp);
        OCSP_RESPONSE_free(ocsp_resp);
        return true;
    }

    bool checkCRL(X509* cert, RevocationStatus& status) {
        if (config.crl_url.empty()) {
            // Try to get CRL URL from certificate
            X509_EXTENSION* ext = X509_get_ext(cert, X509_get_ext_by_NID(cert, NID_crl_distribution_points, -1));
            if (!ext) {
                last_error = "No CRL distribution points found";
                return false;
            }

            STACK_OF(DIST_POINT)* dist_points = (STACK_OF(DIST_POINT)*)X509V3_EXT_d2i(ext);
            if (!dist_points) {
                last_error = "Failed to parse CRL distribution points";
                return false;
            }

            for (int i = 0; i < sk_DIST_POINT_num(dist_points); i++) {
                DIST_POINT* dp = sk_DIST_POINT_value(dist_points, i);
                if (dp->distpoint && dp->distpoint->type == 0) {
                    GENERAL_NAMES* gens = dp->distpoint->name.fullname;
                    for (int j = 0; j < sk_GENERAL_NAME_num(gens); j++) {
                        GENERAL_NAME* gen = sk_GENERAL_NAME_value(gens, j);
                        if (gen->type == GEN_URI) {
                            config.crl_url = (char*)ASN1_STRING_get0_data(gen->d.uniformResourceIdentifier);
                            break;
                        }
                    }
                }
            }
            sk_DIST_POINT_free(dist_points);
        }

        if (config.crl_url.empty()) {
            last_error = "No CRL URL available";
            return false;
        }

        // Download CRL
        CURL* curl = curl_easy_init();
        if (!curl) {
            last_error = "Failed to initialize CURL";
            return false;
        }

        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, config.crl_url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, config.timeout.count());

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            last_error = "Failed to download CRL: " + std::string(curl_easy_strerror(res));
            return false;
        }

        // Parse CRL
        X509_CRL* crl = nullptr;
        const unsigned char* p = (const unsigned char*)response.c_str();
        crl = d2i_X509_CRL(nullptr, &p, response.length());
        if (!crl) {
            last_error = "Failed to parse CRL";
            return false;
        }

        // Check if certificate is in CRL
        ASN1_INTEGER* serial = X509_get_serialNumber(cert);
        if (!serial) {
            X509_CRL_free(crl);
            last_error = "Failed to get certificate serial number";
            return false;
        }

        X509_REVOKED* revoked = nullptr;
        if (X509_CRL_get0_by_serial(crl, &revoked, serial)) {
            status.is_revoked = true;
            status.revocation_time = X509_get0_revocationDate(revoked);
            status.reason = "Certificate revoked";
        }

        X509_CRL_free(crl);
        return true;
    }

    bool updateCRL() {
        if (config.crl_url.empty()) {
            last_error = "No CRL URL available";
            return false;
        }

        // Download CRL
        CURL* curl = curl_easy_init();
        if (!curl) {
            last_error = "Failed to initialize CURL";
            return false;
        }

        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, config.crl_url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, config.timeout.count());

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            last_error = "Failed to download CRL: " + std::string(curl_easy_strerror(res));
            return false;
        }

        // Save CRL to file
        std::ofstream crl_file("crl.pem");
        if (!crl_file) {
            last_error = "Failed to open CRL file for writing";
            return false;
        }

        crl_file << response;
        return true;
    }

    void updateCache(const CertificateKey& key, const RevocationStatus& status) {
        revocation_cache[key] = status;
    }

    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    RevocationConfig config;
    std::unordered_map<CertificateKey, RevocationStatus, CertificateKeyHash> revocation_cache;
    mutable std::mutex mutex;
    std::string last_error;
};

CertificateRevocation::CertificateRevocation(const RevocationConfig& config)
    : pImpl(std::make_unique<Impl>(config)) {}

CertificateRevocation::~CertificateRevocation() = default;

CertificateRevocation::RevocationStatus CertificateRevocation::checkRevocation(X509* cert) {
    return pImpl->checkRevocation(cert);
}

bool CertificateRevocation::updateRevocationInfo() {
    return pImpl->updateRevocationInfo();
}

std::string CertificateRevocation::getLastError() const {
    return pImpl->getLastError();
}

void CertificateRevocation::configure(const RevocationConfig& config) {
    pImpl->configure(config);
}

} // namespace network
} // namespace satox 