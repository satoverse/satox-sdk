/**
 * @file wallet-manager.ts
 * @brief Wallet Manager for TypeScript/JavaScript bindings
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

import { EventEmitter } from 'events';
import { native } from '../../build/Release/satox_sdk_native';
import {
  WalletConfig,
  WalletInfo,
  AddressInfo,
  TransactionInfo,
  IManager,
  SDKEvent,
  WalletEvent,
  SatoxError,
  SatoxWalletError,
  AsyncBoolean,
  AsyncString,
  AsyncNumber,
  AsyncVoid
} from '../types';

/**
 * Wallet Manager providing wallet and address management functionality
 */
export class WalletManager extends EventEmitter implements IManager {
  private _initialized = false;
  private _config: WalletConfig = {};
  private _lastError = '';
  private _stats: Record<string, any> = {};

  constructor() {
    super();
    this.resetStats();
  }

  /**
   * Initialize the wallet manager
   */
  public async initialize(config: WalletConfig): Promise<boolean> {
    try {
      if (this._initialized) {
        throw new SatoxWalletError('Wallet manager already initialized');
      }

      this._config = { ...config };
      
      // Initialize native wallet manager
      const result = native.walletManagerInitialize(JSON.stringify(config));
      if (!result.success) {
        throw new SatoxWalletError(result.error || 'Failed to initialize native wallet manager');
      }

      this._initialized = true;
      this.emit('initialized', { timestamp: new Date().toISOString() });

      return true;
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxWalletError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Shutdown the wallet manager
   */
  public async shutdown(): Promise<void> {
    try {
      if (!this._initialized) {
        return;
      }

      native.walletManagerShutdown();
      this._initialized = false;
      this.emit('shutdown', { timestamp: new Date().toISOString() });
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxWalletError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Check if wallet manager is initialized
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
      wallets_created: 0,
      addresses_generated: 0,
      transactions_sent: 0,
      transactions_received: 0,
      total_balance: 0,
      last_operation: null
    };
  }

  /**
   * Health check for wallet manager
   */
  public async healthCheck(): Promise<boolean> {
    try {
      if (!this._initialized) {
        return false;
      }

      const result = native.walletManagerHealthCheck();
      return result.success;
    } catch {
      return false;
    }
  }

  /**
   * Create a new wallet
   */
  public async createWallet(name: string): Promise<string> {
    try {
      if (!this._initialized) {
        throw new SatoxWalletError('Wallet manager not initialized');
      }

      const result = native.walletManagerCreateWallet(name);
      if (!result.success) {
        throw new SatoxWalletError(result.error || 'Failed to create wallet');
      }

      const walletId = result.wallet_id;
      this._stats.wallets_created++;
      this._stats.last_operation = 'create_wallet';

      const event: WalletEvent = {
        type: 'wallet_created',
        timestamp: new Date().toISOString(),
        wallet_id: walletId,
        data: {
          id: walletId,
          name,
          created_at: new Date().toISOString(),
          balance: 0,
          address_count: 0,
          transaction_count: 0
        }
      };

      this.emit('*', event);
      this.emit('wallet_created', event);

      return walletId;
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxWalletError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Generate a new address for a wallet
   */
  public async generateAddress(walletId: string): Promise<string> {
    try {
      if (!this._initialized) {
        throw new SatoxWalletError('Wallet manager not initialized');
      }

      const result = native.walletManagerGenerateAddress(walletId);
      if (!result.success) {
        throw new SatoxWalletError(result.error || 'Failed to generate address');
      }

      const address = result.address;
      this._stats.addresses_generated++;
      this._stats.last_operation = 'generate_address';

      const event: WalletEvent = {
        type: 'address_generated',
        timestamp: new Date().toISOString(),
        wallet_id: walletId,
        data: {
          address,
          wallet_id: walletId,
          created_at: new Date().toISOString(),
          balance: 0,
          transaction_count: 0,
          is_change: false
        }
      };

      this.emit('*', event);
      this.emit('address_generated', event);

      return address;
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxWalletError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Get wallet balance
   */
  public async getBalance(walletId: string): Promise<number> {
    try {
      if (!this._initialized) {
        throw new SatoxWalletError('Wallet manager not initialized');
      }

      const result = native.walletManagerGetBalance(walletId);
      if (!result.success) {
        throw new SatoxWalletError(result.error || 'Failed to get balance');
      }

      return result.balance;
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxWalletError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Get all addresses for a wallet
   */
  public async getAddresses(walletId: string): Promise<AddressInfo[]> {
    try {
      if (!this._initialized) {
        throw new SatoxWalletError('Wallet manager not initialized');
      }

      const result = native.walletManagerGetAddresses(walletId);
      if (!result.success) {
        throw new SatoxWalletError(result.error || 'Failed to get addresses');
      }

      return result.addresses || [];
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxWalletError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Send a transaction
   */
  public async sendTransaction(
    fromAddress: string,
    toAddress: string,
    amount: number,
    fee?: number
  ): Promise<string> {
    try {
      if (!this._initialized) {
        throw new SatoxWalletError('Wallet manager not initialized');
      }

      const result = native.walletManagerSendTransaction(fromAddress, toAddress, amount, fee || 0);
      if (!result.success) {
        throw new SatoxWalletError(result.error || 'Failed to send transaction');
      }

      const txId = result.transaction_id;
      this._stats.transactions_sent++;
      this._stats.last_operation = 'send_transaction';

      const event: WalletEvent = {
        type: 'transaction_sent',
        timestamp: new Date().toISOString(),
        wallet_id: '', // Will be determined from address
        data: {
          id: txId,
          from_address: fromAddress,
          to_address: toAddress,
          amount,
          fee: fee || 0,
          timestamp: new Date().toISOString(),
          status: 'pending'
        }
      };

      this.emit('*', event);
      this.emit('transaction_sent', event);

      return txId;
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxWalletError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Get transaction information
   */
  public async getTransaction(txId: string): Promise<TransactionInfo> {
    try {
      if (!this._initialized) {
        throw new SatoxWalletError('Wallet manager not initialized');
      }

      const result = native.walletManagerGetTransaction(txId);
      if (!result.success) {
        throw new SatoxWalletError(result.error || 'Failed to get transaction');
      }

      return result.transaction;
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxWalletError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Get wallet information
   */
  public async getWalletInfo(walletId: string): Promise<WalletInfo> {
    try {
      if (!this._initialized) {
        throw new SatoxWalletError('Wallet manager not initialized');
      }

      const result = native.walletManagerGetWalletInfo(walletId);
      if (!result.success) {
        throw new SatoxWalletError(result.error || 'Failed to get wallet info');
      }

      return result.wallet_info;
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxWalletError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Get all wallets
   */
  public async getAllWallets(): Promise<WalletInfo[]> {
    try {
      if (!this._initialized) {
        throw new SatoxWalletError('Wallet manager not initialized');
      }

      const result = native.walletManagerGetAllWallets();
      if (!result.success) {
        throw new SatoxWalletError(result.error || 'Failed to get wallets');
      }

      return result.wallets || [];
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxWalletError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Delete a wallet
   */
  public async deleteWallet(walletId: string): Promise<boolean> {
    try {
      if (!this._initialized) {
        throw new SatoxWalletError('Wallet manager not initialized');
      }

      const result = native.walletManagerDeleteWallet(walletId);
      if (!result.success) {
        throw new SatoxWalletError(result.error || 'Failed to delete wallet');
      }

      const event: WalletEvent = {
        type: 'wallet_deleted',
        timestamp: new Date().toISOString(),
        wallet_id: walletId
      };

      this.emit('*', event);
      this.emit('wallet_deleted', event);

      return true;
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxWalletError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Backup wallet
   */
  public async backupWallet(walletId: string, backupPath: string): Promise<boolean> {
    try {
      if (!this._initialized) {
        throw new SatoxWalletError('Wallet manager not initialized');
      }

      const result = native.walletManagerBackupWallet(walletId, backupPath);
      if (!result.success) {
        throw new SatoxWalletError(result.error || 'Failed to backup wallet');
      }

      return true;
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxWalletError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }

  /**
   * Restore wallet from backup
   */
  public async restoreWallet(backupPath: string, name?: string): Promise<string> {
    try {
      if (!this._initialized) {
        throw new SatoxWalletError('Wallet manager not initialized');
      }

      const result = native.walletManagerRestoreWallet(backupPath, name || 'restored_wallet');
      if (!result.success) {
        throw new SatoxWalletError(result.error || 'Failed to restore wallet');
      }

      const walletId = result.wallet_id;
      this._stats.wallets_created++;
      this._stats.last_operation = 'restore_wallet';

      const event: WalletEvent = {
        type: 'wallet_created',
        timestamp: new Date().toISOString(),
        wallet_id: walletId,
        data: {
          id: walletId,
          name: name || 'restored_wallet',
          created_at: new Date().toISOString(),
          balance: 0,
          address_count: 0,
          transaction_count: 0
        }
      };

      this.emit('*', event);
      this.emit('wallet_created', event);

      return walletId;
    } catch (error) {
      this._lastError = error.message;
      const satoxError = error instanceof SatoxError ? error : new SatoxWalletError(error.message);
      this.emit('error', satoxError);
      throw satoxError;
    }
  }
} 