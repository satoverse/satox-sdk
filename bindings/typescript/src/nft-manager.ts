/**
 * Satox SDK NFT Manager Binding
 */

import { SatoxError } from './error';
import { NFT } from './types';

export class NFTManager {
    private core: any;
    private initialized: boolean = false;
    private callbacks: Array<(event: string, data: any) => void> = [];

    constructor() {
        this.core = require('./native/satox_core');
    }

    async initialize(config: Record<string, any>): Promise<boolean> {
        if (this.initialized) {
            throw new SatoxError('NFTManager already initialized');
        }

        try {
            const result = await this.core.nftInitialize(config);
            this.initialized = result;
            return result;
        } catch (error) {
            throw new SatoxError(`Failed to initialize NFTManager: ${error.message}`);
        }
    }

    async createNFT(config: Record<string, any>): Promise<NFT> {
        this.ensureInitialized();

        try {
            return await this.core.nftCreate(config);
        } catch (error) {
            throw new SatoxError(`Failed to create NFT: ${error.message}`);
        }
    }

    async getNFT(nftId: string): Promise<NFT> {
        this.ensureInitialized();

        try {
            return await this.core.nftGet(nftId);
        } catch (error) {
            throw new SatoxError(`Failed to get NFT: ${error.message}`);
        }
    }

    async updateNFT(nftId: string, metadata: Record<string, any>): Promise<boolean> {
        this.ensureInitialized();

        try {
            return await this.core.nftUpdate(nftId, metadata);
        } catch (error) {
            throw new SatoxError(`Failed to update NFT: ${error.message}`);
        }
    }

    async transferNFT(nftId: string, fromAddress: string, toAddress: string): Promise<string> {
        this.ensureInitialized();

        try {
            return await this.core.nftTransfer(nftId, fromAddress, toAddress);
        } catch (error) {
            throw new SatoxError(`Failed to transfer NFT: ${error.message}`);
        }
    }

    async listNFTs(owner?: string): Promise<NFT[]> {
        this.ensureInitialized();

        try {
            return await this.core.nftList(owner);
        } catch (error) {
            throw new SatoxError(`Failed to list NFTs: ${error.message}`);
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
            throw new SatoxError('NFTManager not initialized');
        }
    }
} 