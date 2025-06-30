/**
 * Thread-safe base class for TypeScript/JavaScript managers
 * Provides synchronization primitives for concurrent access
 */

export interface ThreadSafeState {
  initialized: boolean;
  disposed: boolean;
  error: string | null;
}

export abstract class ThreadSafeManager {
  private state: ThreadSafeState;
  private lock: Promise<void> | null = null;
  private lockPromise: Promise<void> | null = null;
  private lockResolve: (() => void) | null = null;

  constructor() {
    this.state = {
      initialized: false,
      disposed: false,
      error: null
    };
  }

  /**
   * Get current state (thread-safe)
   */
  protected getState(): ThreadSafeState {
    return { ...this.state };
  }

  /**
   * Set state (thread-safe)
   */
  protected setState(updates: Partial<ThreadSafeState>): void {
    this.state = { ...this.state, ...updates };
  }

  /**
   * Acquire a lock for exclusive access
   */
  protected async acquireLock(): Promise<void> {
    if (this.lock) {
      await this.lock;
    }
    
    this.lockPromise = new Promise<void>((resolve) => {
      this.lockResolve = resolve;
    });
    this.lock = this.lockPromise;
  }

  /**
   * Release the current lock
   */
  protected releaseLock(): void {
    if (this.lockResolve) {
      this.lockResolve();
      this.lockResolve = null;
      this.lock = null;
    }
  }

  /**
   * Execute a function with exclusive lock
   */
  protected async withLock<T>(fn: () => T | Promise<T>): Promise<T> {
    await this.acquireLock();
    try {
      return await fn();
    } finally {
      this.releaseLock();
    }
  }

  /**
   * Execute a function with read lock (allows concurrent reads)
   */
  protected async withReadLock<T>(fn: () => T | Promise<T>): Promise<T> {
    // For now, we use the same lock as write operations
    // In a more sophisticated implementation, we could use a read-write lock
    return this.withLock(fn);
  }

  /**
   * Ensure the manager is initialized
   */
  protected ensureInitialized(): void {
    if (this.state.disposed) {
      throw new Error(`${this.constructor.name} is disposed`);
    }
    if (!this.state.initialized) {
      throw new Error(`${this.constructor.name} is not initialized`);
    }
  }

  /**
   * Ensure the manager is not disposed
   */
  protected ensureNotDisposed(): void {
    if (this.state.disposed) {
      throw new Error(`${this.constructor.name} is disposed`);
    }
  }

  /**
   * Set initialization state
   */
  protected setInitialized(initialized: boolean): void {
    this.setState({ initialized });
  }

  /**
   * Set disposed state
   */
  protected setDisposed(disposed: boolean): void {
    this.setState({ disposed });
  }

  /**
   * Set error state
   */
  protected setError(error: string | null): void {
    this.setState({ error });
  }

  /**
   * Get last error
   */
  public getLastError(): string | null {
    return this.state.error;
  }

  /**
   * Clear last error
   */
  public clearLastError(): void {
    this.setError(null);
  }

  /**
   * Check if initialized
   */
  public isInitialized(): boolean {
    return this.state.initialized;
  }

  /**
   * Check if disposed
   */
  public isDisposed(): boolean {
    return this.state.disposed;
  }

  /**
   * Initialize the manager (to be implemented by subclasses)
   */
  public abstract initialize(config?: any): Promise<boolean> | boolean;

  /**
   * Shutdown the manager (to be implemented by subclasses)
   */
  public abstract shutdown(): Promise<void> | void;

  /**
   * Dispose the manager
   */
  public async dispose(): Promise<void> {
    await this.withLock(async () => {
      if (!this.state.disposed) {
        await this.shutdown();
        this.setDisposed(true);
      }
    });
  }
}

/**
 * Thread-safe singleton pattern for managers
 */
export class ThreadSafeSingleton<T extends ThreadSafeManager> {
  private instance: T | null = null;
  private lock: Promise<void> | null = null;
  private lockPromise: Promise<void> | null = null;
  private lockResolve: (() => void) | null = null;

  constructor(private factory: () => T) {}

  /**
   * Get or create the singleton instance
   */
  public async getInstance(): Promise<T> {
    if (this.lock) {
      await this.lock;
    }

    if (!this.instance) {
      this.lockPromise = new Promise<void>((resolve) => {
        this.lockResolve = resolve;
      });
      this.lock = this.lockPromise;

      try {
        this.instance = this.factory();
      } finally {
        if (this.lockResolve) {
          this.lockResolve();
          this.lockResolve = null;
          this.lock = null;
        }
      }
    }

    return this.instance;
  }

  /**
   * Reset the singleton instance (for testing)
   */
  public reset(): void {
    this.instance = null;
  }
}

/**
 * Thread-safe event emitter for manager events
 */
export class ThreadSafeEventEmitter {
  private listeners: Map<string, Set<Function>> = new Map();
  private lock: Promise<void> | null = null;
  private lockPromise: Promise<void> | null = null;
  private lockResolve: (() => void) | null = null;

  /**
   * Add event listener
   */
  public async addListener(event: string, listener: Function): Promise<void> {
    await this.withLock(() => {
      if (!this.listeners.has(event)) {
        this.listeners.set(event, new Set());
      }
      this.listeners.get(event)!.add(listener);
    });
  }

  /**
   * Remove event listener
   */
  public async removeListener(event: string, listener: Function): Promise<void> {
    await this.withLock(() => {
      const eventListeners = this.listeners.get(event);
      if (eventListeners) {
        eventListeners.delete(listener);
        if (eventListeners.size === 0) {
          this.listeners.delete(event);
        }
      }
    });
  }

  /**
   * Emit event to all listeners
   */
  public async emit(event: string, ...args: any[]): Promise<void> {
    await this.withReadLock(async () => {
      const eventListeners = this.listeners.get(event);
      if (eventListeners) {
        const promises = Array.from(eventListeners).map(listener => 
          Promise.resolve().then(() => listener(...args))
        );
        await Promise.all(promises);
      }
    });
  }

  /**
   * Remove all listeners for an event
   */
  public async removeAllListeners(event?: string): Promise<void> {
    await this.withLock(() => {
      if (event) {
        this.listeners.delete(event);
      } else {
        this.listeners.clear();
      }
    });
  }

  private async withLock<T>(fn: () => T | Promise<T>): Promise<T> {
    if (this.lock) {
      await this.lock;
    }
    
    this.lockPromise = new Promise<void>((resolve) => {
      this.lockResolve = resolve;
    });
    this.lock = this.lockPromise;
    
    try {
      return await fn();
    } finally {
      if (this.lockResolve) {
        this.lockResolve();
        this.lockResolve = null;
        this.lock = null;
      }
    }
  }

  private async withReadLock<T>(fn: () => T | Promise<T>): Promise<T> {
    // For now, use the same lock as write operations
    return this.withLock(fn);
  }
} 