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
#include "satox/ipfs/ipfs_manager.hpp"
#include <fstream>
#include <sstream>

namespace satox::ipfs::tests {

class IPFSManagerTest_Initialization_Test : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary test file
        std::ofstream testFile("test_file.txt");
        testFile << "This is a test file for IPFS";
        testFile.close();
    }

    void TearDown() override {
        // Clean up test file
        std::remove("test_file.txt");
        std::remove("downloaded_file.txt");
    }
};

TEST_F(IPFSManagerTest_Initialization_Test, InitializeWithValidConfig) {
    satox::ipfs::Config config;
    config.api_endpoint = "http://127.0.0.1:5001";
    config.gateway_url = "http://127.0.0.1:8080";
    config.timeout_seconds = 30;
    config.enable_pinning = true;
    config.pinning_service = "local";

    auto& manager = IPFSManager::getInstance();
    EXPECT_TRUE(manager.initialize(config));
    EXPECT_TRUE(manager.isInitialized());
}

class IPFSManagerTest_AddFile_Test : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary test file
        std::ofstream testFile("test_file.txt");
        testFile << "This is a test file for IPFS";
        testFile.close();

        // Initialize IPFS manager
        satox::ipfs::Config config;
        config.api_endpoint = "http://127.0.0.1:5001";
        config.gateway_url = "http://127.0.0.1:8080";
        config.timeout_seconds = 30;
        config.enable_pinning = true;
        config.pinning_service = "local";

        auto& manager = IPFSManager::getInstance();
        manager.initialize(config);
    }

    void TearDown() override {
        // Clean up test file
        std::remove("test_file.txt");
        std::remove("downloaded_file.txt");
    }
};

TEST_F(IPFSManagerTest_AddFile_Test, AddFileSuccessfully) {
    auto& manager = IPFSManager::getInstance();
    std::string hash;
    
    EXPECT_TRUE(manager.addFile("test_file.txt", hash));
    EXPECT_FALSE(hash.empty());
}

TEST_F(IPFSManagerTest_AddFile_Test, AddFileDataSuccessfully) {
    auto& manager = IPFSManager::getInstance();
    std::string hash;
    std::string data = "This is test data for IPFS";
    
    EXPECT_TRUE(manager.addFileData(data, hash));
    EXPECT_FALSE(hash.empty());
}

class IPFSManagerTest_GetFile_Test : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary test file
        std::ofstream testFile("test_file.txt");
        testFile << "This is a test file for IPFS";
        testFile.close();

        // Initialize IPFS manager
        satox::ipfs::Config config;
        config.api_endpoint = "http://127.0.0.1:5001";
        config.gateway_url = "http://127.0.0.1:8080";
        config.timeout_seconds = 30;
        config.enable_pinning = true;
        config.pinning_service = "local";

        auto& manager = IPFSManager::getInstance();
        manager.initialize(config);
        
        // Add file to get hash
        manager.addFile("test_file.txt", testHash_);
    }

    void TearDown() override {
        // Clean up test file
        std::remove("test_file.txt");
        std::remove("downloaded_file.txt");
    }

    std::string testHash_;
};

TEST_F(IPFSManagerTest_GetFile_Test, GetFileSuccessfully) {
    auto& manager = IPFSManager::getInstance();
    
    EXPECT_TRUE(manager.getFile(testHash_, "downloaded_file.txt"));
    
    // Verify file content
    std::ifstream downloadedFile("downloaded_file.txt");
    std::string content((std::istreambuf_iterator<char>(downloadedFile)),
                        std::istreambuf_iterator<char>());
    downloadedFile.close();
    
    EXPECT_EQ(content, "This is a test file for IPFS");
}

TEST_F(IPFSManagerTest_GetFile_Test, GetFileDataSuccessfully) {
    auto& manager = IPFSManager::getInstance();
    std::string data;
    
    EXPECT_TRUE(manager.getFile(testHash_, data));
    EXPECT_EQ(data, "This is a test file for IPFS");
}

class IPFSManagerTest_PinFile_Test : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary test file
        std::ofstream testFile("test_file.txt");
        testFile << "This is a test file for IPFS";
        testFile.close();

        // Initialize IPFS manager
        satox::ipfs::Config config;
        config.api_endpoint = "http://127.0.0.1:5001";
        config.gateway_url = "http://127.0.0.1:8080";
        config.timeout_seconds = 30;
        config.enable_pinning = true;
        config.pinning_service = "local";

        auto& manager = IPFSManager::getInstance();
        manager.initialize(config);
        
        // Add file to get hash
        manager.addFile("test_file.txt", testHash_);
    }

    void TearDown() override {
        // Clean up test file
        std::remove("test_file.txt");
    }

    std::string testHash_;
};

TEST_F(IPFSManagerTest_PinFile_Test, PinFileSuccessfully) {
    auto& manager = IPFSManager::getInstance();
    
    EXPECT_TRUE(manager.pinFile(testHash_));
}

TEST_F(IPFSManagerTest_PinFile_Test, GetPinnedFilesSuccessfully) {
    auto& manager = IPFSManager::getInstance();
    
    // Pin the file first
    EXPECT_TRUE(manager.pinFile(testHash_));
    
    // Get pinned files
    std::vector<std::string> pinnedFiles;
    EXPECT_TRUE(manager.getPinnedFiles(pinnedFiles));
    
    // The file should be in the pinned files list
    EXPECT_FALSE(pinnedFiles.empty());
}

class IPFSManagerTest_UnpinFile_Test : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary test file
        std::ofstream testFile("test_file.txt");
        testFile << "This is a test file for IPFS";
        testFile.close();

        // Initialize IPFS manager
        satox::ipfs::Config config;
        config.api_endpoint = "http://127.0.0.1:5001";
        config.gateway_url = "http://127.0.0.1:8080";
        config.timeout_seconds = 30;
        config.enable_pinning = true;
        config.pinning_service = "local";

        auto& manager = IPFSManager::getInstance();
        manager.initialize(config);
        
        // Add file to get hash
        manager.addFile("test_file.txt", testHash_);
        
        // Pin the file first
        manager.pinFile(testHash_);
    }

    void TearDown() override {
        // Clean up test file
        std::remove("test_file.txt");
    }

    std::string testHash_;
};

TEST_F(IPFSManagerTest_UnpinFile_Test, UnpinFileSuccessfully) {
    auto& manager = IPFSManager::getInstance();
    
    EXPECT_TRUE(manager.unpinFile(testHash_));
}

} // namespace satox::ipfs::tests 