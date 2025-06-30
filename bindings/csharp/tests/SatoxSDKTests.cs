using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Xunit;
using Satox.SDK;

namespace Satox.SDK.Tests
{
    public class SatoxSDKTests : IDisposable
    {
        private readonly SatoxSDK _sdk;
        private readonly AssetManager _assetManager;
        private readonly TransactionManager _txManager;
        private readonly IPFSManager _ipfsManager;

        public SatoxSDKTests()
        {
            var config = new SDKConfig
            {
                Network = "testnet",
                RpcPort = 7777,
                P2pPort = 60777
            };

            _sdk = SatoxSDK.Instance;
            _sdk.Initialize(config);

            _assetManager = _sdk.AssetManager;
            _txManager = _sdk.TransactionManager;
            _ipfsManager = _sdk.IPFSManager;
        }

        public void Dispose()
        {
            _sdk.Shutdown();
        }

        [Fact]
        public async Task TestAssetCreation()
        {
            // Create asset
            var asset = new Asset
            {
                Name = "TEST_ASSET",
                Owner = "TEST_ADDRESS",
                Supply = 1000000,
                Metadata = new Dictionary<string, string>
                {
                    { "description", "Test asset" },
                    { "type", "TOKEN" },
                    { "decimals", "8" }
                }
            };

            string assetId = await _assetManager.CreateAssetAsync(asset);
            Assert.NotNull(assetId);

            // Verify asset
            var retrieved = await _assetManager.GetAssetAsync(assetId);
            Assert.NotNull(retrieved);
            Assert.Equal(asset.Name, retrieved.Name);
            Assert.Equal(asset.Owner, retrieved.Owner);
            Assert.Equal(asset.Supply, retrieved.Supply);
        }

        [Fact]
        public async Task TestAssetTransfer()
        {
            // Create asset
            var asset = new Asset
            {
                Name = "TEST_ASSET_TRANSFER",
                Owner = "FROM_ADDRESS",
                Supply = 1000000
            };

            string assetId = await _assetManager.CreateAssetAsync(asset);

            // Transfer asset
            await _assetManager.TransferAssetAsync(assetId, "FROM_ADDRESS", "TO_ADDRESS", 1000);

            // Verify transfer
            var retrieved = await _assetManager.GetAssetAsync(assetId);
            Assert.Equal("TO_ADDRESS", retrieved.Owner);
        }

        [Fact]
        public async Task TestAssetSearch()
        {
            // Create multiple assets
            for (int i = 0; i < 10; i++)
            {
                var asset = new Asset
                {
                    Name = $"TEST_ASSET_{i}",
                    Owner = "TEST_ADDRESS",
                    Supply = 1000000,
                    Metadata = new Dictionary<string, string>
                    {
                        { "type", i % 2 == 0 ? "TOKEN" : "NFT" }
                    }
                };
                await _assetManager.CreateAssetAsync(asset);
            }

            // Search assets
            var criteria = new Dictionary<string, string> { { "type", "TOKEN" } };
            var tokens = await _assetManager.SearchAssetsAsync(criteria);
            Assert.Equal(5, tokens.Count);

            criteria["type"] = "NFT";
            var nfts = await _assetManager.SearchAssetsAsync(criteria);
            Assert.Equal(5, nfts.Count);
        }

        [Fact]
        public async Task TestTransactionCreation()
        {
            // Create transaction
            var transaction = new Transaction
            {
                From = "FROM_ADDRESS",
                To = "TO_ADDRESS",
                Amount = 1000,
                AssetId = "ASSET_ID",
                Fee = 0.001m
            };

            string txId = await _txManager.CreateAndBroadcastTransactionAsync(transaction);
            Assert.NotNull(txId);

            // Verify transaction
            var status = await _txManager.GetTransactionStatusAsync(txId);
            Assert.NotNull(status);
        }

        [Fact]
        public async Task TestTransactionHistory()
        {
            // Create and broadcast multiple transactions
            for (int i = 0; i < 5; i++)
            {
                var transaction = new Transaction
                {
                    From = "FROM_ADDRESS",
                    To = "TO_ADDRESS",
                    Amount = 1000,
                    AssetId = "ASSET_ID"
                };
                await _txManager.CreateAndBroadcastTransactionAsync(transaction);
            }

            // Get transaction history
            var history = await _txManager.GetTransactionHistoryAsync("FROM_ADDRESS");
            Assert.NotNull(history);
            Assert.True(history.Count >= 5);
        }

        [Fact]
        public async Task TestIPFSOperations()
        {
            // Add file to IPFS
            var file = new IPFSFile
            {
                Path = "/path/to/test.txt",
                Name = "test.txt"
            };

            string hash = await _ipfsManager.AddFileAsync(file);
            Assert.NotNull(hash);

            // Get file from IPFS
            var retrieved = await _ipfsManager.GetFileAsync(hash);
            Assert.NotNull(retrieved);
            Assert.Equal(file.Name, retrieved.Name);
        }

        [Fact]
        public async Task TestConcurrentOperations()
        {
            int numThreads = 8;
            int operationsPerThread = 100;
            var tasks = new List<Task>();
            var successCount = 0;

            for (int i = 0; i < numThreads; i++)
            {
                tasks.Add(Task.Run(async () =>
                {
                    try
                    {
                        for (int j = 0; j < operationsPerThread; j++)
                        {
                            var asset = new Asset
                            {
                                Name = $"TEST_ASSET_{Task.CurrentId}_{j}",
                                Owner = "TEST_ADDRESS",
                                Supply = 1000000
                            };
                            if (await _assetManager.CreateAssetAsync(asset) != null)
                            {
                                Interlocked.Increment(ref successCount);
                            }
                        }
                    }
                    catch (Exception)
                    {
                        Assert.True(false, "Concurrent operation failed");
                    }
                }));
            }

            await Task.WhenAll(tasks);

            int totalOperations = numThreads * operationsPerThread;
            Assert.True((double)successCount / totalOperations >= 0.9);
        }

        [Fact]
        public async Task TestErrorHandling()
        {
            // Test invalid asset creation
            try
            {
                var invalidAsset = new Asset
                {
                    Name = "",  // Invalid name
                    Owner = "TEST_ADDRESS",
                    Supply = -1  // Invalid supply
                };
                await _assetManager.CreateAssetAsync(invalidAsset);
                Assert.True(false, "Should throw SatoxException for invalid asset");
            }
            catch (SatoxException ex)
            {
                Assert.NotNull(ex.Message);
                Assert.NotNull(ex.ErrorCode);
            }

            // Test invalid transaction
            try
            {
                var invalidTx = new Transaction
                {
                    From = "",  // Invalid address
                    To = "TO_ADDRESS",
                    Amount = -1  // Invalid amount
                };
                await _txManager.CreateAndBroadcastTransactionAsync(invalidTx);
                Assert.True(false, "Should throw SatoxException for invalid transaction");
            }
            catch (SatoxException ex)
            {
                Assert.NotNull(ex.Message);
                Assert.NotNull(ex.ErrorCode);
            }
        }

        [Fact]
        public async Task TestPerformance()
        {
            // Test asset creation performance
            var startTime = DateTime.Now;
            for (int i = 0; i < 1000; i++)
            {
                var asset = new Asset
                {
                    Name = $"PERF_TEST_ASSET_{i}",
                    Owner = "TEST_ADDRESS",
                    Supply = 1000000
                };
                await _assetManager.CreateAssetAsync(asset);
            }
            var duration = (DateTime.Now - startTime).TotalMilliseconds;
            Assert.True(duration < 10000);  // 10 seconds for 1000 assets

            // Test transaction performance
            startTime = DateTime.Now;
            for (int i = 0; i < 1000; i++)
            {
                var transaction = new Transaction
                {
                    From = "FROM_ADDRESS",
                    To = "TO_ADDRESS",
                    Amount = 1000,
                    AssetId = "ASSET_ID"
                };
                await _txManager.CreateAndBroadcastTransactionAsync(transaction);
            }
            duration = (DateTime.Now - startTime).TotalMilliseconds;
            Assert.True(duration < 10000);  // 10 seconds for 1000 transactions
        }
    }
} 