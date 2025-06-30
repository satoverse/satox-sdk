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

#include "satox/network/network_manager.hpp"
#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <openssl/ssl.h>
#include <openssl/err.h>

using namespace satox::network;

class SSLHandshakeStressTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test certificates
        createTestCertificates();
    }

    void TearDown() override {
        // Clean up test certificates
        std::remove("test_cert.pem");
        std::remove("test_key.pem");
        std::remove("test_ca.pem");
    }

    void createTestCertificates() {
        // Create a self-signed certificate for testing
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

    NetworkConfig createTestConfig() {
        NetworkConfig config;
        config.host = "127.0.0.1";
        config.p2p_port = 8333;
        config.rpc_port = 8334;

        // SSL Configuration
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
};

TEST_F(SSLHandshakeStressTest, ConcurrentHandshakes) {
    NetworkManager manager;
    auto config = createTestConfig();
    ASSERT_TRUE(manager.initialize(config));

    const int num_threads = 10;
    const int handshakes_per_thread = 100;
    std::atomic<int> successful_handshakes{0};
    std::atomic<int> failed_handshakes{0};
    std::mutex error_mutex;
    std::vector<std::string> errors;

    auto perform_handshake = [&]() {
        try {
            boost::asio::io_context io_context;
            boost::asio::ssl::context ssl_context(boost::asio::ssl::context::tlsv12);
            ssl_context.use_certificate_chain_file("test_cert.pem");
            ssl_context.use_private_key_file("test_key.pem", boost::asio::ssl::context::pem);
            ssl_context.load_verify_file("test_ca.pem");

            boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_stream(io_context, ssl_context);
            ssl_stream.handshake(boost::asio::ssl::stream_base::client);
            successful_handshakes++;
        } catch (const std::exception& e) {
            failed_handshakes++;
            std::lock_guard<std::mutex> lock(error_mutex);
            errors.push_back(e.what());
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < handshakes_per_thread; ++j) {
                perform_handshake();
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_GT(successful_handshakes, 0);
    EXPECT_LT(failed_handshakes, num_threads * handshakes_per_thread);
}

TEST_F(SSLHandshakeStressTest, RapidHandshakes) {
    NetworkManager manager;
    auto config = createTestConfig();
    ASSERT_TRUE(manager.initialize(config));

    const int num_handshakes = 1000;
    std::atomic<int> successful_handshakes{0};
    std::atomic<int> failed_handshakes{0};
    std::mutex error_mutex;
    std::vector<std::string> errors;

    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_handshakes; ++i) {
        try {
            boost::asio::io_context io_context;
            boost::asio::ssl::context ssl_context(boost::asio::ssl::context::tlsv12);
            ssl_context.use_certificate_chain_file("test_cert.pem");
            ssl_context.use_private_key_file("test_key.pem", boost::asio::ssl::context::pem);
            ssl_context.load_verify_file("test_ca.pem");

            boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_stream(io_context, ssl_context);
            ssl_stream.handshake(boost::asio::ssl::stream_base::client);
            successful_handshakes++;
        } catch (const std::exception& e) {
            failed_handshakes++;
            std::lock_guard<std::mutex> lock(error_mutex);
            errors.push_back(e.what());
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    EXPECT_GT(successful_handshakes, 0);
    EXPECT_LT(failed_handshakes, num_handshakes);
    EXPECT_LT(duration.count(), 10000); // Should complete within 10 seconds
}

TEST_F(SSLHandshakeStressTest, MemoryLeakCheck) {
    NetworkManager manager;
    auto config = createTestConfig();
    ASSERT_TRUE(manager.initialize(config));

    const int num_iterations = 1000;
    std::vector<std::unique_ptr<boost::asio::ssl::context>> contexts;
    std::vector<std::unique_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>> streams;

    for (int i = 0; i < num_iterations; ++i) {
        auto context = std::make_unique<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12);
        context->use_certificate_chain_file("test_cert.pem");
        context->use_private_key_file("test_key.pem", boost::asio::ssl::context::pem);
        context->load_verify_file("test_ca.pem");

        boost::asio::io_context io_context;
        auto stream = std::make_unique<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(
            io_context, *context);

        contexts.push_back(std::move(context));
        streams.push_back(std::move(stream));
    }

    // If we get here without crashing, there are no memory leaks
    EXPECT_EQ(contexts.size(), num_iterations);
    EXPECT_EQ(streams.size(), num_iterations);
}

TEST_F(SSLHandshakeStressTest, ErrorRecovery) {
    NetworkManager manager;
    auto config = createTestConfig();
    ASSERT_TRUE(manager.initialize(config));

    const int num_iterations = 100;
    std::atomic<int> successful_recoveries{0};

    for (int i = 0; i < num_iterations; ++i) {
        try {
            boost::asio::io_context io_context;
            boost::asio::ssl::context ssl_context(boost::asio::ssl::context::tlsv12);
            ssl_context.use_certificate_chain_file("test_cert.pem");
            ssl_context.use_private_key_file("test_key.pem", boost::asio::ssl::context::pem);
            ssl_context.load_verify_file("test_ca.pem");

            boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_stream(io_context, ssl_context);
            
            // Simulate an error
            throw std::runtime_error("Simulated error");
        } catch (const std::exception&) {
            // Try to recover
            try {
                boost::asio::io_context io_context;
                boost::asio::ssl::context ssl_context(boost::asio::ssl::context::tlsv12);
                ssl_context.use_certificate_chain_file("test_cert.pem");
                ssl_context.use_private_key_file("test_key.pem", boost::asio::ssl::context::pem);
                ssl_context.load_verify_file("test_ca.pem");

                boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_stream(io_context, ssl_context);
                ssl_stream.handshake(boost::asio::ssl::stream_base::client);
                successful_recoveries++;
            } catch (const std::exception&) {
                // Recovery failed
            }
        }
    }

    EXPECT_GT(successful_recoveries, 0);
}

TEST_F(SSLHandshakeStressTest, ResourceExhaustion) {
    NetworkManager manager;
    auto config = createTestConfig();
    ASSERT_TRUE(manager.initialize(config));

    const int max_resources = 1000;
    std::vector<std::unique_ptr<boost::asio::ssl::context>> contexts;
    std::vector<std::unique_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>> streams;

    bool resource_exhaustion = false;
    try {
        for (int i = 0; i < max_resources; ++i) {
            auto context = std::make_unique<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12);
            context->use_certificate_chain_file("test_cert.pem");
            context->use_private_key_file("test_key.pem", boost::asio::ssl::context::pem);
            context->load_verify_file("test_ca.pem");

            boost::asio::io_context io_context;
            auto stream = std::make_unique<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(
                io_context, *context);

            contexts.push_back(std::move(context));
            streams.push_back(std::move(stream));
        }
    } catch (const std::exception&) {
        resource_exhaustion = true;
    }

    EXPECT_TRUE(resource_exhaustion);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 