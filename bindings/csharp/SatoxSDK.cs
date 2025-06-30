/**
 * @file SatoxSDK.cs
 * @brief Main C# binding for the Satox SDK
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

using System;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Text.Json;
using Microsoft.Extensions.Logging;

namespace Satox
{
    /// <summary>
    /// Main SDK class for Satox blockchain operations
    /// </summary>
    public class SatoxSDK : IDisposable
    {
        private CoreManager coreManager;
        private WalletManager walletManager;
        private SecurityManager securityManager;
        private AssetManager assetManager;
        private NFTManager nftManager;
        private BlockchainManager blockchainManager;
        private IPFSManager ipfsManager;
        private NetworkManager networkManager;
        private DatabaseManager databaseManager;
        
        private SDKConfig config;
        private bool isRunning;
        private readonly object lockObject = new object();
        
        /// <summary>
        /// Default constructor with default configuration
        /// </summary>
        public SatoxSDK() : this(new SDKConfig())
        {
        }
        
        /// <summary>
        /// Constructor with custom configuration
        /// </summary>
        /// <param name="config">The SDK configuration</param>
        public SatoxSDK(SDKConfig config)
        {
            this.config = config ?? new SDKConfig();
            this.isRunning = false;
        }
        
        /// <summary>
        /// Initialize the SDK with the current configuration
        /// </summary>
        /// <returns>Task that completes when initialization is done</returns>
        public async Task InitializeAsync()
        {
            lock (lockObject)
            {
                if (isRunning)
                {
                    throw new InvalidOperationException("SDK is already running");
                }
            }
            
            // Initialize core manager
            coreManager = new CoreManager();
            await coreManager.InitializeAsync(config);
            
            // Initialize wallet manager
            walletManager = new WalletManager();
            await walletManager.InitializeAsync(config);
            
            // Initialize security manager
            securityManager = new SecurityManager();
            await securityManager.InitializeAsync(config);
            
            // Initialize asset manager
            assetManager = new AssetManager();
            await assetManager.InitializeAsync(config);
            
            // Initialize NFT manager
            nftManager = new NFTManager();
            await nftManager.InitializeAsync(config);
            
            // Initialize blockchain manager
            blockchainManager = new BlockchainManager();
            await blockchainManager.InitializeAsync(config);
            
            // Initialize IPFS manager
            ipfsManager = new IPFSManager();
            await ipfsManager.InitializeAsync(config);
            
            // Initialize network manager
            networkManager = new NetworkManager();
            await networkManager.InitializeAsync(config);
            
            // Initialize database manager
            databaseManager = new DatabaseManager();
            await databaseManager.InitializeAsync(config);
            
            lock (lockObject)
            {
                isRunning = true;
            }
        }
        
        /// <summary>
        /// Start all SDK services
        /// </summary>
        /// <returns>Task that completes when all services are started</returns>
        public async Task StartAsync()
        {
            lock (lockObject)
            {
                if (!isRunning)
                {
                    throw new InvalidOperationException("SDK is not initialized");
                }
            }
            
            // Start all managers
            await coreManager.StartAsync();
            await walletManager.StartAsync();
            await securityManager.StartAsync();
            await assetManager.StartAsync();
            await nftManager.StartAsync();
            await blockchainManager.StartAsync();
            await ipfsManager.StartAsync();
            await networkManager.StartAsync();
            await databaseManager.StartAsync();
        }
        
        /// <summary>
        /// Stop all SDK services
        /// </summary>
        /// <returns>Task that completes when all services are stopped</returns>
        public async Task StopAsync()
        {
            lock (lockObject)
            {
                if (!isRunning)
                {
                    return;
                }
            }
            
            // Stop all managers
            if (coreManager != null) await coreManager.StopAsync();
            if (walletManager != null) await walletManager.StopAsync();
            if (securityManager != null) await securityManager.StopAsync();
            if (assetManager != null) await assetManager.StopAsync();
            if (nftManager != null) await nftManager.StopAsync();
            if (blockchainManager != null) await blockchainManager.StopAsync();
            if (ipfsManager != null) await ipfsManager.StopAsync();
            if (networkManager != null) await networkManager.StopAsync();
            if (databaseManager != null) await databaseManager.StopAsync();
            
            lock (lockObject)
            {
                isRunning = false;
            }
        }
        
        /// <summary>
        /// Connect to the blockchain network
        /// </summary>
        /// <returns>Task that completes when connected</returns>
        public async Task ConnectAsync()
        {
            lock (lockObject)
            {
                if (!isRunning)
                {
                    throw new InvalidOperationException("SDK is not running");
                }
            }
            await networkManager.ConnectAsync();
        }
        
        /// <summary>
        /// Disconnect from the blockchain network
        /// </summary>
        /// <returns>Task that completes when disconnected</returns>
        public async Task DisconnectAsync()
        {
            lock (lockObject)
            {
                if (!isRunning)
                {
                    return;
                }
            }
            await networkManager.DisconnectAsync();
        }
        
        /// <summary>
        /// Check if the SDK is running
        /// </summary>
        /// <returns>true if running, false otherwise</returns>
        public bool IsRunning
        {
            get
            {
                lock (lockObject)
                {
                    return isRunning;
                }
            }
        }
        
        /// <summary>
        /// Get the core manager
        /// </summary>
        public CoreManager CoreManager => coreManager;
        
        /// <summary>
        /// Get the wallet manager
        /// </summary>
        public WalletManager WalletManager => walletManager;
        
        /// <summary>
        /// Get the security manager
        /// </summary>
        public SecurityManager SecurityManager => securityManager;
        
        /// <summary>
        /// Get the asset manager
        /// </summary>
        public AssetManager AssetManager => assetManager;
        
        /// <summary>
        /// Get the NFT manager
        /// </summary>
        public NFTManager NFTManager => nftManager;
        
        /// <summary>
        /// Get the blockchain manager
        /// </summary>
        public BlockchainManager BlockchainManager => blockchainManager;
        
        /// <summary>
        /// Get the IPFS manager
        /// </summary>
        public IPFSManager IPFSManager => ipfsManager;
        
        /// <summary>
        /// Get the network manager
        /// </summary>
        public NetworkManager NetworkManager => networkManager;
        
        /// <summary>
        /// Get the database manager
        /// </summary>
        public DatabaseManager DatabaseManager => databaseManager;
        
        /// <summary>
        /// Get the current configuration
        /// </summary>
        public SDKConfig Config => config;
        
        /// <summary>
        /// Get the SDK version
        /// </summary>
        public static string Version => "1.0.0";
        
        /// <summary>
        /// Get the SDK build date
        /// </summary>
        public static string BuildDate => "2025-06-30";
        
        /// <summary>
        /// Dispose the SDK
        /// </summary>
        public void Dispose()
        {
            StopAsync().Wait();
        }
        
        /// <summary>
        /// Shutdown the SDK gracefully
        /// </summary>
        /// <returns>Task that completes when shutdown is done</returns>
        public async Task ShutdownAsync()
        {
            await StopAsync();
        }
    }
    
    /// <summary>
    /// SDK configuration class
    /// </summary>
    public class SDKConfig
    {
        public string DataDir { get; set; } = "./data";
        public string Network { get; set; } = "mainnet";
        public bool EnableMining { get; set; } = false;
        public bool EnableSync { get; set; } = true;
        public uint SyncIntervalMs { get; set; } = 1000;
        public uint MiningThreads { get; set; } = 1;
        public uint MaxConnections { get; set; } = 10;
        public ushort RPCPort { get; set; } = 7777;
        public ushort P2PPort { get; set; } = 67777;
        public bool EnableRPC { get; set; } = true;
        public bool EnableP2P { get; set; } = true;
        public string LogLevel { get; set; } = "info";
        public string LogFile { get; set; } = "./logs/satox-sdk.log";
    }
    
    /// <summary>
    /// Core manager class
    /// </summary>
    public class CoreManager
    {
        public Task InitializeAsync(SDKConfig config) => Task.CompletedTask;
        public Task StartAsync() => Task.CompletedTask;
        public Task StopAsync() => Task.CompletedTask;
    }
    
    /// <summary>
    /// Wallet manager class
    /// </summary>
    public class WalletManager
    {
        public Task InitializeAsync(SDKConfig config) => Task.CompletedTask;
        public Task StartAsync() => Task.CompletedTask;
        public Task StopAsync() => Task.CompletedTask;
    }
    
    /// <summary>
    /// Security manager class
    /// </summary>
    public class SecurityManager
    {
        public Task InitializeAsync(SDKConfig config) => Task.CompletedTask;
        public Task StartAsync() => Task.CompletedTask;
        public Task StopAsync() => Task.CompletedTask;
    }
    
    /// <summary>
    /// Asset manager class
    /// </summary>
    public class AssetManager
    {
        public Task InitializeAsync(SDKConfig config) => Task.CompletedTask;
        public Task StartAsync() => Task.CompletedTask;
        public Task StopAsync() => Task.CompletedTask;
    }
    
    /// <summary>
    /// NFT manager class
    /// </summary>
    public class NFTManager
    {
        public Task InitializeAsync(SDKConfig config) => Task.CompletedTask;
        public Task StartAsync() => Task.CompletedTask;
        public Task StopAsync() => Task.CompletedTask;
    }
    
    /// <summary>
    /// Blockchain manager class
    /// </summary>
    public class BlockchainManager
    {
        public Task InitializeAsync(SDKConfig config) => Task.CompletedTask;
        public Task StartAsync() => Task.CompletedTask;
        public Task StopAsync() => Task.CompletedTask;
    }
    
    /// <summary>
    /// IPFS manager class
    /// </summary>
    public class IPFSManager
    {
        public Task InitializeAsync(SDKConfig config) => Task.CompletedTask;
        public Task StartAsync() => Task.CompletedTask;
        public Task StopAsync() => Task.CompletedTask;
    }
    
    /// <summary>
    /// Network manager class
    /// </summary>
    public class NetworkManager
    {
        public Task InitializeAsync(SDKConfig config) => Task.CompletedTask;
        public Task StartAsync() => Task.CompletedTask;
        public Task StopAsync() => Task.CompletedTask;
        public Task ConnectAsync() => Task.CompletedTask;
        public Task DisconnectAsync() => Task.CompletedTask;
    }
    
    /// <summary>
    /// Database manager class
    /// </summary>
    public class DatabaseManager
    {
        public Task InitializeAsync(SDKConfig config) => Task.CompletedTask;
        public Task StartAsync() => Task.CompletedTask;
        public Task StopAsync() => Task.CompletedTask;
    }
} 