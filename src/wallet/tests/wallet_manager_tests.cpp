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

namespace satox::wallet {
namespace test {

class WalletManagerTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test directory
        testDir_ = std::filesystem::temp_directory_path() / "satox_wallet_test";
        std::filesystem::create_directories(testDir_);
        
        // Initialize with Satoxcoin-specific configuration
        NetworkConfig config;
        config.network_id = "satoxcoin";
        config.chain_id = 9007; // SLIP-044 for Satoxcoin
        config.rpc_url = "http://localhost:7777"; // Satoxcoin RPC port
        config.p2p_port = 60777; // Satoxcoin P2P port
        config.timeout = 30;
        config.max_retries = 3;
        
        EXPECT_TRUE(manager_.initialize(config));
    }

    void TearDown() override {
        manager_.shutdown();
        std::filesystem::remove_all(testDir_);
    }

    std::filesystem::path testDir_;
    WalletManager manager_;
};

// Basic Wallet Operations
TEST_F(WalletManagerTests, CreateWalletWithSatoxcoinConfig) {
    std::string walletId = manager_.createWallet("test_wallet");
    EXPECT_FALSE(walletId.empty());
    
    Wallet wallet;
    EXPECT_TRUE(manager_.getWallet(walletId, wallet));
    EXPECT_EQ(wallet.derivationPath, "m/44'/9007'/0'");
}

TEST_F(WalletManagerTests, ImportWalletWithSatoxcoinMnemonic) {
    // Test vector from Satoxcoin's test suite
    std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
    std::string walletId = manager_.importWalletFromMnemonic(mnemonic, "test_wallet");
    EXPECT_FALSE(walletId.empty());
    
    KeyPair keyPair;
    EXPECT_TRUE(manager_.deriveKeyPair(walletId, keyPair));
    std::string address = manager_.deriveAddress(keyPair.publicKey);
    EXPECT_TRUE(address.starts_with("SX")); // Satoxcoin addresses start with SX
}

// Multi-signature Tests
TEST_F(WalletManagerTests, CreateMultisigWallet) {
    std::vector<std::string> cosigners;
    for (int i = 0; i < 3; i++) {
        std::string walletId = manager_.createWallet("cosigner_" + std::to_string(i));
        cosigners.push_back(walletId);
    }
    
    std::string multisigWalletId = manager_.createMultisigWallet("multisig_wallet", 2, cosigners);
    EXPECT_FALSE(multisigWalletId.empty());
    
    Wallet wallet;
    EXPECT_TRUE(manager_.getWallet(multisigWalletId, wallet));
    EXPECT_TRUE(wallet.isMultisig);
    EXPECT_EQ(wallet.requiredSignatures, 2);
    EXPECT_EQ(wallet.cosigners.size(), 3);
}

TEST_F(WalletManagerTests, MultisigTransactionFlow) {
    // Create multisig wallet
    std::vector<std::string> cosigners;
    for (int i = 0; i < 3; i++) {
        std::string walletId = manager_.createWallet("cosigner_" + std::to_string(i));
        cosigners.push_back(walletId);
    }
    std::string multisigWalletId = manager_.createMultisigWallet("multisig_wallet", 2, cosigners);
    
    // Create transaction
    MultisigTransaction transaction;
    EXPECT_TRUE(manager_.createMultisigTransaction(
        multisigWalletId,
        {"input1", "input2"},
        {"output1", "output2"},
        1000,
        "SATOX",
        transaction
    ));
    
    // Sign with first cosigner
    std::vector<uint8_t> signature1;
    EXPECT_TRUE(manager_.signMultisigTransaction(cosigners[0], transaction.txid, signature1));
    
    // Sign with second cosigner
    std::vector<uint8_t> signature2;
    EXPECT_TRUE(manager_.signMultisigTransaction(cosigners[1], transaction.txid, signature2));
    
    // Verify transaction
    EXPECT_TRUE(manager_.verifyMultisigTransaction(transaction));
}

// Hardware Wallet Tests
TEST_F(WalletManagerTests, HardwareWalletDetection) {
    std::vector<HardwareWallet> wallets = manager_.listHardwareWallets();
    // Note: This test might be skipped if no hardware wallet is connected
    if (!wallets.empty()) {
        EXPECT_TRUE(manager_.connectHardwareWallet(wallets[0].id));
        EXPECT_TRUE(manager_.isHardwareWalletConnected(wallets[0].id));
    }
}

// KAWPOW Tests
TEST_F(WalletManagerTests, KAWPOWHashComputation) {
    std::vector<uint8_t> header(80, 0); // Standard block header size
    uint64_t nonce = 12345;
    std::vector<uint8_t> hash;
    
    EXPECT_TRUE(manager_.computeKAWPOWHash(header, nonce, hash));
    EXPECT_EQ(hash.size(), 32); // SHA-256 hash size
    
    // Test with different nonces
    std::vector<uint8_t> hash2;
    EXPECT_TRUE(manager_.computeKAWPOWHash(header, nonce + 1, hash2));
    EXPECT_NE(hash, hash2);
}

// Asset Management Tests
TEST_F(WalletManagerTests, CreateAndTransferSatoxcoinAsset) {
    std::string walletId = manager_.createWallet("test_wallet");
    
    // Create SATOX asset
    Asset asset;
    asset.name = "SATOX";
    asset.symbol = "SATOX";
    asset.amount = 1000000; // 1 SATOX
    asset.owner = walletId;
    EXPECT_TRUE(manager_.createAsset(walletId, asset));
    
    // Create second wallet
    std::string walletId2 = manager_.createWallet("test_wallet2");
    KeyPair keyPair;
    EXPECT_TRUE(manager_.deriveKeyPair(walletId2, keyPair));
    std::string address = manager_.deriveAddress(keyPair.publicKey);
    
    // Transfer SATOX
    EXPECT_TRUE(manager_.transferAsset(walletId, "SATOX", address, 500000)); // 0.5 SATOX
    
    // Verify balances
    uint64_t balance1, balance2;
    EXPECT_TRUE(manager_.getAssetBalance(walletId, "SATOX", balance1));
    EXPECT_TRUE(manager_.getAssetBalance(walletId2, "SATOX", balance2));
    EXPECT_EQ(balance1, 500000);
    EXPECT_EQ(balance2, 500000);
}

// IPFS Integration Tests
TEST_F(WalletManagerTests, IPFSDataWithMetadata) {
    std::string walletId = manager_.createWallet("test_wallet");
    
    // Create test data with metadata
    std::vector<uint8_t> data = {'t', 'e', 's', 't'};
    nlohmann::json metadata = {
        {"type", "test"},
        {"timestamp", std::time(nullptr)},
        {"version", "1.0"}
    };
    
    std::string hash;
    EXPECT_TRUE(manager_.addIPFSData(walletId, data, hash));
    
    // Verify IPFS data
    std::vector<IPFSData> ipfsData = manager_.getWalletIPFSData(walletId);
    EXPECT_EQ(ipfsData.size(), 1);
    EXPECT_EQ(ipfsData[0].hash, hash);
    EXPECT_EQ(ipfsData[0].data, data);
}

// Error Handling Tests
TEST_F(WalletManagerTests, InvalidNetworkConfig) {
    NetworkConfig config;
    config.network_id = "";
    config.chain_id = 0;
    config.rpc_url = "invalid_url";
    config.timeout = 0;
    config.max_retries = 0;
    
    WalletManager invalidManager;
    EXPECT_FALSE(invalidManager.initialize(config));
    EXPECT_FALSE(invalidManager.getLastError().empty());
}

// Concurrency Tests
TEST_F(WalletManagerTests, ConcurrentWalletOperations) {
    std::vector<std::thread> threads;
    std::mutex mutex;
    std::condition_variable cv;
    bool ready = false;
    
    // Create multiple wallets concurrently
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([&, i]() {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock, [&]() { return ready; });
            
            std::string walletId = manager_.createWallet("concurrent_wallet_" + std::to_string(i));
            EXPECT_FALSE(walletId.empty());
            
            KeyPair keyPair;
            EXPECT_TRUE(manager_.deriveKeyPair(walletId, keyPair));
            std::string address = manager_.deriveAddress(keyPair.publicKey);
            EXPECT_TRUE(manager_.validateAddress(address));
        });
    }
    
    // Start all threads
    {
        std::lock_guard<std::mutex> lock(mutex);
        ready = true;
    }
    cv.notify_all();
    
    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }
}

// Performance Tests
TEST_F(WalletManagerTests, LargeDataHandling) {
    std::string walletId = manager_.createWallet("test_wallet");
    
    // Create large IPFS data (1MB)
    std::vector<uint8_t> largeData(1024 * 1024, 0);
    std::string hash;
    EXPECT_TRUE(manager_.addIPFSData(walletId, largeData, hash));
    
    // Measure retrieval time
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<uint8_t> retrievedData;
    EXPECT_TRUE(manager_.getIPFSData(hash, retrievedData));
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_LT(duration.count(), 1000); // Should complete within 1 second
    EXPECT_EQ(retrievedData, largeData);
}

// Recovery Tests
TEST_F(WalletManagerTests, WalletRecovery) {
    // Create and backup wallet
    std::string walletId = manager_.createWallet("test_wallet");
    std::string backupPath = (testDir_ / "wallet_backup.json").string();
    EXPECT_TRUE(manager_.backupKeys(walletId, backupPath));
    
    // Delete wallet
    EXPECT_TRUE(manager_.deleteWallet(walletId));
    
    // Restore wallet
    std::string restoredWalletId = manager_.restoreKeys(walletId, backupPath);
    EXPECT_FALSE(restoredWalletId.empty());
    
    // Verify restored wallet
    Wallet wallet;
    EXPECT_TRUE(manager_.getWallet(restoredWalletId, wallet));
    EXPECT_EQ(wallet.name, "test_wallet");
}

} // namespace test
} // namespace satox::wallet 