/**
 * @file nft-manager.ts
 * @brief NFT Manager for TypeScript/JavaScript bindings
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

import { EventEmitter } from 'events';
import { native } from '../../build/Release/satox_sdk_native';
import {
  NFTConfig,
  IManager,
  SDKEvent,
  SatoxError,
  SatoxNFTError,
  AsyncBoolean,
  AsyncString,
  AsyncNumber,
  AsyncVoid
} from '../types';

/**
 * NFT Manager providing NFT management functionality
 */
export class NFTManager extends EventEmitter implements IManager {
  private _initialized = false;
  private _config: NFTConfig = {};
  private _lastError = '';
  private _stats: Record<string, any> = {};

  constructor() {
    super();
    this.resetStats();
  }

  /**
   * Initialize the NFT manager
   */
  public async initialize(config: NFTConfig): Promise<boolean> {
    try {
      if (this._initialized) {
        throw new SatoxNFTError('NFT manager already initialized');
      }

      this._config = { ...config };
      
      // Initialize native NFT manager
      const result = native.nftManagerInitialize(JSON.stringify(config));
      if (!result.success) {
        throw new SatoxNFTError(result.error || 'Failed to initialize native NFT manager');
      }

      this._initialized = true;
      this.emit('initialized', { timestamp: new Date().toISOString() });

      return true;
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxNFTError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Shutdown the NFT manager
   */
  public async shutdown(): Promise<void> {
    try {
      if (!this._initialized) {
        return;
      }

      native.nftManagerShutdown();
      this._initialized = false;
      this.emit('shutdown', { timestamp: new Date().toISOString() });
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxNFTError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Check if NFT manager is initialized
   */
  public isInitialized(): boolean {
    return this._initialized;
  }

  /**
   * Get last error
   */
  public getLastError(): string {
    return this._lastError;
  }

  /**
   * Get statistics
   */
  public getStats(): Record<string, any> {
    return { ...this._stats };
  }

  /**
   * Reset statistics
   */
  public resetStats(): void {
    this._stats = {
      nfts_created: 0,
      nfts_transferred: 0,
      nfts_burned: 0,
      last_operation: null
    };
  }

  /**
   * Health check for NFT manager
   */
  public async healthCheck(): Promise<boolean> {
    try {
      if (!this._initialized) {
        return false;
      }

      const result = native.nftManagerHealthCheck();
      return result.success;
    } catch {
      return false;
    }
  }
} 