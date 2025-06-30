/**
 * Satox SDK Network Manager Binding
 */

import { SatoxError } from './error';

export class NetworkManager {
    private core: any;
    private initialized: boolean = false;
    private connected: boolean = false;
    private callbacks: Array<(event: string, data: any) => void> = [];

    constructor() {
        this.core = require('./native/satox_core');
    }

    async initialize(config: Record<string, any>): Promise<boolean> {
        if (this.initialized) {
            throw new SatoxError('NetworkManager already initialized');
        }

        try {
            const result = await this.core.networkInitialize(config);
            this.initialized = result;
            return result;
        } catch (error) {
            throw new SatoxError(`Failed to initialize NetworkManager: ${error.message}`);
        }
    }

    async connect(host: string, port: number): Promise<boolean> {
        this.ensureInitialized();

        try {
            const result = await this.core.networkConnect(host, port);
            this.connected = result;
            return result;
        } catch (error) {
            throw new SatoxError(`Failed to connect to network: ${error.message}`);
        }
    }

    async disconnect(): Promise<boolean> {
        if (!this.connected) {
            return true;
        }

        try {
            const result = await this.core.networkDisconnect();
            this.connected = false;
            return result;
        } catch (error) {
            throw new SatoxError(`Failed to disconnect: ${error.message}`);
        }
    }

    async sendMessage(message: Record<string, any>): Promise<boolean> {
        this.ensureConnected();

        try {
            return await this.core.networkSendMessage(message);
        } catch (error) {
            throw new SatoxError(`Failed to send message: ${error.message}`);
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
            throw new SatoxError('NetworkManager not initialized');
        }
    }

    private ensureConnected(): void {
        if (!this.connected) {
            throw new SatoxError('Not connected to network');
        }
    }
} 