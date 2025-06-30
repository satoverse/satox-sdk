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
#include "satox/wallet/wallet_manager.hpp"

using namespace satox::wallet;

class WalletManagerTest : public ::testing::Test {
protected:
    WalletManager* manager;

    void SetUp() override {
        manager = &WalletManager::getInstance();
        manager->shutdown(); // Ensure clean state
        ASSERT_TRUE(manager->initialize());
    }

    void TearDown() override {
        manager->shutdown();
    }
};

TEST_F(WalletManagerTest, Singleton) {
    // Test singleton pattern
    WalletManager& instance1 = WalletManager::getInstance();
    WalletManager& instance2 = WalletManager::getInstance();
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(WalletManagerTest, Initialization) {
    // Test initialization
    EXPECT_TRUE(manager->isInitialized());
    EXPECT_TRUE(manager->isHealthy());
}

TEST_F(WalletManagerTest, ErrorHandling) {
    // Test error handling
    EXPECT_TRUE(manager->getLastError().empty());
    
    // Test with invalid wallet ID for operations that set errors
    WalletManager::KeyPair keyPair;
    EXPECT_FALSE(manager->deriveKeyPair("invalid_id", keyPair));
    EXPECT_FALSE(manager->getLastError().empty());
    
    manager->clearLastError();
    EXPECT_TRUE(manager->getLastError().empty());
}

TEST_F(WalletManagerTest, WalletCreation) {
    // Test basic wallet creation without complex operations
    std::string walletId = manager->createWallet("Test Wallet");
    EXPECT_FALSE(walletId.empty());
    
    // Verify wallet exists with basic check
    WalletManager::Wallet wallet;
    bool found = manager->getWallet(walletId, wallet);
    EXPECT_TRUE(found);
    if (found) {
        EXPECT_EQ(wallet.name, "Test Wallet");
        EXPECT_EQ(wallet.id, walletId);
    }
}

TEST_F(WalletManagerTest, WalletManagement) {
    // Create a wallet
    std::string walletId = manager->createWallet("Test Wallet");
    
    // Test wallet status
    EXPECT_EQ(manager->getWalletStatus(walletId), WalletManager::Status::ACTIVE);
    
    // Test wallet locking/unlocking
    EXPECT_TRUE(manager->lockWallet(walletId));
    EXPECT_EQ(manager->getWalletStatus(walletId), WalletManager::Status::LOCKED);
    
    EXPECT_TRUE(manager->unlockWallet(walletId, "password"));
    EXPECT_EQ(manager->getWalletStatus(walletId), WalletManager::Status::ACTIVE);
}

TEST_F(WalletManagerTest, KeyManagement) {
    // Create a wallet
    std::string walletId = manager->createWallet("Test Wallet");
    
    // Test key pair derivation
    WalletManager::KeyPair keyPair;
    EXPECT_TRUE(manager->deriveKeyPair(walletId, keyPair));
    EXPECT_FALSE(keyPair.privateKey.empty());
    EXPECT_FALSE(keyPair.publicKey.empty());
    EXPECT_FALSE(keyPair.address.empty());
}

TEST_F(WalletManagerTest, AddressValidation) {
    // Test address validation
    EXPECT_TRUE(manager->validateAddress("STX1qwertyuiopasdfghjklzxcvbnm"));
    EXPECT_FALSE(manager->validateAddress("invalid_address"));
}

TEST_F(WalletManagerTest, MessageSigning) {
    // Create a wallet
    std::string walletId = manager->createWallet("Test Wallet");
    
    // Test message signing
    std::vector<uint8_t> signature;
    EXPECT_TRUE(manager->signMessage(walletId, "Hello, World!", signature));
    EXPECT_FALSE(signature.empty());
    
    // Test signature verification
    WalletManager::KeyPair keyPair;
    manager->deriveKeyPair(walletId, keyPair);
    EXPECT_TRUE(manager->verifySignature(keyPair.address, "Hello, World!", signature));
}

TEST_F(WalletManagerTest, MultisigWallet) {
    // Test multisig wallet creation
    std::vector<std::string> cosigners = {"cosigner1", "cosigner2"};
    std::string walletId = manager->createMultisigWallet("Multisig Wallet", 2, cosigners);
    EXPECT_FALSE(walletId.empty());
    
    // Verify multisig wallet
    WalletManager::Wallet wallet;
    EXPECT_TRUE(manager->getWallet(walletId, wallet));
    EXPECT_TRUE(wallet.isMultisig);
    EXPECT_EQ(wallet.requiredSignatures, 2);
    EXPECT_EQ(wallet.cosigners, cosigners);
}

TEST_F(WalletManagerTest, AssetManagement) {
    // Create a wallet
    std::string walletId = manager->createWallet("Test Wallet");
    
    // Test asset creation
    WalletManager::Asset asset;
    asset.name = "TestAsset";
    asset.symbol = "TST";
    asset.amount = 1000;
    asset.owner = walletId;
    
    EXPECT_TRUE(manager->createAsset(walletId, asset));
    
    // Test asset balance
    uint64_t balance;
    EXPECT_TRUE(manager->getAssetBalance(walletId, "TestAsset", balance));
    EXPECT_EQ(balance, 1000);
    
    // Test asset transfer
    EXPECT_TRUE(manager->transferAsset(walletId, "TestAsset", "recipient_address", 500));
    
    // Verify balance after transfer
    EXPECT_TRUE(manager->getAssetBalance(walletId, "TestAsset", balance));
    EXPECT_EQ(balance, 500);
}

TEST_F(WalletManagerTest, IPFSData) {
    // Create a wallet
    std::string walletId = manager->createWallet("Test Wallet");
    
    // Test IPFS data addition
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    std::string hash;
    EXPECT_TRUE(manager->addIPFSData(walletId, data, hash));
    EXPECT_FALSE(hash.empty());
    
    // Test IPFS data retrieval
    std::vector<uint8_t> retrievedData;
    EXPECT_TRUE(manager->getIPFSData(hash, retrievedData));
    EXPECT_EQ(retrievedData, data);
    
    // Test IPFS data pinning
    EXPECT_TRUE(manager->pinIPFSData(walletId, hash));
    EXPECT_TRUE(manager->unpinIPFSData(walletId, hash));
}

TEST_F(WalletManagerTest, Callbacks) {
    bool callbackCalled = false;
    std::string callbackWalletId;
    WalletManager::Status callbackStatus;
    
    // Register callback
    manager->registerCallback([&](const WalletManager::Wallet& wallet, WalletManager::Status status) {
        callbackCalled = true;
        callbackWalletId = wallet.id;
        callbackStatus = status;
    });
    
    // Create wallet to trigger callback
    std::string walletId = manager->createWallet("Callback Test");
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(callbackWalletId, walletId);
    
    // Unregister callback
    manager->unregisterCallback(nullptr);
}

TEST_F(WalletManagerTest, HardwareWallet) {
    // Test hardware wallet listing
    auto hardwareWallets = manager->listHardwareWallets();
    // This might be empty if no hardware wallets are connected, which is expected
    
    // Test hardware wallet connection (should fail if no hardware wallet is connected)
    EXPECT_FALSE(manager->connectHardwareWallet("nonexistent_wallet"));
}

TEST_F(WalletManagerTest, BackupAndRestore) {
    // Create a wallet
    std::string walletId = manager->createWallet("Backup Test");
    
    // Test backup
    EXPECT_TRUE(manager->backupKeys(walletId, "backup_test.json"));
    
    // Test restore
    EXPECT_TRUE(manager->restoreKeys(walletId, "backup_test.json"));
    
    // Clean up
    std::filesystem::remove("backup_test.json");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 