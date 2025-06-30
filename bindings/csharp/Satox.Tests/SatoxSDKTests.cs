using System;
using System.Threading;
using System.Threading.Tasks;
using Xunit;
using Satox.Core;
using Satox.API;
using Satox.Transactions;
using Satox.Assets;
using Satox.Security;
using Satox.Database;
using Satox.IPFS;
using Satox.Wallet;

namespace Satox.Tests
{
    public class SatoxSDKTests : IDisposable
    {
        private readonly SatoxSDK _sdk;
        private readonly BlockchainManager _blockchainManager;
        private readonly TransactionManager _transactionManager;
        private readonly AssetManager _assetManager;
        private readonly SecurityManager _securityManager;
        private readonly DatabaseManager _databaseManager;
        private readonly APIManager _apiManager;
        private readonly IPFSManager _ipfsManager;
        private readonly WalletManager _walletManager;

        public SatoxSDKTests()
        {
            _sdk = SatoxSDK.Instance;
            _blockchainManager = _sdk.BlockchainManager;
            _transactionManager = _sdk.TransactionManager;
            _assetManager = _sdk.AssetManager;
            _securityManager = _sdk.SecurityManager;
            _databaseManager = _sdk.DatabaseManager;
            _apiManager = _sdk.APIManager;
            _ipfsManager = _sdk.IPFSManager;
            _walletManager = _sdk.WalletManager;
        }

        public void Dispose()
        {
            _sdk?.Dispose();
        }

        [Fact]
        public void TestSDKInitialization()
        {
            Assert.NotNull(_sdk);
            Assert.NotNull(_blockchainManager);
            Assert.NotNull(_transactionManager);
            Assert.NotNull(_assetManager);
            Assert.NotNull(_securityManager);
            Assert.NotNull(_databaseManager);
            Assert.NotNull(_apiManager);
            Assert.NotNull(_ipfsManager);
            Assert.NotNull(_walletManager);
        }

        [Fact]
        public void TestBlockchainManager()
        {
            // Test blockchain operations
            Assert.True(_blockchainManager.Initialize());
            Assert.True(_blockchainManager.IsConnected);
            Assert.NotNull(_blockchainManager.GetBlockchainInfo());
            Assert.True(_blockchainManager.Shutdown());
        }

        [Fact]
        public void TestTransactionManager()
        {
            // Test transaction operations
            Assert.True(_transactionManager.Initialize());
            Assert.NotNull(_transactionManager.CreateTransaction("from", "to", 1.0, 0.001));
            Assert.True(_transactionManager.ValidateTransaction("txid"));
            Assert.True(_transactionManager.Shutdown());
        }

        [Fact]
        public void TestAssetManager()
        {
            // Test asset operations
            Assert.True(_assetManager.Initialize());
            Assert.NotNull(_assetManager.CreateAsset("TestAsset", "Test Asset", 1000));
            Assert.True(_assetManager.TransferAsset("assetId", "from", "to", 100));
            Assert.True(_assetManager.Shutdown());
        }

        [Fact]
        public void TestSecurityManager()
        {
            // Test security operations
            Assert.True(_securityManager.Initialize());
            Assert.NotNull(_securityManager.GenerateKeyPair());
            Assert.True(_securityManager.SignMessage("message", "privateKey"));
            Assert.True(_securityManager.VerifyMessage("message", "signature", "publicKey"));
            Assert.True(_securityManager.Shutdown());
        }

        [Fact]
        public void TestDatabaseManager()
        {
            // Test database operations
            Assert.True(_databaseManager.Initialize());
            Assert.True(_databaseManager.CreateDatabase("test_db"));
            Assert.True(_databaseManager.CreateTable("test_db", "test_table", "schema"));
            Assert.True(_databaseManager.Insert("test_db", "test_table", "data"));
            Assert.NotNull(_databaseManager.Find("test_db", "test_table", "query"));
            Assert.True(_databaseManager.Shutdown());
        }

        [Fact]
        public void TestAPIManager()
        {
            // Test API operations
            Assert.True(_apiManager.Initialize());
            Assert.True(_apiManager.RegisterEndpoint("/test", "GET", request => "response"));
            Assert.True(_apiManager.RegisterWebSocketHandler("/ws", message => { }));
            Assert.True(_apiManager.Shutdown());
        }

        [Fact]
        public void TestIPFSManager()
        {
            // Test IPFS operations
            Assert.True(_ipfsManager.Initialize());
            Assert.NotNull(_ipfsManager.UploadFile("test.txt", System.Text.Encoding.UTF8.GetBytes("content")));
            Assert.NotNull(_ipfsManager.DownloadFile("hash"));
            Assert.True(_ipfsManager.PinFile("hash"));
            Assert.True(_ipfsManager.Shutdown());
        }

        [Fact]
        public void TestWalletManager()
        {
            // Test wallet operations
            Assert.True(_walletManager.Initialize());
            Assert.NotNull(_walletManager.CreateWallet("test_wallet"));
            Assert.NotNull(_walletManager.GenerateAddress("test_wallet"));
            Assert.True(_walletManager.GetBalance("test_wallet") >= 0);
            Assert.True(_walletManager.Shutdown());
        }

        [Fact]
        public async Task TestConcurrency()
        {
            // Test concurrent operations
            Assert.True(_sdk.IsThreadSafe);

            // Test concurrent blockchain operations
            var tasks = new Task[10];
            for (int i = 0; i < tasks.Length; i++)
            {
                tasks[i] = Task.Run(() =>
                {
                    Assert.True(_blockchainManager.Initialize() &&
                              _blockchainManager.IsConnected &&
                              _blockchainManager.Shutdown());
                });
            }

            await Task.WhenAll(tasks);
        }

        [Fact]
        public void TestErrorHandling()
        {
            // Test error handling
            Assert.Throws<InvalidOperationException>(() =>
            {
                _blockchainManager.Initialize();
                _blockchainManager.Initialize(); // Should throw exception
            });

            Assert.Throws<ArgumentNullException>(() =>
            {
                _transactionManager.CreateTransaction(null, "to", 1.0, 0.001);
            });
        }

        [Fact]
        public void TestRecovery()
        {
            // Test recovery mechanisms
            Assert.True(_blockchainManager.Initialize());
            Assert.True(_blockchainManager.Recover());
            Assert.True(_blockchainManager.Shutdown());

            Assert.True(_transactionManager.Initialize());
            Assert.True(_transactionManager.Recover());
            Assert.True(_transactionManager.Shutdown());
        }

        [Fact]
        public void TestPerformance()
        {
            // Test performance
            var startTime = DateTime.Now;

            // Test blockchain operations performance
            Assert.True(_blockchainManager.Initialize() &&
                       _blockchainManager.IsConnected &&
                       _blockchainManager.Shutdown());

            var endTime = DateTime.Now;
            Assert.True((endTime - startTime).TotalSeconds < 5);

            // Test transaction operations performance
            startTime = DateTime.Now;
            for (int i = 0; i < 1000; i++)
            {
                Assert.NotNull(_transactionManager.CreateTransaction("from", "to", 1.0, 0.001));
            }
            endTime = DateTime.Now;
            Assert.True((endTime - startTime).TotalSeconds < 5);
        }
    }
} 