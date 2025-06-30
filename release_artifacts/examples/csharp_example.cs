using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Satox.SDK;
using Satox.SDK.Blockchain;
using Satox.SDK.Asset;
using Satox.SDK.NFT;
using Satox.SDK.Security;
using Satox.SDK.Quantum;
using Satox.SDK.Wallet;
using Satox.SDK.Transaction;
using Satox.SDK.Network;
using Satox.SDK.Database;
using Satox.SDK.API;
using Satox.SDK.Core;
using Satox.SDK.IPFS;
using Satox.SDK.Error;

namespace Satox.Examples
{
    class Program
    {
        static async Task Main(string[] args)
        {
            Console.WriteLine("🚀 Satox SDK C# Example Application");
            Console.WriteLine("====================================\n");

            try
            {
                // Initialize core manager
                Console.WriteLine("1. Initializing Core Manager...");
                var coreManager = CoreManager.Instance;
                await coreManager.InitializeAsync();
                Console.WriteLine("   ✅ Core Manager initialized successfully\n");

                // Initialize blockchain manager
                Console.WriteLine("2. Initializing Blockchain Manager...");
                var blockchainManager = BlockchainManager.Instance;
                await blockchainManager.InitializeAsync();
                Console.WriteLine("   ✅ Blockchain Manager initialized successfully\n");

                // Initialize security manager
                Console.WriteLine("3. Initializing Security Manager...");
                var securityManager = SecurityManager.Instance;
                await securityManager.InitializeAsync();
                Console.WriteLine("   ✅ Security Manager initialized successfully\n");

                // Initialize quantum manager
                Console.WriteLine("4. Initializing Quantum Manager...");
                var quantumManager = QuantumManager.Instance;
                await quantumManager.InitializeAsync();
                Console.WriteLine("   ✅ Quantum Manager initialized successfully\n");

                // Initialize wallet manager
                Console.WriteLine("5. Initializing Wallet Manager...");
                var walletManager = WalletManager.Instance;
                await walletManager.InitializeAsync();
                Console.WriteLine("   ✅ Wallet Manager initialized successfully\n");

                // Initialize asset manager
                Console.WriteLine("6. Initializing Asset Manager...");
                var assetManager = AssetManager.Instance;
                await assetManager.InitializeAsync();
                Console.WriteLine("   ✅ Asset Manager initialized successfully\n");

                // Initialize NFT manager
                Console.WriteLine("7. Initializing NFT Manager...");
                var nftManager = NFTManager.Instance;
                await nftManager.InitializeAsync();
                Console.WriteLine("   ✅ NFT Manager initialized successfully\n");

                // Initialize transaction manager
                Console.WriteLine("8. Initializing Transaction Manager...");
                var transactionManager = TransactionManager.Instance;
                await transactionManager.InitializeAsync();
                Console.WriteLine("   ✅ Transaction Manager initialized successfully\n");

                // Initialize network manager
                Console.WriteLine("9. Initializing Network Manager...");
                var networkManager = NetworkManager.Instance;
                await networkManager.InitializeAsync();
                Console.WriteLine("   ✅ Network Manager initialized successfully\n");

                // Initialize database manager
                Console.WriteLine("10. Initializing Database Manager...");
                var databaseManager = DatabaseManager.Instance;
                await databaseManager.InitializeAsync();
                Console.WriteLine("   ✅ Database Manager initialized successfully\n");

                // Initialize API manager
                Console.WriteLine("11. Initializing API Manager...");
                var apiManager = APIManager.Instance;
                await apiManager.InitializeAsync();
                Console.WriteLine("   ✅ API Manager initialized successfully\n");

                // Initialize IPFS manager
                Console.WriteLine("12. Initializing IPFS Manager...");
                var ipfsManager = IPFSManager.Instance;
                await ipfsManager.InitializeAsync();
                Console.WriteLine("   ✅ IPFS Manager initialized successfully\n");

                // Demonstrate wallet operations
                Console.WriteLine("13. Wallet Operations Demo...");
                var walletAddress = await walletManager.CreateWalletAsync();
                Console.WriteLine($"   ✅ Created wallet: {walletAddress}");

                var balance = await walletManager.GetBalanceAsync(walletAddress);
                Console.WriteLine($"   ✅ Wallet balance: {balance} SATOX");
                Console.WriteLine();

                // Demonstrate quantum security
                Console.WriteLine("14. Quantum Security Demo...");
                var quantumKeyPair = await quantumManager.GenerateKeyPairAsync();
                Console.WriteLine("   ✅ Generated quantum-resistant key pair");

                var message = System.Text.Encoding.UTF8.GetBytes("Hello Quantum World!");
                var quantumSignature = await quantumManager.SignAsync(message, quantumKeyPair.PrivateKey);
                Console.WriteLine("   ✅ Created quantum-resistant signature");

                var isValid = await quantumManager.VerifyAsync(message, quantumSignature, quantumKeyPair.PublicKey);
                Console.WriteLine($"   ✅ Quantum signature verification: {isValid}");
                Console.WriteLine();

                // Demonstrate asset operations
                Console.WriteLine("15. Asset Operations Demo...");
                var metadata = new Dictionary<string, object>
                {
                    ["website"] = "https://example.com",
                    ["category"] = "utility"
                };

                var assetConfig = new AssetConfig
                {
                    Name = "Test Token",
                    Symbol = "TEST",
                    TotalSupply = 1000000,
                    Decimals = 8,
                    Description = "A test token for demonstration",
                    Metadata = metadata
                };

                var assetId = await assetManager.CreateAssetAsync(assetConfig);
                Console.WriteLine($"   ✅ Created asset with ID: {assetId}");

                var assetInfo = await assetManager.GetAssetInfoAsync(assetId);
                Console.WriteLine($"   ✅ Asset info: {assetInfo.Name} ({assetInfo.Symbol})");
                Console.WriteLine();

                // Demonstrate NFT operations
                Console.WriteLine("16. NFT Operations Demo...");
                var nftMetadata = new Dictionary<string, object>
                {
                    ["image"] = "https://example.com/image.png",
                    ["attributes"] = new Dictionary<string, object>
                    {
                        ["rarity"] = "common",
                        ["power"] = 100
                    }
                };

                var nftConfig = new NFTConfig
                {
                    Name = "Test NFT",
                    Symbol = "TNFT",
                    Description = "A test NFT for demonstration",
                    Metadata = nftMetadata
                };

                var nftId = await nftManager.CreateNFTAsync(nftConfig);
                Console.WriteLine($"   ✅ Created NFT with ID: {nftId}");

                var nftInfo = await nftManager.GetNFTInfoAsync(nftId);
                Console.WriteLine($"   ✅ NFT info: {nftInfo.Name} ({nftInfo.Symbol})");
                Console.WriteLine();

                // Demonstrate transaction operations
                Console.WriteLine("17. Transaction Operations Demo...");
                var txConfig = new TransactionConfig
                {
                    From = walletAddress,
                    To = "recipient_address",
                    Amount = 1000,
                    AssetId = assetId,
                    Fee = 100
                };

                var txId = await transactionManager.CreateTransactionAsync(txConfig);
                Console.WriteLine($"   ✅ Created transaction with ID: {txId}");

                var txStatus = await transactionManager.GetTransactionStatusAsync(txId);
                Console.WriteLine($"   ✅ Transaction status: {txStatus}");
                Console.WriteLine();

                // Demonstrate blockchain operations
                Console.WriteLine("18. Blockchain Operations Demo...");
                var blockchainInfo = await blockchainManager.GetBlockchainInfoAsync();
                Console.WriteLine($"   ✅ Blockchain: {blockchainInfo.Name} (Height: {blockchainInfo.CurrentHeight})");

                var latestBlock = await blockchainManager.GetLatestBlockAsync();
                Console.WriteLine($"   ✅ Latest block: {latestBlock.Hash} with {latestBlock.Transactions.Count} transactions");
                Console.WriteLine();

                // Demonstrate network operations
                Console.WriteLine("19. Network Operations Demo...");
                var networkInfo = await networkManager.GetNetworkInfoAsync();
                Console.WriteLine($"   ✅ Network: {networkInfo.Connections} connections");

                var peers = await networkManager.GetPeersAsync();
                Console.WriteLine($"   ✅ Connected peers: {peers.Count}");
                Console.WriteLine();

                // Demonstrate IPFS operations
                Console.WriteLine("20. IPFS Operations Demo...");
                var data = System.Text.Encoding.UTF8.GetBytes("Hello IPFS World!");
                var ipfsHash = await ipfsManager.UploadDataAsync(data);
                Console.WriteLine($"   ✅ Uploaded data to IPFS: {ipfsHash}");

                var retrievedData = await ipfsManager.DownloadDataAsync(ipfsHash);
                Console.WriteLine($"   ✅ Retrieved data from IPFS: {retrievedData.Length} bytes");
                Console.WriteLine();

                // Demonstrate database operations
                Console.WriteLine("21. Database Operations Demo...");
                var dbInfo = await databaseManager.GetDatabaseInfoAsync();
                Console.WriteLine($"   ✅ Database: {dbInfo.TableCount} tables");

                var dbStats = await databaseManager.GetDatabaseStatsAsync();
                Console.WriteLine($"   ✅ Database size: {dbStats.SizeMB:F2} MB");
                Console.WriteLine();

                // Demonstrate API operations
                Console.WriteLine("22. API Operations Demo...");
                var apiInfo = await apiManager.GetAPIInfoAsync();
                Console.WriteLine($"   ✅ API: {apiInfo.EndpointCount} endpoints available");

                var apiStats = await apiManager.GetAPIStatsAsync();
                Console.WriteLine($"   ✅ API requests: {apiStats.TotalRequests} total");
                Console.WriteLine();

                // Demonstrate security operations
                Console.WriteLine("23. Security Operations Demo...");
                var securityInfo = await securityManager.GetSecurityInfoAsync();
                Console.WriteLine($"   ✅ Security: {securityInfo.AlgorithmCount} algorithms supported");

                var encryptionKey = await securityManager.GenerateEncryptionKeyAsync();
                Console.WriteLine($"   ✅ Generated encryption key: {encryptionKey.Length} bytes");
                Console.WriteLine();

                // Demonstrate batch operations
                Console.WriteLine("24. Batch Operations Demo...");
                var tx1 = new TransactionConfig
                {
                    From = walletAddress,
                    To = "recipient1",
                    Amount = 100,
                    AssetId = assetId,
                    Fee = 10
                };

                var tx2 = new TransactionConfig
                {
                    From = walletAddress,
                    To = "recipient2",
                    Amount = 200,
                    AssetId = assetId,
                    Fee = 10
                };

                var batchConfig = new BatchTransactionConfig
                {
                    Transactions = new List<TransactionConfig> { tx1, tx2 }
                };

                var batchId = await transactionManager.CreateBatchTransactionAsync(batchConfig);
                Console.WriteLine($"   ✅ Created batch transaction with ID: {batchId}");
                Console.WriteLine();

                // Demonstrate error handling
                Console.WriteLine("25. Error Handling Demo...");
                try
                {
                    await transactionManager.GetTransactionStatusAsync("invalid_tx_id");
                    Console.WriteLine("   ❌ Unexpected success");
                }
                catch (SatoxException ex)
                {
                    Console.WriteLine($"   ✅ Properly handled error: {ex.Message}");
                }
                Console.WriteLine();

                // Demonstrate concurrent operations
                Console.WriteLine("26. Concurrent Operations Demo...");
                var tasks = new List<Task<string>>();
                for (int i = 0; i < 3; i++)
                {
                    tasks.Add(Task.Run(async () =>
                    {
                        var wm = WalletManager.Instance;
                        await wm.InitializeAsync();
                        var address = await wm.CreateWalletAsync();
                        await wm.ShutdownAsync();
                        return address;
                    }));
                }

                var addresses = await Task.WhenAll(tasks);
                Console.WriteLine($"   ✅ Concurrent wallets created: {string.Join(", ", addresses)}");
                Console.WriteLine();

                // Demonstrate event handling
                Console.WriteLine("27. Event Handling Demo...");
                transactionManager.TransactionStatusChanged += (sender, e) =>
                {
                    Console.WriteLine($"   📡 Transaction {e.TransactionId} status changed to {e.Status}");
                };

                // Demonstrate cleanup
                Console.WriteLine("28. Cleanup Operations...");
                await apiManager.ShutdownAsync();
                await databaseManager.ShutdownAsync();
                await ipfsManager.ShutdownAsync();
                await networkManager.ShutdownAsync();
                await transactionManager.ShutdownAsync();
                await nftManager.ShutdownAsync();
                await assetManager.ShutdownAsync();
                await walletManager.ShutdownAsync();
                await quantumManager.ShutdownAsync();
                await securityManager.ShutdownAsync();
                await blockchainManager.ShutdownAsync();
                await coreManager.ShutdownAsync();
                Console.WriteLine("   ✅ All managers shut down successfully\n");

                Console.WriteLine("🎉 C# Example Application Completed Successfully!");
                Console.WriteLine("All SDK features demonstrated and working correctly.");

            }
            catch (Exception ex)
            {
                Console.WriteLine($"❌ Error in C# example: {ex.Message}");
                Console.WriteLine(ex.StackTrace);
            }
        }

        // Test methods for demonstration
        public static async Task TestCoreInitializationAsync()
        {
            var coreManager = CoreManager.Instance;
            await coreManager.InitializeAsync();
            await coreManager.ShutdownAsync();
        }

        public static async Task TestQuantumOperationsAsync()
        {
            var quantumManager = QuantumManager.Instance;
            await quantumManager.InitializeAsync();

            var keyPair = await quantumManager.GenerateKeyPairAsync();
            var message = System.Text.Encoding.UTF8.GetBytes("Test message");
            var signature = await quantumManager.SignAsync(message, keyPair.PrivateKey);
            var isValid = await quantumManager.VerifyAsync(message, signature, keyPair.PublicKey);

            if (!isValid)
            {
                throw new SatoxException("Quantum signature verification failed");
            }

            await quantumManager.ShutdownAsync();
        }

        public static async Task TestAssetOperationsAsync()
        {
            var assetManager = AssetManager.Instance;
            await assetManager.InitializeAsync();

            var config = new AssetConfig
            {
                Name = "Test Asset",
                Symbol = "TEST",
                TotalSupply = 1000000,
                Decimals = 8,
                Description = "Test asset",
                Metadata = new Dictionary<string, object>()
            };

            var assetId = await assetManager.CreateAssetAsync(config);
            var assetInfo = await assetManager.GetAssetInfoAsync(assetId);

            if (assetInfo.Name != "Test Asset")
            {
                throw new SatoxException($"Unexpected asset name: {assetInfo.Name}");
            }

            await assetManager.ShutdownAsync();
        }

        public static async Task TestConcurrentOperationsAsync()
        {
            var tasks = new List<Task<string>>();
            for (int i = 0; i < 5; i++)
            {
                tasks.Add(Task.Run(async () =>
                {
                    var wm = WalletManager.Instance;
                    await wm.InitializeAsync();
                    var address = await wm.CreateWalletAsync();
                    await wm.ShutdownAsync();
                    return address;
                }));
            }

            var addresses = await Task.WhenAll(tasks);
            Console.WriteLine($"   ✅ Concurrent wallets created: {string.Join(", ", addresses)}");
        }

        public static async Task TestPerformanceAsync()
        {
            var walletManager = WalletManager.Instance;
            await walletManager.InitializeAsync();

            var stopwatch = System.Diagnostics.Stopwatch.StartNew();
            var addresses = new List<string>();

            for (int i = 0; i < 100; i++)
            {
                var address = await walletManager.CreateWalletAsync();
                addresses.Add(address);
            }

            stopwatch.Stop();
            Console.WriteLine($"   ⚡ Created 100 wallets in {stopwatch.ElapsedMilliseconds}ms");

            await walletManager.ShutdownAsync();
        }
    }
} 