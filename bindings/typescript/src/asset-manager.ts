/**
 * Satox SDK Asset Manager Binding
 */

import { SatoxError } from './error';
import { Asset, AssetType } from './types';

export class AssetManager {
    private core: any;
    private initialized: boolean = false;
    private callbacks: Array<(event: string, data: any) => void> = [];

    constructor() {
        this.core = require('./native/satox_core');
    }

    async initialize(config: Record<string, any>): Promise<boolean> {
        if (this.initialized) {
            throw new SatoxError('AssetManager already initialized');
        }

        try {
            const result = await this.core.assetInitialize(config);
            this.initialized = result;
            return result;
        } catch (error) {
            throw new SatoxError(`Failed to initialize AssetManager: ${error.message}`);
        }
    }

    async createAsset(config: Record<string, any>): Promise<Asset> {
        this.ensureInitialized();

        try {
            return await this.core.assetCreate(config);
        } catch (error) {
            throw new SatoxError(`Failed to create asset: ${error.message}`);
        }
    }

    async getAsset(assetId: string): Promise<Asset> {
        this.ensureInitialized();

        try {
            return await this.core.assetGet(assetId);
        } catch (error) {
            throw new SatoxError(`Failed to get asset: ${error.message}`);
        }
    }

    async updateAsset(assetId: string, metadata: Record<string, any>): Promise<boolean> {
        this.ensureInitialized();

        try {
            return await this.core.assetUpdate(assetId, metadata);
        } catch (error) {
            throw new SatoxError(`Failed to update asset: ${error.message}`);
        }
    }

    async transferAsset(assetId: string, fromAddress: string, toAddress: string, amount: number): Promise<string> {
        this.ensureInitialized();

        try {
            return await this.core.assetTransfer(assetId, fromAddress, toAddress, amount);
        } catch (error) {
            throw new SatoxError(`Failed to transfer asset: ${error.message}`);
        }
    }

    async getBalance(address: string, assetId?: string): Promise<Record<string, number>> {
        this.ensureInitialized();

        try {
            return await this.core.assetGetBalance(address, assetId);
        } catch (error) {
            throw new SatoxError(`Failed to get balance: ${error.message}`);
        }
    }

    async listAssets(owner?: string, assetType?: AssetType): Promise<Asset[]> {
        this.ensureInitialized();

        try {
            return await this.core.assetList(owner, assetType);
        } catch (error) {
            throw new SatoxError(`Failed to list assets: ${error.message}`);
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
            throw new SatoxError('AssetManager not initialized');
        }
    }
} 