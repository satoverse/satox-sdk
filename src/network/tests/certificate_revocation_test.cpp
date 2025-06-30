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

#include <gtest/gtest.h>
#include <satox/network/certificate_revocation.hpp>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>

using namespace satox::network;

class CertificateRevocationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize OpenSSL
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
    }

    void TearDown() override {
        // Cleanup OpenSSL
        EVP_cleanup();
        ERR_free_strings();
    }

    // Helper function to create a test certificate
    X509* createTestCertificate() {
        X509* cert = X509_new();
        if (!cert) return nullptr;

        // Set version
        X509_set_version(cert, 2);

        // Set serial number
        ASN1_INTEGER* serial = ASN1_INTEGER_new();
        if (!serial) {
            X509_free(cert);
            return nullptr;
        }
        ASN1_INTEGER_set(serial, 1);
        X509_set_serialNumber(cert, serial);
        ASN1_INTEGER_free(serial);

        // Set issuer and subject
        X509_NAME* name = X509_NAME_new();
        if (!name) {
            X509_free(cert);
            return nullptr;
        }
        X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char*)"Test CA", -1, -1, 0);
        X509_set_issuer_name(cert, name);
        X509_set_subject_name(cert, name);
        X509_NAME_free(name);

        // Set validity
        X509_gmtime_adj(X509_get_notBefore(cert), 0);
        X509_gmtime_adj(X509_get_notAfter(cert), 365 * 24 * 60 * 60);

        return cert;
    }

    // Helper function to add OCSP responder URL to certificate
    bool addOCSPResponderURL(X509* cert, const std::string& url) {
        AUTHORITY_INFO_ACCESS* aia = AUTHORITY_INFO_ACCESS_new();
        if (!aia) return false;

        ACCESS_DESCRIPTION* ad = ACCESS_DESCRIPTION_new();
        if (!ad) {
            AUTHORITY_INFO_ACCESS_free(aia);
            return false;
        }

        ad->method = OBJ_nid2obj(NID_ad_OCSP);
        ad->location = GENERAL_NAME_new();
        if (!ad->location) {
            ACCESS_DESCRIPTION_free(ad);
            AUTHORITY_INFO_ACCESS_free(aia);
            return false;
        }

        ad->location->type = GEN_URI;
        ad->location->d.uniformResourceIdentifier = ASN1_IA5STRING_new();
        if (!ad->location->d.uniformResourceIdentifier) {
            GENERAL_NAME_free(ad->location);
            ACCESS_DESCRIPTION_free(ad);
            AUTHORITY_INFO_ACCESS_free(aia);
            return false;
        }

        ASN1_STRING_set(ad->location->d.uniformResourceIdentifier, url.c_str(), url.length());
        sk_ACCESS_DESCRIPTION_push(aia, ad);

        X509_add1_ext_i2d(cert, NID_info_access, aia, 0, X509V3_ADD_DEFAULT);

        AUTHORITY_INFO_ACCESS_free(aia);
        return true;
    }

    // Helper function to add CRL distribution point to certificate
    bool addCRLDistributionPoint(X509* cert, const std::string& url) {
        STACK_OF(DIST_POINT)* dist_points = sk_DIST_POINT_new_null();
        if (!dist_points) return false;

        DIST_POINT* dp = DIST_POINT_new();
        if (!dp) {
            sk_DIST_POINT_free(dist_points);
            return false;
        }

        dp->distpoint = DIST_POINT_NAME_new();
        if (!dp->distpoint) {
            DIST_POINT_free(dp);
            sk_DIST_POINT_free(dist_points);
            return false;
        }

        dp->distpoint->type = 0;
        dp->distpoint->name.fullname = sk_GENERAL_NAME_new_null();
        if (!dp->distpoint->name.fullname) {
            DIST_POINT_NAME_free(dp->distpoint);
            DIST_POINT_free(dp);
            sk_DIST_POINT_free(dist_points);
            return false;
        }

        GENERAL_NAME* gen = GENERAL_NAME_new();
        if (!gen) {
            sk_GENERAL_NAME_free(dp->distpoint->name.fullname);
            DIST_POINT_NAME_free(dp->distpoint);
            DIST_POINT_free(dp);
            sk_DIST_POINT_free(dist_points);
            return false;
        }

        gen->type = GEN_URI;
        gen->d.uniformResourceIdentifier = ASN1_IA5STRING_new();
        if (!gen->d.uniformResourceIdentifier) {
            GENERAL_NAME_free(gen);
            sk_GENERAL_NAME_free(dp->distpoint->name.fullname);
            DIST_POINT_NAME_free(dp->distpoint);
            DIST_POINT_free(dp);
            sk_DIST_POINT_free(dist_points);
            return false;
        }

        ASN1_STRING_set(gen->d.uniformResourceIdentifier, url.c_str(), url.length());
        sk_GENERAL_NAME_push(dp->distpoint->name.fullname, gen);
        sk_DIST_POINT_push(dist_points, dp);

        X509_add1_ext_i2d(cert, NID_crl_distribution_points, dist_points, 0, X509V3_ADD_DEFAULT);

        sk_DIST_POINT_free(dist_points);
        return true;
    }
};

TEST_F(CertificateRevocationTest, BasicConfiguration) {
    CertificateRevocation::RevocationConfig config;
    config.enable_ocsp = true;
    config.enable_crl = true;
    config.cache_duration = std::chrono::hours(24);
    config.timeout = std::chrono::seconds(30);

    CertificateRevocation revocation(config);
    EXPECT_TRUE(revocation.getLastError().empty());
}

TEST_F(CertificateRevocationTest, InvalidCertificate) {
    CertificateRevocation::RevocationConfig config;
    CertificateRevocation revocation(config);

    auto status = revocation.checkRevocation(nullptr);
    EXPECT_TRUE(status.is_revoked);
    EXPECT_FALSE(status.reason.empty());
}

TEST_F(CertificateRevocationTest, OCSPConfiguration) {
    CertificateRevocation::RevocationConfig config;
    config.enable_ocsp = true;
    config.ocsp_responder_url = "http://ocsp.example.com";
    config.timeout = std::chrono::seconds(30);

    CertificateRevocation revocation(config);
    EXPECT_TRUE(revocation.getLastError().empty());
}

TEST_F(CertificateRevocationTest, CRLConfiguration) {
    CertificateRevocation::RevocationConfig config;
    config.enable_crl = true;
    config.crl_url = "http://crl.example.com/crl.pem";
    config.timeout = std::chrono::seconds(30);

    CertificateRevocation revocation(config);
    EXPECT_TRUE(revocation.getLastError().empty());
}

TEST_F(CertificateRevocationTest, CacheConfiguration) {
    CertificateRevocation::RevocationConfig config;
    config.cache_duration = std::chrono::hours(24);

    CertificateRevocation revocation(config);
    EXPECT_TRUE(revocation.getLastError().empty());
}

TEST_F(CertificateRevocationTest, CertificateWithOCSP) {
    X509* cert = createTestCertificate();
    ASSERT_NE(cert, nullptr);

    ASSERT_TRUE(addOCSPResponderURL(cert, "http://ocsp.example.com"));

    CertificateRevocation::RevocationConfig config;
    config.enable_ocsp = true;
    config.timeout = std::chrono::seconds(30);

    CertificateRevocation revocation(config);
    auto status = revocation.checkRevocation(cert);
    EXPECT_FALSE(status.is_revoked);

    X509_free(cert);
}

TEST_F(CertificateRevocationTest, CertificateWithCRL) {
    X509* cert = createTestCertificate();
    ASSERT_NE(cert, nullptr);

    ASSERT_TRUE(addCRLDistributionPoint(cert, "http://crl.example.com/crl.pem"));

    CertificateRevocation::RevocationConfig config;
    config.enable_crl = true;
    config.timeout = std::chrono::seconds(30);

    CertificateRevocation revocation(config);
    auto status = revocation.checkRevocation(cert);
    EXPECT_FALSE(status.is_revoked);

    X509_free(cert);
}

TEST_F(CertificateRevocationTest, CacheExpiration) {
    X509* cert = createTestCertificate();
    ASSERT_NE(cert, nullptr);

    CertificateRevocation::RevocationConfig config;
    config.cache_duration = std::chrono::seconds(1);

    CertificateRevocation revocation(config);
    
    // First check
    auto status1 = revocation.checkRevocation(cert);
    EXPECT_FALSE(status1.is_revoked);

    // Wait for cache to expire
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Second check should trigger a new verification
    auto status2 = revocation.checkRevocation(cert);
    EXPECT_FALSE(status2.is_revoked);

    X509_free(cert);
}

TEST_F(CertificateRevocationTest, ConfigurationUpdate) {
    CertificateRevocation::RevocationConfig config;
    config.enable_ocsp = true;
    config.ocsp_responder_url = "http://ocsp.example.com";

    CertificateRevocation revocation(config);
    EXPECT_TRUE(revocation.getLastError().empty());

    // Update configuration
    config.enable_ocsp = false;
    config.enable_crl = true;
    config.crl_url = "http://crl.example.com/crl.pem";

    revocation.configure(config);
    EXPECT_TRUE(revocation.getLastError().empty());
}

TEST_F(CertificateRevocationTest, ErrorHandling) {
    CertificateRevocation::RevocationConfig config;
    config.enable_ocsp = true;
    config.ocsp_responder_url = "invalid://url";

    CertificateRevocation revocation(config);
    auto status = revocation.checkRevocation(createTestCertificate());
    EXPECT_FALSE(revocation.getLastError().empty());
}

TEST_F(CertificateRevocationTest, RevocationInfoUpdate) {
    CertificateRevocation::RevocationConfig config;
    config.enable_crl = true;
    config.crl_url = "http://crl.example.com/crl.pem";

    CertificateRevocation revocation(config);
    EXPECT_TRUE(revocation.updateRevocationInfo());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 