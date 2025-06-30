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
#include "satox/network/network_manager.hpp"
#include "satox/network/letsencrypt_manager.hpp"
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <openssl/ssl.h>
#include <openssl/err.h>

namespace satox {
namespace tests {

class NetworkManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        networkManager = &network::NetworkManager::getInstance();
        letsEncryptManager = std::make_unique<network::LetsEncryptManager>();
        createTestCertificates();
    }

    void TearDown() override {
        networkManager->shutdown();
        letsEncryptManager.reset();
        std::remove("test_cert.pem");
        std::remove("test_key.pem");
        std::remove("test_ca.pem");
    }

    void createTestCertificates() {
        std::ofstream cert_file("test_cert.pem");
        cert_file << "-----BEGIN CERTIFICATE-----\n"
                  << "MIIDazCCAlOgAwIBAgIUJx8w3Q=="
                  << "-----END CERTIFICATE-----\n";
        cert_file.close();

        std::ofstream key_file("test_key.pem");
        key_file << "-----BEGIN PRIVATE KEY-----\n"
                 << "MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQC7VJTUt9Us8cKj"
                 << "-----END PRIVATE KEY-----\n";
        key_file.close();

        std::ofstream ca_file("test_ca.pem");
        ca_file << "-----BEGIN CERTIFICATE-----\n"
                << "MIIDazCCAlOgAwIBAgIUJx8w3Q=="
                << "-----END CERTIFICATE-----\n";
        ca_file.close();
    }

    network::NetworkManager::NetworkConfig createTestConfig() {
        network::NetworkManager::NetworkConfig config;
        config.host = "127.0.0.1";
        config.p2p_port = 8333;
        config.rpc_port = 8334;
        config.max_connections = 100;

        config.security.enable_encryption = true;
        config.security.verify_hostname = true;
        config.security.ssl_cert_path = "test_cert.pem";
        config.security.ssl_key_path = "test_key.pem";
        config.security.ssl_ca_path = "test_ca.pem";
        config.security.allowed_ciphers = {
            "ECDHE-ECDSA-AES256-GCM-SHA384",
            "ECDHE-RSA-AES256-GCM-SHA384"
        };

        return config;
    }

    network::NetworkManager* networkManager;
    std::unique_ptr<network::LetsEncryptManager> letsEncryptManager;
};

TEST_F(NetworkManagerTest, InitializationTest) {
    auto config = createTestConfig();
    EXPECT_TRUE(networkManager->initialize(config));
}

TEST_F(NetworkManagerTest, SSLInitialization) {
    auto config = createTestConfig();

    EXPECT_TRUE(networkManager->initialize(config));
    EXPECT_TRUE(networkManager->isInitialized());
}

TEST_F(NetworkManagerTest, SSLConfiguration) {
    auto config = createTestConfig();

    ASSERT_TRUE(networkManager->initialize(config));

    network::NetworkManager::SecurityConfig ssl_config;
    ssl_config.enable_encryption = true;
    ssl_config.verify_hostname = true;
    ssl_config.ssl_cert_path = "test_cert.pem";
    ssl_config.ssl_key_path = "test_key.pem";
    ssl_config.ssl_ca_path = "test_ca.pem";

    EXPECT_TRUE(networkManager->enableSSL(ssl_config));
    EXPECT_TRUE(networkManager->disableSSL());
}

TEST_F(NetworkManagerTest, SSLShutdown) {
    auto config = createTestConfig();

    ASSERT_TRUE(networkManager->initialize(config));

    network::NetworkManager::SecurityConfig ssl_config;
    ssl_config.enable_encryption = true;
    ssl_config.ssl_cert_path = "test_cert.pem";
    ssl_config.ssl_key_path = "test_key.pem";
    ASSERT_TRUE(networkManager->enableSSL(ssl_config));

    networkManager->shutdown();
    EXPECT_FALSE(networkManager->isInitialized());
}

TEST_F(NetworkManagerTest, GetNetworkStats) {
    auto config = createTestConfig();
    ASSERT_TRUE(networkManager->initialize(config));
    
    auto stats = networkManager->getNetworkStats();
    EXPECT_GE(stats.bytes_sent, 0);
    EXPECT_GE(stats.bytes_received, 0);
}

} // namespace tests
} // namespace satox 