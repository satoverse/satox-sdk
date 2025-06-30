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
#include "satox/ipfs/ipfs_manager.hpp"
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <future>
#include <vector>
#include <random>

using namespace satox::ipfs;
using namespace testing;
namespace fs = std::filesystem;

class IPFSManagerComprehensiveTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test directory
        testDir = fs::temp_directory_path() / "ipfs_test";
        fs::create_directories(testDir);
        
        // Create test files
        createTestFile("test1.txt", "Test content 1");
        createTestFile("test2.txt", "Test content 2");
        createTestFile("large.txt", std::string(1024 * 1024, 'a')); // 1MB file
        createTestFile("binary.bin", std::string(1024, '\0')); // Binary file
        createTestFile("empty.txt", ""); // Empty file
        createTestFile("special.txt", "!@#$%^&*()_+{}|:<>?~`-=[]\\;',./"); // Special characters
        createTestFile("unicode.txt", "Hello, 世界!"); // Unicode characters
        createTestFile("multiline.txt", "Line 1\nLine 2\nLine 3"); // Multiline file
        
        // Initialize IPFS manager
        IPFSManager::Config config;
        config.api_endpoint = "http://127.0.0.1:5001";
        ASSERT_TRUE(manager.initialize(config));
    }

    void TearDown() override {
        // Clean up test files
        fs::remove_all(testDir);
    }

    void createTestFile(const std::string& filename, const std::string& content) {
        std::ofstream file(testDir / filename);
        file << content;
        file.close();
    }

    bool fileContentsMatch(const std::string& file1, const std::string& file2) {
        std::ifstream f1(file1, std::ios::binary);
        std::ifstream f2(file2, std::ios::binary);
        return std::equal(
            std::istreambuf_iterator<char>(f1),
            std::istreambuf_iterator<char>(),
            std::istreambuf_iterator<char>(f2)
        );
    }

    IPFSManager manager;
    fs::path testDir;
};

// Basic Functionality Tests
TEST_F(IPFSManagerComprehensiveTest, Initialization) {
    IPFSManager::Config config;
    config.api_endpoint = "http://127.0.0.1:5001";
    EXPECT_TRUE(manager.initialize(config));
    
    // Test invalid endpoint
    config.api_endpoint = "invalid://endpoint";
    EXPECT_FALSE(manager.initialize(config));
    
    // Test empty endpoint
    config.api_endpoint = "";
    EXPECT_FALSE(manager.initialize(config));
    
    // Test malformed endpoint
    config.api_endpoint = "http://";
    EXPECT_FALSE(manager.initialize(config));
}

// File Operations Tests
TEST_F(IPFSManagerComprehensiveTest, FileOperations) {
    std::string hash;
    
    // Test file addition
    EXPECT_TRUE(manager.addFile((testDir / "test1.txt").string(), hash));
    EXPECT_FALSE(hash.empty());
    
    // Test file retrieval
    std::string outputPath = (testDir / "retrieved.txt").string();
    EXPECT_TRUE(manager.getFile(hash, outputPath));
    EXPECT_TRUE(fileContentsMatch((testDir / "test1.txt").string(), outputPath));
    
    // Test file pinning
    EXPECT_TRUE(manager.pinFile(hash));
    
    // Test file unpinning
    EXPECT_TRUE(manager.unpinFile(hash));
    
    // Test duplicate file addition
    std::string hash2;
    EXPECT_TRUE(manager.addFile((testDir / "test1.txt").string(), hash2));
    EXPECT_EQ(hash, hash2); // Same content should produce same hash
}

// Large File Tests
TEST_F(IPFSManagerComprehensiveTest, LargeFileOperations) {
    std::string hash;
    
    // Test large file addition
    EXPECT_TRUE(manager.addFile((testDir / "large.txt").string(), hash));
    EXPECT_FALSE(hash.empty());
    
    // Test large file retrieval
    std::string outputPath = (testDir / "retrieved_large.txt").string();
    EXPECT_TRUE(manager.getFile(hash, outputPath));
    EXPECT_TRUE(fileContentsMatch((testDir / "large.txt").string(), outputPath));
    
    // Test large file pinning
    EXPECT_TRUE(manager.pinFile(hash));
    
    // Test large file unpinning
    EXPECT_TRUE(manager.unpinFile(hash));
}

// Binary File Tests
TEST_F(IPFSManagerComprehensiveTest, BinaryFileOperations) {
    std::string hash;
    
    // Test binary file addition
    EXPECT_TRUE(manager.addFile((testDir / "binary.bin").string(), hash));
    EXPECT_FALSE(hash.empty());
    
    // Test binary file retrieval
    std::string outputPath = (testDir / "retrieved_binary.bin").string();
    EXPECT_TRUE(manager.getFile(hash, outputPath));
    EXPECT_TRUE(fileContentsMatch((testDir / "binary.bin").string(), outputPath));
}

// Empty File Tests
TEST_F(IPFSManagerComprehensiveTest, EmptyFileOperations) {
    std::string hash;
    
    // Test empty file addition
    EXPECT_TRUE(manager.addFile((testDir / "empty.txt").string(), hash));
    EXPECT_FALSE(hash.empty());
    
    // Test empty file retrieval
    std::string outputPath = (testDir / "retrieved_empty.txt").string();
    EXPECT_TRUE(manager.getFile(hash, outputPath));
    EXPECT_TRUE(fileContentsMatch((testDir / "empty.txt").string(), outputPath));
}

// Special Character Tests
TEST_F(IPFSManagerComprehensiveTest, SpecialCharacterOperations) {
    std::string hash;
    
    // Test special character file addition
    EXPECT_TRUE(manager.addFile((testDir / "special.txt").string(), hash));
    EXPECT_FALSE(hash.empty());
    
    // Test special character file retrieval
    std::string outputPath = (testDir / "retrieved_special.txt").string();
    EXPECT_TRUE(manager.getFile(hash, outputPath));
    EXPECT_TRUE(fileContentsMatch((testDir / "special.txt").string(), outputPath));
}

// Unicode Tests
TEST_F(IPFSManagerComprehensiveTest, UnicodeOperations) {
    std::string hash;
    
    // Test unicode file addition
    EXPECT_TRUE(manager.addFile((testDir / "unicode.txt").string(), hash));
    EXPECT_FALSE(hash.empty());
    
    // Test unicode file retrieval
    std::string outputPath = (testDir / "retrieved_unicode.txt").string();
    EXPECT_TRUE(manager.getFile(hash, outputPath));
    EXPECT_TRUE(fileContentsMatch((testDir / "unicode.txt").string(), outputPath));
}

// Multiline File Tests
TEST_F(IPFSManagerComprehensiveTest, MultilineFileOperations) {
    std::string hash;
    
    // Test multiline file addition
    EXPECT_TRUE(manager.addFile((testDir / "multiline.txt").string(), hash));
    EXPECT_FALSE(hash.empty());
    
    // Test multiline file retrieval
    std::string outputPath = (testDir / "retrieved_multiline.txt").string();
    EXPECT_TRUE(manager.getFile(hash, outputPath));
    EXPECT_TRUE(fileContentsMatch((testDir / "multiline.txt").string(), outputPath));
}

// Error Handling Tests
TEST_F(IPFSManagerComprehensiveTest, ErrorHandling) {
    std::string hash;
    
    // Test non-existent file
    EXPECT_FALSE(manager.addFile("non_existent.txt", hash));
    EXPECT_TRUE(hash.empty());
    
    // Test invalid hash
    EXPECT_FALSE(manager.getFile("invalid_hash", "output.txt"));
    
    // Test invalid pin
    EXPECT_FALSE(manager.pinFile("invalid_hash"));
    
    // Test invalid unpin
    EXPECT_FALSE(manager.unpinFile("invalid_hash"));
    
    // Test invalid output path
    EXPECT_FALSE(manager.getFile(hash, "/invalid/path/output.txt"));
    
    // Test file with invalid permissions
    fs::permissions(testDir / "test1.txt", fs::perms::none);
    EXPECT_FALSE(manager.addFile((testDir / "test1.txt").string(), hash));
    fs::permissions(testDir / "test1.txt", fs::perms::all);
}

// Concurrency Tests
TEST_F(IPFSManagerComprehensiveTest, Concurrency) {
    std::vector<std::future<bool>> futures;
    std::vector<std::string> hashes;
    
    // Test concurrent file additions
    for (int i = 0; i < 5; ++i) {
        futures.push_back(std::async(std::launch::async, [&, i]() {
            std::string hash;
            return manager.addFile((testDir / "test1.txt").string(), hash);
        }));
    }
    
    for (auto& future : futures) {
        EXPECT_TRUE(future.get());
    }
    
    // Test concurrent file retrievals
    futures.clear();
    for (int i = 0; i < 5; ++i) {
        futures.push_back(std::async(std::launch::async, [&, i]() {
            std::string outputPath = (testDir / ("retrieved_" + std::to_string(i) + ".txt")).string();
            return manager.getFile(hashes[0], outputPath);
        }));
    }
    
    for (auto& future : futures) {
        EXPECT_TRUE(future.get());
    }
    
    // Test concurrent pinning operations
    futures.clear();
    for (int i = 0; i < 5; ++i) {
        futures.push_back(std::async(std::launch::async, [&, i]() {
            return manager.pinFile(hashes[0]);
        }));
    }
    
    for (auto& future : futures) {
        EXPECT_TRUE(future.get());
    }
}

// Edge Case Tests
TEST_F(IPFSManagerComprehensiveTest, EdgeCases) {
    std::string hash;
    
    // Test empty file
    createTestFile("empty.txt", "");
    EXPECT_TRUE(manager.addFile((testDir / "empty.txt").string(), hash));
    EXPECT_FALSE(hash.empty());
    
    // Test file with special characters
    createTestFile("special.txt", "!@#$%^&*()_+{}|:<>?~`-=[]\\;',./");
    EXPECT_TRUE(manager.addFile((testDir / "special.txt").string(), hash));
    EXPECT_FALSE(hash.empty());
    
    // Test file with unicode characters
    createTestFile("unicode.txt", "Hello, 世界!");
    EXPECT_TRUE(manager.addFile((testDir / "unicode.txt").string(), hash));
    EXPECT_FALSE(hash.empty());
    
    // Test file with maximum path length
    std::string longPath(255, 'a');
    createTestFile(longPath, "test");
    EXPECT_TRUE(manager.addFile((testDir / longPath).string(), hash));
    EXPECT_FALSE(hash.empty());
}

// Performance Tests
TEST_F(IPFSManagerComprehensiveTest, Performance) {
    std::string hash;
    auto start = std::chrono::high_resolution_clock::now();
    
    // Test file addition performance
    EXPECT_TRUE(manager.addFile((testDir / "test1.txt").string(), hash));
    
    auto addDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start);
    EXPECT_LT(addDuration.count(), 1000); // Should add file in less than 1 second
    
    // Test file retrieval performance
    start = std::chrono::high_resolution_clock::now();
    std::string outputPath = (testDir / "retrieved.txt").string();
    EXPECT_TRUE(manager.getFile(hash, outputPath));
    
    auto getDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start);
    EXPECT_LT(getDuration.count(), 1000); // Should retrieve file in less than 1 second
    
    // Test pinning performance
    start = std::chrono::high_resolution_clock::now();
    EXPECT_TRUE(manager.pinFile(hash));
    
    auto pinDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start);
    EXPECT_LT(pinDuration.count(), 1000); // Should pin file in less than 1 second
}

// Cleanup Tests
TEST_F(IPFSManagerComprehensiveTest, Cleanup) {
    std::string hash;
    
    // Add and pin a file
    EXPECT_TRUE(manager.addFile((testDir / "test1.txt").string(), hash));
    EXPECT_TRUE(manager.pinFile(hash));
    
    // Unpin and verify cleanup
    EXPECT_TRUE(manager.unpinFile(hash));
    
    // Verify file can still be retrieved
    std::string outputPath = (testDir / "retrieved.txt").string();
    EXPECT_TRUE(manager.getFile(hash, outputPath));
    EXPECT_TRUE(fileContentsMatch((testDir / "test1.txt").string(), outputPath));
    
    // Test cleanup of temporary files
    fs::path tempDir = fs::temp_directory_path() / "ipfs_temp";
    fs::create_directories(tempDir);
    std::string tempFilePath = (tempDir / "temp.txt").string();
    EXPECT_TRUE(manager.getFile(hash, tempFilePath));
    EXPECT_TRUE(fs::exists(tempFilePath));
    manager.shutdown();
    EXPECT_FALSE(fs::exists(tempFilePath));
} 