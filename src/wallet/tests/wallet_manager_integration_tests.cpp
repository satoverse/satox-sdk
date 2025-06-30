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

#include "satox/wallet/wallet_manager.hpp"
#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <random>
#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

namespace satox::wallet {
namespace test {

class WalletManagerIntegrationTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test directory
        testDir_ = std::filesystem::temp_directory_path() / "satox_wallet_integration_test";
        std::filesystem::create_directories(testDir_);
        
        // Initialize with local Satoxcoin node configuration
        NetworkConfig config;
        config.network_id = "satoxcoin";
        config.chain_id = 9007; // SLIP-044 for Satoxcoin
        config.rpc_url = "http://localhost:7777"; // Local Satoxcoin RPC port
        config.p2p_port = 60777; // Local Satoxcoin P2P port
        config.timeout = 30;
        config.max_retries = 3;
        
        EXPECT_TRUE(manager_.initialize(config));
        
        // Wait for node connection
        ASSERT_TRUE(waitForNodeConnection());
    }

    void TearDown() override {
        manager_.shutdown();
        std::filesystem::remove_all(testDir_);
    }

    bool waitForNodeConnection(int maxAttempts = 10) {
        for (int i = 0; i < maxAttempts; i++) {
            if (manager_.isConnected()) {
                return true;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        return false;
    }

    std::filesystem::path testDir_;
    WalletManager manager_;
};

// Network Integration Tests
TEST_F(WalletManagerIntegrationTests, ConnectToLocalNode) {
    EXPECT_TRUE(manager_.isConnected());
    
    // Get network info
    nlohmann::json networkInfo;
    EXPECT_TRUE(manager_.getNetworkInfo(networkInfo));
    EXPECT_EQ(networkInfo["network"], "satoxcoin");
    EXPECT_EQ(networkInfo["chain_id"], 9007);
}

TEST_F(WalletManagerIntegrationTests, GetBlockchainInfo) {
    nlohmann::json blockchainInfo;
    EXPECT_TRUE(manager_.getBlockchainInfo(blockchainInfo));
    EXPECT_GT(blockchainInfo["blocks"].get<int64_t>(), 0);
    EXPECT_GT(blockchainInfo["difficulty"].get<double>(), 0);
    EXPECT_FALSE(blockchainInfo["bestblockhash"].get<std::string>().empty());
}

// Transaction Integration Tests
TEST_F(WalletManagerIntegrationTests, CreateAndBroadcastTransaction) {
    // Create test wallet
    std::string walletId = manager_.createWallet("integration_test_wallet");
    ASSERT_FALSE(walletId.empty());
    
    // Get test address
    KeyPair keyPair;
    EXPECT_TRUE(manager_.deriveKeyPair(walletId, keyPair));
    std::string address = manager_.deriveAddress(keyPair.publicKey);
    
    // Create transaction
    Transaction tx;
    tx.from = address;
    tx.to = address; // Self-transfer for testing
    tx.amount = 100000; // 0.1 SATOX
    tx.fee = 1000;
    
    // Sign and broadcast
    std::vector<uint8_t> signature;
    EXPECT_TRUE(manager_.signTransaction(walletId, tx, signature));
    EXPECT_TRUE(manager_.broadcastTransaction(tx));
    
    // Wait for transaction confirmation
    std::string txid = tx.id;
    bool confirmed = false;
    for (int i = 0; i < 30 && !confirmed; i++) {
        nlohmann::json txInfo;
        if (manager_.getTransaction(txid, txInfo)) {
            if (txInfo["confirmations"].get<int>() > 0) {
                confirmed = true;
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    EXPECT_TRUE(confirmed);
}

// Asset Integration Tests
TEST_F(WalletManagerIntegrationTests, CreateAndTransferAsset) {
    // Create test wallet
    std::string walletId = manager_.createWallet("asset_test_wallet");
    ASSERT_FALSE(walletId.empty());
    
    // Create asset
    Asset asset;
    asset.name = "TEST_ASSET";
    asset.symbol = "TEST";
    asset.amount = 1000000; // 1.0 units
    asset.owner = walletId;
    
    EXPECT_TRUE(manager_.createAsset(walletId, asset));
    
    // Wait for asset creation confirmation
    bool created = false;
    for (int i = 0; i < 30 && !created; i++) {
        std::vector<Asset> assets = manager_.getWalletAssets(walletId);
        for (const auto& a : assets) {
            if (a.name == "TEST_ASSET") {
                created = true;
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    EXPECT_TRUE(created);
    
    // Create second wallet for transfer
    std::string walletId2 = manager_.createWallet("asset_test_wallet2");
    KeyPair keyPair;
    EXPECT_TRUE(manager_.deriveKeyPair(walletId2, keyPair));
    std::string address = manager_.deriveAddress(keyPair.publicKey);
    
    // Transfer asset
    EXPECT_TRUE(manager_.transferAsset(walletId, "TEST_ASSET", address, 500000));
    
    // Wait for transfer confirmation
    bool transferred = false;
    for (int i = 0; i < 30 && !transferred; i++) {
        uint64_t balance;
        if (manager_.getAssetBalance(walletId2, "TEST_ASSET", balance)) {
            if (balance == 500000) {
                transferred = true;
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    EXPECT_TRUE(transferred);
}

// Edge Cases and Error Scenarios
TEST_F(WalletManagerIntegrationTests, InvalidTransactionAmount) {
    std::string walletId = manager_.createWallet("edge_test_wallet");
    ASSERT_FALSE(walletId.empty());
    
    // Try to create transaction with zero amount
    Transaction tx;
    tx.from = manager_.deriveAddress(manager_.deriveKeyPair(walletId, KeyPair()).publicKey);
    tx.to = tx.from;
    tx.amount = 0;
    tx.fee = 1000;
    
    std::vector<uint8_t> signature;
    EXPECT_FALSE(manager_.signTransaction(walletId, tx, signature));
}

TEST_F(WalletManagerIntegrationTests, InsufficientFunds) {
    std::string walletId = manager_.createWallet("edge_test_wallet2");
    ASSERT_FALSE(walletId.empty());
    
    // Try to create transaction with very large amount
    Transaction tx;
    tx.from = manager_.deriveAddress(manager_.deriveKeyPair(walletId, KeyPair()).publicKey);
    tx.to = tx.from;
    tx.amount = 1000000000000; // 1 million SATOX
    tx.fee = 1000;
    
    std::vector<uint8_t> signature;
    EXPECT_FALSE(manager_.signTransaction(walletId, tx, signature));
}

TEST_F(WalletManagerIntegrationTests, DuplicateAssetName) {
    std::string walletId = manager_.createWallet("edge_test_wallet3");
    ASSERT_FALSE(walletId.empty());
    
    // Create first asset
    Asset asset1;
    asset1.name = "DUPLICATE_ASSET";
    asset1.symbol = "DUP";
    asset1.amount = 1000000;
    asset1.owner = walletId;
    
    EXPECT_TRUE(manager_.createAsset(walletId, asset1));
    
    // Try to create asset with same name
    Asset asset2;
    asset2.name = "DUPLICATE_ASSET";
    asset2.symbol = "DUP2";
    asset2.amount = 1000000;
    asset2.owner = walletId;
    
    EXPECT_FALSE(manager_.createAsset(walletId, asset2));
}

TEST_F(WalletManagerIntegrationTests, NetworkDisconnection) {
    // Simulate network disconnection
    manager_.shutdown();
    
    // Try operations while disconnected
    std::string walletId = manager_.createWallet("disconnect_test_wallet");
    EXPECT_TRUE(walletId.empty());
    
    // Reconnect
    NetworkConfig config;
    config.network_id = "satoxcoin";
    config.chain_id = 9007;
    config.rpc_url = "http://localhost:7777";
    config.p2p_port = 60777;
    config.timeout = 30;
    config.max_retries = 3;
    
    EXPECT_TRUE(manager_.initialize(config));
    EXPECT_TRUE(waitForNodeConnection());
}

TEST_F(WalletManagerIntegrationTests, ConcurrentTransactions) {
    std::string walletId = manager_.createWallet("concurrent_test_wallet");
    ASSERT_FALSE(walletId.empty());
    
    // Create multiple transactions concurrently
    std::vector<std::thread> threads;
    std::mutex mutex;
    std::condition_variable cv;
    bool ready = false;
    
    for (int i = 0; i < 5; i++) {
        threads.emplace_back([&, i]() {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock, [&]() { return ready; });
            
            Transaction tx;
            tx.from = manager_.deriveAddress(manager_.deriveKeyPair(walletId, KeyPair()).publicKey);
            tx.to = tx.from;
            tx.amount = 10000 * (i + 1);
            tx.fee = 1000;
            
            std::vector<uint8_t> signature;
            EXPECT_TRUE(manager_.signTransaction(walletId, tx, signature));
            EXPECT_TRUE(manager_.broadcastTransaction(tx));
        });
    }
    
    {
        std::lock_guard<std::mutex> lock(mutex);
        ready = true;
    }
    cv.notify_all();
    
    for (auto& thread : threads) {
        thread.join();
    }
}

TEST_F(WalletManagerIntegrationTests, InvalidAssetTransfer) {
    std::string walletId = manager_.createWallet("invalid_transfer_test_wallet");
    ASSERT_FALSE(walletId.empty());
    
    // Try to transfer non-existent asset
    std::string address = manager_.deriveAddress(manager_.deriveKeyPair(walletId, KeyPair()).publicKey);
    EXPECT_FALSE(manager_.transferAsset(walletId, "NON_EXISTENT_ASSET", address, 1000));
    
    // Try to transfer to invalid address
    EXPECT_FALSE(manager_.transferAsset(walletId, "SATOX", "invalid_address", 1000));
    
    // Try to transfer zero amount
    EXPECT_FALSE(manager_.transferAsset(walletId, "SATOX", address, 0));
}

} // namespace test
} // namespace satox::wallet 