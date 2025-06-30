/**
 * @file sdk.ts
 * @brief Main SDK class for TypeScript/JavaScript bindings
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

import { EventEmitter } from 'events';
import { native } from '../build/Release/satox_sdk_native';
import {
  SDKConfig,
  ISDK,
  SDKEvent,
  EventCallback,
  SatoxError,
  SatoxConfigError,
  AsyncBoolean,
  AsyncVoid
} from './types';
import { WalletManager } from './managers/wallet-manager';
import { SecurityManager } from './managers/security-manager';
import { AssetManager } from './managers/asset-manager';
import { NFTManager } from './managers/nft-manager';
import { DatabaseManager } from './managers/database-manager';
import { BlockchainManager } from './managers/blockchain-manager';
import { IPFSManager } from './managers/ipfs-manager';
import { NetworkManager } from './managers/network-manager';
import { CoreManager } from './managers/core-manager';

/**
 * Main Satox SDK class providing access to all blockchain functionality
 */
export class SDK extends EventEmitter implements ISDK {
  private _initialized = false;
  private _config: SDKConfig = {};
  private _startTime?: Date;
  private _version = '1.0.0';

  // Manager instances
  public readonly wallet: WalletManager;
  public readonly security: SecurityManager;
  public readonly asset: AssetManager;
  public readonly nft: NFTManager;
  public readonly database: DatabaseManager;
  public readonly blockchain: BlockchainManager;
  public readonly ipfs: IPFSManager;
  public readonly network: NetworkManager;
  public readonly core: CoreManager;

  constructor() {
    super();
    
    // Initialize managers
    this.wallet = new WalletManager();
    this.security = new SecurityManager();
    this.asset = new AssetManager();
    this.nft = new NFTManager();
    this.database = new DatabaseManager();
    this.blockchain = new BlockchainManager();
    this.ipfs = new IPFSManager();
    this.network = new NetworkManager();
    this.core = new CoreManager();

    // Set up event forwarding from managers
    this.setupEventForwarding();
  }

  /**
   * Initialize the SDK with configuration
   */
  public async initialize(config: SDKConfig): Promise<boolean> {
    try {
      if (this._initialized) {
        throw new SatoxConfigError('SDK already initialized');
      }

      this._config = { ...config };
      this._startTime = new Date();

      // Initialize native SDK
      const result = native.initialize(JSON.stringify(config));
      if (!result.success) {
        throw new SatoxConfigError(result.error || 'Failed to initialize native SDK');
      }

      // Initialize managers
      await this.initializeManagers(config);

      this._initialized = true;
      this.emit('initialized', { timestamp: new Date().toISOString() });

      return true;
    } catch (error) {
      const satoxError = error instanceof SatoxError ? error : new SatoxError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Shutdown the SDK
   */
  public async shutdown(): Promise<void> {
    try {
      if (!this._initialized) {
        return;
      }

      // Shutdown managers
      await this.shutdownManagers();

      // Shutdown native SDK
      native.shutdown();

      this._initialized = false;
      this.emit('shutdown', { timestamp: new Date().toISOString() });
    } catch (error) {
      const satoxError = error instanceof SatoxError ? error : new SatoxError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Check if SDK is initialized
   */
  public isInitialized(): boolean {
    return this._initialized;
  }

  /**
   * Get SDK version
   */
  public getVersion(): string {
    return this._version;
  }

  /**
   * Get current configuration
   */
  public getConfig(): SDKConfig {
    return { ...this._config };
  }

  /**
   * Get SDK uptime
   */
  public getUptime(): number {
    if (!this._startTime) {
      return 0;
    }
    return Date.now() - this._startTime.getTime();
  }

  /**
   * Get SDK status
   */
  public getStatus(): Record<string, any> {
    return {
      initialized: this._initialized,
      version: this._version,
      uptime: this.getUptime(),
      startTime: this._startTime?.toISOString(),
      config: this._config,
      managers: {
        wallet: this.wallet.isInitialized(),
        security: this.security.isInitialized(),
        asset: this.asset.isInitialized(),
        nft: this.nft.isInitialized(),
        database: this.database.isInitialized(),
        blockchain: this.blockchain.isInitialized(),
        ipfs: this.ipfs.isInitialized(),
        network: this.network.isInitialized(),
        core: this.core.isInitialized()
      }
    };
  }

  /**
   * Get SDK statistics
   */
  public getStats(): Record<string, any> {
    return {
      version: this._version,
      uptime: this.getUptime(),
      initialized: this._initialized,
      eventListeners: this.listenerCount('*'),
      managers: {
        wallet: this.wallet.getStats(),
        security: this.security.getStats(),
        asset: this.asset.getStats(),
        nft: this.nft.getStats(),
        database: this.database.getStats(),
        blockchain: this.blockchain.getStats(),
        ipfs: this.ipfs.getStats(),
        network: this.network.getStats(),
        core: this.core.getStats()
      }
    };
  }

  /**
   * Reset SDK statistics
   */
  public resetStats(): void {
    this.wallet.resetStats();
    this.security.resetStats();
    this.asset.resetStats();
    this.nft.resetStats();
    this.database.resetStats();
    this.blockchain.resetStats();
    this.ipfs.resetStats();
    this.network.resetStats();
    this.core.resetStats();
  }

  /**
   * Get last error from native SDK
   */
  public getLastError(): string {
    try {
      return native.getLastError() || '';
    } catch {
      return '';
    }
  }

  /**
   * Health check for the SDK
   */
  public async healthCheck(): Promise<boolean> {
    try {
      if (!this._initialized) {
        return false;
      }

      // Check native SDK health
      const nativeHealth = native.healthCheck();
      if (!nativeHealth.success) {
        return false;
      }

      // Check all managers
      const managerHealth = await Promise.all([
        this.wallet.healthCheck(),
        this.security.healthCheck(),
        this.asset.healthCheck(),
        this.nft.healthCheck(),
        this.database.healthCheck(),
        this.blockchain.healthCheck(),
        this.ipfs.healthCheck(),
        this.network.healthCheck(),
        this.core.healthCheck()
      ]);

      return managerHealth.every(healthy => healthy);
    } catch {
      return false;
    }
  }

  /**
   * Initialize all managers
   */
  private async initializeManagers(config: SDKConfig): Promise<void> {
    const initPromises = [];

    if (config.core) {
      initPromises.push(this.core.initialize(config.core));
    }

    if (config.database) {
      initPromises.push(this.database.initialize(config.database));
    }

    if (config.security) {
      initPromises.push(this.security.initialize(config.security));
    }

    if (config.wallet) {
      initPromises.push(this.wallet.initialize(config.wallet));
    }

    if (config.asset) {
      initPromises.push(this.asset.initialize(config.asset));
    }

    if (config.nft) {
      initPromises.push(this.nft.initialize(config.nft));
    }

    if (config.ipfs) {
      initPromises.push(this.ipfs.initialize(config.ipfs));
    }

    if (config.blockchain) {
      initPromises.push(this.blockchain.initialize(config.blockchain));
    }

    if (config.network) {
      initPromises.push(this.network.initialize(config.network));
    }

    await Promise.all(initPromises);
  }

  /**
   * Shutdown all managers
   */
  private async shutdownManagers(): Promise<void> {
    const shutdownPromises = [
      this.wallet.shutdown(),
      this.security.shutdown(),
      this.asset.shutdown(),
      this.nft.shutdown(),
      this.database.shutdown(),
      this.blockchain.shutdown(),
      this.ipfs.shutdown(),
      this.network.shutdown(),
      this.core.shutdown()
    ];

    await Promise.all(shutdownPromises);
  }

  /**
   * Set up event forwarding from managers to SDK
   */
  private setupEventForwarding(): void {
    const managers = [
      { name: 'wallet', manager: this.wallet },
      { name: 'security', manager: this.security },
      { name: 'asset', manager: this.asset },
      { name: 'nft', manager: this.nft },
      { name: 'database', manager: this.database },
      { name: 'blockchain', manager: this.blockchain },
      { name: 'ipfs', manager: this.ipfs },
      { name: 'network', manager: this.network },
      { name: 'core', manager: this.core }
    ];

    managers.forEach(({ name, manager }) => {
      manager.on('*', (event: SDKEvent) => {
        // Forward events with manager prefix
        const prefixedEvent = {
          ...event,
          type: `${name}.${event.type}`,
          manager: name
        };
        this.emit('*', prefixedEvent);
        this.emit(prefixedEvent.type, prefixedEvent);
      });
    });
  }

  /**
   * Get manager by name
   */
  public getManager(name: string): any {
    const managers: Record<string, any> = {
      wallet: this.wallet,
      security: this.security,
      asset: this.asset,
      nft: this.nft,
      database: this.database,
      blockchain: this.blockchain,
      ipfs: this.ipfs,
      network: this.network,
      core: this.core
    };

    return managers[name];
  }

  /**
   * Get all managers
   */
  public getManagers(): Record<string, any> {
    return {
      wallet: this.wallet,
      security: this.security,
      asset: this.asset,
      nft: this.nft,
      database: this.database,
      blockchain: this.blockchain,
      ipfs: this.ipfs,
      network: this.network,
      core: this.core
    };
  }
}

// Export singleton instance
export const sdk = new SDK(); 