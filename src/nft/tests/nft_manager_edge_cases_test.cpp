#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "nft_manager.hpp"
#include <thread>
#include <future>
#include <chrono>
#include <random>
#include <memory>
#include <stdexcept>

using namespace satox;
using namespace testing;

class NFTManagerEdgeCasesTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<NFTManager>();
        manager->initialize();
    }

    void TearDown() override {
        if (manager) {
            manager->shutdown();
        }
    }

    std::unique_ptr<NFTManager> manager;
};

// ============================================================================
// BOUNDARY VALUE TESTS
// ============================================================================

TEST_F(NFTManagerEdgeCasesTest, EmptyNFTMetadata) {
    // Test creating NFT with empty metadata
    NFTMetadata metadata;
    metadata.name = "";
    metadata.description = "";
    metadata.image_url = "";

    auto result = manager->createNFT(metadata);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error_code, NFTError::INVALID_METADATA);
}

TEST_F(NFTManagerEdgeCasesTest, SingleCharacterName) {
    // Test creating NFT with single character name
    NFTMetadata metadata;
    metadata.name = "a";
    metadata.description = "Test NFT";
    metadata.image_url = "http://example.com/image.jpg";

    auto result = manager->createNFT(metadata);
    EXPECT_TRUE(result.success);
    EXPECT_FALSE(result.token_id.empty());
}

TEST_F(NFTManagerEdgeCasesTest, MaximumNameLength) {
    // Test creating NFT with maximum name length
    NFTMetadata metadata;
    metadata.name = std::string(255, 'x');
    metadata.description = "Test NFT";
    metadata.image_url = "http://example.com/image.jpg";

    auto result = manager->createNFT(metadata);
    EXPECT_TRUE(result.success);
}

TEST_F(NFTManagerEdgeCasesTest, ExceedMaximumNameLength) {
    // Test creating NFT with name exceeding maximum length
    NFTMetadata metadata;
    metadata.name = std::string(256, 'x');
    metadata.description = "Test NFT";
    metadata.image_url = "http://example.com/image.jpg";

    auto result = manager->createNFT(metadata);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error_code, NFTError::NAME_TOO_LONG);
}

TEST_F(NFTManagerEdgeCasesTest, ZeroTokenId) {
    // Test operations with zero token ID
    auto result = manager->getNFT("0");
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error_code, NFTError::INVALID_TOKEN_ID);
}

TEST_F(NFTManagerEdgeCasesTest, MaximumTokenId) {
    // Test operations with maximum token ID
    std::string maxTokenId = std::to_string(std::numeric_limits<uint64_t>::max());
    auto result = manager->getNFT(maxTokenId);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error_code, NFTError::TOKEN_NOT_FOUND);
}

TEST_F(NFTManagerEdgeCasesTest, EmptyOwnerAddress) {
    // Test operations with empty owner address
    auto result = manager->getNFTsByOwner("");
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error_code, NFTError::INVALID_ADDRESS);
}

TEST_F(NFTManagerEdgeCasesTest, SingleCharacterOwnerAddress) {
    // Test operations with single character owner address
    auto result = manager->getNFTsByOwner("a");
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error_code, NFTError::INVALID_ADDRESS);
}

// ============================================================================
// INVALID INPUT TESTS
// ============================================================================

TEST_F(NFTManagerEdgeCasesTest, NullNFTMetadata) {
    // Test creating NFT with null metadata
    NFTMetadata metadata;
    metadata.name = "";
    metadata.description = "";
    metadata.image_url = "";

    auto result = manager->createNFT(metadata);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error_code, NFTError::INVALID_METADATA);
}

TEST_F(NFTManagerEdgeCasesTest, InvalidTokenId) {
    // Test operations with invalid token ID
    auto result = manager->getNFT("invalid_token_id");
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error_code, NFTError::INVALID_TOKEN_ID);
}

TEST_F(NFTManagerEdgeCasesTest, InvalidOwnerAddress) {
    // Test operations with invalid owner address
    auto result = manager->getNFTsByOwner("invalid_address");
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error_code, NFTError::INVALID_ADDRESS);
}

TEST_F(NFTManagerEdgeCasesTest, InvalidImageUrl) {
    // Test creating NFT with invalid image URL
    NFTMetadata metadata;
    metadata.name = "Test NFT";
    metadata.description = "Test Description";
    metadata.image_url = "invalid_url";

    auto result = manager->createNFT(metadata);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error_code, NFTError::INVALID_IMAGE_URL);
}

TEST_F(NFTManagerEdgeCasesTest, DuplicateTokenId) {
    // Test creating NFT with duplicate token ID
    NFTMetadata metadata1;
    metadata1.name = "NFT 1";
    metadata1.description = "Description 1";
    metadata1.image_url = "http://example.com/image1.jpg";

    auto result1 = manager->createNFT(metadata1);
    EXPECT_TRUE(result1.success);

    NFTMetadata metadata2;
    metadata2.name = "NFT 2";
    metadata2.description = "Description 2";
    metadata2.image_url = "http://example.com/image2.jpg";
    metadata2.token_id = result1.token_id; // Use same token ID

    auto result2 = manager->createNFT(metadata2);
    EXPECT_FALSE(result2.success);
    EXPECT_EQ(result2.error_code, NFTError::TOKEN_ID_EXISTS);
}

TEST_F(NFTManagerEdgeCasesTest, InvalidTransfer) {
    // Test invalid transfer scenarios
    auto nft = manager->createNFT({"Test NFT", "Test Description", "http://example.com/image.jpg"});
    EXPECT_TRUE(nft.success);

    auto result = manager->transfer(nft.token_id, "", "receiver");
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error_code, NFTError::INVALID_SENDER);
}

// ============================================================================
// RESOURCE EXHAUSTION TESTS
// ============================================================================

TEST_F(NFTManagerEdgeCasesTest, LargeNumberOfNFTs) {
    // Test creating large number of NFTs
    const int numNFTs = 10000;
    std::vector<std::string> tokenIds;

    for (int i = 0; i < numNFTs; ++i) {
        auto result = manager->createNFT({
            "NFT " + std::to_string(i),
            "Description " + std::to_string(i),
            "http://example.com/image" + std::to_string(i) + ".jpg"
        });
        if (result.success) {
            tokenIds.push_back(result.token_id);
        }
    }

    EXPECT_EQ(tokenIds.size(), numNFTs);
}

TEST_F(NFTManagerEdgeCasesTest, LargeMetadata) {
    // Test creating NFT with large metadata
    NFTMetadata metadata;
    metadata.name = "Large Metadata NFT";
    metadata.description = std::string(10000, 'x'); // 10KB description
    metadata.image_url = "http://example.com/large_image.jpg";

    auto result = manager->createNFT(metadata);
    EXPECT_TRUE(result.success);
}

TEST_F(NFTManagerEdgeCasesTest, MemoryExhaustionNFTCreation) {
    // Test NFT creation under memory pressure
    const int numNFTs = 100000;
    std::vector<std::string> tokenIds;

    for (int i = 0; i < numNFTs; ++i) {
        auto result = manager->createNFT({
            "Memory NFT " + std::to_string(i),
            "Description " + std::to_string(i),
            "http://example.com/image" + std::to_string(i) + ".jpg"
        });
        if (result.success) {
            tokenIds.push_back(result.token_id);
        }
    }

    // Should handle gracefully
    EXPECT_GT(tokenIds.size(), 0);
}

TEST_F(NFTManagerEdgeCasesTest, ExcessiveOwnership) {
    // Test excessive ownership scenarios
    const int numNFTs = 1000;
    std::vector<std::string> tokenIds;

    for (int i = 0; i < numNFTs; ++i) {
        auto result = manager->createNFT({
            "Ownership NFT " + std::to_string(i),
            "Description " + std::to_string(i),
            "http://example.com/image" + std::to_string(i) + ".jpg"
        });
        if (result.success) {
            tokenIds.push_back(result.token_id);
        }
    }

    // Transfer all to same owner
    for (const auto& tokenId : tokenIds) {
        auto transferResult = manager->transfer(tokenId, "creator", "excessive_owner");
        EXPECT_TRUE(transferResult.success);
    }
}

// ============================================================================
// CONCURRENCY TESTS
// ============================================================================

TEST_F(NFTManagerEdgeCasesTest, ConcurrentNFTCreation) {
    const int numThreads = 10;
    const int nftsPerThread = 100;
    std::vector<std::future<std::vector<std::string>>> futures;

    for (int i = 0; i < numThreads; ++i) {
        futures.push_back(std::async(std::launch::async, [this, i, nftsPerThread]() {
            std::vector<std::string> tokenIds;
            for (int j = 0; j < nftsPerThread; ++j) {
                auto result = manager->createNFT({
                    "Thread " + std::to_string(i) + " NFT " + std::to_string(j),
                    "Description " + std::to_string(i) + "_" + std::to_string(j),
                    "http://example.com/image" + std::to_string(i) + "_" + std::to_string(j) + ".jpg"
                });
                if (result.success) {
                    tokenIds.push_back(result.token_id);
                }
            }
            return tokenIds;
        }));
    }

    std::vector<std::string> allTokenIds;
    for (auto& future : futures) {
        auto tokenIds = future.get();
        allTokenIds.insert(allTokenIds.end(), tokenIds.begin(), tokenIds.end());
    }

    EXPECT_EQ(allTokenIds.size(), numThreads * nftsPerThread);
}

TEST_F(NFTManagerEdgeCasesTest, ConcurrentTransfers) {
    auto nft = manager->createNFT({"Concurrent NFT", "Test Description", "http://example.com/image.jpg"});
    EXPECT_TRUE(nft.success);

    const int numThreads = 5;
    const int transfersPerThread = 50;

    std::vector<std::future<bool>> futures;

    for (int i = 0; i < numThreads; ++i) {
        futures.push_back(std::async(std::launch::async, [this, &nft, i, transfersPerThread]() {
            for (int j = 0; j < transfersPerThread; ++j) {
                auto result = manager->transfer(
                    nft.token_id,
                    "sender_" + std::to_string(i),
                    "receiver_" + std::to_string(i) + "_" + std::to_string(j)
                );
                if (!result.success) {
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

TEST_F(NFTManagerEdgeCasesTest, ConcurrentQueries) {
    auto nft = manager->createNFT({"Query NFT", "Test Description", "http://example.com/image.jpg"});
    EXPECT_TRUE(nft.success);

    const int numThreads = 10;
    const int queriesPerThread = 100;

    std::vector<std::future<bool>> futures;

    for (int i = 0; i < numThreads; ++i) {
        futures.push_back(std::async(std::launch::async, [this, &nft, queriesPerThread]() {
            for (int j = 0; j < queriesPerThread; ++j) {
                auto result = manager->getNFT(nft.token_id);
                if (!result.success) {
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

// ============================================================================
// FAILURE INJECTION TESTS
// ============================================================================

TEST_F(NFTManagerEdgeCasesTest, InvalidNFTState) {
    // Test operations on NFT in invalid state
    auto nft = manager->createNFT({"Test NFT", "Test Description", "http://example.com/image.jpg"});
    EXPECT_TRUE(nft.success);

    // Simulate NFT corruption
    auto result = manager->getNFT(nft.token_id);
    EXPECT_TRUE(result.success);

    // Try to operate on corrupted NFT
    auto transferResult = manager->transfer(nft.token_id, "sender", "receiver");
    EXPECT_FALSE(transferResult.success);
    EXPECT_EQ(transferResult.error_code, NFTError::NFT_CORRUPTED);
}

TEST_F(NFTManagerEdgeCasesTest, UnauthorizedTransfer) {
    // Test unauthorized transfer attempts
    auto nft = manager->createNFT({"Test NFT", "Test Description", "http://example.com/image.jpg"});
    EXPECT_TRUE(nft.success);

    auto result = manager->transfer(nft.token_id, "unauthorized_sender", "receiver");
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error_code, NFTError::UNAUTHORIZED);
}

TEST_F(NFTManagerEdgeCasesTest, NFTLocked) {
    // Test operations on locked NFT
    auto nft = manager->createNFT({"Test NFT", "Test Description", "http://example.com/image.jpg"});
    EXPECT_TRUE(nft.success);

    // Lock the NFT
    auto lockResult = manager->lockNFT(nft.token_id);
    EXPECT_TRUE(lockResult.success);

    // Try to transfer locked NFT
    auto transferResult = manager->transfer(nft.token_id, "sender", "receiver");
    EXPECT_FALSE(transferResult.success);
    EXPECT_EQ(transferResult.error_code, NFTError::NFT_LOCKED);
}

TEST_F(NFTManagerEdgeCasesTest, InvalidTransaction) {
    // Test invalid transaction scenarios
    auto nft = manager->createNFT({"Test NFT", "Test Description", "http://example.com/image.jpg"});
    EXPECT_TRUE(nft.success);

    auto result = manager->transferWithTransaction(nft.token_id, "sender", "receiver", "invalid_tx_id");
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error_code, NFTError::INVALID_TRANSACTION);
}

// ============================================================================
// SECURITY VULNERABILITY TESTS
// ============================================================================

TEST_F(NFTManagerEdgeCasesTest, NFTForgery) {
    // Test NFT forgery attempts
    std::string forgedTokenId = "forged_token_id";

    auto result = manager->getNFT(forgedTokenId);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error_code, NFTError::TOKEN_NOT_FOUND);
}

TEST_F(NFTManagerEdgeCasesTest, MetadataManipulation) {
    // Test metadata manipulation attempts
    auto nft = manager->createNFT({"Test NFT", "Test Description", "http://example.com/image.jpg"});
    EXPECT_TRUE(nft.success);

    // Try to manipulate metadata directly
    auto result = manager->updateMetadata(nft.token_id, {"Manipulated NFT", "Manipulated Description", "http://example.com/manipulated.jpg"});
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error_code, NFTError::UNAUTHORIZED);
}

TEST_F(NFTManagerEdgeCasesTest, OwnershipHijacking) {
    // Test ownership hijacking attempts
    auto nft = manager->createNFT({"Test NFT", "Test Description", "http://example.com/image.jpg"});
    EXPECT_TRUE(nft.success);

    // Try to hijack ownership
    auto result = manager->setOwner(nft.token_id, "hijacker");
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error_code, NFTError::UNAUTHORIZED);
}

TEST_F(NFTManagerEdgeCasesTest, MaliciousMetadata) {
    // Test malicious metadata upload
    NFTMetadata maliciousMetadata;
    maliciousMetadata.name = "Malicious NFT";
    maliciousMetadata.description = "Malicious\x00description<script>alert('xss')</script>";
    maliciousMetadata.image_url = "javascript:alert('xss')";

    auto result = manager->createNFT(maliciousMetadata);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error_code, NFTError::MALICIOUS_METADATA);
}

// ============================================================================
// UNUSUAL USAGE PATTERN TESTS
// ============================================================================

TEST_F(NFTManagerEdgeCasesTest, RapidNFTCreation) {
    // Test rapid NFT creation
    const int numNFTs = 1000;
    std::vector<std::string> tokenIds;

    for (int i = 0; i < numNFTs; ++i) {
        auto result = manager->createNFT({
            "Rapid NFT " + std::to_string(i),
            "Description " + std::to_string(i),
            "http://example.com/image" + std::to_string(i) + ".jpg"
        });
        if (result.success) {
            tokenIds.push_back(result.token_id);
        }
    }

    EXPECT_EQ(tokenIds.size(), numNFTs);
}

TEST_F(NFTManagerEdgeCasesTest, NFTMerging) {
    // Test NFT merging functionality
    auto nft1 = manager->createNFT({"NFT 1", "Description 1", "http://example.com/image1.jpg"});
    auto nft2 = manager->createNFT({"NFT 2", "Description 2", "http://example.com/image2.jpg"});
    EXPECT_TRUE(nft1.success);
    EXPECT_TRUE(nft2.success);

    auto mergeResult = manager->mergeNFTs(nft1.token_id, nft2.token_id);
    EXPECT_TRUE(mergeResult.success);

    // Original NFTs should be invalidated
    auto info1 = manager->getNFT(nft1.token_id);
    EXPECT_FALSE(info1.success);

    auto info2 = manager->getNFT(nft2.token_id);
    EXPECT_FALSE(info2.success);
}

TEST_F(NFTManagerEdgeCasesTest, NFTSplitting) {
    // Test NFT splitting functionality
    auto nft = manager->createNFT({"Split NFT", "Split Description", "http://example.com/split.jpg"});
    EXPECT_TRUE(nft.success);

    auto splitResult = manager->splitNFT(nft.token_id, 2);
    EXPECT_TRUE(splitResult.success);
    EXPECT_EQ(splitResult.new_token_ids.size(), 2);
}

TEST_F(NFTManagerEdgeCasesTest, NFTBurning) {
    // Test NFT burning functionality
    auto nft = manager->createNFT({"Burn NFT", "Burn Description", "http://example.com/burn.jpg"});
    EXPECT_TRUE(nft.success);

    auto burnResult = manager->burnNFT(nft.token_id);
    EXPECT_TRUE(burnResult.success);

    auto info = manager->getNFT(nft.token_id);
    EXPECT_FALSE(info.success);
    EXPECT_EQ(info.error_code, NFTError::TOKEN_BURNED);
}

// ============================================================================
// STRESS TESTS
// ============================================================================

TEST_F(NFTManagerEdgeCasesTest, StressTestNFTCreation) {
    const int numNFTs = 10000;
    std::vector<std::string> tokenIds;

    for (int i = 0; i < numNFTs; ++i) {
        auto result = manager->createNFT({
            "Stress NFT " + std::to_string(i),
            "Description " + std::to_string(i),
            "http://example.com/image" + std::to_string(i) + ".jpg"
        });
        if (result.success) {
            tokenIds.push_back(result.token_id);
        }
    }

    EXPECT_EQ(tokenIds.size(), numNFTs);
}

TEST_F(NFTManagerEdgeCasesTest, StressTestTransfers) {
    auto nft = manager->createNFT({"Stress Transfer NFT", "Test Description", "http://example.com/image.jpg"});
    EXPECT_TRUE(nft.success);

    const int numTransfers = 10000;

    for (int i = 0; i < numTransfers; ++i) {
        auto result = manager->transfer(
            nft.token_id,
            "sender_" + std::to_string(i % 100),
            "receiver_" + std::to_string(i)
        );
        EXPECT_TRUE(result.success);
    }
}

TEST_F(NFTManagerEdgeCasesTest, StressTestQueries) {
    auto nft = manager->createNFT({"Stress Query NFT", "Test Description", "http://example.com/image.jpg"});
    EXPECT_TRUE(nft.success);

    const int numQueries = 10000;

    for (int i = 0; i < numQueries; ++i) {
        auto result = manager->getNFT(nft.token_id);
        EXPECT_TRUE(result.success);
    }
}

// ============================================================================
// MEMORY LEAK TESTS
// ============================================================================

TEST_F(NFTManagerEdgeCasesTest, MemoryLeakNFTCreation) {
    const int numIterations = 1000;

    for (int i = 0; i < numIterations; ++i) {
        auto result = manager->createNFT({
            "Leak Test NFT " + std::to_string(i),
            "Description " + std::to_string(i),
            "http://example.com/image" + std::to_string(i) + ".jpg"
        });
        EXPECT_TRUE(result.success);
        // NFT should be automatically cleaned up when manager is destroyed
    }
}

TEST_F(NFTManagerEdgeCasesTest, MemoryLeakTransfers) {
    auto nft = manager->createNFT({"Leak Transfer NFT", "Test Description", "http://example.com/image.jpg"});
    EXPECT_TRUE(nft.success);

    const int numIterations = 1000;

    for (int i = 0; i < numIterations; ++i) {
        auto result = manager->transfer(
            nft.token_id,
            "sender_" + std::to_string(i % 10),
            "receiver_" + std::to_string(i)
        );
        EXPECT_TRUE(result.success);
        // Transfer records should be automatically cleaned up
    }
}

// ============================================================================
// EXCEPTION SAFETY TESTS
// ============================================================================

TEST_F(NFTManagerEdgeCasesTest, ExceptionSafetyNFTCreation) {
    try {
        auto result = manager->createNFT({"Exception Test NFT", "Test Description", "http://example.com/image.jpg"});
        EXPECT_TRUE(result.success);
    } catch (const std::exception& e) {
        FAIL() << "Unexpected exception: " << e.what();
    }
}

TEST_F(NFTManagerEdgeCasesTest, ExceptionSafetyTransfer) {
    auto nft = manager->createNFT({"Exception Transfer NFT", "Test Description", "http://example.com/image.jpg"});
    EXPECT_TRUE(nft.success);

    try {
        auto result = manager->transfer(nft.token_id, "sender", "receiver");
        EXPECT_TRUE(result.success);
    } catch (const std::exception& e) {
        FAIL() << "Unexpected exception: " << e.what();
    }
}

TEST_F(NFTManagerEdgeCasesTest, ExceptionSafetyInvalidInput) {
    try {
        auto result = manager->createNFT({"", "", ""});
        EXPECT_FALSE(result.success);
    } catch (const std::exception& e) {
        FAIL() << "Unexpected exception: " << e.what();
    }
}

// ============================================================================
// INTEGRATION EDGE CASES
// ============================================================================

TEST_F(NFTManagerEdgeCasesTest, IntegrationWithAssetManager) {
    // Test integration with asset manager
    auto nft = manager->createNFT({"Integration NFT", "Test Description", "http://example.com/image.jpg"});
    EXPECT_TRUE(nft.success);

    // Simulate asset manager integration
    auto assetResult = manager->createAssetFromNFT(nft.token_id);
    EXPECT_TRUE(assetResult.success);
}

TEST_F(NFTManagerEdgeCasesTest, IntegrationWithIPFSManager) {
    // Test integration with IPFS manager
    auto nft = manager->createNFT({"IPFS NFT", "Test Description", "http://example.com/image.jpg"});
    EXPECT_TRUE(nft.success);

    // Simulate IPFS manager integration
    auto ipfsResult = manager->uploadToIPFS(nft.token_id);
    EXPECT_TRUE(ipfsResult.success);
}

// ============================================================================
// PERFORMANCE EDGE CASES
// ============================================================================

TEST_F(NFTManagerEdgeCasesTest, PerformanceUnderLoad) {
    const int numOperations = 1000;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < numOperations; ++i) {
        auto result = manager->createNFT({
            "Performance NFT " + std::to_string(i),
            "Description " + std::to_string(i),
            "http://example.com/image" + std::to_string(i) + ".jpg"
        });
        EXPECT_TRUE(result.success);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Should complete within reasonable time
    EXPECT_LT(duration.count(), 10000); // 10 seconds
}

TEST_F(NFTManagerEdgeCasesTest, PerformanceWithLargeMetadata) {
    NFTMetadata largeMetadata;
    largeMetadata.name = "Large Metadata NFT";
    largeMetadata.description = std::string(10000, 'x'); // 10KB description
    largeMetadata.image_url = "http://example.com/large_image.jpg";

    auto start = std::chrono::high_resolution_clock::now();

    auto result = manager->createNFT(largeMetadata);
    EXPECT_TRUE(result.success);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Should complete within reasonable time
    EXPECT_LT(duration.count(), 5000); // 5 seconds
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);

    // Set up test environment
    std::cout << "Running NFT Manager Edge Cases Tests..." << std::endl;

    int result = RUN_ALL_TESTS();

    std::cout << "NFT Manager Edge Cases Tests completed." << std::endl;
    return result;
}
