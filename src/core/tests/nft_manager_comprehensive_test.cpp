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
#include "satox/core/nft_manager.hpp"
#include <thread>
#include <future>
#include <random>
#include <chrono>

using namespace satox::core;

class NFTManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = &NFTManager::getInstance();
        manager->initialize(nlohmann::json::object());
    }

    void TearDown() override {
        manager->shutdown();
    }

    NFTManager* manager;
};

// Initialization Tests
TEST_F(NFTManagerTest, Initialization) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    EXPECT_FALSE(manager->initialize(nlohmann::json::object())); // Already initialized
}

// NFT Creation Tests
TEST_F(NFTManagerTest, CreateNFT) {
    NFTMetadata metadata{
        "Test NFT",
        "Test Description",
        "Creator",
        "Owner",
        NFTType::IMAGE,
        "ipfs://test",
        "hash123",
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        nlohmann::json::object(),
        nlohmann::json::object()
    };

    EXPECT_TRUE(manager->createNFT("asset1", metadata));
    EXPECT_FALSE(manager->createNFT("", metadata)); // Invalid asset ID
}

// NFT Minting Tests
TEST_F(NFTManagerTest, MintNFT) {
    NFTMetadata metadata{
        "Test NFT",
        "Test Description",
        "Creator",
        "Owner",
        NFTType::IMAGE,
        "ipfs://test",
        "hash123",
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        nlohmann::json::object(),
        nlohmann::json::object()
    };

    manager->createNFT("asset1", metadata);
    auto nft = manager->getNFT("nft_asset1_0");
    EXPECT_TRUE(manager->mintNFT(nft.id));
    EXPECT_EQ(manager->getNFT(nft.id).state, NFTState::MINTED);
}

// NFT Transfer Tests
TEST_F(NFTManagerTest, TransferNFT) {
    NFTMetadata metadata{
        "Test NFT",
        "Test Description",
        "Creator",
        "Owner",
        NFTType::IMAGE,
        "ipfs://test",
        "hash123",
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        nlohmann::json::object(),
        nlohmann::json::object()
    };

    manager->createNFT("asset1", metadata);
    auto nft = manager->getNFT("nft_asset1_0");
    manager->mintNFT(nft.id);
    EXPECT_TRUE(manager->transferNFT(nft.id, "NewOwner"));
    EXPECT_EQ(manager->getNFT(nft.id).metadata.owner, "NewOwner");
}

// NFT Burning Tests
TEST_F(NFTManagerTest, BurnNFT) {
    NFTMetadata metadata{
        "Test NFT",
        "Test Description",
        "Creator",
        "Owner",
        NFTType::IMAGE,
        "ipfs://test",
        "hash123",
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        nlohmann::json::object(),
        nlohmann::json::object()
    };

    manager->createNFT("asset1", metadata);
    auto nft = manager->getNFT("nft_asset1_0");
    manager->mintNFT(nft.id);
    EXPECT_TRUE(manager->burnNFT(nft.id));
    EXPECT_EQ(manager->getNFT(nft.id).state, NFTState::BURNED);
}

// NFT Metadata Tests
TEST_F(NFTManagerTest, UpdateMetadata) {
    NFTMetadata metadata{
        "Test NFT",
        "Test Description",
        "Creator",
        "Owner",
        NFTType::IMAGE,
        "ipfs://test",
        "hash123",
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        nlohmann::json::object(),
        nlohmann::json::object()
    };

    manager->createNFT("asset1", metadata);
    auto nft = manager->getNFT("nft_asset1_0");

    NFTMetadata newMetadata = metadata;
    newMetadata.name = "Updated NFT";
    EXPECT_TRUE(manager->updateNFTMetadata(nft.id, newMetadata));
    EXPECT_EQ(manager->getNFT(nft.id).metadata.name, "Updated NFT");
}

// NFT Query Tests
TEST_F(NFTManagerTest, QueryNFTs) {
    NFTMetadata metadata1{
        "Test NFT 1",
        "Test Description 1",
        "Creator",
        "Owner1",
        NFTType::IMAGE,
        "ipfs://test1",
        "hash123",
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        nlohmann::json::object(),
        nlohmann::json::object()
    };

    NFTMetadata metadata2{
        "Test NFT 2",
        "Test Description 2",
        "Creator",
        "Owner2",
        NFTType::VIDEO,
        "ipfs://test2",
        "hash456",
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        nlohmann::json::object(),
        nlohmann::json::object()
    };

    manager->createNFT("asset1", metadata1);
    manager->createNFT("asset2", metadata2);

    auto nftsByOwner = manager->getNFTsByOwner("Owner1");
    EXPECT_EQ(nftsByOwner.size(), 1);

    auto nftsByType = manager->getNFTsByType(NFTType::VIDEO);
    EXPECT_EQ(nftsByType.size(), 1);
}

// NFT Attribute Tests
TEST_F(NFTManagerTest, ManageAttributes) {
    NFTMetadata metadata{
        "Test NFT",
        "Test Description",
        "Creator",
        "Owner",
        NFTType::IMAGE,
        "ipfs://test",
        "hash123",
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        nlohmann::json::object(),
        nlohmann::json::object()
    };

    manager->createNFT("asset1", metadata);
    auto nft = manager->getNFT("nft_asset1_0");

    EXPECT_TRUE(manager->addNFTAttribute(nft.id, "color", "red"));
    EXPECT_TRUE(manager->removeNFTAttribute(nft.id, "color"));
}

// NFT History Tests
TEST_F(NFTManagerTest, ManageHistory) {
    NFTMetadata metadata{
        "Test NFT",
        "Test Description",
        "Creator",
        "Owner",
        NFTType::IMAGE,
        "ipfs://test",
        "hash123",
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        nlohmann::json::object(),
        nlohmann::json::object()
    };

    manager->createNFT("asset1", metadata);
    auto nft = manager->getNFT("nft_asset1_0");

    EXPECT_TRUE(manager->addNFTHistoryEntry(nft.id, "Test history entry"));
    auto history = manager->getNFTHistory(nft.id);
    EXPECT_FALSE(history.empty());

    EXPECT_TRUE(manager->clearNFTHistory(nft.id));
    history = manager->getNFTHistory(nft.id);
    EXPECT_TRUE(history.empty());
}

// NFT Statistics Tests
TEST_F(NFTManagerTest, Statistics) {
    NFTMetadata metadata{
        "Test NFT",
        "Test Description",
        "Creator",
        "Owner",
        NFTType::IMAGE,
        "ipfs://test",
        "hash123",
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        nlohmann::json::object(),
        nlohmann::json::object()
    };

    manager->enableStats(true);
    manager->createNFT("asset1", metadata);
    auto nft = manager->getNFT("nft_asset1_0");
    manager->mintNFT(nft.id);

    auto stats = manager->getStats();
    EXPECT_EQ(stats.totalNFTs, 1);
    EXPECT_EQ(stats.mintedNFTs, 1);
}

// Callback Tests
TEST_F(NFTManagerTest, Callbacks) {
    bool nftCallbackCalled = false;
    bool metadataCallbackCalled = false;
    bool errorCallbackCalled = false;

    manager->registerNFTCallback([&](const std::string& id, NFTState state) {
        nftCallbackCalled = true;
    });

    manager->registerMetadataCallback([&](const std::string& id, const NFTMetadata& metadata) {
        metadataCallbackCalled = true;
    });

    manager->registerErrorCallback([&](const std::string& id, const std::string& error) {
        errorCallbackCalled = true;
    });

    NFTMetadata metadata{
        "Test NFT",
        "Test Description",
        "Creator",
        "Owner",
        NFTType::IMAGE,
        "ipfs://test",
        "hash123",
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        nlohmann::json::object(),
        nlohmann::json::object()
    };

    manager->createNFT("asset1", metadata);
    auto nft = manager->getNFT("nft_asset1_0");
    manager->mintNFT(nft.id);

    EXPECT_TRUE(nftCallbackCalled);
    EXPECT_TRUE(metadataCallbackCalled);
}

// Error Handling Tests
TEST_F(NFTManagerTest, ErrorHandling) {
    EXPECT_FALSE(manager->mintNFT("nonexistent"));
    EXPECT_FALSE(manager->getLastError().empty());
    manager->clearLastError();
    EXPECT_TRUE(manager->getLastError().empty());
}

// Concurrency Tests
TEST_F(NFTManagerTest, Concurrency) {
    const int numThreads = 10;
    const int numNFTsPerThread = 100;
    std::vector<std::future<void>> futures;

    for (int i = 0; i < numThreads; ++i) {
        futures.push_back(std::async(std::launch::async, [this, i, numNFTsPerThread]() {
            for (int j = 0; j < numNFTsPerThread; ++j) {
                NFTMetadata metadata{
                    "Test NFT " + std::to_string(i) + "_" + std::to_string(j),
                    "Test Description",
                    "Creator",
                    "Owner",
                    NFTType::IMAGE,
                    "ipfs://test",
                    "hash123",
                    std::chrono::system_clock::now(),
                    std::chrono::system_clock::now(),
                    nlohmann::json::object(),
                    nlohmann::json::object()
                };

                manager->createNFT("asset" + std::to_string(i) + "_" + std::to_string(j), metadata);
            }
        }));
    }

    for (auto& future : futures) {
        future.wait();
    }

    auto stats = manager->getStats();
    EXPECT_EQ(stats.totalNFTs, numThreads * numNFTsPerThread);
}

// Edge Cases Tests
TEST_F(NFTManagerTest, EdgeCases) {
    // Empty metadata
    NFTMetadata emptyMetadata{
        "",
        "",
        "",
        "",
        NFTType::IMAGE,
        "",
        "",
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        nlohmann::json::object(),
        nlohmann::json::object()
    };

    EXPECT_FALSE(manager->createNFT("asset1", emptyMetadata));

    // Long strings
    std::string longString(1000, 'a');
    NFTMetadata longMetadata{
        longString,
        longString,
        longString,
        longString,
        NFTType::IMAGE,
        longString,
        longString,
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        nlohmann::json::object(),
        nlohmann::json::object()
    };

    EXPECT_FALSE(manager->createNFT("asset1", longMetadata));
}

// Cleanup Tests
TEST_F(NFTManagerTest, Cleanup) {
    NFTMetadata metadata{
        "Test NFT",
        "Test Description",
        "Creator",
        "Owner",
        NFTType::IMAGE,
        "ipfs://test",
        "hash123",
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        nlohmann::json::object(),
        nlohmann::json::object()
    };

    manager->createNFT("asset1", metadata);
    manager->shutdown();
    EXPECT_FALSE(manager->isNFTExists("nft_asset1_0"));
}

// Stress Tests
TEST_F(NFTManagerTest, StressTest) {
    const int numNFTs = 10000;
    std::vector<std::string> nftIds;

    for (int i = 0; i < numNFTs; ++i) {
        NFTMetadata metadata{
            "Test NFT " + std::to_string(i),
            "Test Description",
            "Creator",
            "Owner",
            NFTType::IMAGE,
            "ipfs://test",
            "hash123",
            std::chrono::system_clock::now(),
            std::chrono::system_clock::now(),
            nlohmann::json::object(),
            nlohmann::json::object()
        };

        manager->createNFT("asset" + std::to_string(i), metadata);
        nftIds.push_back("nft_asset" + std::to_string(i) + "_0");
    }

    for (const auto& id : nftIds) {
        manager->mintNFT(id);
    }

    auto stats = manager->getStats();
    EXPECT_EQ(stats.totalNFTs, numNFTs);
    EXPECT_EQ(stats.mintedNFTs, numNFTs);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 