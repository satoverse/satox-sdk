/**
 * @file database-manager.ts
 * @brief Database Manager for TypeScript/JavaScript bindings
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

import { EventEmitter } from 'events';
import { native } from '../../build/Release/satox_sdk_native';
import {
  DatabaseConfig,
  IManager,
  SDKEvent,
  SatoxError,
  SatoxDatabaseError,
  AsyncBoolean,
  AsyncString,
  AsyncNumber,
  AsyncVoid
} from '../types';

/**
 * Database Manager providing database functionality
 */
export class DatabaseManager extends EventEmitter implements IManager {
  private _initialized = false;
  private _config: DatabaseConfig = {};
  private _lastError = '';
  private _stats: Record<string, any> = {};

  constructor() {
    super();
    this.resetStats();
  }

  /**
   * Initialize the database manager
   */
  public async initialize(config: DatabaseConfig): Promise<boolean> {
    try {
      if (this._initialized) {
        throw new SatoxDatabaseError('Database manager already initialized');
      }

      this._config = { ...config };
      
      // Initialize native database manager
      const result = native.databaseManagerInitialize(JSON.stringify(config));
      if (!result.success) {
        throw new SatoxDatabaseError(result.error || 'Failed to initialize native database manager');
      }

      this._initialized = true;
      this.emit('initialized', { timestamp: new Date().toISOString() });

      return true;
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxDatabaseError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Shutdown the database manager
   */
  public async shutdown(): Promise<void> {
    try {
      if (!this._initialized) {
        return;
      }

      native.databaseManagerShutdown();
      this._initialized = false;
      this.emit('shutdown', { timestamp: new Date().toISOString() });
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxDatabaseError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Check if database manager is initialized
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
      databases_created: 0,
      queries_executed: 0,
      last_operation: null
    };
  }

  /**
   * Health check for database manager
   */
  public async healthCheck(): Promise<boolean> {
    try {
      if (!this._initialized) {
        return false;
      }

      const result = native.databaseManagerHealthCheck();
      return result.success;
    } catch {
      return false;
    }
  }
} 