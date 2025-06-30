import { ThreadSafeManager, ThreadState, ThreadSafeStats } from '../thread_safe_manager';

/**
 * Blockchain Manager for WASM Bindings with Full Thread Safety
 * Provides thread-safe blockchain operations using Web Workers and SharedArrayBuffer
 */

export interface BlockchainConfig {
    rpcUrl: string;
    rpcPort: number;
    rpcUsername?: string;
    rpcPassword?: string;
    timeoutSeconds: number;
    enableSsl: boolean;
}

export interface BlockInfo {
    hash: string;
    height: number;
    timestamp: number;
    size: number;
    weight: number;
    version: number;
    merkleroot: string;
    txCount: number;
    previousHash?: string;
    nextHash?: string;
}

export interface TransactionInfo {
    txid: string;
    blockHash?: string;
    blockHeight?: number;
    timestamp: number;
    size: number;
    weight: number;
    version: number;
    locktime: number;
    confirmations: number;
    fee?: number;
    amount?: number;
}

export class BlockchainManager extends ThreadSafeManager {
    private handle: any; // C++ handle
    private memory: any; // Memory management
    private workers: Worker[] = [];

    constructor(config: BlockchainConfig) {
        super("BlockchainManager");
        this.handle = null;
        this.memory = null;
        this.initializeHandle(config);
    }

    private initializeHandle(config: BlockchainConfig): void {
        // Initialize the C++ handle through WASM
        const configJson = JSON.stringify(config);
        this.handle = (globalThis as any).satox.blockchain_manager_create(configJson);
        if (!this.handle) {
            throw new Error("Failed to create blockchain manager");
        }
    }

    async initialize(): Promise<boolean> {
        const operationId = await this.beginOperation();

        try {
            // Check current state
            const currentState = await this.getState();
            if (currentState === ThreadState.INITIALIZED) {
                await this.endOperation(operationId);
                return true;
            }

            // Set initializing state
            await this.setState(ThreadState.INITIALIZING);

            // Call C++ initialization
            const result = (globalThis as any).satox.blockchain_manager_initialize(this.handle);
            if (result === 0) {
                await this.setState(ThreadState.INITIALIZED);
                await this.clearError();
                await this.emitEvent("initialized", null);
                await this.endOperation(operationId);
                return true;
            } else {
                await this.setError("Failed to initialize blockchain manager");
                await this.endOperation(operationId);
                return false;
            }
        } catch (error) {
            await this.setError(`Initialization error: ${error}`);
            await this.endOperation(operationId);
            return false;
        }
    }

    async shutdown(): Promise<boolean> {
        const operationId = await this.beginOperation();

        try {
            const currentState = await this.getState();
            if (currentState === ThreadState.SHUTDOWN) {
                await this.endOperation(operationId);
                return true;
            }

            await this.setState(ThreadState.SHUTTING_DOWN);

            // Call C++ shutdown
            const result = (globalThis as any).satox.blockchain_manager_shutdown(this.handle);
            if (result === 0) {
                await this.setState(ThreadState.SHUTDOWN);
                await this.clearError();
                await this.emitEvent("shutdown", null);
                await this.endOperation(operationId);
                return true;
            } else {
                await this.setError("Failed to shutdown blockchain manager");
                await this.endOperation(operationId);
                return false;
            }
        } catch (error) {
            await this.setError(`Shutdown error: ${error}`);
            await this.endOperation(operationId);
            return false;
        }
    }

    async getBlockHeight(): Promise<number> {
        const operationId = await this.beginOperation();

        try {
            // Ensure initialized
            if (!(await this.ensureInitialized())) {
                throw new Error(await this.getLastError());
            }

            // Call C++ function
            const height = (globalThis as any).satox.blockchain_manager_get_block_height(this.handle);
            if (height === null || height === undefined) {
                await this.setError("Failed to get block height");
                throw new Error(await this.getLastError());
            }

            const heightEvent = { height };
            await this.emitEvent("height_queried", heightEvent);
            await this.endOperation(operationId);
            return height;
        } catch (error) {
            await this.setError(`Failed to get block height: ${error}`);
            await this.endOperation(operationId);
            throw new Error(await this.getLastError());
        }
    }

    async getBlockInfo(hash: string): Promise<BlockInfo> {
        const operationId = await this.beginOperation();

        try {
            // Ensure initialized
            if (!(await this.ensureInitialized())) {
                throw new Error(await this.getLastError());
            }

            if (!hash) {
                throw new Error("Invalid block hash");
            }

            // Check cache first
            const cacheKey = `block_${hash}`;
            const cachedInfo = await this.getCache(cacheKey);
            if (cachedInfo) {
                await this.endOperation(operationId);
                return cachedInfo as BlockInfo;
            }

            // Call C++ function
            const blockJson = (globalThis as any).satox.blockchain_manager_get_block_info(this.handle, hash);
            if (!blockJson) {
                await this.setError("Failed to get block info");
                throw new Error(await this.getLastError());
            }

            const blockInfo: BlockInfo = JSON.parse(blockJson);

            // Cache the result for 5 minutes
            await this.setCache(cacheKey, blockInfo, 300000);

            const blockEvent = { hash, info: blockInfo };
            await this.emitEvent("block_info_queried", blockEvent);
            await this.endOperation(operationId);
            return blockInfo;
        } catch (error) {
            await this.setError(`Failed to get block info: ${error}`);
            await this.endOperation(operationId);
            throw new Error(await this.getLastError());
        }
    }

    async getTransactionInfo(txid: string): Promise<TransactionInfo> {
        const operationId = await this.beginOperation();

        try {
            // Ensure initialized
            if (!(await this.ensureInitialized())) {
                throw new Error(await this.getLastError());
            }

            if (!txid) {
                throw new Error("Invalid transaction ID");
            }

            // Check cache first
            const cacheKey = `tx_${txid}`;
            const cachedInfo = await this.getCache(cacheKey);
            if (cachedInfo) {
                await this.endOperation(operationId);
                return cachedInfo as TransactionInfo;
            }

            // Call C++ function
            const txJson = (globalThis as any).satox.blockchain_manager_get_transaction_info(this.handle, txid);
            if (!txJson) {
                await this.setError("Failed to get transaction info");
                throw new Error(await this.getLastError());
            }

            const txInfo: TransactionInfo = JSON.parse(txJson);

            // Cache the result for 2 minutes
            await this.setCache(cacheKey, txInfo, 120000);

            const txEvent = { txid, info: txInfo };
            await this.emitEvent("transaction_info_queried", txEvent);
            await this.endOperation(operationId);
            return txInfo;
        } catch (error) {
            await this.setError(`Failed to get transaction info: ${error}`);
            await this.endOperation(operationId);
            throw new Error(await this.getLastError());
        }
    }

    async getBestBlockHash(): Promise<string> {
        const operationId = await this.beginOperation();

        try {
            // Ensure initialized
            if (!(await this.ensureInitialized())) {
                throw new Error(await this.getLastError());
            }

            // Call C++ function
            const hash = (globalThis as any).satox.blockchain_manager_get_best_block_hash(this.handle);
            if (!hash) {
                await this.setError("Failed to get best block hash");
                throw new Error(await this.getLastError());
            }

            const hashEvent = { hash };
            await this.emitEvent("best_block_hash_queried", hashEvent);
            await this.endOperation(operationId);
            return hash;
        } catch (error) {
            await this.setError(`Failed to get best block hash: ${error}`);
            await this.endOperation(operationId);
            throw new Error(await this.getLastError());
        }
    }

    async getBlockHash(height: number): Promise<string> {
        const operationId = await this.beginOperation();

        try {
            // Ensure initialized
            if (!(await this.ensureInitialized())) {
                throw new Error(await this.getLastError());
            }

            // Call C++ function
            const hash = (globalThis as any).satox.blockchain_manager_get_block_hash(this.handle, height);
            if (!hash) {
                await this.setError("Failed to get block hash");
                throw new Error(await this.getLastError());
            }

            const hashEvent = { height, hash };
            await this.emitEvent("block_hash_queried", hashEvent);
            await this.endOperation(operationId);
            return hash;
        } catch (error) {
            await this.setError(`Failed to get block hash: ${error}`);
            await this.endOperation(operationId);
            throw new Error(await this.getLastError());
        }
    }

    async getStatistics(): Promise<any> {
        const operationId = await this.beginOperation();

        try {
            // Ensure initialized
            if (!(await this.ensureInitialized())) {
                throw new Error(await this.getLastError());
            }

            // Call C++ function
            const statsJson = (globalThis as any).satox.blockchain_manager_get_statistics(this.handle);
            if (!statsJson) {
                await this.setError("Failed to get blockchain statistics");
                throw new Error(await this.getLastError());
            }

            const stats = JSON.parse(statsJson);
            await this.emitEvent("statistics_queried", stats);
            await this.endOperation(operationId);
            return stats;
        } catch (error) {
            await this.setError(`Failed to get statistics: ${error}`);
            await this.endOperation(operationId);
            throw new Error(await this.getLastError());
        }
    }

    async validateAddress(address: string): Promise<boolean> {
        const operationId = await this.beginOperation();

        try {
            // Ensure initialized
            if (!(await this.ensureInitialized())) {
                throw new Error(await this.getLastError());
            }

            if (!address) {
                await this.endOperation(operationId);
                return false;
            }

            // Call C++ function
            const result = (globalThis as any).satox.blockchain_manager_validate_address(this.handle, address);
            const isValid = result === 1;

            const validationEvent = { address, valid: isValid };
            await this.emitEvent("address_validated", validationEvent);
            await this.endOperation(operationId);
            return isValid;
        } catch (error) {
            await this.setError(`Failed to validate address: ${error}`);
            await this.endOperation(operationId);
            throw new Error(await this.getLastError());
        }
    }

    async clearCache(): Promise<void> {
        const operationId = await this.beginOperation();
        await super.clearCache();
        await this.emitEvent("cache_cleared", null);
        await this.endOperation(operationId);
    }

    async getThreadSafetyStats(): Promise<ThreadSafeStats> {
        return await this.getStats();
    }

    // Worker-based parallel operations
    async getMultipleBlockInfos(hashes: string[]): Promise<BlockInfo[]> {
        const operationId = await this.beginOperation();

        try {
            // Ensure initialized
            if (!(await this.ensureInitialized())) {
                throw new Error(await this.getLastError());
            }

            // Create workers for parallel processing
            const workerPromises = hashes.map(async (hash, index) => {
                const worker = await this.createWorker(`
                    importScripts('satox_worker.js');
                    
                    self.onmessage = function(e) {
                        const { hash, index } = e.data;
                        const blockInfo = satox.getBlockInfo(hash);
                        self.postMessage({ blockInfo, index });
                    };
                `);

                return new Promise<{ blockInfo: BlockInfo; index: number }>((resolve) => {
                    worker.onmessage = (e) => {
                        resolve(e.data);
                        this.terminateWorker(worker);
                    };
                    worker.postMessage({ hash, index });
                });
            });

            const results = await Promise.all(workerPromises);
            const blockInfos = new Array<BlockInfo>(hashes.length);
            
            for (const result of results) {
                blockInfos[result.index] = result.blockInfo;
            }

            await this.emitEvent("multiple_block_infos_queried", { count: hashes.length });
            await this.endOperation(operationId);
            return blockInfos;
        } catch (error) {
            await this.setError(`Failed to get multiple block infos: ${error}`);
            await this.endOperation(operationId);
            throw new Error(await this.getLastError());
        }
    }

    async getMultipleTransactionInfos(txids: string[]): Promise<TransactionInfo[]> {
        const operationId = await this.beginOperation();

        try {
            // Ensure initialized
            if (!(await this.ensureInitialized())) {
                throw new Error(await this.getLastError());
            }

            // Create workers for parallel processing
            const workerPromises = txids.map(async (txid, index) => {
                const worker = await this.createWorker(`
                    importScripts('satox_worker.js');
                    
                    self.onmessage = function(e) {
                        const { txid, index } = e.data;
                        const txInfo = satox.getTransactionInfo(txid);
                        self.postMessage({ txInfo, index });
                    };
                `);

                return new Promise<{ txInfo: TransactionInfo; index: number }>((resolve) => {
                    worker.onmessage = (e) => {
                        resolve(e.data);
                        this.terminateWorker(worker);
                    };
                    worker.postMessage({ txid, index });
                });
            });

            const results = await Promise.all(workerPromises);
            const txInfos = new Array<TransactionInfo>(txids.length);
            
            for (const result of results) {
                txInfos[result.index] = result.txInfo;
            }

            await this.emitEvent("multiple_transaction_infos_queried", { count: txids.length });
            await this.endOperation(operationId);
            return txInfos;
        } catch (error) {
            await this.setError(`Failed to get multiple transaction infos: ${error}`);
            await this.endOperation(operationId);
            throw new Error(await this.getLastError());
        }
    }

    protected async disposeImpl(): Promise<void> {
        try {
            if (await this.getState() === ThreadState.INITIALIZED) {
                await this.shutdown();
            }
        } catch (error) {
            await this.setError(`Disposal error: ${error}`);
        }
    }
}

// Export the module
export default BlockchainManager; 