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

#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <functional>
#include <nlohmann/json.hpp>
#include <chrono>

namespace satox::core {

class WalletManager {
public:
    // Wallet types
    enum class WalletType {
        HD,         // Hierarchical Deterministic
        LEGACY,     // Legacy wallet
        MULTISIG,   // Multi-signature wallet
        WATCH_ONLY  // Watch-only wallet
    };

    // Wallet states
    enum class WalletState {
        UNINITIALIZED,
        INITIALIZED,
        UNLOCKED,
        LOCKED,
        ERROR
    };

    // Transaction types
    enum class TransactionType {
        SEND,
        RECEIVE,
        STAKE,
        UNSTAKE,
        ASSET_ISSUE,
        ASSET_TRANSFER,
        ASSET_BURN,
        NFT_MINT,
        NFT_TRANSFER,
        NFT_BURN
    };

    // Transaction states
    enum class TransactionState {
        PENDING,
        CONFIRMED,
        FAILED,
        DROPPED
    };

    // Wallet information structure
    struct WalletInfo {
        std::string id;
        std::string name;
        WalletType type;
        WalletState state;
        std::string address;
        std::string publicKey;
        std::string encryptedPrivateKey;
        std::string passwordHash;
        std::chrono::system_clock::time_point createdAt;
        std::chrono::system_clock::time_point lastModified;
        nlohmann::json metadata;
    };

    // Transaction information structure
    struct TransactionInfo {
        std::string id;
        std::string walletId;
        TransactionType type;
        TransactionState state;
        std::string fromAddress;
        std::string toAddress;
        double amount;
        std::string assetId;
        std::string txHash;
        int confirmations;
        std::chrono::system_clock::time_point timestamp;
        nlohmann::json metadata;
    };

    // Wallet statistics structure
    struct WalletStats {
        size_t totalWallets;
        size_t activeWallets;
        size_t lockedWallets;
        size_t errorWallets;
        size_t totalTransactions;
        size_t pendingTransactions;
        size_t confirmedTransactions;
        size_t failedTransactions;
        double totalBalance;
        std::map<std::string, double> assetBalances;
        std::chrono::system_clock::time_point lastUpdated;
    };

    // Callback types
    using WalletCallback = std::function<void(const std::string&, WalletState)>;
    using TransactionCallback = std::function<void(const std::string&, TransactionState)>;
    using BalanceCallback = std::function<void(const std::string&, double)>;
    using ErrorCallback = std::function<void(const std::string&, const std::string&)>;

    // Singleton instance
    static WalletManager& getInstance();

    // Initialization and shutdown
    bool initialize(const nlohmann::json& config);
    void shutdown();

    // Wallet management
    bool createWallet(const std::string& id, const std::string& name, WalletType type);
    bool importWallet(const std::string& id, const std::string& name, const std::string& privateKey);
    bool exportWallet(const std::string& id, std::string& privateKey);
    bool deleteWallet(const std::string& id);
    bool lockWallet(const std::string& id);
    bool unlockWallet(const std::string& id, const std::string& password);
    bool isWalletLocked(const std::string& id) const;
    bool isWalletExists(const std::string& id) const;
    WalletInfo getWalletInfo(const std::string& id) const;
    std::vector<WalletInfo> getAllWallets() const;
    std::vector<WalletInfo> getWalletsByType(WalletType type) const;
    std::vector<WalletInfo> getWalletsByState(WalletState state) const;

    // Address management
    std::string generateAddress(const std::string& walletId);
    bool validateAddress(const std::string& address) const;
    std::string getAddressFromPrivateKey(const std::string& privateKey) const;
    std::string getPublicKeyFromPrivateKey(const std::string& privateKey) const;

    // Transaction management
    bool createTransaction(const std::string& walletId, TransactionType type,
                          const std::string& toAddress, double amount,
                          const std::string& assetId = "");
    bool signTransaction(const std::string& walletId, const std::string& txId);
    bool broadcastTransaction(const std::string& walletId, const std::string& txId);
    bool confirmTransaction(const std::string& walletId, const std::string& txId);
    bool failTransaction(const std::string& walletId, const std::string& txId);
    TransactionInfo getTransactionInfo(const std::string& walletId, const std::string& txId) const;
    std::vector<TransactionInfo> getTransactionsByType(const std::string& walletId,
                                                     TransactionType type) const;
    std::vector<TransactionInfo> getTransactionsByState(const std::string& walletId,
                                                      TransactionState state) const;

    // Balance management
    double getBalance(const std::string& walletId) const;
    double getAssetBalance(const std::string& walletId, const std::string& assetId) const;
    std::map<std::string, double> getAllBalances(const std::string& walletId) const;
    bool updateBalance(const std::string& walletId, double amount);
    bool updateAssetBalance(const std::string& walletId, const std::string& assetId,
                           double amount);

    // Statistics
    WalletStats getStats() const;
    void resetStats();
    bool enableStats(bool enable);

    // Callback registration
    void registerWalletCallback(WalletCallback callback);
    void registerTransactionCallback(TransactionCallback callback);
    void registerBalanceCallback(BalanceCallback callback);
    void registerErrorCallback(ErrorCallback callback);
    void unregisterWalletCallback();
    void unregisterTransactionCallback();
    void unregisterBalanceCallback();
    void unregisterErrorCallback();

    // Error handling
    std::string getLastError() const;
    void clearLastError();
    void setLastError(const std::string& error);

private:
    WalletManager() = default;
    ~WalletManager() = default;
    WalletManager(const WalletManager&) = delete;
    WalletManager& operator=(const WalletManager&) = delete;

    // Helper methods
    bool validateConfig(const nlohmann::json& config) const;
    bool validateWalletId(const std::string& id) const;
    bool validatePrivateKey(const std::string& privateKey) const;
    bool checkWalletLimit() const;
    void updateWalletState(const std::string& id, WalletState state);
    void updateTransactionState(const std::string& walletId, const std::string& txId,
                              TransactionState state);
    void updateWalletStats();
    void notifyWalletChange(const std::string& id, WalletState state);
    void notifyTransactionChange(const std::string& walletId, const std::string& txId,
                               TransactionState state);
    void notifyBalanceChange(const std::string& walletId, double balance);
    void notifyError(const std::string& id, const std::string& error);

    // Address and transaction helper methods
    std::string generateHDAddress(const std::string& walletId);
    std::string generateLegacyAddress(const std::string& walletId);
    std::string generateTransactionId();
    std::string encryptPrivateKey(const std::string& privateKey);
    std::string decryptPrivateKey(const std::string& encryptedKey);
    bool verifyPassword(const std::string& walletId, const std::string& password);
    std::string signTransactionData(const std::string& txId, const std::string& encryptedPrivateKey);
    std::string broadcastTransactionToNetwork(const TransactionInfo& tx);

    // Additional helper methods
    std::string hashWalletId(const std::string& walletId);
    std::string generateChecksum(const std::string& data);
    std::string hashString(const std::string& data);
    std::string generateEncryptionKey();
    std::string encryptData(const std::string& data, const std::string& key);
    std::string decryptData(const std::string& encrypted, const std::string& key);
    std::string createTransactionData(const std::string& txId);
    std::string signData(const std::string& data, const std::string& privateKey);
    bool validateTransaction(const TransactionInfo& tx);
    std::string generateTransactionHash(const TransactionInfo& tx);

    // Member variables
    mutable std::mutex mutex_;
    bool initialized_ = false;
    bool statsEnabled_ = false;
    std::string lastError_;
    std::map<std::string, WalletInfo> wallets_;
    std::map<std::string, std::vector<TransactionInfo>> transactions_;
    std::map<std::string, std::map<std::string, double>> balances_;
    WalletStats stats_;
    std::vector<WalletCallback> walletCallbacks_;
    std::vector<TransactionCallback> transactionCallbacks_;
    std::vector<BalanceCallback> balanceCallbacks_;
    std::vector<ErrorCallback> errorCallbacks_;
};

} // namespace satox::core 