/**
 * @file config.ts
 * @brief Configuration utility functions for the Satox SDK
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

import { SDKConfig, CoreConfig, DatabaseConfig, SecurityConfig, WalletConfig, AssetConfig, NFTConfig, IPFSConfig, BlockchainConfig, NetworkConfig } from '../types';
import { DEFAULT_CONFIG, NETWORKS, DEFAULT_NETWORK } from '../constants';

/**
 * Create a default configuration
 */
export function createDefaultConfig(): SDKConfig {
  return JSON.parse(JSON.stringify(DEFAULT_CONFIG));
}

/**
 * Create a mainnet configuration
 */
export function createMainnetConfig(): SDKConfig {
  const config = createDefaultConfig();
  config.core = {
    ...config.core,
    network: NETWORKS.MAINNET
  };
  return config;
}

/**
 * Create a testnet configuration
 */
export function createTestnetConfig(): SDKConfig {
  const config = createDefaultConfig();
  config.core = {
    ...config.core,
    network: NETWORKS.TESTNET
  };
  return config;
}

/**
 * Create a regtest configuration
 */
export function createRegtestConfig(): SDKConfig {
  const config = createDefaultConfig();
  config.core = {
    ...config.core,
    network: NETWORKS.REGTEST
  };
  return config;
}

/**
 * Merge configurations
 */
export function mergeConfig(base: SDKConfig, override: Partial<SDKConfig>): SDKConfig {
  const merged = { ...base };
  
  // Deep merge each section
  if (override.core) {
    merged.core = { ...merged.core, ...override.core };
  }
  if (override.database) {
    merged.database = { ...merged.database, ...override.database };
  }
  if (override.security) {
    merged.security = { ...merged.security, ...override.security };
  }
  if (override.wallet) {
    merged.wallet = { ...merged.wallet, ...override.wallet };
  }
  if (override.asset) {
    merged.asset = { ...merged.asset, ...override.asset };
  }
  if (override.nft) {
    merged.nft = { ...merged.nft, ...override.nft };
  }
  if (override.ipfs) {
    merged.ipfs = { ...merged.ipfs, ...override.ipfs };
  }
  if (override.blockchain) {
    merged.blockchain = { ...merged.blockchain, ...override.blockchain };
  }
  if (override.network) {
    merged.network = { ...merged.network, ...override.network };
  }
  
  return merged;
}

/**
 * Validate configuration
 */
export function validateConfig(config: SDKConfig): { valid: boolean; errors: string[] } {
  const errors: string[] = [];
  
  // Validate core config
  if (config.core) {
    if (config.core.network && !Object.values(NETWORKS).includes(config.core.network)) {
      errors.push('Invalid network in core config');
    }
    if (config.core.sync_interval_ms && config.core.sync_interval_ms < 100) {
      errors.push('Sync interval must be at least 100ms');
    }
    if (config.core.mining_threads && config.core.mining_threads < 1) {
      errors.push('Mining threads must be at least 1');
    }
    if (config.core.timeout_ms && config.core.timeout_ms < 1000) {
      errors.push('Timeout must be at least 1000ms');
    }
  }
  
  // Validate database config
  if (config.database) {
    if (config.database.maxConnections && config.database.maxConnections < 1) {
      errors.push('Max connections must be at least 1');
    }
    if (config.database.connectionTimeout && config.database.connectionTimeout < 1000) {
      errors.push('Connection timeout must be at least 1000ms');
    }
  }
  
  // Validate network config
  if (config.network) {
    if (config.network.port && (config.network.port < 1 || config.network.port > 65535)) {
      errors.push('Port must be between 1 and 65535');
    }
    if (config.network.max_connections && config.network.max_connections < 1) {
      errors.push('Max connections must be at least 1');
    }
    if (config.network.connection_timeout && config.network.connection_timeout < 1000) {
      errors.push('Connection timeout must be at least 1000ms');
    }
  }
  
  // Validate asset config
  if (config.asset) {
    if (config.asset.maxAssets && config.asset.maxAssets < 1) {
      errors.push('Max assets must be at least 1');
    }
    if (config.asset.historyRetentionDays && config.asset.historyRetentionDays < 1) {
      errors.push('History retention days must be at least 1');
    }
  }
  
  // Validate NFT config
  if (config.nft) {
    if (config.nft.maxNFTs && config.nft.maxNFTs < 1) {
      errors.push('Max NFTs must be at least 1');
    }
  }
  
  return {
    valid: errors.length === 0,
    errors
  };
}

/**
 * Load configuration from file
 */
export async function loadConfigFromFile(filePath: string): Promise<SDKConfig> {
  try {
    const fs = await import('fs/promises');
    const content = await fs.readFile(filePath, 'utf-8');
    const config = JSON.parse(content) as SDKConfig;
    
    const validation = validateConfig(config);
    if (!validation.valid) {
      throw new Error(`Invalid configuration: ${validation.errors.join(', ')}`);
    }
    
    return config;
  } catch (error) {
    throw new Error(`Failed to load configuration from ${filePath}: ${error.message}`);
  }
}

/**
 * Save configuration to file
 */
export async function saveConfigToFile(config: SDKConfig, filePath: string): Promise<void> {
  try {
    const fs = await import('fs/promises');
    const path = await import('path');
    
    // Ensure directory exists
    const dir = path.dirname(filePath);
    await fs.mkdir(dir, { recursive: true });
    
    // Validate config before saving
    const validation = validateConfig(config);
    if (!validation.valid) {
      throw new Error(`Invalid configuration: ${validation.errors.join(', ')}`);
    }
    
    // Save config
    await fs.writeFile(filePath, JSON.stringify(config, null, 2), 'utf-8');
  } catch (error) {
    throw new Error(`Failed to save configuration to ${filePath}: ${error.message}`);
  }
}

/**
 * Get configuration for specific environment
 */
export function getConfigForEnvironment(environment: 'development' | 'staging' | 'production'): SDKConfig {
  const baseConfig = createDefaultConfig();
  
  switch (environment) {
    case 'development':
      return mergeConfig(baseConfig, {
        core: {
          network: NETWORKS.REGTEST,
          enable_mining: true,
          enable_sync: false
        },
        database: {
          enableLogging: true
        },
        security: {
          enablePQC: false // Disable for faster development
        }
      });
      
    case 'staging':
      return mergeConfig(baseConfig, {
        core: {
          network: NETWORKS.TESTNET
        },
        database: {
          enableLogging: true
        }
      });
      
    case 'production':
      return mergeConfig(baseConfig, {
        core: {
          network: NETWORKS.MAINNET,
          enable_mining: false,
          enable_sync: true
        },
        database: {
          enableLogging: false
        },
        security: {
          enablePQC: true,
          enableInputValidation: true,
          enableRateLimiting: true
        }
      });
      
    default:
      return baseConfig;
  }
}

/**
 * Expand configuration paths (replace ~ with home directory)
 */
export function expandConfigPaths(config: SDKConfig): SDKConfig {
  const expanded = { ...config };
  
  if (expanded.core?.data_dir) {
    expanded.core.data_dir = expanded.core.data_dir.replace(/^~/, process.env.HOME || process.env.USERPROFILE || '');
  }
  
  if (expanded.database?.logPath) {
    expanded.database.logPath = expanded.database.logPath.replace(/^~/, process.env.HOME || process.env.USERPROFILE || '');
  }
  
  if (expanded.security?.logPath) {
    expanded.security.logPath = expanded.security.logPath.replace(/^~/, process.env.HOME || process.env.USERPROFILE || '');
  }
  
  if (expanded.wallet?.logPath) {
    expanded.wallet.logPath = expanded.wallet.logPath.replace(/^~/, process.env.HOME || process.env.USERPROFILE || '');
  }
  
  if (expanded.asset?.logPath) {
    expanded.asset.logPath = expanded.asset.logPath.replace(/^~/, process.env.HOME || process.env.USERPROFILE || '');
  }
  
  if (expanded.nft?.logPath) {
    expanded.nft.logPath = expanded.nft.logPath.replace(/^~/, process.env.HOME || process.env.USERPROFILE || '');
  }
  
  if (expanded.ipfs?.logPath) {
    expanded.ipfs.logPath = expanded.ipfs.logPath.replace(/^~/, process.env.HOME || process.env.USERPROFILE || '');
  }
  
  if (expanded.blockchain?.logPath) {
    expanded.blockchain.logPath = expanded.blockchain.logPath.replace(/^~/, process.env.HOME || process.env.USERPROFILE || '');
  }
  
  if (expanded.network?.logPath) {
    expanded.network.logPath = expanded.network.logPath.replace(/^~/, process.env.HOME || process.env.USERPROFILE || '');
  }
  
  return expanded;
}

/**
 * Get configuration summary
 */
export function getConfigSummary(config: SDKConfig): Record<string, any> {
  return {
    network: config.core?.network || DEFAULT_NETWORK,
    data_dir: config.core?.data_dir,
    enable_mining: config.core?.enable_mining || false,
    enable_sync: config.core?.enable_sync || true,
    enable_pqc: config.security?.enablePQC || true,
    max_assets: config.asset?.maxAssets,
    max_nfts: config.nft?.maxNFTs,
    max_connections: config.network?.max_connections,
    enable_logging: {
      database: config.database?.enableLogging || false,
      security: config.security?.enableLogging || false,
      wallet: config.wallet?.enableLogging || false,
      asset: config.asset?.enableLogging || false,
      nft: config.nft?.enableLogging || false,
      ipfs: config.ipfs?.enableLogging || false,
      blockchain: config.blockchain?.enableLogging || false,
      network: config.network?.enableLogging || false
    }
  };
} 