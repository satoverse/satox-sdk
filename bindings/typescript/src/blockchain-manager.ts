/**
 * Satox SDK Blockchain Manager Binding
 */

import { SatoxError } from './error';
import { NetworkType, BlockInfo, TransactionInfo } from './types';

export class BlockchainManager {
    private core: any;
    private initialized: boolean = false;
    private connected: boolean = false;
    private callbacks: Array<(event: string, data: any) => void> = [];

    constructor() {
        this.core = require('./native/satox_core');
    }

    async initialize(config: Record<string, any>): Promise<boolean> {
        if (this.initialized) {
            throw new SatoxError('BlockchainManager already initialized');
        }

        try {
            const result = await this.core.blockchainInitialize(config);
            this.initialized = result;
            return result;
        } catch (error) {
            throw new SatoxError(`Failed to initialize BlockchainManager: ${error.message}`);
        }
    }

    async connect(network: NetworkType, rpcEndpoint: string, credentials?: Record<string, any>): Promise<boolean> {
        this.ensureInitialized();

        try {
            const config = {
                network: network,
                rpcEndpoint: rpcEndpoint,
                credentials: credentials || {}
            };
            const result = await this.core.blockchainConnect(config);
            this.connected = result;
            return result;
        } catch (error) {
            throw new SatoxError(`Failed to connect to blockchain: ${error.message}`);
        }
    }

    async getBlockHeight(): Promise<number> {
        this.ensureConnected();

        try {
            return await this.core.blockchainGetBlockHeight();
        } catch (error) {
            throw new SatoxError(`Failed to get block height: ${error.message}`);
        }
    }

    async getBlockInfo(blockHash: string): Promise<BlockInfo> {
        this.ensureConnected();

        try {
            return await this.core.blockchainGetBlockInfo(blockHash);
        } catch (error) {
            throw new SatoxError(`Failed to get block info: ${error.message}`);
        }
    }

    async getTransactionInfo(txid: string): Promise<TransactionInfo> {
        this.ensureConnected();

        try {
            return await this.core.blockchainGetTransactionInfo(txid);
        } catch (error) {
            throw new SatoxError(`Failed to get transaction info: ${error.message}`);
        }
    }

    async sendRawTransaction(rawTx: string): Promise<string> {
        this.ensureConnected();

        try {
            return await this.core.blockchainSendRawTransaction(rawTx);
        } catch (error) {
            throw new SatoxError(`Failed to send raw transaction: ${error.message}`);
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
            throw new SatoxError('BlockchainManager not initialized');
        }
    }

    private ensureConnected(): void {
        if (!this.connected) {
            throw new SatoxError('Not connected to blockchain');
        }
    }
} 