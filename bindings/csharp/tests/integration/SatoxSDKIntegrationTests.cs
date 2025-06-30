using System;
using System.Threading.Tasks;
using Xunit;
using Satox.SDK;
using Satox.SDK.Models;
using Satox.SDK.Managers;

namespace Satox.SDK.Tests.Integration
{
    public class SatoxSDKIntegrationTests : IDisposable
    {
        private readonly SatoxManager _satox;
        private readonly BlockchainManager _blockchainManager;
        private readonly WalletManager _walletManager;
        private readonly AssetManager _assetManager;
        private readonly NFTManager _nftManager;
        private readonly IPFSManager _ipfsManager;
        private readonly DatabaseManager _dbManager;
        private readonly APIManager _apiManager;
        private readonly SecurityManager _securityManager;

        public SatoxSDKIntegrationTests()
        {
            var config = new Config
            {
                Network = "testnet",
                ApiEndpoint = "https://testnet-api.satox.com",
                DatabaseConnection = "test_connection_string"
            };

            _satox = new SatoxManager(config);
            _satox.Initialize().Wait();

            _blockchainManager = _satox.GetComponent<BlockchainManager>();
            _walletManager = _satox.GetComponent<WalletManager>();
            _assetManager = _satox.GetComponent<AssetManager>();
            _nftManager = _satox.GetComponent<NFTManager>();
            _ipfsManager = _satox.GetComponent<IPFSManager>();
            _dbManager = _satox.GetComponent<DatabaseManager>();
            _apiManager = _satox.GetComponent<APIManager>();
            _securityManager = _satox.GetComponent<SecurityManager>();
        }

        public void Dispose()
        {
            _satox.Shutdown().Wait();
        }

        [Fact]
        public async Task TestWalletCreationAndManagement()
        {
            // Create wallet
            var wallet = await _walletManager.CreateWallet();
            Assert.NotNull(wallet);
            Assert.NotNull(wallet.Address);

            // Backup wallet
            var backup = await _walletManager.BackupWallet(wallet.Address);
            Assert.NotNull(backup);

            // Restore wallet
            var restoredWallet = await _walletManager.RestoreWallet(backup);
            Assert.Equal(wallet.Address, restoredWallet.Address);
        }

        [Fact]
        public async Task TestAssetCreationAndTransfer()
        {
            // Create asset
            var assetData = new AssetData
            {
                Name = "Test Token",
                Symbol = "TEST",
                Decimals = 18,
                TotalSupply = 1000000
            };
            var asset = await _assetManager.CreateAsset(assetData);
            Assert.NotNull(asset);
            Assert.Equal(assetData.Name, asset.Name);

            // Create wallets for transfer
            var fromWallet = await _walletManager.CreateWallet();
            var toWallet = await _walletManager.CreateWallet();

            // Transfer asset
            var transferResult = await _assetManager.TransferAsset(
                fromWallet.Address,
                toWallet.Address,
                asset.Id,
                100
            );
            Assert.True(transferResult.Success);

            // Check balances
            var fromBalance = await _assetManager.GetAssetBalance(fromWallet.Address, asset.Id);
            var toBalance = await _assetManager.GetAssetBalance(toWallet.Address, asset.Id);
            Assert.Equal(100, toBalance);
        }

        [Fact]
        public async Task TestNFTCreationAndTransfer()
        {
            // Create NFT
            var nftData = new NFTData
            {
                Name = "Test NFT",
                Description = "Test Description",
                ImageUrl = "https://test.com/image.png",
                Metadata = new Dictionary<string, string>
                {
                    { "key", "value" }
                }
            };
            var nft = await _nftManager.CreateNFT(nftData);
            Assert.NotNull(nft);
            Assert.Equal(nftData.Name, nft.Name);

            // Create wallets for transfer
            var fromWallet = await _walletManager.CreateWallet();
            var toWallet = await _walletManager.CreateWallet();

            // Transfer NFT
            var transferResult = await _nftManager.TransferNFT(
                fromWallet.Address,
                toWallet.Address,
                nft.Id
            );
            Assert.True(transferResult.Success);

            // Check owner
            var owner = await _nftManager.GetNFTOwner(nft.Id);
            Assert.Equal(toWallet.Address, owner);
        }

        [Fact]
        public async Task TestIPFSOperations()
        {
            // Upload content
            var content = System.Text.Encoding.UTF8.GetBytes("Test content");
            var uploadResult = await _ipfsManager.UploadContent(content);
            Assert.NotNull(uploadResult);
            Assert.NotNull(uploadResult.Hash);

            // Download content
            var downloadedContent = await _ipfsManager.DownloadContent(uploadResult.Hash);
            Assert.Equal(content, downloadedContent);

            // Pin content
            var pinResult = await _ipfsManager.PinContent(uploadResult.Hash);
            Assert.True(pinResult.Success);

            // Search content
            var searchResults = await _ipfsManager.SearchContent("test");
            Assert.NotNull(searchResults);
            Assert.NotEmpty(searchResults);
        }

        [Fact]
        public async Task TestDatabaseOperations()
        {
            // Connect to database
            var connected = await _dbManager.Connect("test_connection_string");
            Assert.True(connected);

            // Execute query
            var query = "SELECT * FROM test_table WHERE id = @id";
            var parameters = new { id = 1 };
            var result = await _dbManager.Query<TestEntity>(query, parameters);
            Assert.NotNull(result);

            // Execute command
            var command = "INSERT INTO test_table (name) VALUES (@name)";
            var commandParams = new { name = "test" };
            var affectedRows = await _dbManager.Execute(command, commandParams);
            Assert.True(affectedRows > 0);
        }

        [Fact]
        public async Task TestAPIOperations()
        {
            // Send GET request
            var response = await _apiManager.Get("/api/v1/test");
            Assert.NotNull(response);
            Assert.True(response.Success);

            // Send POST request
            var data = new { name = "test" };
            var postResponse = await _apiManager.Post("/api/v1/test", data);
            Assert.NotNull(postResponse);
            Assert.True(postResponse.Success);
        }

        [Fact]
        public async Task TestSecurityOperations()
        {
            // Validate request
            var request = new Request
            {
                Method = "GET",
                Path = "/api/v1/test",
                Headers = new Dictionary<string, string>
                {
                    { "Authorization", "Bearer test_token" }
                }
            };
            var isValid = await _securityManager.ValidateRequest(request);
            Assert.True(isValid);

            // Check permissions
            var hasPermission = await _securityManager.CheckPermissions("test_user", "test_resource");
            Assert.True(hasPermission);

            // Generate token
            var token = await _securityManager.GenerateToken("test_user");
            Assert.NotNull(token);
        }

        [Fact]
        public async Task TestErrorHandling()
        {
            // Test invalid block hash
            await Assert.ThrowsAsync<SatoxError>(async () =>
            {
                await _blockchainManager.GetBlock("invalid_hash");
            });

            // Test invalid wallet address
            await Assert.ThrowsAsync<SatoxError>(async () =>
            {
                await _walletManager.GetWallet("invalid_address");
            });

            // Test invalid asset ID
            await Assert.ThrowsAsync<SatoxError>(async () =>
            {
                await _assetManager.GetAsset("invalid_id");
            });
        }

        private class TestEntity
        {
            public int Id { get; set; }
            public string Name { get; set; }
        }
    }
} 