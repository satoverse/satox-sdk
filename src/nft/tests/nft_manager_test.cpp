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
#include "satox/nft/nft_manager.hpp"
#include <thread>
#include <vector>
#include <random>
#include <chrono>

using namespace satox::nft;

class NFTManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = &NFTManager::getInstance();
        // Ensure clean state for each test
        manager->shutdown();
        manager->initialize();
    }

    void TearDown() override {
        manager->shutdown();
    }

    NFTManager* manager;
    std::string testContractAddress = "0x1234567890123456789012345678901234567890";
    std::string testCreator = "0xabcdef1234567890abcdef1234567890abcdef12";
    std::string testOwner = "0xabcdef1234567890abcdef1234567890abcdef12";
    std::string testRecipient = "0x9876543210987654321098765432109876543210";

    NFTManager::NFTMetadata createTestMetadata() {
        NFTManager::NFTMetadata metadata;
        metadata.name = "Test NFT";
        metadata.description = "Test Description";
        metadata.image = "ipfs://test-image";
        metadata.attributes["rarity"] = "common";
        metadata.attributes["type"] = "test";
        return metadata;
    }
};

// Initialization Tests
TEST_F(NFTManagerTest, DoubleInitialization) {
    EXPECT_TRUE(manager->initialize());  // Should return true for second initialization
}

TEST_F(NFTManagerTest, ShutdownAndReinitialize) {
    manager->shutdown();
    EXPECT_TRUE(manager->initialize());
}

// NFT Creation Tests
TEST_F(NFTManagerTest, CreateValidNFT) {
    auto metadata = createTestMetadata();
    std::string nftId = manager->createNFT(testContractAddress, metadata, testCreator, true, true);
    EXPECT_FALSE(nftId.empty());
    
    NFTManager::NFT nft = manager->getNFT(nftId);
    EXPECT_EQ(nft.owner, testCreator);
    EXPECT_EQ(nft.creator, testCreator);
    EXPECT_EQ(nft.contractAddress, testContractAddress);
}

TEST_F(NFTManagerTest, CreateNFTWithInvalidMetadata) {
    NFTManager::NFTMetadata invalidMetadata;
    std::string nftId = manager->createNFT(testContractAddress, invalidMetadata, testCreator, true, true);
    EXPECT_TRUE(nftId.empty());
}

// NFT Transfer Tests
TEST_F(NFTManagerTest, ValidTransfer) {
    auto metadata = createTestMetadata();
    std::string nftId = manager->createNFT(testContractAddress, metadata, testCreator, true, true);
    
    EXPECT_TRUE(manager->transferNFT(nftId, testCreator, testRecipient));
    
    NFTManager::NFT nft = manager->getNFT(nftId);
    EXPECT_EQ(nft.owner, testRecipient);
}

TEST_F(NFTManagerTest, TransferToRestrictedAddress) {
    auto metadata = createTestMetadata();
    std::string nftId = manager->createNFT(testContractAddress, metadata, testCreator, true, true);
    
    EXPECT_FALSE(manager->transferNFT(nftId, testCreator, "0x0000000000000000000000000000000000000000"));
}

TEST_F(NFTManagerTest, TransferNonTransferableNFT) {
    auto metadata = createTestMetadata();
    std::string nftId = manager->createNFT(testContractAddress, metadata, testCreator, false, true);
    
    EXPECT_FALSE(manager->transferNFT(nftId, testCreator, testRecipient));
}

// Ownership Tracking Tests
TEST_F(NFTManagerTest, OwnershipHistory) {
    auto metadata = createTestMetadata();
    std::string nftId = manager->createNFT(testContractAddress, metadata, testCreator, true, true);
    
    EXPECT_TRUE(manager->transferNFT(nftId, testCreator, testRecipient));
    
    auto history = manager->getOwnershipHistory(nftId);
    EXPECT_EQ(history.size(), 2);  // Creation + Transfer
    EXPECT_EQ(history[0].owner, testCreator);
    EXPECT_EQ(history[1].owner, testRecipient);
}

TEST_F(NFTManagerTest, GetNFTsByOwner) {
    auto metadata = createTestMetadata();
    std::string nftId1 = manager->createNFT(testContractAddress, metadata, testCreator, true, true);
    std::string nftId2 = manager->createNFT(testContractAddress, metadata, testCreator, true, true);
    
    auto ownerNFTs = manager->getNFTsByOwner(testCreator);
    EXPECT_EQ(ownerNFTs.size(), 2);
}

// Rate Limiting Tests
TEST_F(NFTManagerTest, RateLimiting) {
    auto metadata = createTestMetadata();
    std::vector<std::string> nftIds;
    
    // Create multiple NFTs
    for (int i = 0; i < 10; i++) {
        nftIds.push_back(manager->createNFT(testContractAddress, metadata, testCreator, true, true));
    }
    
    // Transfer all NFTs
    for (const auto& nftId : nftIds) {
        EXPECT_TRUE(manager->transferNFT(nftId, testCreator, testRecipient));
    }
    
    // Try to transfer more (should fail due to rate limit)
    std::string newNftId = manager->createNFT(testContractAddress, metadata, testCreator, true, true);
    EXPECT_FALSE(manager->transferNFT(newNftId, testCreator, testRecipient));
}

// Concurrency Tests
TEST_F(NFTManagerTest, ConcurrentTransfers) {
    auto metadata = createTestMetadata();
    std::string nftId = manager->createNFT(testContractAddress, metadata, testCreator, true, true);
    
    std::vector<std::thread> threads;
    std::vector<bool> results(10);
    
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([&, i]() {
            results[i] = manager->transferNFT(nftId, testCreator, testRecipient);
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Only one transfer should succeed
    int successCount = std::count(results.begin(), results.end(), true);
    EXPECT_EQ(successCount, 1);
}

// Error Handling Tests
TEST_F(NFTManagerTest, InvalidNFTId) {
    EXPECT_FALSE(manager->transferNFT("invalid_id", testCreator, testRecipient));
    auto error = manager->getLastError();
    EXPECT_FALSE(error.message.empty());
}

TEST_F(NFTManagerTest, InvalidAddresses) {
    auto metadata = createTestMetadata();
    std::string nftId = manager->createNFT(testContractAddress, metadata, testCreator, true, true);
    
    EXPECT_FALSE(manager->transferNFT(nftId, "", testRecipient));
    EXPECT_FALSE(manager->transferNFT(nftId, testCreator, ""));
}

// Metadata Update Tests
TEST_F(NFTManagerTest, UpdateMetadata) {
    auto metadata = createTestMetadata();
    std::string nftId = manager->createNFT(testContractAddress, metadata, testCreator, true, true);
    
    metadata.name = "Updated Name";
    EXPECT_TRUE(manager->updateNFTMetadata(nftId, metadata, testCreator));
    
    NFTManager::NFT nft = manager->getNFT(nftId);
    EXPECT_EQ(nft.metadata.name, "Updated Name");
}

// Burn Tests
TEST_F(NFTManagerTest, BurnNFT) {
    auto metadata = createTestMetadata();
    std::string nftId = manager->createNFT(testContractAddress, metadata, testCreator, true, true);
    
    EXPECT_TRUE(manager->burnNFT(nftId, testCreator));
    
    // Try to get the burned NFT - should return empty NFT
    NFTManager::NFT nft = manager->getNFT(nftId);
    EXPECT_TRUE(nft.id.empty());
}

TEST_F(NFTManagerTest, BurnNonBurnableNFT) {
    auto metadata = createTestMetadata();
    std::string nftId = manager->createNFT(testContractAddress, metadata, testCreator, true, false);
    
    EXPECT_FALSE(manager->burnNFT(nftId, testCreator));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 