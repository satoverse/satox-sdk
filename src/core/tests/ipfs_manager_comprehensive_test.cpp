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
#include "satox/core/ipfs_manager.hpp"
#include <thread>
#include <chrono>
#include <random>
#include <fstream>
#include <filesystem>

using namespace satox::core;

class IPFSManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = &IPFSManager::getInstance();
        manager->shutdown(); // Ensure clean state
        manager->initialize(nlohmann::json::object());
        
        // Create test files
        createTestFiles();
    }

    void TearDown() override {
        manager->shutdown();
        cleanupTestFiles();
    }

    void createTestFiles() {
        // Create test directory
        std::filesystem::create_directory("test_data");
        
        // Create test files
        std::ofstream textFile("test_data/test.txt");
        textFile << "Hello, World!";
        textFile.close();

        std::ofstream jsonFile("test_data/test.json");
        jsonFile << "{\"key\": \"value\"}";
        jsonFile.close();

        std::ofstream binaryFile("test_data/test.bin", std::ios::binary);
        std::vector<uint8_t> data = {0x00, 0x01, 0x02, 0x03, 0x04};
        binaryFile.write(reinterpret_cast<const char*>(data.data()), data.size());
        binaryFile.close();
    }

    void cleanupTestFiles() {
        std::filesystem::remove_all("test_data");
    }

    IPFSManager* manager;
};

// Initialization Tests
TEST_F(IPFSManagerTest, Initialization) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    EXPECT_FALSE(manager->initialize(nlohmann::json::object())); // Should fail on second init
}

// Node Management Tests
TEST_F(IPFSManagerTest, NodeManagement) {
    std::string address = "/ip4/127.0.0.1/tcp/4001";
    EXPECT_TRUE(manager->connectNode(address));
    EXPECT_TRUE(manager->isNodeConnected(address));
    
    auto nodeInfo = manager->getNodeInfo(address);
    EXPECT_EQ(nodeInfo.address, address);
    EXPECT_EQ(nodeInfo.state, NodeState::CONNECTED);
    
    EXPECT_TRUE(manager->disconnectNode(address));
    EXPECT_FALSE(manager->isNodeConnected(address));
}

// Content Management Tests
TEST_F(IPFSManagerTest, ContentManagement) {
    // Add content
    std::vector<uint8_t> data = {0x00, 0x01, 0x02, 0x03, 0x04};
    EXPECT_TRUE(manager->addContent("test.bin", ContentType::BINARY, data));
    
    // Get content
    std::vector<uint8_t> retrievedData;
    EXPECT_TRUE(manager->getContent("Qm0000000000000000000000000000000000000000000000",
                                  retrievedData));
    EXPECT_EQ(retrievedData, data);
    
    // Pin content
    EXPECT_TRUE(manager->pinContent("Qm0000000000000000000000000000000000000000000000"));
    EXPECT_TRUE(manager->isContentPinned("Qm0000000000000000000000000000000000000000000000"));
    
    // Unpin content
    EXPECT_TRUE(manager->unpinContent("Qm0000000000000000000000000000000000000000000000"));
    EXPECT_FALSE(manager->isContentPinned("Qm0000000000000000000000000000000000000000000000"));
}

// File Operations Tests
TEST_F(IPFSManagerTest, FileOperations) {
    // Add content from file
    EXPECT_TRUE(manager->addContentFromFile("test_data/test.txt"));
    EXPECT_TRUE(manager->addContentFromFile("test_data/test.json"));
    EXPECT_TRUE(manager->addContentFromFile("test_data/test.bin"));
    
    // Add content from directory
    EXPECT_TRUE(manager->addContentFromDirectory("test_data"));
    
    // Get content to file
    EXPECT_TRUE(manager->getContentToFile("Qm0000000000000000000000000000000000000000000000",
                                        "test_data/output.txt"));
}

// Content Search Tests
TEST_F(IPFSManagerTest, ContentSearch) {
    // Add test content
    std::vector<uint8_t> data = {0x00, 0x01, 0x02, 0x03, 0x04};
    EXPECT_TRUE(manager->addContent("test.bin", ContentType::BINARY, data));
    
    // Search by type
    auto typeResults = manager->getContentByType(ContentType::BINARY);
    EXPECT_FALSE(typeResults.empty());
    
    // Search by tag
    EXPECT_TRUE(manager->addContentTag("Qm0000000000000000000000000000000000000000000000",
                                     "test"));
    auto tagResults = manager->getContentByTag("test");
    EXPECT_FALSE(tagResults.empty());
    
    // Search by query
    auto searchResults = manager->searchContent("test");
    EXPECT_FALSE(searchResults.empty());
}

// Metadata Management Tests
TEST_F(IPFSManagerTest, MetadataManagement) {
    // Add test content
    std::vector<uint8_t> data = {0x00, 0x01, 0x02, 0x03, 0x04};
    EXPECT_TRUE(manager->addContent("test.bin", ContentType::BINARY, data));
    
    // Update metadata
    nlohmann::json metadata = {{"key", "value"}};
    EXPECT_TRUE(manager->updateContentMetadata("Qm0000000000000000000000000000000000000000000000",
                                             metadata));
    
    // Update name
    EXPECT_TRUE(manager->updateContentName("Qm0000000000000000000000000000000000000000000000",
                                         "new_name.bin"));
    
    // Update MIME type
    EXPECT_TRUE(manager->updateContentMimeType("Qm0000000000000000000000000000000000000000000000",
                                             "application/octet-stream"));
}

// Statistics Tests
TEST_F(IPFSManagerTest, Statistics) {
    EXPECT_TRUE(manager->enableStats(true));
    
    // Add test content
    std::vector<uint8_t> data = {0x00, 0x01, 0x02, 0x03, 0x04};
    EXPECT_TRUE(manager->addContent("test.bin", ContentType::BINARY, data));
    
    auto stats = manager->getStats();
    EXPECT_EQ(stats.totalContent, 1);
    EXPECT_EQ(stats.totalSize, data.size());
    
    manager->resetStats();
    stats = manager->getStats();
    EXPECT_EQ(stats.totalContent, 0);
    EXPECT_EQ(stats.totalSize, 0);
}

// Callback Tests
TEST_F(IPFSManagerTest, Callbacks) {
    bool nodeCallbackCalled = false;
    bool contentCallbackCalled = false;
    bool errorCallbackCalled = false;

    manager->registerNodeCallback(
        [&](const std::string& address, NodeState state) {
            nodeCallbackCalled = true;
        }
    );

    manager->registerContentCallback(
        [&](const std::string& hash, const ContentInfo& info) {
            contentCallbackCalled = true;
        }
    );

    manager->registerErrorCallback(
        [&](const std::string& id, const std::string& error) {
            errorCallbackCalled = true;
        }
    );

    // Test node callback
    EXPECT_TRUE(manager->connectNode("/ip4/127.0.0.1/tcp/4001"));
    EXPECT_TRUE(nodeCallbackCalled);

    // Test content callback
    std::vector<uint8_t> data = {0x00, 0x01, 0x02, 0x03, 0x04};
    EXPECT_TRUE(manager->addContent("test.bin", ContentType::BINARY, data));
    EXPECT_TRUE(contentCallbackCalled);

    // Test error callback
    EXPECT_FALSE(manager->connectNode("invalid_address"));
    EXPECT_TRUE(errorCallbackCalled);
}

// Error Handling Tests
TEST_F(IPFSManagerTest, ErrorHandling) {
    // Test invalid node address
    EXPECT_FALSE(manager->connectNode("invalid_address"));
    EXPECT_FALSE(manager->getLastError().empty());
    
    // Test non-existent content
    EXPECT_FALSE(manager->getContent("nonexistent", std::vector<uint8_t>()));
    EXPECT_FALSE(manager->getLastError().empty());
    
    manager->clearLastError();
    EXPECT_TRUE(manager->getLastError().empty());
}

// Concurrency Tests
TEST_F(IPFSManagerTest, Concurrency) {
    const int numThreads = 10;
    const int numOperationsPerThread = 100;
    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([this, i, numOperationsPerThread]() {
            for (int j = 0; j < numOperationsPerThread; ++j) {
                std::string address = "/ip4/127.0.0.1/tcp/" + std::to_string(4001 + i);
                manager->connectNode(address);
                
                std::vector<uint8_t> data = {0x00, 0x01, 0x02, 0x03, 0x04};
                manager->addContent("test_" + std::to_string(i) + "_" + std::to_string(j) + ".bin",
                                  ContentType::BINARY, data);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    auto stats = manager->getStats();
    EXPECT_EQ(stats.totalNodes, numThreads);
    EXPECT_EQ(stats.totalContent, numThreads * numOperationsPerThread);
}

// Edge Cases Tests
TEST_F(IPFSManagerTest, EdgeCases) {
    // Test empty content
    EXPECT_FALSE(manager->addContent("empty.bin", ContentType::BINARY,
                                   std::vector<uint8_t>()));
    
    // Test maximum length names
    std::string longName(255, 'a');
    std::vector<uint8_t> data = {0x00, 0x01, 0x02, 0x03, 0x04};
    EXPECT_TRUE(manager->addContent(longName, ContentType::BINARY, data));
    
    // Test special characters in names
    EXPECT_TRUE(manager->addContent("test!@#$%^&*().bin", ContentType::BINARY, data));
}

// Cleanup Tests
TEST_F(IPFSManagerTest, Cleanup) {
    std::vector<uint8_t> data = {0x00, 0x01, 0x02, 0x03, 0x04};
    EXPECT_TRUE(manager->addContent("test.bin", ContentType::BINARY, data));
    
    manager->shutdown();
    EXPECT_FALSE(manager->isNodeConnected("/ip4/127.0.0.1/tcp/4001"));
}

// Stress Tests
TEST_F(IPFSManagerTest, StressTest) {
    const int numContent = 1000;
    std::vector<std::string> contentHashes;

    // Add many content items
    for (int i = 0; i < numContent; ++i) {
        std::vector<uint8_t> data = {0x00, 0x01, 0x02, 0x03, 0x04};
        EXPECT_TRUE(manager->addContent("test_" + std::to_string(i) + ".bin",
                                      ContentType::BINARY, data));
        contentHashes.push_back("Qm0000000000000000000000000000000000000000000000");
    }

    // Perform operations on all content
    for (const auto& hash : contentHashes) {
        EXPECT_TRUE(manager->pinContent(hash));
        EXPECT_TRUE(manager->addContentTag(hash, "test"));
        EXPECT_TRUE(manager->unpinContent(hash));
    }

    auto stats = manager->getStats();
    EXPECT_EQ(stats.totalContent, numContent);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 