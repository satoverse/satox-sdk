/**
 * @file security-manager.ts
 * @brief Security Manager for TypeScript/JavaScript bindings
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

import { EventEmitter } from 'events';
import { native } from '../../build/Release/satox_sdk_native';
import {
  SecurityConfig,
  IManager,
  SDKEvent,
  SatoxError,
  SatoxSecurityError,
  AsyncBoolean,
  AsyncString,
  AsyncNumber,
  AsyncVoid
} from '../types';

/**
 * Security Manager providing cryptographic functionality
 */
export class SecurityManager extends EventEmitter implements IManager {
  private _initialized = false;
  private _config: SecurityConfig = {};
  private _lastError = '';
  private _stats: Record<string, any> = {};

  constructor() {
    super();
    this.resetStats();
  }

  /**
   * Initialize the security manager
   */
  public async initialize(config: SecurityConfig): Promise<boolean> {
    try {
      if (this._initialized) {
        throw new SatoxSecurityError('Security manager already initialized');
      }

      this._config = { ...config };
      
      // Initialize native security manager
      const result = native.securityManagerInitialize(JSON.stringify(config));
      if (!result.success) {
        throw new SatoxSecurityError(result.error || 'Failed to initialize native security manager');
      }

      this._initialized = true;
      this.emit('initialized', { timestamp: new Date().toISOString() });

      return true;
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxSecurityError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Shutdown the security manager
   */
  public async shutdown(): Promise<void> {
    try {
      if (!this._initialized) {
        return;
      }

      native.securityManagerShutdown();
      this._initialized = false;
      this.emit('shutdown', { timestamp: new Date().toISOString() });
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxSecurityError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Check if security manager is initialized
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
      keypairs_generated: 0,
      signatures_created: 0,
      signatures_verified: 0,
      data_encrypted: 0,
      data_decrypted: 0,
      last_operation: null
    };
  }

  /**
   * Health check for security manager
   */
  public async healthCheck(): Promise<boolean> {
    try {
      if (!this._initialized) {
        return false;
      }

      const result = native.securityManagerHealthCheck();
      return result.success;
    } catch {
      return false;
    }
  }

  /**
   * Generate a new keypair
   */
  public async generateKeypair(): Promise<{ publicKey: string; privateKey: string }> {
    try {
      if (!this._initialized) {
        throw new SatoxSecurityError('Security manager not initialized');
      }

      const result = native.securityManagerGenerateKeypair();
      if (!result.success) {
        throw new SatoxSecurityError(result.error || 'Failed to generate keypair');
      }

      this._stats.keypairs_generated++;
      this._stats.last_operation = 'generate_keypair';

      return {
        publicKey: result.public_key,
        privateKey: result.private_key
      };
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxSecurityError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Sign data with private key
   */
  public async signData(data: string, privateKey: string): Promise<string> {
    try {
      if (!this._initialized) {
        throw new SatoxSecurityError('Security manager not initialized');
      }

      const result = native.securityManagerSignData(data, privateKey);
      if (!result.success) {
        throw new SatoxSecurityError(result.error || 'Failed to sign data');
      }

      this._stats.signatures_created++;
      this._stats.last_operation = 'sign_data';

      return result.signature;
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxSecurityError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Verify signature
   */
  public async verifySignature(data: string, signature: string, publicKey: string): Promise<boolean> {
    try {
      if (!this._initialized) {
        throw new SatoxSecurityError('Security manager not initialized');
      }

      const result = native.securityManagerVerifySignature(data, signature, publicKey);
      if (!result.success) {
        throw new SatoxSecurityError(result.error || 'Failed to verify signature');
      }

      this._stats.signatures_verified++;
      this._stats.last_operation = 'verify_signature';

      return result.valid;
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxSecurityError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }
} 