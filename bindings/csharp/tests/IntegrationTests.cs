using System;
using System.Threading.Tasks;
using Xunit;
using SatoxSDK;
using SatoxSDK.Blockchain;
using SatoxSDK.Transactions;
using SatoxSDK.Assets;
using SatoxSDK.Wallet;
using SatoxSDK.Quantum;

namespace SatoxSDK.Tests
{
    public class IntegrationTests
    {
        private readonly SatoxManager _manager;
        private readonly BlockchainManager _blockchainManager;
        private readonly TransactionManager _transactionManager;
        private readonly AssetManager _assetManager;
        private readonly WalletManager _walletManager;
        private readonly QuantumManager _quantumManager;

        public IntegrationTests()
        {
            _manager = new SatoxManager();
            _blockchainManager = new BlockchainManager();
            _transactionManager = new TransactionManager();
            _assetManager = new AssetManager();
            _walletManager = new WalletManager();
            _quantumManager = new QuantumManager();
        }

        [Fact]
        public async Task TestFullTransactionFlow()
        {
            await _manager.InitializeAsync();
            await _blockchainManager.InitializeAsync();
            await _transactionManager.InitializeAsync();
            await _assetManager.InitializeAsync();
            await _walletManager.InitializeAsync();
            await _quantumManager.InitializeAsync();

            // Create wallet
            var wallet = await _walletManager.CreateWalletAsync("test-wallet");
            Assert.NotNull(wallet);

            // Create asset
            var asset = await _assetManager.CreateAssetAsync("test-asset", 1000);
            Assert.NotNull(asset);

            // Create transaction
            var transaction = await _transactionManager.CreateTransactionAsync(
                wallet.Address,
                "recipient-address",
                asset.Id,
                100
            );
            Assert.NotNull(transaction);

            // Sign transaction
            var signedTransaction = await _walletManager.SignTransactionAsync(transaction, wallet);
            Assert.NotNull(signedTransaction);

            // Broadcast transaction
            var result = await _blockchainManager.BroadcastTransactionAsync(signedTransaction);
            Assert.True(result.Success);
        }

        [Fact]
        public async Task TestAssetManagementFlow()
        {
            await _manager.InitializeAsync();
            await _assetManager.InitializeAsync();
            await _walletManager.InitializeAsync();

            // Create wallet
            var wallet = await _walletManager.CreateWalletAsync("test-wallet");
            Assert.NotNull(wallet);

            // Create asset
            var asset = await _assetManager.CreateAssetAsync("test-asset", 1000);
            Assert.NotNull(asset);

            // Transfer asset
            var transferResult = await _assetManager.TransferAssetAsync(
                asset.Id,
                wallet.Address,
                "recipient-address",
                100
            );
            Assert.True(transferResult.Success);

            // Verify balance
            var balance = await _assetManager.GetBalanceAsync(asset.Id, wallet.Address);
            Assert.Equal(900, balance);
        }

        [Fact]
        public async Task TestQuantumSecurityFlow()
        {
            await _manager.InitializeAsync();
            await _quantumManager.InitializeAsync();
            await _walletManager.InitializeAsync();

            // Generate quantum-resistant keys
            var (publicKey, privateKey) = await _quantumManager.GenerateHybridKeyPairAsync();
            Assert.NotNull(publicKey);
            Assert.NotNull(privateKey);

            // Store keys
            await _quantumManager.StoreKeyPairAsync("test-keys", publicKey, privateKey);

            // Encrypt sensitive data
            var sensitiveData = "sensitive-information";
            var encrypted = await _quantumManager.EncryptHybridAsync(sensitiveData, publicKey);
            Assert.NotNull(encrypted);

            // Decrypt data
            var decrypted = await _quantumManager.DecryptHybridAsync(encrypted, privateKey);
            Assert.Equal(sensitiveData, decrypted);
        }
    }
} 