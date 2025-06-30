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
#include "satox/nft/nft_manager.hpp"
#include <thread>
#include <chrono>
#include <random>
#include <future>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <memory>

using namespace satox::nft;
using ::testing::_;
using ::testing::Return;
using ::testing::Throw;

class NFTManagerComprehensiveTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<NFTManager>();
        manager->initialize();
        
        // Test data
        testContractAddress = "0x1234567890123456789012345678901234567890";
        testCreator = "0xabcdef1234567890abcdef1234567890abcdef12";
        testRecipient = "0x9876543210987654321098765432109876543210";
        
        // Create test metadata
        testMetadata.name = "Test NFT";
        testMetadata.description = "A test NFT for comprehensive testing";
        testMetadata.image = "ipfs://QmTest123";
        testMetadata.attributes["rarity"] = "legendary";
        testMetadata.properties["type"] = "digital art";
    }

    void TearDown() override {
        manager->shutdown();
    }

    std::unique_ptr<NFTManager> manager;
    std::string testContractAddress;
    std::string testCreator;
    std::string testRecipient;
    NFTMetadata testMetadata;
};

// Basic Functionality Tests
TEST_F(NFTManagerComprehensiveTest, Initialization) {
    EXPECT_TRUE(manager->isInitialized());
    EXPECT_NO_THROW(manager->shutdown());
    EXPECT_NO_THROW(manager->initialize());
}

// NFT Creation Tests
TEST_F(NFTManagerComprehensiveTest, CreateNFT) {
    auto nft = manager->mintNFT(testContractAddress, testCreator, testMetadata);
    EXPECT_FALSE(nft.id.empty());
    EXPECT_EQ(nft.contractAddress, testContractAddress);
    EXPECT_EQ(nft.creator, testCreator);
    EXPECT_EQ(nft.owner, testCreator);
    EXPECT_EQ(nft.metadata.name, testMetadata.name);
    EXPECT_EQ(nft.metadata.description, testMetadata.description);
    EXPECT_EQ(nft.metadata.image, testMetadata.image);
    EXPECT_EQ(nft.status, "active");
}

TEST_F(NFTManagerComprehensiveTest, CreateNFTWithInvalidData) {
    NFTMetadata invalidMetadata;
    EXPECT_THROW(manager->mintNFT("", testCreator, invalidMetadata), std::runtime_error);
    EXPECT_THROW(manager->mintNFT(testContractAddress, "", invalidMetadata), std::runtime_error);
}

// NFT Transfer Tests
TEST_F(NFTManagerComprehensiveTest, TransferNFT) {
    auto nft = manager->mintNFT(testContractAddress, testCreator, testMetadata);
    EXPECT_TRUE(manager->transferNFT(nft.id, testCreator, testRecipient));
    
    auto updatedNFT = manager->getNFT(nft.id);
    EXPECT_EQ(updatedNFT.owner, testRecipient);
}

TEST_F(NFTManagerComprehensiveTest, TransferNFTInvalid) {
    auto nft = manager->mintNFT(testContractAddress, testCreator, testMetadata);
    EXPECT_FALSE(manager->transferNFT(nft.id, testRecipient, testCreator)); // Wrong owner
    EXPECT_FALSE(manager->transferNFT("invalid_id", testCreator, testRecipient)); // Invalid NFT
}

// NFT Metadata Tests
TEST_F(NFTManagerComprehensiveTest, UpdateMetadata) {
    auto nft = manager->mintNFT(testContractAddress, testCreator, testMetadata);
    
    NFTMetadata newMetadata = testMetadata;
    newMetadata.name = "Updated NFT";
    newMetadata.description = "Updated description";
    
    EXPECT_TRUE(manager->updateNFTMetadata(nft.id, newMetadata));
    
    auto updatedNFT = manager->getNFT(nft.id);
    EXPECT_EQ(updatedNFT.metadata.name, "Updated NFT");
    EXPECT_EQ(updatedNFT.metadata.description, "Updated description");
}

TEST_F(NFTManagerComprehensiveTest, UpdateMetadataInvalid) {
    auto nft = manager->mintNFT(testContractAddress, testCreator, testMetadata);
    
    NFTMetadata invalidMetadata;
    EXPECT_FALSE(manager->updateNFTMetadata(nft.id, invalidMetadata));
    EXPECT_FALSE(manager->updateNFTMetadata("invalid_id", testMetadata));
}

// NFT Query Tests
TEST_F(NFTManagerComprehensiveTest, GetNFTsByOwner) {
    auto nft1 = manager->mintNFT(testContractAddress, testCreator, testMetadata);
    auto nft2 = manager->mintNFT(testContractAddress, testCreator, testMetadata);
    
    manager->transferNFT(nft2.id, testCreator, testRecipient);
    
    auto creatorNFTs = manager->getNFTsByOwner(testCreator);
    auto recipientNFTs = manager->getNFTsByOwner(testRecipient);
    
    EXPECT_EQ(creatorNFTs.size(), 1);
    EXPECT_EQ(recipientNFTs.size(), 1);
    EXPECT_EQ(creatorNFTs[0].id, nft1.id);
    EXPECT_EQ(recipientNFTs[0].id, nft2.id);
}

TEST_F(NFTManagerComprehensiveTest, GetNFTsByContract) {
    auto nft1 = manager->mintNFT(testContractAddress, testCreator, testMetadata);
    auto nft2 = manager->mintNFT(testContractAddress, testCreator, testMetadata);
    
    auto contractNFTs = manager->getNFTsByContract(testContractAddress);
    EXPECT_EQ(contractNFTs.size(), 2);
}

// NFT Burn Tests
TEST_F(NFTManagerComprehensiveTest, BurnNFT) {
    auto nft = manager->mintNFT(testContractAddress, testCreator, testMetadata);
    EXPECT_TRUE(manager->burnNFT(nft.id, testCreator));
    
    auto burnedNFT = manager->getNFT(nft.id);
    EXPECT_EQ(burnedNFT.status, "burned");
}

TEST_F(NFTManagerComprehensiveTest, BurnNFTInvalid) {
    auto nft = manager->mintNFT(testContractAddress, testCreator, testMetadata);
    EXPECT_FALSE(manager->burnNFT(nft.id, testRecipient)); // Wrong owner
    EXPECT_FALSE(manager->burnNFT("invalid_id", testCreator)); // Invalid NFT
}

// Concurrency Tests
TEST_F(NFTManagerComprehensiveTest, ConcurrentOperations) {
    auto nft = manager->mintNFT(testContractAddress, testCreator, testMetadata);
    
    std::vector<std::future<bool>> futures;
    for (int i = 0; i < 10; ++i) {
        futures.push_back(std::async(std::launch::async, [&]() {
            return manager->transferNFT(nft.id, testCreator, testRecipient);
        }));
    }
    
    int successCount = 0;
    for (auto& future : futures) {
        if (future.get()) {
            successCount++;
        }
    }
    
    EXPECT_EQ(successCount, 1); // Only one transfer should succeed
}

// Performance Tests
TEST_F(NFTManagerComprehensiveTest, Performance) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Create 1000 NFTs
    for (int i = 0; i < 1000; ++i) {
        manager->mintNFT(testContractAddress, testCreator, testMetadata);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_LT(duration.count(), 5000); // Should complete in less than 5 seconds
}

// Edge Case Tests
TEST_F(NFTManagerComprehensiveTest, EdgeCases) {
    // Test with maximum metadata size
    NFTMetadata largeMetadata = testMetadata;
    for (int i = 0; i < 1000; ++i) {
        largeMetadata.attributes["key" + std::to_string(i)] = "value" + std::to_string(i);
    }
    
    auto nft = manager->mintNFT(testContractAddress, testCreator, largeMetadata);
    EXPECT_FALSE(nft.id.empty());
    
    // Test with special characters in metadata
    testMetadata.name = "NFT with special chars: !@#$%^&*()";
    nft = manager->mintNFT(testContractAddress, testCreator, testMetadata);
    EXPECT_FALSE(nft.id.empty());
}

// Recovery Tests
TEST_F(NFTManagerComprehensiveTest, Recovery) {
    auto nft = manager->mintNFT(testContractAddress, testCreator, testMetadata);
    
    // Simulate a failure during transfer
    manager->shutdown();
    manager->initialize();
    
    // Should be able to recover the NFT state
    auto recoveredNFT = manager->getNFT(nft.id);
    EXPECT_EQ(recoveredNFT.owner, testCreator);
    EXPECT_EQ(recoveredNFT.status, "active");
} 