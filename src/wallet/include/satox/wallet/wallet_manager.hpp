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
#include <chrono>
#include <nlohmann/json.hpp>

namespace satox {
namespace wallet {

// SLIP-0044 constants for Satox
constexpr uint32_t SATOX_COIN_TYPE = 9007;  // Official Satox coin type
constexpr uint32_t PURPOSE = 44;            // BIP-44 purpose
constexpr uint32_t ACCOUNT_INDEX = 0;       // Default account index

// KAWPOW constants
constexpr uint32_t KAWPOW_EPOCH_LENGTH = 7500;
constexpr uint32_t KAWPOW_PERIOD_LENGTH = 3;
constexpr uint32_t KAWPOW_CACHE_ROUNDS = 2048;

// Multi-signature constants
constexpr uint32_t MAX_SIGNERS = 15;
constexpr uint32_t MIN_SIGNERS = 1;

class WalletManager {
public:
    // Wallet structure
    struct Wallet {
        std::string id;
        std::string name;
        std::vector<uint8_t> publicKey;
        std::vector<uint8_t> encryptedPrivateKey;
        std::string mnemonic;
        std::vector<uint8_t> seed;
        std::string derivationPath;
        std::chrono::system_clock::time_point createdAt;
        std::chrono::system_clock::time_point lastAccessed;
        nlohmann::json metadata;
        bool isMultisig;
        uint32_t requiredSignatures;
        std::vector<std::string> cosigners;
        std::string hardwareWalletId;
    };

    // Key pair structure
    struct KeyPair {
        std::vector<uint8_t> privateKey;
        std::vector<uint8_t> publicKey;
        std::string address;
        std::string fingerprint;
        uint32_t derivationIndex;
        bool isHardwareKey;
    };

    // Hardware wallet structure
    struct HardwareWallet {
        std::string id;
        std::string type;
        std::string name;
        std::string model;
        std::string firmwareVersion;
        bool isConnected;
        std::vector<std::string> supportedFeatures;
        nlohmann::json metadata;
    };

    // Multi-signature transaction structure
    struct MultisigTransaction {
        std::string txid;
        std::string walletId;
        std::vector<std::string> inputs;
        std::vector<std::string> outputs;
        uint64_t amount;
        std::string assetName;
        std::vector<std::string> signatures;
        uint32_t requiredSignatures;
        uint32_t currentSignatures;
        std::chrono::system_clock::time_point createdAt;
        std::chrono::system_clock::time_point expiresAt;
        nlohmann::json metadata;
    };

    // Asset structure
    struct Asset {
        std::string name;
        std::string symbol;
        uint64_t amount;
        std::string owner;
        nlohmann::json metadata;
    };

    // IPFS data structure
    struct IPFSData {
        std::string hash;
        std::vector<uint8_t> data;
        std::string owner;
        nlohmann::json metadata;
    };

    // Wallet status
    enum class Status {
        ACTIVE,
        LOCKED,
        DISABLED,
        PENDING_SIGNATURE,
        EXPIRED
    };

    // Hardware wallet types
    enum class HardwareWalletType {
        LEDGER,
        TREZOR,
        COLD_CARD,
        BITBOX,
        UNKNOWN
    };

    // Callback types
    using WalletCallback = std::function<void(const Wallet&, Status)>;
    using HardwareWalletCallback = std::function<void(const HardwareWallet&, bool)>;
    using TransactionCallback = std::function<void(const MultisigTransaction&)>;

    // Constructor and destructor
    WalletManager();
    ~WalletManager();

    // Singleton pattern
    static WalletManager& getInstance();

    // Initialization and shutdown
    bool initialize();
    void shutdown();
    bool isInitialized() const;
    bool isHealthy() const;

    // Wallet management
    std::string createWallet(const std::string& name);
    std::string createMultisigWallet(const std::string& name, uint32_t requiredSignatures, const std::vector<std::string>& cosigners);
    std::string importWalletFromMnemonic(const std::string& mnemonic, const std::string& name);
    std::string importWalletFromPrivateKey(const std::vector<uint8_t>& privateKey, const std::string& name);
    bool exportWallet(const std::string& walletId, std::string& mnemonic);
    bool deleteWallet(const std::string& walletId);
    bool lockWallet(const std::string& walletId);
    bool unlockWallet(const std::string& walletId, const std::string& password);
    Status getWalletStatus(const std::string& walletId) const;
    bool getWallet(const std::string& walletId, Wallet& wallet) const;

    // Hardware wallet management
    std::vector<HardwareWallet> listHardwareWallets() const;
    bool connectHardwareWallet(const std::string& walletId);
    bool disconnectHardwareWallet(const std::string& walletId);
    bool isHardwareWalletConnected(const std::string& walletId) const;
    bool getHardwareWalletInfo(const std::string& walletId, HardwareWallet& wallet) const;
    bool signWithHardwareWallet(const std::string& walletId, const std::vector<uint8_t>& data, std::vector<uint8_t>& signature);

    // Multi-signature operations
    bool createMultisigTransaction(const std::string& walletId, const std::vector<std::string>& inputs,
                                 const std::vector<std::string>& outputs, uint64_t amount,
                                 const std::string& assetName, MultisigTransaction& transaction);
    bool signMultisigTransaction(const std::string& walletId, const std::string& txid, std::vector<uint8_t>& signature);
    bool verifyMultisigTransaction(const MultisigTransaction& transaction) const;
    bool broadcastMultisigTransaction(const std::string& txid);
    std::vector<MultisigTransaction> getPendingMultisigTransactions(const std::string& walletId) const;

    // Key management
    bool deriveKeyPair(const std::string& walletId, KeyPair& keyPair);
    bool deriveHardwareKeyPair(const std::string& walletId, uint32_t index, KeyPair& keyPair);
    bool validateAddress(const std::string& address) const;
    std::string deriveAddress(const std::vector<uint8_t>& publicKey) const;
    bool signMessage(const std::string& walletId, const std::string& message, std::vector<uint8_t>& signature);
    bool verifySignature(const std::string& address, const std::string& message, const std::vector<uint8_t>& signature) const;
    bool backupKeys(const std::string& walletId, const std::string& backupPath);
    bool restoreKeys(const std::string& walletId, const std::string& backupPath);

    // Asset management
    bool createAsset(const std::string& walletId, const Asset& asset);
    bool transferAsset(const std::string& walletId, const std::string& assetName, const std::string& toAddress, uint64_t amount);
    bool getAssetBalance(const std::string& walletId, const std::string& assetName, uint64_t& balance);
    std::vector<Asset> getWalletAssets(const std::string& walletId);

    // IPFS management
    bool addIPFSData(const std::string& walletId, const std::vector<uint8_t>& data, std::string& hash);
    bool getIPFSData(const std::string& hash, std::vector<uint8_t>& data);
    bool pinIPFSData(const std::string& walletId, const std::string& hash);
    bool unpinIPFSData(const std::string& walletId, const std::string& hash);
    std::vector<IPFSData> getWalletIPFSData(const std::string& walletId);

    // Wallet monitoring
    void registerCallback(WalletCallback callback);
    void unregisterCallback(WalletCallback callback);
    void registerHardwareWalletCallback(HardwareWalletCallback callback);
    void unregisterHardwareWalletCallback(HardwareWalletCallback callback);
    void registerTransactionCallback(TransactionCallback callback);
    void unregisterTransactionCallback(TransactionCallback callback);

    // Error handling
    std::string getLastError() const;
    void clearLastError();

private:
    // Internal helper methods
    bool generateMnemonic(std::string& mnemonic);
    bool mnemonicToSeed(const std::string& mnemonic, std::vector<uint8_t>& seed);
    bool deriveKeyPairFromSeed(const std::vector<uint8_t>& seed, KeyPair& keyPair);
    bool encryptPrivateKey(const std::vector<uint8_t>& privateKey, const std::string& password, std::vector<uint8_t>& encrypted);
    bool decryptPrivateKey(const std::vector<uint8_t>& encrypted, const std::string& password, std::vector<uint8_t>& privateKey);
    std::string generateWalletId() const;
    bool validateDerivationPath(const std::string& path) const;
    std::string getDerivationPath(uint32_t accountIndex = ACCOUNT_INDEX) const;
    bool computeKAWPOWHash(const std::vector<uint8_t>& header, uint64_t nonce, std::vector<uint8_t>& hash);
    bool validateMultisigConfiguration(uint32_t requiredSignatures, const std::vector<std::string>& cosigners) const;
    bool detectHardwareWallets();
    bool initializeHardwareWallet(const std::string& walletId);
    bool verifyHardwareWalletSignature(const std::string& walletId, const std::vector<uint8_t>& data,
                                     const std::vector<uint8_t>& signature) const;
    void setError(const std::string& error);

    // Member variables
    std::map<std::string, Wallet> wallets_;
    std::map<std::string, HardwareWallet> hardwareWallets_;
    std::map<std::string, MultisigTransaction> pendingTransactions_;
    std::vector<WalletCallback> callbacks_;
    std::vector<HardwareWalletCallback> hardwareCallbacks_;
    std::vector<TransactionCallback> transactionCallbacks_;
    mutable std::mutex mutex_;
    std::string lastError_;
    bool initialized_;
};

} // namespace wallet
} // namespace satox 