/**
 * Satox SDK Security Manager Binding
 */

import { SatoxError } from './error';

export class SecurityManager {
    private core: any;
    private initialized: boolean = false;
    private callbacks: Array<(event: string, data: any) => void> = [];

    constructor() {
        this.core = require('./native/satox_core');
    }

    async initialize(config: Record<string, any>): Promise<boolean> {
        if (this.initialized) {
            throw new SatoxError('SecurityManager already initialized');
        }

        try {
            const result = await this.core.securityInitialize(config);
            this.initialized = result;
            return result;
        } catch (error) {
            throw new SatoxError(`Failed to initialize SecurityManager: ${error.message}`);
        }
    }

    async generateKeyPair(): Promise<Record<string, string>> {
        this.ensureInitialized();

        try {
            return await this.core.securityGenerateKeyPair();
        } catch (error) {
            throw new SatoxError(`Failed to generate key pair: ${error.message}`);
        }
    }

    async encryptData(data: Buffer, publicKey: string): Promise<Buffer> {
        this.ensureInitialized();

        try {
            return await this.core.securityEncryptData(data, publicKey);
        } catch (error) {
            throw new SatoxError(`Failed to encrypt data: ${error.message}`);
        }
    }

    async decryptData(encryptedData: Buffer, privateKey: string): Promise<Buffer> {
        this.ensureInitialized();

        try {
            return await this.core.securityDecryptData(encryptedData, privateKey);
        } catch (error) {
            throw new SatoxError(`Failed to decrypt data: ${error.message}`);
        }
    }

    async signData(data: Buffer, privateKey: string): Promise<Buffer> {
        this.ensureInitialized();

        try {
            return await this.core.securitySignData(data, privateKey);
        } catch (error) {
            throw new SatoxError(`Failed to sign data: ${error.message}`);
        }
    }

    async verifySignature(data: Buffer, signature: Buffer, publicKey: string): Promise<boolean> {
        this.ensureInitialized();

        try {
            return await this.core.securityVerifySignature(data, signature, publicKey);
        } catch (error) {
            throw new SatoxError(`Failed to verify signature: ${error.message}`);
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
            throw new SatoxError('SecurityManager not initialized');
        }
    }
} 