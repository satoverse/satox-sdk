/**
 * Satox SDK Core Manager Binding
 */

import { SatoxError } from './error';
import { CoreConfig, CoreStatus } from './types';

export class CoreManager {
    private core: any; // Native binding
    private initialized: boolean = false;
    private callbacks: Array<(event: string, success: boolean) => void> = [];

    constructor() {
        // Initialize native binding
        this.core = require('./native/satox_core');
    }

    async initialize(config?: CoreConfig): Promise<boolean> {
        if (this.initialized) {
            throw new SatoxError('CoreManager already initialized');
        }

        try {
            const result = await this.core.initialize(config || {});
            this.initialized = result;
            return result;
        } catch (error) {
            throw new SatoxError(`Failed to initialize CoreManager: ${error.message}`);
        }
    }

    async shutdown(): Promise<boolean> {
        if (!this.initialized) {
            return true;
        }

        try {
            const result = await this.core.shutdown();
            this.initialized = false;
            return result;
        } catch (error) {
            throw new SatoxError(`Failed to shutdown CoreManager: ${error.message}`);
        }
    }

    async getStatus(): Promise<CoreStatus> {
        this.ensureInitialized();
        return await this.core.getStatus();
    }

    registerCallback(callback: (event: string, success: boolean) => void): void {
        this.callbacks.push(callback);
    }

    unregisterCallback(callback: (event: string, success: boolean) => void): void {
        const index = this.callbacks.indexOf(callback);
        if (index > -1) {
            this.callbacks.splice(index, 1);
        }
    }

    private ensureInitialized(): void {
        if (!this.initialized) {
            throw new SatoxError('CoreManager not initialized');
        }
    }
} 