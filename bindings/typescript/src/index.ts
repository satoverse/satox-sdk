/**
 * @file index.ts
 * @brief Main entry point for Satox SDK TypeScript/JavaScript bindings
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

// Export the native addon
export const native = require('../build/Release/satox_sdk_native');

// Export types
export * from './types';

// Export main SDK class
export { SDK } from './sdk';

// Export manager classes
export { WalletManager } from './managers/wallet-manager';
export { SecurityManager } from './managers/security-manager';
export { AssetManager } from './managers/asset-manager';
export { NFTManager } from './managers/nft-manager';
export { DatabaseManager } from './managers/database-manager';
export { BlockchainManager } from './managers/blockchain-manager';
export { IPFSManager } from './managers/ipfs-manager';
export { NetworkManager } from './managers/network-manager';
export { CoreManager } from './managers/core-manager';

// Export utility functions
export * from './utils/config';
export * from './utils/validation';
export * from './utils/helpers';

// Export constants
export * from './constants';

// Export version information
export const VERSION = '1.0.0';
export const AUTHOR = 'Satoxcoin Core Developers';
export const EMAIL = 'dev@satoverse.io';
export const LICENSE = 'MIT';
export const URL = 'https://github.com/satoverse/satox-sdk';

// Default export
export default {
  SDK,
  WalletManager,
  SecurityManager,
  AssetManager,
  NFTManager,
  DatabaseManager,
  BlockchainManager,
  IPFSManager,
  NetworkManager,
  CoreManager,
  VERSION,
  AUTHOR,
  EMAIL,
  LICENSE,
  URL,
  native
}; 