/**
 * @typedef {Object} SDKConfig
 * @property {string} [network] - Network configuration
 * @property {string} [database_path] - Database path
 * @property {boolean} [enable_logging] - Enable logging
 */

/**
 * @typedef {Object} AssetConfig
 * @property {string} name - Asset name
 * @property {string} symbol - Asset symbol
 * @property {number} total_supply - Total supply
 * @property {number} decimals - Decimal places
 * @property {string} [description] - Asset description
 */

/**
 * @typedef {Object} AssetInfo
 * @property {string} id - Asset ID
 * @property {string} name - Asset name
 * @property {string} symbol - Asset symbol
 * @property {number} total_supply - Total supply
 * @property {number} decimals - Decimal places
 * @property {string} owner - Owner address
 * @property {string} created_at - Creation timestamp
 */

/**
 * @typedef {Object} NFTMetadata
 * @property {string} name - NFT name
 * @property {string} [description] - NFT description
 * @property {string} [image] - Image URL
 * @property {Object} [attributes] - Custom attributes
 */

/**
 * @typedef {Object} NFTInfo
 * @property {string} id - NFT ID
 * @property {string} asset_id - Asset ID
 * @property {NFTMetadata} metadata - NFT metadata
 * @property {string} owner - Owner address
 * @property {string} created_at - Creation timestamp
 */

/**
 * @typedef {Object} TransactionInfo
 * @property {string} txid - Transaction ID
 * @property {string} from_address - Sender address
 * @property {string} to_address - Recipient address
 * @property {number} amount - Transaction amount
 * @property {string} asset_id - Asset ID
 * @property {string} status - Transaction status
 * @property {string} created_at - Creation timestamp
 */

/**
 * @typedef {Object} BlockInfo
 * @property {string} hash - Block hash
 * @property {number} height - Block height
 * @property {string} previous_hash - Previous block hash
 * @property {string} merkle_root - Merkle root
 * @property {number} timestamp - Block timestamp
 * @property {number} difficulty - Block difficulty
 * @property {string[]} transactions - Transaction IDs
 */

/**
 * @typedef {Object} WalletInfo
 * @property {string} address - Wallet address
 * @property {string} public_key - Public key
 * @property {Object.<string, number>} balances - Asset balances
 * @property {string[]} nfts - Owned NFT IDs
 */

/**
 * @typedef {Object} IPFSFileInfo
 * @property {string} hash - IPFS hash
 * @property {string} name - File name
 * @property {number} size - File size in bytes
 * @property {string} mime_type - MIME type
 * @property {string} gateway_url - Gateway URL
 */

/**
 * Satox SDK JavaScript Binding
 * @typedef {Object} SatoxSDK
 * @property {function(): number} initialize - Initialize the SDK
 * @property {function(): void} shutdown - Shutdown the SDK
 * @property {function(): string} getVersion - Get SDK version
 * @property {function(SDKConfig): Object} createCoreManager - Create core manager
 * @property {function(SDKConfig): Object} createWalletManager - Create wallet manager
 * @property {function(SDKConfig): Object} createAssetManager - Create asset manager
 * @property {function(SDKConfig): Object} createBlockchainManager - Create blockchain manager
 * @property {function(SDKConfig): Object} createDatabaseManager - Create database manager
 * @property {function(SDKConfig): Object} createSecurityManager - Create security manager
 * @property {function(SDKConfig): Object} createNetworkManager - Create network manager
 * @property {function(SDKConfig): Object} createIPFSManager - Create IPFS manager
 */

module.exports = {} 