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
#include "satox/core/wallet_manager.hpp"
#include <thread>
#include <chrono>
#include <random>

using namespace satox::core;

class WalletManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = &WalletManager::getInstance();
        manager->shutdown(); // Ensure clean state
        manager->initialize(nlohmann::json::object());
    }

    void TearDown() override {
        manager->shutdown();
    }

    WalletManager* manager;
};

// Initialization Tests
TEST_F(WalletManagerTest, Initialization) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    EXPECT_FALSE(manager->initialize(nlohmann::json::object())); // Should fail on second init
}

// Wallet Creation Tests
TEST_F(WalletManagerTest, CreateWallet) {
    EXPECT_TRUE(manager->createWallet("wallet1", "Test Wallet", WalletType::HD));
    EXPECT_TRUE(manager->isWalletExists("wallet1"));
    EXPECT_FALSE(manager->createWallet("wallet1", "Test Wallet", WalletType::HD)); // Should fail for duplicate ID
}

// Wallet Import/Export Tests
TEST_F(WalletManagerTest, ImportExportWallet) {
    std::string privateKey = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
    EXPECT_TRUE(manager->importWallet("wallet1", "Test Wallet", privateKey));
    EXPECT_TRUE(manager->isWalletExists("wallet1"));

    std::string exportedKey;
    EXPECT_TRUE(manager->exportWallet("wallet1", exportedKey));
    EXPECT_EQ(exportedKey, privateKey);
}

// Wallet State Management Tests
TEST_F(WalletManagerTest, WalletStateManagement) {
    EXPECT_TRUE(manager->createWallet("wallet1", "Test Wallet", WalletType::HD));
    EXPECT_TRUE(manager->lockWallet("wallet1"));
    EXPECT_TRUE(manager->isWalletLocked("wallet1"));
    EXPECT_TRUE(manager->unlockWallet("wallet1", "password"));
    EXPECT_FALSE(manager->isWalletLocked("wallet1"));
}

// Address Management Tests
TEST_F(WalletManagerTest, AddressManagement) {
    EXPECT_TRUE(manager->createWallet("wallet1", "Test Wallet", WalletType::HD));
    std::string address = manager->generateAddress("wallet1");
    EXPECT_TRUE(manager->validateAddress(address));
    EXPECT_FALSE(manager->validateAddress("invalid_address"));
}

// Transaction Management Tests
TEST_F(WalletManagerTest, TransactionManagement) {
    EXPECT_TRUE(manager->createWallet("wallet1", "Test Wallet", WalletType::HD));
    manager->updateBalance("wallet1", 100.0);

    // Create transaction
    EXPECT_TRUE(manager->createTransaction("wallet1", TransactionType::SEND,
                                         "S1234567890123456789012345678901234567890", 50.0));
    
    // Get transactions
    auto txs = manager->getTransactionsByType("wallet1", TransactionType::SEND);
    EXPECT_EQ(txs.size(), 1);
    EXPECT_EQ(txs[0].amount, 50.0);

    // Sign and broadcast transaction
    EXPECT_TRUE(manager->signTransaction("wallet1", txs[0].id));
    EXPECT_TRUE(manager->broadcastTransaction("wallet1", txs[0].id));
    
    // Confirm transaction
    EXPECT_TRUE(manager->confirmTransaction("wallet1", txs[0].id));
    
    // Get transaction info
    auto txInfo = manager->getTransactionInfo("wallet1", txs[0].id);
    EXPECT_EQ(txInfo.state, TransactionState::CONFIRMED);
}

// Balance Management Tests
TEST_F(WalletManagerTest, BalanceManagement) {
    EXPECT_TRUE(manager->createWallet("wallet1", "Test Wallet", WalletType::HD));
    
    // Update balance
    EXPECT_TRUE(manager->updateBalance("wallet1", 100.0));
    EXPECT_DOUBLE_EQ(manager->getBalance("wallet1"), 100.0);
    
    // Update asset balance
    EXPECT_TRUE(manager->updateAssetBalance("wallet1", "ASSET1", 50.0));
    EXPECT_DOUBLE_EQ(manager->getAssetBalance("wallet1", "ASSET1"), 50.0);
    
    // Get all balances
    auto balances = manager->getAllBalances("wallet1");
    EXPECT_DOUBLE_EQ(balances["SATOX"], 100.0);
    EXPECT_DOUBLE_EQ(balances["ASSET1"], 50.0);
}

// Statistics Tests
TEST_F(WalletManagerTest, Statistics) {
    EXPECT_TRUE(manager->enableStats(true));
    EXPECT_TRUE(manager->createWallet("wallet1", "Test Wallet", WalletType::HD));
    EXPECT_TRUE(manager->createWallet("wallet2", "Test Wallet", WalletType::LEGACY));
    
    manager->updateBalance("wallet1", 100.0);
    manager->updateBalance("wallet2", 200.0);
    
    auto stats = manager->getStats();
    EXPECT_EQ(stats.totalWallets, 2);
    EXPECT_DOUBLE_EQ(stats.totalBalance, 300.0);
    
    manager->resetStats();
    stats = manager->getStats();
    EXPECT_EQ(stats.totalWallets, 0);
    EXPECT_DOUBLE_EQ(stats.totalBalance, 0.0);
}

// Callback Tests
TEST_F(WalletManagerTest, Callbacks) {
    bool walletCallbackCalled = false;
    bool transactionCallbackCalled = false;
    bool balanceCallbackCalled = false;
    bool errorCallbackCalled = false;

    manager->registerWalletCallback(
        [&](const std::string& id, WalletState state) {
            walletCallbackCalled = true;
        }
    );

    manager->registerTransactionCallback(
        [&](const std::string& id, TransactionState state) {
            transactionCallbackCalled = true;
        }
    );

    manager->registerBalanceCallback(
        [&](const std::string& id, double balance) {
            balanceCallbackCalled = true;
        }
    );

    manager->registerErrorCallback(
        [&](const std::string& id, const std::string& error) {
            errorCallbackCalled = true;
        }
    );

    EXPECT_TRUE(manager->createWallet("wallet1", "Test Wallet", WalletType::HD));
    EXPECT_TRUE(walletCallbackCalled);

    manager->updateBalance("wallet1", 100.0);
    EXPECT_TRUE(balanceCallbackCalled);

    EXPECT_TRUE(manager->createTransaction("wallet1", TransactionType::SEND,
                                         "S1234567890123456789012345678901234567890", 50.0));
    EXPECT_TRUE(transactionCallbackCalled);

    EXPECT_FALSE(manager->createWallet("", "Test Wallet", WalletType::HD));
    EXPECT_TRUE(errorCallbackCalled);
}

// Error Handling Tests
TEST_F(WalletManagerTest, ErrorHandling) {
    EXPECT_TRUE(manager->createWallet("wallet1", "Test Wallet", WalletType::HD));
    
    // Test invalid wallet ID
    EXPECT_FALSE(manager->createWallet("", "Test Wallet", WalletType::HD));
    EXPECT_FALSE(manager->getLastError().empty());
    
    // Test non-existent wallet
    EXPECT_FALSE(manager->lockWallet("nonexistent"));
    EXPECT_FALSE(manager->getLastError().empty());
    
    manager->clearLastError();
    EXPECT_TRUE(manager->getLastError().empty());
}

// Concurrency Tests
TEST_F(WalletManagerTest, Concurrency) {
    const int numThreads = 10;
    const int numWalletsPerThread = 100;
    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([this, i, numWalletsPerThread]() {
            for (int j = 0; j < numWalletsPerThread; ++j) {
                std::string id = "wallet_" + std::to_string(i) + "_" + std::to_string(j);
                manager->createWallet(id, "Test Wallet", WalletType::HD);
                manager->updateBalance(id, 100.0);
                manager->createTransaction(id, TransactionType::SEND,
                                        "S1234567890123456789012345678901234567890", 50.0);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    auto stats = manager->getStats();
    EXPECT_EQ(stats.totalWallets, numThreads * numWalletsPerThread);
    EXPECT_DOUBLE_EQ(stats.totalBalance, numThreads * numWalletsPerThread * 100.0);
}

// Edge Cases Tests
TEST_F(WalletManagerTest, EdgeCases) {
    // Test empty wallet ID
    EXPECT_FALSE(manager->createWallet("", "Test Wallet", WalletType::HD));
    
    // Test maximum length IDs
    std::string longId(64, 'a');
    EXPECT_TRUE(manager->createWallet(longId, "Test Wallet", WalletType::HD));
    
    // Test special characters in wallet name
    EXPECT_TRUE(manager->createWallet("wallet1", "Test Wallet !@#$%^&*()", WalletType::HD));
    
    // Test zero amount transaction
    EXPECT_TRUE(manager->createWallet("wallet1", "Test Wallet", WalletType::HD));
    EXPECT_FALSE(manager->createTransaction("wallet1", TransactionType::SEND,
                                          "S1234567890123456789012345678901234567890", 0.0));
}

// Cleanup Tests
TEST_F(WalletManagerTest, Cleanup) {
    EXPECT_TRUE(manager->createWallet("wallet1", "Test Wallet", WalletType::HD));
    manager->updateBalance("wallet1", 100.0);
    
    manager->shutdown();
    EXPECT_FALSE(manager->isWalletExists("wallet1"));
}

// Stress Tests
TEST_F(WalletManagerTest, StressTest) {
    const int numWallets = 1000;
    std::vector<std::string> walletIds;

    // Create many wallets
    for (int i = 0; i < numWallets; ++i) {
        std::string id = "wallet_" + std::to_string(i);
        EXPECT_TRUE(manager->createWallet(id, "Test Wallet", WalletType::HD));
        manager->updateBalance(id, 100.0);
        walletIds.push_back(id);
    }

    // Perform operations on all wallets
    for (const auto& id : walletIds) {
        EXPECT_TRUE(manager->createTransaction(id, TransactionType::SEND,
                                            "S1234567890123456789012345678901234567890", 50.0));
    }

    auto stats = manager->getStats();
    EXPECT_EQ(stats.totalWallets, numWallets);
    EXPECT_DOUBLE_EQ(stats.totalBalance, numWallets * 100.0);
} 