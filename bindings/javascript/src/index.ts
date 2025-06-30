/**
 * @file index.ts
 * @brief Main JavaScript binding for the Satox SDK
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

// Core SDK exports
export { default as SatoxSDK } from './src/sdk';
export { default as CoreManager } from './src/core-manager';
export { default as WalletManager } from './src/wallet-manager';
export { default as SecurityManager } from './src/security-manager';
export { default as AssetManager } from './src/asset-manager';
export { default as NFTManager } from './src/nft-manager';
export { default as BlockchainManager } from './src/blockchain-manager';
export { default as IPFSManager } from './src/ipfs-manager';
export { default as NetworkManager } from './src/network-manager';
export { default as DatabaseManager } from './src/database-manager';

// Core types and utilities
export * from './src/types';
export * from './src/error';
export * from './src/api';
export * from './src/constants';

// Component-specific exports
export * from './src/wallet';
export * from './src/asset';
export * from './src/nft';
export * from './src/blockchain';
export * from './src/ipfs';
export * from './src/network';
export * from './src/database';
export * from './src/security';
export * from './src/transaction';

// Thread-safe utilities
export * from './src/thread-safe';

// Version information
export const VERSION = '1.0.0';
export const BUILD_DATE = '2025-06-30';

// Default configuration
export const DEFAULT_CONFIG = {
    data_dir: './data',
    network: 'mainnet',
    enable_mining: false,
    enable_sync: true,
    sync_interval_ms: 1000,
    mining_threads: 1,
    max_connections: 10,
    rpc_port: 7777,
    p2p_port: 67777,
    enable_rpc: true,
    enable_p2p: true,
    log_level: 'info',
    log_file: './logs/satox-sdk.log'
};

// SDK initialization function
export async function initializeSDK(config = DEFAULT_CONFIG) {
    const sdk = new SatoxSDK();
    await sdk.initialize(config);
    return sdk;
}

// SDK shutdown function
export async function shutdownSDK(sdk: any) {
    if (sdk && typeof sdk.shutdown === 'function') {
        await sdk.shutdown();
    }
}

// Error handling utilities
export class SatoxError extends Error {
    constructor(message: string, public code?: string, public details?: any) {
        super(message);
        this.name = 'SatoxError';
    }
}

// Logging utilities
export const Logger = {
    info: (message: string, ...args: any[]) => console.log(`[INFO] ${message}`, ...args),
    warn: (message: string, ...args: any[]) => console.warn(`[WARN] ${message}`, ...args),
    error: (message: string, ...args: any[]) => console.error(`[ERROR] ${message}`, ...args),
    debug: (message: string, ...args: any[]) => console.debug(`[DEBUG] ${message}`, ...args)
};

// Export default SDK instance
export default SatoxSDK; 