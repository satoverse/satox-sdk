/**
 * @file asset-manager.ts
 * @brief Asset Manager for TypeScript/JavaScript bindings
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

import { EventEmitter } from 'events';
import { native } from '../../build/Release/satox_sdk_native';
import {
  AssetConfig,
  IManager,
  SDKEvent,
  SatoxError,
  SatoxAssetError,
  AsyncBoolean,
  AsyncString,
  AsyncNumber,
  AsyncVoid
} from '../types';

/**
 * Asset Manager providing asset management functionality
 */
export class AssetManager extends EventEmitter implements IManager {
  private _initialized = false;
  private _config: AssetConfig = {};
  private _lastError = '';
  private _stats: Record<string, any> = {};

  constructor() {
    super();
    this.resetStats();
  }

  /**
   * Initialize the asset manager
   */
  public async initialize(config: AssetConfig): Promise<boolean> {
    try {
      if (this._initialized) {
        throw new SatoxAssetError('Asset manager already initialized');
      }

      this._config = { ...config };
      
      // Initialize native asset manager
      const result = native.assetManagerInitialize(JSON.stringify(config));
      if (!result.success) {
        throw new SatoxAssetError(result.error || 'Failed to initialize native asset manager');
      }

      this._initialized = true;
      this.emit('initialized', { timestamp: new Date().toISOString() });

      return true;
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxAssetError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Shutdown the asset manager
   */
  public async shutdown(): Promise<void> {
    try {
      if (!this._initialized) {
        return;
      }

      native.assetManagerShutdown();
      this._initialized = false;
      this.emit('shutdown', { timestamp: new Date().toISOString() });
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxAssetError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Check if asset manager is initialized
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
      assets_created: 0,
      assets_transferred: 0,
      assets_burned: 0,
      last_operation: null
    };
  }

  /**
   * Health check for asset manager
   */
  public async healthCheck(): Promise<boolean> {
    try {
      if (!this._initialized) {
        return false;
      }

      const result = native.assetManagerHealthCheck();
      return result.success;
    } catch {
      return false;
    }
  }
} 