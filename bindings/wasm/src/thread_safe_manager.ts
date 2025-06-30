/**
 * Thread-Safe Manager for WASM Bindings
 * Provides comprehensive thread safety using Web Workers and SharedArrayBuffer
 */

// Thread state enumeration
export enum ThreadState {
    UNINITIALIZED = "uninitialized",
    INITIALIZING = "initializing",
    INITIALIZED = "initialized",
    SHUTTING_DOWN = "shutting_down",
    SHUTDOWN = "shutdown",
    ERROR = "error"
}

// Thread safety statistics
export interface ThreadSafeStats {
    totalOperations: number;
    concurrentOperations: number;
    maxConcurrentOperations: number;
    averageOperationTime: number;
    totalOperationTime: number;
    lockContentionCount: number;
    lastOperationTime: number;
}

export class ThreadSafeStatsImpl implements ThreadSafeStats {
    totalOperations = 0;
    concurrentOperations = 0;
    maxConcurrentOperations = 0;
    averageOperationTime = 0.0;
    totalOperationTime = 0.0;
    lockContentionCount = 0;
    lastOperationTime = 0.0;
}

// Event handler type
export type EventHandler = (data: any) => void;

// Lock implementation using SharedArrayBuffer
class SharedLock {
    private buffer: SharedArrayBuffer;
    private lockArray: Int32Array;
    private lockId: number;

    constructor() {
        this.buffer = new SharedArrayBuffer(4);
        this.lockArray = new Int32Array(this.buffer);
        this.lockId = Math.floor(Math.random() * 1000000);
    }

    async acquire(): Promise<void> {
        while (true) {
            const oldValue = Atomics.compareExchange(this.lockArray, 0, 0, this.lockId);
            if (oldValue === 0) {
                return;
            }
            await new Promise(resolve => setTimeout(resolve, 1));
        }
    }

    release(): void {
        Atomics.compareExchange(this.lockArray, 0, this.lockId, 0);
    }

    getBuffer(): SharedArrayBuffer {
        return this.buffer;
    }
}

// Thread-safe manager base class for WASM bindings
export class ThreadSafeManager {
    private name: string;
    private state: ThreadState = ThreadState.UNINITIALIZED;
    private initialized = false;
    private disposed = false;
    private lastError = "";
    private errorCount = 0;

    // Thread safety primitives
    private stateLock = new SharedLock();
    private errorLock = new SharedLock();
    private statsLock = new SharedLock();
    private operationLock = new SharedLock();
    private eventLock = new SharedLock();
    private resourceLock = new SharedLock();
    private configLock = new SharedLock();
    private cacheLock = new SharedLock();

    // Statistics
    private stats = new ThreadSafeStatsImpl();
    private operationStartTimes = new Map<number, number>();
    private activeOperations = 0;
    private operationCounter = 0;

    // Event handling
    private eventHandlers = new Map<string, EventHandler[]>();
    private eventStats = new Map<string, number>();

    // Resource management
    private resources = new Map<string, any>();

    // Configuration
    private config = new Map<string, any>();

    // Cache
    private cache = new Map<string, any>();
    private cacheTimestamps = new Map<string, number>();

    // Performance monitoring
    private performanceEnabled = true;
    private startTime = performance.now();

    // Worker management
    private workers: Worker[] = [];
    private workerLock = new SharedLock();

    constructor(name = "ThreadSafeManager") {
        this.name = name;
    }

    async ensureInitialized(): Promise<boolean> {
        await this.stateLock.acquire();
        try {
            if (this.state === ThreadState.UNINITIALIZED) {
                this.setError("Manager not initialized");
                return false;
            }
            if (this.state === ThreadState.SHUTDOWN) {
                this.setError("Manager has been disposed");
                return false;
            }
            if (this.state === ThreadState.ERROR) {
                this.setError("Manager is in error state");
                return false;
            }
            return true;
        } finally {
            this.stateLock.release();
        }
    }

    async ensureNotDisposed(): Promise<boolean> {
        await this.stateLock.acquire();
        try {
            if (this.state === ThreadState.SHUTDOWN) {
                this.setError("Manager has been disposed");
                return false;
            }
            return true;
        } finally {
            this.stateLock.release();
        }
    }

    async setState(state: ThreadState): Promise<void> {
        await this.stateLock.acquire();
        try {
            this.state = state;
        } finally {
            this.stateLock.release();
        }
    }

    async getState(): Promise<ThreadState> {
        await this.stateLock.acquire();
        try {
            return this.state;
        } finally {
            this.stateLock.release();
        }
    }

    async waitForState(targetState: ThreadState, timeout = 5000): Promise<boolean> {
        const startTime = performance.now();
        while (performance.now() - startTime < timeout) {
            if (await this.getState() === targetState) {
                return true;
            }
            await new Promise(resolve => setTimeout(resolve, 10));
        }
        return false;
    }

    async beginOperation(): Promise<number> {
        if (!this.performanceEnabled) {
            return 0;
        }

        await this.statsLock.acquire();
        try {
            const operationId = this.operationCounter++;
            this.activeOperations++;
            this.operationStartTimes.set(operationId, performance.now());
            this.stats.concurrentOperations = this.activeOperations;
            this.stats.maxConcurrentOperations = Math.max(
                this.stats.maxConcurrentOperations,
                this.activeOperations
            );
            return operationId;
        } finally {
            this.statsLock.release();
        }
    }

    async endOperation(operationId: number): Promise<void> {
        if (!this.performanceEnabled || operationId === 0) {
            return;
        }

        await this.statsLock.acquire();
        try {
            const startTime = this.operationStartTimes.get(operationId);
            if (startTime) {
                const operationTime = performance.now() - startTime;
                this.operationStartTimes.delete(operationId);

                this.activeOperations--;
                this.stats.totalOperations++;
                this.stats.totalOperationTime += operationTime;
                this.stats.averageOperationTime = this.stats.totalOperationTime / this.stats.totalOperations;
                this.stats.lastOperationTime = performance.now() - this.startTime;
                this.stats.concurrentOperations = this.activeOperations;
            }
        } finally {
            this.statsLock.release();
        }
    }

    async setError(error: string): Promise<void> {
        await this.errorLock.acquire();
        try {
            this.lastError = error;
            this.errorCount++;
            await this.setState(ThreadState.ERROR);
        } finally {
            this.errorLock.release();
        }
    }

    async clearError(): Promise<void> {
        await this.errorLock.acquire();
        try {
            this.lastError = "";
            if (this.state === ThreadState.ERROR) {
                await this.setState(ThreadState.INITIALIZED);
            }
        } finally {
            this.errorLock.release();
        }
    }

    async getLastError(): Promise<string> {
        await this.errorLock.acquire();
        try {
            return this.lastError;
        } finally {
            this.errorLock.release();
        }
    }

    async getErrorCount(): Promise<number> {
        await this.errorLock.acquire();
        try {
            return this.errorCount;
        } finally {
            this.errorLock.release();
        }
    }

    async isInitialized(): Promise<boolean> {
        return await this.getState() === ThreadState.INITIALIZED;
    }

    async isDisposed(): Promise<boolean> {
        return await this.getState() === ThreadState.SHUTDOWN;
    }

    async getStats(): Promise<ThreadSafeStats> {
        await this.statsLock.acquire();
        try {
            return {
                totalOperations: this.stats.totalOperations,
                concurrentOperations: this.stats.concurrentOperations,
                maxConcurrentOperations: this.stats.maxConcurrentOperations,
                averageOperationTime: this.stats.averageOperationTime,
                totalOperationTime: this.stats.totalOperationTime,
                lockContentionCount: this.stats.lockContentionCount,
                lastOperationTime: this.stats.lastOperationTime
            };
        } finally {
            this.statsLock.release();
        }
    }

    async resetStats(): Promise<void> {
        await this.statsLock.acquire();
        try {
            this.stats = new ThreadSafeStatsImpl();
            this.operationStartTimes.clear();
            this.activeOperations = 0;
            this.operationCounter = 0;
        } finally {
            this.statsLock.release();
        }
    }

    enablePerformanceMonitoring(): void {
        this.performanceEnabled = true;
    }

    disablePerformanceMonitoring(): void {
        this.performanceEnabled = false;
    }

    getUptime(): number {
        return performance.now() - this.startTime;
    }

    async setConfig(key: string, value: any): Promise<void> {
        await this.configLock.acquire();
        try {
            this.config.set(key, value);
        } finally {
            this.configLock.release();
        }
    }

    async getConfig(key: string, defaultValue?: any): Promise<any> {
        await this.configLock.acquire();
        try {
            return this.config.get(key) ?? defaultValue;
        } finally {
            this.configLock.release();
        }
    }

    async setCache(key: string, value: any, ttl = 300000): Promise<void> {
        await this.cacheLock.acquire();
        try {
            this.cache.set(key, value);
            this.cacheTimestamps.set(key, performance.now() + ttl);
        } finally {
            this.cacheLock.release();
        }
    }

    async getCache(key: string, defaultValue?: any): Promise<any> {
        await this.cacheLock.acquire();
        try {
            if (this.cache.has(key)) {
                const timestamp = this.cacheTimestamps.get(key);
                if (timestamp && performance.now() < timestamp) {
                    return this.cache.get(key);
                } else {
                    // Expired, remove from cache
                    this.cache.delete(key);
                    this.cacheTimestamps.delete(key);
                }
            }
            return defaultValue;
        } finally {
            this.cacheLock.release();
        }
    }

    async clearCache(): Promise<void> {
        await this.cacheLock.acquire();
        try {
            this.cache.clear();
            this.cacheTimestamps.clear();
        } finally {
            this.cacheLock.release();
        }
    }

    async addEventHandler(event: string, handler: EventHandler): Promise<void> {
        await this.eventLock.acquire();
        try {
            if (!this.eventHandlers.has(event)) {
                this.eventHandlers.set(event, []);
            }
            this.eventHandlers.get(event)!.push(handler);
        } finally {
            this.eventLock.release();
        }
    }

    async removeEventHandler(event: string, handlerIndex: number): Promise<void> {
        await this.eventLock.acquire();
        try {
            const handlers = this.eventHandlers.get(event);
            if (handlers && handlers[handlerIndex]) {
                handlers.splice(handlerIndex, 1);
            }
        } finally {
            this.eventLock.release();
        }
    }

    async emitEvent(event: string, data: any): Promise<void> {
        let handlers: EventHandler[] = [];
        await this.eventLock.acquire();
        try {
            if (this.eventHandlers.has(event)) {
                handlers = [...this.eventHandlers.get(event)!];
                this.eventStats.set(event, (this.eventStats.get(event) || 0) + 1);
            }
        } finally {
            this.eventLock.release();
        }

        // Call handlers outside of lock to prevent deadlocks
        for (const handler of handlers) {
            try {
                handler(data);
            } catch (error) {
                await this.setError(`Error in event handler: ${error}`);
            }
        }
    }

    async addResource(name: string, resource: any): Promise<void> {
        await this.resourceLock.acquire();
        try {
            this.resources.set(name, resource);
        } finally {
            this.resourceLock.release();
        }
    }

    async getResource<T>(name: string): Promise<T | undefined> {
        await this.resourceLock.acquire();
        try {
            return this.resources.get(name);
        } finally {
            this.resourceLock.release();
        }
    }

    async removeResource(name: string): Promise<void> {
        await this.resourceLock.acquire();
        try {
            this.resources.delete(name);
        } finally {
            this.resourceLock.release();
        }
    }

    async clearResources(): Promise<void> {
        await this.resourceLock.acquire();
        try {
            this.resources.clear();
        } finally {
            this.resourceLock.release();
        }
    }

    // Worker management for true parallelism
    async createWorker(script: string): Promise<Worker> {
        await this.workerLock.acquire();
        try {
            const worker = new Worker(script, { type: 'module' });
            this.workers.push(worker);
            return worker;
        } finally {
            this.workerLock.release();
        }
    }

    async terminateWorker(worker: Worker): Promise<void> {
        await this.workerLock.acquire();
        try {
            worker.terminate();
            const index = this.workers.indexOf(worker);
            if (index > -1) {
                this.workers.splice(index, 1);
            }
        } finally {
            this.workerLock.release();
        }
    }

    async terminateAllWorkers(): Promise<void> {
        await this.workerLock.acquire();
        try {
            for (const worker of this.workers) {
                worker.terminate();
            }
            this.workers = [];
        } finally {
            this.workerLock.release();
        }
    }

    async dispose(): Promise<void> {
        const currentState = await this.getState();
        if (currentState === ThreadState.SHUTDOWN) {
            return;
        }

        await this.setState(ThreadState.SHUTTING_DOWN);

        // Clear cache and resources
        await this.clearCache();
        await this.clearResources();
        await this.terminateAllWorkers();

        // Call implementation-specific disposal
        await this.disposeImpl();

        await this.setState(ThreadState.SHUTDOWN);
    }

    protected async disposeImpl(): Promise<void> {
        // Override in subclasses
    }
}

// Thread-safe singleton pattern
export class ThreadSafeSingleton<T> {
    private static instances = new Map<string, any>();
    private static instanceLocks = new Map<string, SharedLock>();
    private static singletonLock = new SharedLock();

    static async getOrCreate<T>(
        name: string,
        creator: () => Promise<T>
    ): Promise<T> {
        await ThreadSafeSingleton.singletonLock.acquire();
        try {
            if (!ThreadSafeSingleton.instances.has(name)) {
                const instance = await creator();
                ThreadSafeSingleton.instances.set(name, instance);
                ThreadSafeSingleton.instanceLocks.set(name, new SharedLock());
            }
            return ThreadSafeSingleton.instances.get(name);
        } finally {
            ThreadSafeSingleton.singletonLock.release();
        }
    }

    static async get<T>(name: string): Promise<T | undefined> {
        await ThreadSafeSingleton.singletonLock.acquire();
        try {
            return ThreadSafeSingleton.instances.get(name);
        } finally {
            ThreadSafeSingleton.singletonLock.release();
        }
    }

    static async resetInstance(name: string): Promise<void> {
        await ThreadSafeSingleton.singletonLock.acquire();
        try {
            ThreadSafeSingleton.instances.delete(name);
            ThreadSafeSingleton.instanceLocks.delete(name);
        } finally {
            ThreadSafeSingleton.singletonLock.release();
        }
    }
}

// Thread-safe event emitter
export class ThreadSafeEventEmitter {
    private lock = new SharedLock();
    private listeners = new Map<string, EventHandler[]>();
    private eventStats = new Map<string, number>();

    async on(event: string, callback: EventHandler): Promise<void> {
        await this.lock.acquire();
        try {
            if (!this.listeners.has(event)) {
                this.listeners.set(event, []);
            }
            this.listeners.get(event)!.push(callback);
        } finally {
            this.lock.release();
        }
    }

    async off(event: string, callback: EventHandler): Promise<void> {
        await this.lock.acquire();
        try {
            const handlers = this.listeners.get(event);
            if (handlers) {
                const index = handlers.indexOf(callback);
                if (index > -1) {
                    handlers.splice(index, 1);
                }
            }
        } finally {
            this.lock.release();
        }
    }

    async emit(event: string, data: any): Promise<void> {
        let handlers: EventHandler[] = [];
        await this.lock.acquire();
        try {
            if (this.listeners.has(event)) {
                handlers = [...this.listeners.get(event)!];
                this.eventStats.set(event, (this.eventStats.get(event) || 0) + 1);
            }
        } finally {
            this.lock.release();
        }

        // Call handlers outside of lock to prevent deadlocks
        for (const handler of handlers) {
            try {
                handler(data);
            } catch (error) {
                console.error("Error in event listener:", error);
            }
        }
    }

    async clearListeners(event?: string): Promise<void> {
        await this.lock.acquire();
        try {
            if (event) {
                this.listeners.delete(event);
                this.eventStats.delete(event);
            } else {
                this.listeners.clear();
                this.eventStats.clear();
            }
        } finally {
            this.lock.release();
        }
    }

    async getEventStats(): Promise<Map<string, number>> {
        await this.lock.acquire();
        try {
            return new Map(this.eventStats);
        } finally {
            this.lock.release();
        }
    }

    async getListenerCount(event: string): Promise<number> {
        await this.lock.acquire();
        try {
            return this.listeners.get(event)?.length || 0;
        } finally {
            this.lock.release();
        }
    }
}

// Utility for creating thread-safe promises
export class ThreadSafePromise<T> {
    private promise: Promise<T>;
    private resolve!: (value: T) => void;
    private reject!: (reason: any) => void;
    private lock = new SharedLock();

    constructor() {
        this.promise = new Promise<T>((resolve, reject) => {
            this.resolve = resolve;
            this.reject = reject;
        });
    }

    async resolve(value: T): Promise<void> {
        await this.lock.acquire();
        try {
            this.resolve(value);
        } finally {
            this.lock.release();
        }
    }

    async reject(reason: any): Promise<void> {
        await this.lock.acquire();
        try {
            this.reject(reason);
        } finally {
            this.lock.release();
        }
    }

    getPromise(): Promise<T> {
        return this.promise;
    }
} 