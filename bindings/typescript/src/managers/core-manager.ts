/**
 * @file core-manager.ts
 * @brief Core Manager for TypeScript/JavaScript bindings
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

import { EventEmitter } from 'events';
import { native } from '../../build/Release/satox_sdk_native';
import {
  CoreConfig,
  IManager,
  SDKEvent,
  SatoxError,
  SatoxCoreError,
  AsyncBoolean,
  AsyncString,
  AsyncNumber,
  AsyncVoid
} from '../types';

/**
 * Core Manager providing core SDK functionality
 */
export class CoreManager extends EventEmitter implements IManager {
  private _initialized = false;
  private _config: CoreConfig = {};
  private _lastError = '';
  private _stats: Record<string, any> = {};

  constructor() {
    super();
    this.resetStats();
  }

  /**
   * Initialize the core manager
   */
  public async initialize(config: CoreConfig): Promise<boolean> {
    try {
      if (this._initialized) {
        throw new SatoxCoreError('Core manager already initialized');
      }

      this._config = { ...config };
      
      // Initialize native core manager
      const result = native.coreManagerInitialize(JSON.stringify(config));
      if (!result.success) {
        throw new SatoxCoreError(result.error || 'Failed to initialize native core manager');
      }

      this._initialized = true;
      this.emit('initialized', { timestamp: new Date().toISOString() });

      return true;
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxCoreError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Shutdown the core manager
   */
  public async shutdown(): Promise<void> {
    try {
      if (!this._initialized) {
        return;
      }

      native.coreManagerShutdown();
      this._initialized = false;
      this.emit('shutdown', { timestamp: new Date().toISOString() });
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxCoreError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Check if core manager is initialized
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
      operations_performed: 0,
      errors_encountered: 0,
      last_operation: null,
      uptime: 0
    };
  }

  /**
   * Health check for core manager
   */
  public async healthCheck(): Promise<boolean> {
    try {
      if (!this._initialized) {
        return false;
      }

      const result = native.coreManagerHealthCheck();
      return result.success;
    } catch {
      return false;
    }
  }

  /**
   * Get SDK version
   */
  public async getVersion(): Promise<string> {
    try {
      if (!this._initialized) {
        throw new SatoxCoreError('Core manager not initialized');
      }

      const result = native.coreManagerGetVersion();
      if (!result.success) {
        throw new SatoxCoreError(result.error || 'Failed to get version');
      }

      return result.version;
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxCoreError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Get configuration
   */
  public getConfig(): CoreConfig {
    return { ...this._config };
  }

  /**
   * Update configuration
   */
  public async updateConfig(config: Partial<CoreConfig>): Promise<boolean> {
    try {
      if (!this._initialized) {
        throw new SatoxCoreError('Core manager not initialized');
      }

      const newConfig = { ...this._config, ...config };
      const result = native.coreManagerUpdateConfig(JSON.stringify(newConfig));
      if (!result.success) {
        throw new SatoxCoreError(result.error || 'Failed to update configuration');
      }

      this._config = newConfig;
      this._stats.operations_performed++;
      this._stats.last_operation = 'update_config';

      this.emit('config_updated', {
        timestamp: new Date().toISOString(),
        config: newConfig
      });

      return true;
    } catch (error) {
      this._lastError = error.message;
      this._stats.errors_encountered++;
      const satoxError = error instanceof SatoxError ? error : new SatoxCoreError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }
} 