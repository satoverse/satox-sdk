/**
 * @file ipfs-manager.ts
 * @brief IPFS Manager for TypeScript/JavaScript bindings
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

import { EventEmitter } from 'events';
import { native } from '../../build/Release/satox_sdk_native';
import {
  IPFSConfig,
  IManager,
  SDKEvent,
  SatoxError,
  SatoxIPFSError,
  AsyncBoolean,
  AsyncString,
  AsyncNumber,
  AsyncVoid
} from '../types';

/**
 * IPFS Manager providing IPFS functionality
 */
export class IPFSManager extends EventEmitter implements IManager {
  private _initialized = false;
  private _config: IPFSConfig = {};
  private _lastError = '';
  private _stats: Record<string, any> = {};

  constructor() {
    super();
    this.resetStats();
  }

  /**
   * Initialize the IPFS manager
   */
  public async initialize(config: IPFSConfig): Promise<boolean> {
    try {
      if (this._initialized) {
        throw new SatoxIPFSError('IPFS manager already initialized');
      }

      this._config = { ...config };
      
      // Initialize native IPFS manager
      const result = native.ipfsManagerInitialize(JSON.stringify(config));
      if (!result.success) {
        throw new SatoxIPFSError(result.error || 'Failed to initialize native IPFS manager');
      }

      this._initialized = true;
      this.emit('initialized', { timestamp: new Date().toISOString() });

      return true;
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxIPFSError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Shutdown the IPFS manager
   */
  public async shutdown(): Promise<void> {
    try {
      if (!this._initialized) {
        return;
      }

      native.ipfsManagerShutdown();
      this._initialized = false;
      this.emit('shutdown', { timestamp: new Date().toISOString() });
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxIPFSError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Check if IPFS manager is initialized
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
      files_uploaded: 0,
      files_downloaded: 0,
      last_operation: null
    };
  }

  /**
   * Health check for IPFS manager
   */
  public async healthCheck(): Promise<boolean> {
    try {
      if (!this._initialized) {
        return false;
      }

      const result = native.ipfsManagerHealthCheck();
      return result.success;
    } catch {
      return false;
    }
  }
} 