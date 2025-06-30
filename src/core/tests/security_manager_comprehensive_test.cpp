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
#include <gmock/gmock.h>
#include <chrono>
#include <thread>
#include <future>
#include <random>
#include <memory>
#include "satox/core/security_manager.hpp"
#include <fstream>
#include <filesystem>

using namespace satox::core;
using namespace testing;

class SecurityManagerComprehensiveTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<SecurityManager>();
        ASSERT_TRUE(manager->initialize("test_network"));
    }

    void TearDown() override {
        manager.reset();
    }

    std::unique_ptr<SecurityManager> manager;
    std::random_device rd;
    std::mt19937 gen{rd()};

    std::string generateRandomString(size_t length) {
        std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::uniform_int_distribution<> dis(0, chars.size() - 1);
        std::string result;
        result.reserve(length);
        for (size_t i = 0; i < length; ++i) {
            result += chars[dis(gen)];
        }
        return result;
    }
};

// Basic Functionality Tests
TEST_F(SecurityManagerComprehensiveTest, KeyPairGeneration) {
    std::string key_id = "test_key_" + generateRandomString(8);
    ASSERT_TRUE(manager->generateKeyPair(key_id));
}

TEST_F(SecurityManagerComprehensiveTest, SignAndVerify) {
    std::string key_id = "test_key_" + generateRandomString(8);
    ASSERT_TRUE(manager->generateKeyPair(key_id));

    std::string data = "Test data for signing";
    auto signature = manager->sign(key_id, data);
    ASSERT_TRUE(signature.has_value());
    EXPECT_TRUE(manager->verify(key_id, data, signature.value()));
}

TEST_F(SecurityManagerComprehensiveTest, EncryptAndDecrypt) {
    std::string key_id = "test_key_" + generateRandomString(8);
    ASSERT_TRUE(manager->generateKeyPair(key_id));

    std::string data = "Test data for encryption";
    auto encrypted = manager->encrypt(key_id, data);
    ASSERT_TRUE(encrypted.has_value());

    auto decrypted = manager->decrypt(key_id, encrypted.value());
    ASSERT_TRUE(decrypted.has_value());
    EXPECT_EQ(decrypted.value(), data);
}

// Key Management Tests
TEST_F(SecurityManagerComprehensiveTest, KeyStorage) {
    std::string key_id = "test_key_" + generateRandomString(8);
    ASSERT_TRUE(manager->generateKeyPair(key_id));

    // Test key persistence
    auto new_manager = std::make_unique<SecurityManager>();
    ASSERT_TRUE(new_manager->initialize("test_network"));
    EXPECT_TRUE(new_manager->verify(key_id, "test", manager->sign(key_id, "test").value()));
}

// Security Feature Tests
TEST_F(SecurityManagerComprehensiveTest, SecureCommunication) {
    std::string key_id = "test_key_" + generateRandomString(8);
    ASSERT_TRUE(manager->generateKeyPair(key_id));

    // Test secure message exchange
    std::string message = "Secure message";
    auto encrypted = manager->encrypt(key_id, message);
    ASSERT_TRUE(encrypted.has_value());

    auto signature = manager->sign(key_id, encrypted.value());
    ASSERT_TRUE(signature.has_value());

    // Verify and decrypt
    EXPECT_TRUE(manager->verify(key_id, encrypted.value(), signature.value()));
    auto decrypted = manager->decrypt(key_id, encrypted.value());
    ASSERT_TRUE(decrypted.has_value());
    EXPECT_EQ(decrypted.value(), message);
}

// Performance Tests
TEST_F(SecurityManagerComprehensiveTest, PerformanceBenchmark) {
    std::string key_id = "test_key_" + generateRandomString(8);
    ASSERT_TRUE(manager->generateKeyPair(key_id));

    const int num_operations = 1000;
    std::string data = generateRandomString(1024); // 1KB of data

    // Encryption performance
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_operations; ++i) {
        ASSERT_TRUE(manager->encrypt(key_id, data).has_value());
    }
    auto encrypt_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start);
    EXPECT_LT(encrypt_duration.count(), 5000); // Should encrypt 1000 1KB blocks in less than 5 seconds

    // Decryption performance
    auto encrypted = manager->encrypt(key_id, data);
    ASSERT_TRUE(encrypted.has_value());
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_operations; ++i) {
        ASSERT_TRUE(manager->decrypt(key_id, encrypted.value()).has_value());
    }
    auto decrypt_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start);
    EXPECT_LT(decrypt_duration.count(), 5000); // Should decrypt 1000 1KB blocks in less than 5 seconds
}

// Concurrency Tests
TEST_F(SecurityManagerComprehensiveTest, ConcurrentOperations) {
    const int num_threads = 10;
    const int num_operations = 100;
    std::vector<std::future<bool>> futures;

    for (int i = 0; i < num_threads; ++i) {
        futures.push_back(std::async(std::launch::async, [&, i]() {
            std::string key_id = "test_key_" + std::to_string(i);
            if (!manager->generateKeyPair(key_id)) {
                return false;
            }

            std::string data = "Test data " + std::to_string(i);
            for (int j = 0; j < num_operations; ++j) {
                auto encrypted = manager->encrypt(key_id, data);
                if (!encrypted.has_value()) {
                    return false;
                }

                auto decrypted = manager->decrypt(key_id, encrypted.value());
                if (!decrypted.has_value() || decrypted.value() != data) {
                    return false;
                }
            }
            return true;
        }));
    }

    for (auto& future : futures) {
        EXPECT_TRUE(future.get());
    }
}

// Edge Case Tests
TEST_F(SecurityManagerComprehensiveTest, EdgeCases) {
    std::string key_id = "test_key_" + generateRandomString(8);
    ASSERT_TRUE(manager->generateKeyPair(key_id));

    // Test with empty data
    EXPECT_TRUE(manager->encrypt(key_id, "").has_value());
    EXPECT_TRUE(manager->sign(key_id, "").has_value());

    // Test with large data
    std::string large_data = generateRandomString(1024 * 1024); // 1MB
    auto encrypted = manager->encrypt(key_id, large_data);
    ASSERT_TRUE(encrypted.has_value());
    auto decrypted = manager->decrypt(key_id, encrypted.value());
    ASSERT_TRUE(decrypted.has_value());
    EXPECT_EQ(decrypted.value(), large_data);

    // Test with special characters
    std::string special_chars = "!@#$%^&*()_+{}|:<>?~`-=[]\\;',./";
    encrypted = manager->encrypt(key_id, special_chars);
    ASSERT_TRUE(encrypted.has_value());
    decrypted = manager->decrypt(key_id, encrypted.value());
    ASSERT_TRUE(decrypted.has_value());
    EXPECT_EQ(decrypted.value(), special_chars);
}

// Error Handling Tests
TEST_F(SecurityManagerComprehensiveTest, ErrorHandling) {
    // Test with invalid key ID
    EXPECT_FALSE(manager->encrypt("invalid_key", "test").has_value());
    EXPECT_FALSE(manager->decrypt("invalid_key", "test").has_value());
    EXPECT_FALSE(manager->sign("invalid_key", "test").has_value());
    EXPECT_FALSE(manager->verify("invalid_key", "test", "signature"));

    // Test with invalid encrypted data
    std::string key_id = "test_key_" + generateRandomString(8);
    ASSERT_TRUE(manager->generateKeyPair(key_id));
    EXPECT_FALSE(manager->decrypt(key_id, "invalid_encrypted_data").has_value());

    // Test with invalid signature
    EXPECT_FALSE(manager->verify(key_id, "test", "invalid_signature"));
}

// Recovery Tests
TEST_F(SecurityManagerComprehensiveTest, RecoveryScenarios) {
    std::string key_id = "test_key_" + generateRandomString(8);
    ASSERT_TRUE(manager->generateKeyPair(key_id));

    // Test recovery after failed encryption
    std::string data = "Test data";
    auto encrypted = manager->encrypt(key_id, data);
    ASSERT_TRUE(encrypted.has_value());

    // Simulate corruption
    std::string corrupted = encrypted.value();
    corrupted[0] = 'X';

    // Should fail to decrypt corrupted data
    EXPECT_FALSE(manager->decrypt(key_id, corrupted).has_value());

    // Original data should still be decryptable
    auto decrypted = manager->decrypt(key_id, encrypted.value());
    ASSERT_TRUE(decrypted.has_value());
    EXPECT_EQ(decrypted.value(), data);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 