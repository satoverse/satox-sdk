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
    /// Configuration class for the Satox SDK.
    /// This class holds all configuration options for the SDK components.
    /// </summary>
    public class SDKConfig
    {
        /// <summary>
        /// Configuration for the blockchain manager.
        /// </summary>
        public BlockchainConfig BlockchainConfig { get; set; }

        /// <summary>
        /// Configuration for the transaction manager.
        /// </summary>
        public TransactionConfig TransactionConfig { get; set; }

        /// <summary>
        /// Configuration for the asset manager.
        /// </summary>
        public AssetConfig AssetConfig { get; set; }

        /// <summary>
        /// Configuration for the security manager.
        /// </summary>
        public SecurityConfig SecurityConfig { get; set; }

        /// <summary>
        /// Configuration for the database manager.
        /// </summary>
        public DatabaseConfig DatabaseConfig { get; set; }

        /// <summary>
        /// Configuration for the API manager.
        /// </summary>
        public APIConfig APIConfig { get; set; }

        /// <summary>
        /// Configuration for the IPFS manager.
        /// </summary>
        public IPFSConfig IPFSConfig { get; set; }

        /// <summary>
        /// Configuration for the wallet manager.
        /// </summary>
        public WalletConfig WalletConfig { get; set; }

        /// <summary>
        /// Create a new SDK configuration with default values.
        /// </summary>
        public SDKConfig()
        {
            BlockchainConfig = new BlockchainConfig();
            TransactionConfig = new TransactionConfig();
            AssetConfig = new AssetConfig();
            SecurityConfig = new SecurityConfig();
            DatabaseConfig = new DatabaseConfig();
            APIConfig = new APIConfig();
            IPFSConfig = new IPFSConfig();
            WalletConfig = new WalletConfig();
        }
    }
} 