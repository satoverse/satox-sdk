/**
 * Satox SDK IPFS Manager Binding
 */

import { SatoxError } from './error';

export class IPFSManager {
    private core: any;
    private initialized: boolean = false;
    private callbacks: Array<(event: string, data: any) => void> = [];

    constructor() {
        this.core = require('./native/satox_core');
    }

    async initialize(config: Record<string, any>): Promise<boolean> {
        if (this.initialized) {
            throw new SatoxError('IPFSManager already initialized');
        }

        try {
            const result = await this.core.ipfsInitialize(config);
            this.initialized = result;
            return result;
        } catch (error) {
            throw new SatoxError(`Failed to initialize IPFSManager: ${error.message}`);
        }
    }

    async uploadFile(filePath: string): Promise<string> {
        this.ensureInitialized();

        try {
            return await this.core.ipfsUploadFile(filePath);
        } catch (error) {
            throw new SatoxError(`Failed to upload file: ${error.message}`);
        }
    }

    async downloadFile(ipfsHash: string, outputPath: string): Promise<boolean> {
        this.ensureInitialized();

        try {
            return await this.core.ipfsDownloadFile(ipfsHash, outputPath);
        } catch (error) {
            throw new SatoxError(`Failed to download file: ${error.message}`);
        }
    }

    async uploadData(data: Buffer): Promise<string> {
        this.ensureInitialized();

        try {
            return await this.core.ipfsUploadData(data);
        } catch (error) {
            throw new SatoxError(`Failed to upload data: ${error.message}`);
        }
    }

    async getData(ipfsHash: string): Promise<Buffer> {
        this.ensureInitialized();

        try {
            return await this.core.ipfsGetData(ipfsHash);
        } catch (error) {
            throw new SatoxError(`Failed to get data: ${error.message}`);
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
            throw new SatoxError('IPFSManager not initialized');
        }
    }
} 