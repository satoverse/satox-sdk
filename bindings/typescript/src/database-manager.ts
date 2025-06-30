/**
 * Satox SDK Database Manager Binding
 */

import { SatoxError } from './error';
import { DatabaseType, QueryResult, DatabaseConfig } from './types';

export class DatabaseManager {
    private core: any;
    private initialized: boolean = false;
    private connected: boolean = false;
    private callbacks: Array<(event: string, data: any) => void> = [];

    constructor() {
        this.core = require('./native/satox_core');
    }

    async initialize(config: DatabaseConfig): Promise<boolean> {
        if (this.initialized) {
            throw new SatoxError('DatabaseManager already initialized');
        }

        try {
            const result = await this.core.databaseInitialize(config);
            this.initialized = result;
            return result;
        } catch (error) {
            throw new SatoxError(`Failed to initialize DatabaseManager: ${error.message}`);
        }
    }

    async connect(dbType: DatabaseType, connectionString: string): Promise<boolean> {
        this.ensureInitialized();

        try {
            const result = await this.core.databaseConnect(dbType, connectionString);
            this.connected = result;
            return result;
        } catch (error) {
            throw new SatoxError(`Failed to connect to database: ${error.message}`);
        }
    }

    async executeQuery(query: string, params?: Record<string, any>): Promise<QueryResult> {
        this.ensureConnected();

        try {
            const result = await this.core.databaseExecuteQuery(query, params || {});
            return result;
        } catch (error) {
            return {
                success: false,
                rows: [],
                error: error.message,
                affectedRows: 0,
                lastInsertId: null
            };
        }
    }

    async executeTransaction(queries: string[]): Promise<boolean> {
        this.ensureConnected();

        try {
            return await this.core.databaseExecuteTransaction(queries);
        } catch (error) {
            throw new SatoxError(`Failed to execute transaction: ${error.message}`);
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
            throw new SatoxError('DatabaseManager not initialized');
        }
    }

    private ensureConnected(): void {
        if (!this.connected) {
            throw new SatoxError('Not connected to database');
        }
    }
} 