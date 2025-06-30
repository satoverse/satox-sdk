using System;
using System.Runtime.InteropServices;
using Satox.SDK.Blockchain;
using Satox.SDK.Transaction;
using Satox.SDK.Asset;
using Satox.SDK.Security;
using Satox.SDK.Database;
using Satox.SDK.API;
using Satox.SDK.IPFS;
using Satox.SDK.Wallet;

namespace Satox.SDK
{
    /// <summary>
    /// Main entry point for the Satox SDK.
    /// This class provides access to all SDK components and manages their lifecycle.
    /// </summary>
    public class SatoxSDK : IDisposable
    {
        private static SatoxSDK instance;
        private readonly BlockchainManager blockchainManager;
        private readonly TransactionManager transactionManager;
        private readonly AssetManager assetManager;
        private readonly SecurityManager securityManager;
        private readonly DatabaseManager databaseManager;
        private readonly APIManager apiManager;
        private readonly IPFSManager ipfsManager;
        private readonly WalletManager walletManager;
        private bool disposed;

        private SatoxSDK()
        {
            // Initialize native library
            NativeMethods.Initialize();

            // Initialize managers
            blockchainManager = new BlockchainManager();
            transactionManager = new TransactionManager();
            assetManager = new AssetManager();
            securityManager = new SecurityManager();
            databaseManager = new DatabaseManager();
            apiManager = new APIManager();
            ipfsManager = new IPFSManager();
            walletManager = new WalletManager();
        }

        /// <summary>
        /// Get the singleton instance of the SDK.
        /// </summary>
        /// <returns>The SDK instance</returns>
        public static SatoxSDK GetInstance()
        {
            if (instance == null)
            {
                lock (typeof(SatoxSDK))
                {
                    if (instance == null)
                    {
                        instance = new SatoxSDK();
                    }
                }
            }
            return instance;
        }

        /// <summary>
        /// Initialize the SDK with the given configuration.
        /// </summary>
        /// <param name="config">The SDK configuration</param>
        /// <returns>true if initialization was successful, false otherwise</returns>
        public bool Initialize(SDKConfig config)
        {
            try
            {
                // Initialize each manager
                if (!blockchainManager.Initialize(config.BlockchainConfig))
                {
                    return false;
                }
                if (!transactionManager.Initialize(config.TransactionConfig))
                {
                    return false;
                }
                if (!assetManager.Initialize(config.AssetConfig))
                {
                    return false;
                }
                if (!securityManager.Initialize(config.SecurityConfig))
                {
                    return false;
                }
                if (!databaseManager.Initialize(config.DatabaseConfig))
                {
                    return false;
                }
                if (!apiManager.Initialize(config.APIConfig))
                {
                    return false;
                }
                if (!ipfsManager.Initialize(config.IPFSConfig))
                {
                    return false;
                }
                if (!walletManager.Initialize(config.WalletConfig))
                {
                    return false;
                }
                return true;
            }
            catch (Exception)
            {
                return false;
            }
        }

        /// <summary>
        /// Get the blockchain manager.
        /// </summary>
        /// <returns>The blockchain manager instance</returns>
        public BlockchainManager GetBlockchainManager()
        {
            return blockchainManager;
        }

        /// <summary>
        /// Get the transaction manager.
        /// </summary>
        /// <returns>The transaction manager instance</returns>
        public TransactionManager GetTransactionManager()
        {
            return transactionManager;
        }

        /// <summary>
        /// Get the asset manager.
        /// </summary>
        /// <returns>The asset manager instance</returns>
        public AssetManager GetAssetManager()
        {
            return assetManager;
        }

        /// <summary>
        /// Get the security manager.
        /// </summary>
        /// <returns>The security manager instance</returns>
        public SecurityManager GetSecurityManager()
        {
            return securityManager;
        }

        /// <summary>
        /// Get the database manager.
        /// </summary>
        /// <returns>The database manager instance</returns>
        public DatabaseManager GetDatabaseManager()
        {
            return databaseManager;
        }

        /// <summary>
        /// Get the API manager.
        /// </summary>
        /// <returns>The API manager instance</returns>
        public APIManager GetAPIManager()
        {
            return apiManager;
        }

        /// <summary>
        /// Get the IPFS manager.
        /// </summary>
        /// <returns>The IPFS manager instance</returns>
        public IPFSManager GetIPFSManager()
        {
            return ipfsManager;
        }

        /// <summary>
        /// Get the wallet manager.
        /// </summary>
        /// <returns>The wallet manager instance</returns>
        public WalletManager GetWalletManager()
        {
            return walletManager;
        }

        /// <summary>
        /// Shutdown the SDK and release all resources.
        /// </summary>
        public void Shutdown()
        {
            try
            {
                // Shutdown each manager in reverse order
                walletManager.Shutdown();
                ipfsManager.Shutdown();
                apiManager.Shutdown();
                databaseManager.Shutdown();
                securityManager.Shutdown();
                assetManager.Shutdown();
                transactionManager.Shutdown();
                blockchainManager.Shutdown();
            }
            catch (Exception)
            {
                // Log error but continue shutdown
            }
        }

        /// <summary>
        /// Dispose of the SDK instance and release all resources.
        /// </summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Dispose of the SDK instance and release all resources.
        /// </summary>
        /// <param name="disposing">Whether this is being called from Dispose()</param>
        protected virtual void Dispose(bool disposing)
        {
            if (!disposed)
            {
                if (disposing)
                {
                    Shutdown();
                }
                disposed = true;
            }
        }

        /// <summary>
        /// Finalizer for the SDK instance.
        /// </summary>
        ~SatoxSDK()
        {
            Dispose(false);
        }
    }

    /// <summary>
    /// Native method declarations for the SDK.
    /// </summary>
    internal static class NativeMethods
    {
        [DllImport("satox-sdk")]
        public static extern void Initialize();
    }
} 