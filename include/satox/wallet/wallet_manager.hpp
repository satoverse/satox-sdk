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
#include <memory>
#include <mutex>
#include <unordered_map>
#include <functional>
#include <optional>
#include <chrono>

namespace satox {
namespace wallet {

struct WalletConfig {
    std::string data_dir;
    std::string network;  // "mainnet", "testnet", "regtest"
    bool enable_encryption;
    std::string encryption_key;
    uint32_t timeout_ms;
    bool enable_backup;
    std::string backup_dir;
};

struct WalletInfo {
    std::string id;
    std::string name;
    std::string network;
    std::chrono::system_clock::time_point created_time;
    std::chrono::system_clock::time_point last_access_time;
    bool is_encrypted;
    bool is_locked;
    std::unordered_map<std::string, std::string> metadata;
};

struct AddressInfo {
    std::string address;
    std::string public_key;
    std::string derivation_path;
    std::chrono::system_clock::time_point created_time;
    std::chrono::system_clock::time_point last_used_time;
    uint64_t balance;
    std::unordered_map<std::string, std::string> metadata;
};

struct TransactionInfo {
    std::string txid;
    std::string from_address;
    std::string to_address;
    uint64_t amount;
    uint64_t fee;
    std::chrono::system_clock::time_point timestamp;
    std::string status;  // "pending", "confirmed", "failed"
    std::unordered_map<std::string, std::string> metadata;
};

struct WalletStatistics {
    size_t total_wallets;
    size_t total_addresses;
    size_t total_transactions;
    uint64_t total_balance;
    std::chrono::system_clock::time_point last_sync_time;
};

class WalletManager {
public:
    using WalletCallback = std::function<void(const std::string&, const WalletInfo&)>;
    using AddressCallback = std::function<void(const std::string&, const AddressInfo&)>;
    using TransactionCallback = std::function<void(const std::string&, const TransactionInfo&)>;
    using ErrorCallback = std::function<void(const std::string&)>;
    using ProgressCallback = std::function<void(const std::string&, uint32_t)>;

    WalletManager();
    ~WalletManager();

    // Initialization and shutdown
    bool initialize(const WalletConfig& config);
    void shutdown();
    std::string getLastError() const;

    // Wallet management
    std::string createWallet(const std::string& name, const std::string& password,
                           const std::unordered_map<std::string, std::string>& metadata = {});
    bool importWallet(const std::string& name, const std::string& mnemonic,
                     const std::string& password, const std::unordered_map<std::string, std::string>& metadata = {});
    bool removeWallet(const std::string& wallet_id);
    std::optional<WalletInfo> getWalletInfo(const std::string& wallet_id) const;
    std::vector<std::string> listWallets() const;
    bool lockWallet(const std::string& wallet_id);
    bool unlockWallet(const std::string& wallet_id, const std::string& password);
    bool isWalletLocked(const std::string& wallet_id) const;
    bool updateWalletMetadata(const std::string& wallet_id,
                            const std::unordered_map<std::string, std::string>& metadata);

    // Address management
    std::string generateAddress(const std::string& wallet_id,
                              const std::unordered_map<std::string, std::string>& metadata = {});
    bool removeAddress(const std::string& wallet_id, const std::string& address);
    std::optional<AddressInfo> getAddressInfo(const std::string& wallet_id,
                                            const std::string& address) const;
    std::vector<std::string> listAddresses(const std::string& wallet_id) const;
    bool updateAddressMetadata(const std::string& wallet_id, const std::string& address,
                             const std::unordered_map<std::string, std::string>& metadata);

    // Transaction management
    std::string createTransaction(const std::string& wallet_id, const std::string& from_address,
                                const std::string& to_address, uint64_t amount, uint64_t fee,
                                const std::unordered_map<std::string, std::string>& metadata = {});
    bool signTransaction(const std::string& wallet_id, const std::string& txid,
                        const std::string& password);
    bool broadcastTransaction(const std::string& wallet_id, const std::string& txid);
    std::optional<TransactionInfo> getTransactionInfo(const std::string& wallet_id,
                                                    const std::string& txid) const;
    std::vector<std::string> listTransactions(const std::string& wallet_id) const;
    bool updateTransactionMetadata(const std::string& wallet_id, const std::string& txid,
                                 const std::unordered_map<std::string, std::string>& metadata);

    // Balance management
    uint64_t getBalance(const std::string& wallet_id, const std::string& address) const;
    uint64_t getTotalBalance(const std::string& wallet_id) const;

    // Backup and restore
    bool backupWallet(const std::string& wallet_id, const std::string& backup_path);
    bool restoreWallet(const std::string& backup_path, const std::string& password);
    bool verifyBackup(const std::string& backup_path) const;

    // Statistics
    std::optional<WalletStatistics> getStatistics() const;

    // Callback registration
    void registerWalletCallback(WalletCallback callback);
    void registerAddressCallback(AddressCallback callback);
    void registerTransactionCallback(TransactionCallback callback);
    void registerErrorCallback(ErrorCallback callback);
    void registerProgressCallback(ProgressCallback callback);

private:
    // Helper methods
    bool validateConfig(const WalletConfig& config) const;
    bool validateWalletId(const std::string& wallet_id) const;
    bool validateAddress(const std::string& address) const;
    bool validateTransaction(const std::string& txid) const;
    void updateWalletAccessTime(const std::string& wallet_id);
    void updateAddressAccessTime(const std::string& wallet_id, const std::string& address);
    bool persistWallet(const std::string& wallet_id, const WalletInfo& wallet);
    bool loadWallet(const std::string& wallet_id, WalletInfo& wallet);
    void notifyWalletUpdate(const std::string& wallet_id, const WalletInfo& wallet);
    void notifyAddressUpdate(const std::string& wallet_id, const std::string& address,
                           const AddressInfo& address_info);
    void notifyTransactionUpdate(const std::string& wallet_id, const std::string& txid,
                               const TransactionInfo& tx);
    void notifyError(const std::string& error);
    void notifyProgress(const std::string& wallet_id, uint32_t progress);

    // Member variables
    WalletConfig config_;
    std::unordered_map<std::string, WalletInfo> wallets_;
    std::unordered_map<std::string, std::unordered_map<std::string, AddressInfo>> addresses_;
    std::unordered_map<std::string, std::unordered_map<std::string, TransactionInfo>> transactions_;
    std::vector<WalletCallback> walletCallbacks_;
    std::vector<AddressCallback> addressCallbacks_;
    std::vector<TransactionCallback> transactionCallbacks_;
    std::vector<ErrorCallback> errorCallbacks_;
    std::vector<ProgressCallback> progressCallbacks_;
    mutable std::mutex mutex_;
    std::string lastError_;
};

} // namespace wallet
} // namespace satox 