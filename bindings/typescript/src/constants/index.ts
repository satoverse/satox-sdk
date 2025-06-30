/**
 * @file index.ts
 * @brief Constants for the Satox SDK TypeScript/JavaScript bindings
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

// SDK Constants
export const SDK_VERSION = '1.0.0';
export const SDK_NAME = 'Satox SDK';
export const SDK_DESCRIPTION = 'Quantum-Resistant Blockchain Toolkit';

// Network Constants
export const NETWORKS = {
  MAINNET: 'mainnet',
  TESTNET: 'testnet',
  REGTEST: 'regtest'
} as const;

export const DEFAULT_NETWORK = NETWORKS.TESTNET;

// Asset Constants
export const ASSET_STATES = {
  CREATED: 'created',
  ISSUED: 'issued',
  TRANSFERRED: 'transferred',
  BURNED: 'burned'
} as const;

export const NFT_STATES = {
  CREATED: 'created',
  MINTED: 'minted',
  TRANSFERRED: 'transferred',
  BURNED: 'burned'
} as const;

// Transaction Constants
export const TRANSACTION_STATUS = {
  PENDING: 'pending',
  CONFIRMED: 'confirmed',
  FAILED: 'failed'
} as const;

// Security Constants
export const SECURITY_ALGORITHMS = {
  PQC_KYBER: 'PQC_KYBER',
  PQC_DILITHIUM: 'PQC_DILITHIUM',
  PQC_SPHINCS: 'PQC_SPHINCS'
} as const;

export const DEFAULT_SECURITY_ALGORITHM = SECURITY_ALGORITHMS.PQC_KYBER;

// Database Constants
export const DATABASE_TYPES = {
  SQLITE: 'sqlite',
  POSTGRESQL: 'postgresql',
  MYSQL: 'mysql',
  MONGODB: 'mongodb',
  SUPABASE: 'supabase',
  FIREBASE: 'firebase',
  AWS: 'aws',
  AZURE: 'azure',
  GOOGLE_CLOUD: 'google_cloud'
} as const;

// IPFS Constants
export const IPFS_GATEWAYS = {
  INFURA: 'https://ipfs.infura.io',
  PINATA: 'https://gateway.pinata.cloud',
  CLOUDFLARE: 'https://cloudflare-ipfs.com'
} as const;

// Default Configuration
export const DEFAULT_CONFIG = {
  core: {
    network: DEFAULT_NETWORK,
    data_dir: '~/.satox',
    enable_mining: false,
    enable_sync: true,
    sync_interval_ms: 1000,
    mining_threads: 1,
    timeout_ms: 30000
  },
  database: {
    name: 'satox_db',
    enableLogging: true,
    logPath: 'logs/database',
    maxConnections: 10,
    connectionTimeout: 5000
  },
  security: {
    enablePQC: true,
    enableInputValidation: true,
    enableRateLimiting: true,
    enableLogging: true,
    logPath: 'logs/security'
  },
  wallet: {
    enableLogging: true,
    logPath: 'logs/wallet',
    encryptionEnabled: true,
    backupEnabled: true,
    autoBackupInterval: 86400 // 24 hours
  },
  asset: {
    enableLogging: true,
    logPath: 'logs/asset',
    maxAssets: 1000,
    enableHistory: true,
    historyRetentionDays: 365
  },
  nft: {
    enableLogging: true,
    logPath: 'logs/nft',
    maxNFTs: 10000,
    enableMetadata: true,
    metadataValidation: true
  },
  ipfs: {
    enableLogging: true,
    logPath: 'logs/ipfs',
    api_endpoint: 'http://localhost:5001',
    enable_pinning: true,
    gateway_url: IPFS_GATEWAYS.INFURA
  },
  blockchain: {
    enableLogging: true,
    logPath: 'logs/blockchain',
    enable_sync: true,
    sync_mode: 'fast',
    prune_height: 0
  },
  network: {
    enableLogging: true,
    logPath: 'logs/network',
    host: '0.0.0.0',
    port: 8333,
    enable_ssl: false,
    max_connections: 125,
    connection_timeout: 5000
  }
} as const;

// Error Codes
export const ERROR_CODES = {
  // General errors
  UNKNOWN_ERROR: 'UNKNOWN_ERROR',
  NOT_INITIALIZED: 'NOT_INITIALIZED',
  ALREADY_INITIALIZED: 'ALREADY_INITIALIZED',
  INVALID_CONFIG: 'INVALID_CONFIG',
  TIMEOUT: 'TIMEOUT',
  NETWORK_ERROR: 'NETWORK_ERROR',
  
  // Wallet errors
  WALLET_NOT_FOUND: 'WALLET_NOT_FOUND',
  ADDRESS_NOT_FOUND: 'ADDRESS_NOT_FOUND',
  INSUFFICIENT_BALANCE: 'INSUFFICIENT_BALANCE',
  INVALID_ADDRESS: 'INVALID_ADDRESS',
  INVALID_AMOUNT: 'INVALID_AMOUNT',
  TRANSACTION_FAILED: 'TRANSACTION_FAILED',
  
  // Security errors
  INVALID_KEY: 'INVALID_KEY',
  SIGNATURE_FAILED: 'SIGNATURE_FAILED',
  VERIFICATION_FAILED: 'VERIFICATION_FAILED',
  ENCRYPTION_FAILED: 'ENCRYPTION_FAILED',
  DECRYPTION_FAILED: 'DECRYPTION_FAILED',
  
  // Asset errors
  ASSET_NOT_FOUND: 'ASSET_NOT_FOUND',
  ASSET_ALREADY_EXISTS: 'ASSET_ALREADY_EXISTS',
  INVALID_ASSET_METADATA: 'INVALID_ASSET_METADATA',
  ASSET_TRANSFER_FAILED: 'ASSET_TRANSFER_FAILED',
  
  // NFT errors
  NFT_NOT_FOUND: 'NFT_NOT_FOUND',
  NFT_ALREADY_EXISTS: 'NFT_ALREADY_EXISTS',
  INVALID_NFT_METADATA: 'INVALID_NFT_METADATA',
  NFT_TRANSFER_FAILED: 'NFT_TRANSFER_FAILED',
  
  // Database errors
  DATABASE_CONNECTION_FAILED: 'DATABASE_CONNECTION_FAILED',
  QUERY_FAILED: 'QUERY_FAILED',
  TRANSACTION_ROLLBACK: 'TRANSACTION_ROLLBACK',
  
  // IPFS errors
  IPFS_UPLOAD_FAILED: 'IPFS_UPLOAD_FAILED',
  IPFS_DOWNLOAD_FAILED: 'IPFS_DOWNLOAD_FAILED',
  IPFS_PIN_FAILED: 'IPFS_PIN_FAILED',
  
  // Blockchain errors
  BLOCKCHAIN_SYNC_FAILED: 'BLOCKCHAIN_SYNC_FAILED',
  BLOCK_NOT_FOUND: 'BLOCK_NOT_FOUND',
  TRANSACTION_NOT_FOUND: 'TRANSACTION_NOT_FOUND'
} as const;

// Event Types
export const EVENT_TYPES = {
  // SDK events
  INITIALIZED: 'initialized',
  SHUTDOWN: 'shutdown',
  ERROR: 'error',
  
  // Wallet events
  WALLET_CREATED: 'wallet_created',
  WALLET_DELETED: 'wallet_deleted',
  ADDRESS_GENERATED: 'address_generated',
  TRANSACTION_SENT: 'transaction_sent',
  TRANSACTION_RECEIVED: 'transaction_received',
  
  // Asset events
  ASSET_CREATED: 'asset_created',
  ASSET_TRANSFERRED: 'asset_transferred',
  ASSET_BURNED: 'asset_burned',
  
  // NFT events
  NFT_CREATED: 'nft_created',
  NFT_MINTED: 'nft_minted',
  NFT_TRANSFERRED: 'nft_transferred',
  NFT_BURNED: 'nft_burned',
  
  // Security events
  KEY_GENERATED: 'key_generated',
  DATA_SIGNED: 'data_signed',
  DATA_VERIFIED: 'data_verified',
  DATA_ENCRYPTED: 'data_encrypted',
  DATA_DECRYPTED: 'data_decrypted'
} as const;

// Log Levels
export const LOG_LEVELS = {
  TRACE: 'trace',
  DEBUG: 'debug',
  INFO: 'info',
  WARN: 'warn',
  ERROR: 'error',
  FATAL: 'fatal'
} as const;

export const DEFAULT_LOG_LEVEL = LOG_LEVELS.INFO;

// Time Constants
export const TIME_CONSTANTS = {
  SECOND: 1000,
  MINUTE: 60 * 1000,
  HOUR: 60 * 60 * 1000,
  DAY: 24 * 60 * 60 * 1000,
  WEEK: 7 * 24 * 60 * 60 * 1000,
  MONTH: 30 * 24 * 60 * 60 * 1000,
  YEAR: 365 * 24 * 60 * 60 * 1000
} as const;

// Size Constants
export const SIZE_CONSTANTS = {
  BYTE: 1,
  KILOBYTE: 1024,
  MEGABYTE: 1024 * 1024,
  GIGABYTE: 1024 * 1024 * 1024,
  TERABYTE: 1024 * 1024 * 1024 * 1024
} as const;

// API Endpoints
export const API_ENDPOINTS = {
  MAINNET: {
    RPC: 'https://mainnet.satoverse.io/rpc',
    EXPLORER: 'https://explorer.satoverse.io',
    API: 'https://api.satoverse.io'
  },
  TESTNET: {
    RPC: 'https://testnet.satoverse.io/rpc',
    EXPLORER: 'https://testnet-explorer.satoverse.io',
    API: 'https://testnet-api.satoverse.io'
  }
} as const;

// Default Ports
export const DEFAULT_PORTS = {
  RPC: 8332,
  P2P: 8333,
  API: 8080,
  WEBSOCKET: 8081
} as const;

// File Extensions
export const FILE_EXTENSIONS = {
  WALLET_BACKUP: '.satox',
  CONFIG: '.json',
  LOG: '.log',
  KEY: '.key',
  CERT: '.crt'
} as const;

// Validation Patterns
export const VALIDATION_PATTERNS = {
  ADDRESS: /^[13][a-km-zA-HJ-NP-Z1-9]{25,34}$/,
  PRIVATE_KEY: /^[5KL][1-9A-HJ-NP-Za-km-z]{50,51}$/,
  PUBLIC_KEY: /^[02-9A-HJ-NP-Za-km-z]{66}$/,
  TRANSACTION_ID: /^[a-fA-F0-9]{64}$/,
  BLOCK_HASH: /^[a-fA-F0-9]{64}$/,
  IPFS_HASH: /^Qm[1-9A-HJ-NP-Za-km-z]{44}$/
} as const; 