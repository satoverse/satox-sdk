/**
 * @file blockchain-manager.ts
 * @brief Blockchain Manager for TypeScript/JavaScript bindings
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

import { EventEmitter } from 'events';
import { native } from '../../build/Release/satox_sdk_native';
import {
  BlockchainConfig,
  IManager,
  SDKEvent,
  SatoxError,
  SatoxBlockchainError,
  AsyncBoolean,
  AsyncString,
  AsyncNumber,
  AsyncVoid
} from '../types';

/**
 * Blockchain Manager providing blockchain functionality
 */
export class BlockchainManager extends EventEmitter implements IManager {
  private _initialized = false;
  private _config: BlockchainConfig = {};
  private _lastError = '';
  private _stats: Record<string, any> = {};

  constructor() {
    super();
    this.resetStats();
  }

  /**
   * Initialize the blockchain manager
   */
  public async initialize(config: BlockchainConfig): Promise<boolean> {
    try {
      if (this._initialized) {
        throw new SatoxBlockchainError('Blockchain manager already initialized');
      }

      this._config = { ...config };
      
      // Initialize native blockchain manager
      const result = native.blockchainManagerInitialize(JSON.stringify(config));
      if (!result.success) {
        throw new SatoxBlockchainError(result.error || 'Failed to initialize native blockchain manager');
      }

      this._initialized = true;
      this.emit('initialized', { timestamp: new Date().toISOString() });

      return true;
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxBlockchainError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Shutdown the blockchain manager
   */
  public async shutdown(): Promise<void> {
    try {
      if (!this._initialized) {
        return;
      }

      native.blockchainManagerShutdown();
      this._initialized = false;
      this.emit('shutdown', { timestamp: new Date().toISOString() });
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxBlockchainError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Check if blockchain manager is initialized
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
      blocks_processed: 0,
      transactions_processed: 0,
      last_operation: null
    };
  }

  /**
   * Health check for blockchain manager
   */
  public async healthCheck(): Promise<boolean> {
    try {
      if (!this._initialized) {
        return false;
      }

      const result = native.blockchainManagerHealthCheck();
      return result.success;
    } catch {
      return false;
    }
  }
} 