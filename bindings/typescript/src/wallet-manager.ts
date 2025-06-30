/**
 * Satox SDK Wallet Manager Binding
 */

import { SatoxError } from './error';
import { Wallet } from './types';

export class WalletManager {
    private core: any;
    private initialized: boolean = false;
    private callbacks: Array<(event: string, data: any) => void> = [];

    constructor() {
        this.core = require('./native/satox_core');
    }

    async initialize(config: Record<string, any>): Promise<boolean> {
        if (this.initialized) {
            throw new SatoxError('WalletManager already initialized');
        }

        try {
            const result = await this.core.walletInitialize(config);
            this.initialized = result;
            return result;
        } catch (error) {
            throw new SatoxError(`Failed to initialize WalletManager: ${error.message}`);
        }
    }

    async createWallet(password?: string): Promise<Wallet> {
        this.ensureInitialized();

        try {
            return await this.core.walletCreate(password);
        } catch (error) {
            throw new SatoxError(`Failed to create wallet: ${error.message}`);
        }
    }

    async loadWallet(walletPath: string, password?: string): Promise<Wallet> {
        this.ensureInitialized();

        try {
            return await this.core.walletLoad(walletPath, password);
        } catch (error) {
            throw new SatoxError(`Failed to load wallet: ${error.message}`);
        }
    }

    async getBalance(address: string): Promise<Record<string, number>> {
        this.ensureInitialized();

        try {
            return await this.core.walletGetBalance(address);
        } catch (error) {
            throw new SatoxError(`Failed to get balance: ${error.message}`);
        }
    }

    async sendTransaction(fromAddress: string, toAddress: string, amount: number, assetId?: string): Promise<string> {
        this.ensureInitialized();

        try {
            return await this.core.walletSendTransaction(fromAddress, toAddress, amount, assetId);
        } catch (error) {
            throw new SatoxError(`Failed to send transaction: ${error.message}`);
        }
    }

    async getTransactionHistory(address: string): Promise<Record<string, any>[]> {
        this.ensureInitialized();

        try {
            return await this.core.walletGetTransactionHistory(address);
        } catch (error) {
            throw new SatoxError(`Failed to get transaction history: ${error.message}`);
        }
    }

    registerCallback(callback: (event: string, data: any) => void): void {
        this.callbacks.push(callback);
    }

    unregisterCallback(callback: (event: string, data: any) => void): void {
        const index = this.callbacks.indexOf(callback);
        if (index > -1) {
            this.callbacks.splice(index, 1);
        }
    }

    private ensureInitialized(): void {
        if (!this.initialized) {
            throw new SatoxError('WalletManager not initialized');
        }
    }
} 