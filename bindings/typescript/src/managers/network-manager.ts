/**
 * @file network-manager.ts
 * @brief Network Manager for TypeScript/JavaScript bindings
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

import { EventEmitter } from 'events';
import { native } from '../../build/Release/satox_sdk_native';
import {
  NetworkConfig,
  IManager,
  SDKEvent,
  SatoxError,
  SatoxNetworkError,
  AsyncBoolean,
  AsyncString,
  AsyncNumber,
  AsyncVoid
} from '../types';

/**
 * Network Manager providing network functionality
 */
export class NetworkManager extends EventEmitter implements IManager {
  private _initialized = false;
  private _config: NetworkConfig = {};
  private _lastError = '';
  private _stats: Record<string, any> = {};

  constructor() {
    super();
    this.resetStats();
  }

  /**
   * Initialize the network manager
   */
  public async initialize(config: NetworkConfig): Promise<boolean> {
    try {
      if (this._initialized) {
        throw new SatoxNetworkError('Network manager already initialized');
      }

      this._config = { ...config };
      
      // Initialize native network manager
      const result = native.networkManagerInitialize(JSON.stringify(config));
      if (!result.success) {
        throw new SatoxNetworkError(result.error || 'Failed to initialize native network manager');
      }

      this._initialized = true;
      this.emit('initialized', { timestamp: new Date().toISOString() });

      return true;
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxNetworkError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Shutdown the network manager
   */
  public async shutdown(): Promise<void> {
    try {
      if (!this._initialized) {
        return;
      }

      native.networkManagerShutdown();
      this._initialized = false;
      this.emit('shutdown', { timestamp: new Date().toISOString() });
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxNetworkError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Check if network manager is initialized
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
      connections_established: 0,
      messages_sent: 0,
      messages_received: 0,
      last_operation: null
    };
  }

  /**
   * Health check for network manager
   */
  public async healthCheck(): Promise<boolean> {
    try {
      if (!this._initialized) {
        return false;
      }

      const result = native.networkManagerHealthCheck();
      return result.success;
    } catch {
      return false;
    }
  }
} 