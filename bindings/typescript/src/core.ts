// Removed unused import
// import { satox_native } from './native/satox_native';

import { ThreadSafeManager } from './thread-safe';

export interface CoreConfig {
  logLevel?: string;
  dataDir?: string;
  enableDebug?: boolean;
  maxConnections?: number;
}

export interface CoreInfo {
  version: string;
  buildDate: string;
  platform: string;
  architecture: string;
  uptime: number;
  memoryUsage: number;
}

export interface CoreStats {
  totalRequests: number;
  successfulRequests: number;
  failedRequests: number;
  averageResponseTime: number;
  activeConnections: number;
}

export enum CoreState {
  UNINITIALIZED = 0,
  INITIALIZING = 1,
  INITIALIZED = 2,
  RUNNING = 3,
  SHUTTING_DOWN = 4,
  SHUTDOWN = 5,
  ERROR = 6
}

export class CoreManager extends ThreadSafeManager {
  private logLevel: string = 'info';
  private dataDir: string = './data';
  private debugEnabled: boolean = false;
  private maxConnections: number = 100;
  private stats: CoreStats = {
    totalRequests: 0,
    successfulRequests: 0,
    failedRequests: 0,
    averageResponseTime: 0,
    activeConnections: 0
  };

  constructor() {
    super();
    // Remove reference to non-existent CoreManager
    // this.manager = new satox_native.CoreManager();
  }

  /**
   * Initialize the core manager
   */
  async initialize(config: CoreConfig): Promise<boolean> {
    return this.withLock(async () => {
      try {
        this.ensureNotDisposed();
        
        if (config.logLevel) this.logLevel = config.logLevel;
        if (config.dataDir) this.dataDir = config.dataDir;
        if (config.enableDebug !== undefined) this.debugEnabled = config.enableDebug;
        if (config.maxConnections) this.maxConnections = config.maxConnections;
        
        this.setInitialized(true);
        this.setError(null);
        return true;
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        console.error('Failed to initialize Core Manager:', error);
        return false;
      }
    });
  }

  /**
   * Shutdown the core manager
   */
  async shutdown(): Promise<void> {
    return this.withLock(async () => {
      try {
        this.ensureNotDisposed();
        this.setInitialized(false);
        this.setError(null);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        console.error('Failed to shutdown Core Manager:', error);
        throw error;
      }
    });
  }

  /**
   * Get the current core state
   */
  getCoreState(): CoreState {
    const state = super.getState();
    if (state.disposed) return CoreState.SHUTDOWN;
    if (!state.initialized) return CoreState.UNINITIALIZED;
    return CoreState.INITIALIZED;
  }

  /**
   * Get core information
   */
  getInfo(): CoreInfo {
    return {
      version: '1.0.0',
      buildDate: new Date().toISOString(),
      platform: process.platform,
      architecture: process.arch,
      uptime: process.uptime(),
      memoryUsage: process.memoryUsage().heapUsed
    };
  }

  /**
   * Get core statistics
   */
  getStats(): CoreStats {
    return { ...this.stats };
  }

  /**
   * Reset statistics
   */
  async resetStats(): Promise<void> {
    return this.withLock(async () => {
      this.ensureInitialized();
      this.stats = {
        totalRequests: 0,
        successfulRequests: 0,
        failedRequests: 0,
        averageResponseTime: 0,
        activeConnections: 0
      };
    });
  }

  /**
   * Enable or disable statistics collection
   */
  async enableStats(_enable: boolean): Promise<boolean> {
    return this.withLock(async () => {
      this.ensureInitialized();
      // Placeholder implementation
      return true;
    });
  }

  /**
   * Set log level
   */
  async setLogLevel(level: string): Promise<boolean> {
    return this.withLock(async () => {
      this.ensureInitialized();
      this.logLevel = level;
      return true;
    });
  }

  /**
   * Get current log level
   */
  getLogLevel(): string {
    return this.logLevel;
  }

  /**
   * Set data directory
   */
  async setDataDir(path: string): Promise<boolean> {
    return this.withLock(async () => {
      this.ensureInitialized();
      this.dataDir = path;
      return true;
    });
  }

  /**
   * Get data directory
   */
  getDataDir(): string {
    return this.dataDir;
  }

  /**
   * Enable or disable debug mode
   */
  async enableDebug(enable: boolean): Promise<boolean> {
    return this.withLock(async () => {
      this.ensureInitialized();
      this.debugEnabled = enable;
      return true;
    });
  }

  /**
   * Check if debug mode is enabled
   */
  isDebugEnabled(): boolean {
    return this.debugEnabled;
  }

  /**
   * Set maximum connections
   */
  async setMaxConnections(max: number): Promise<boolean> {
    return this.withLock(async () => {
      this.ensureInitialized();
      this.maxConnections = max;
      return true;
    });
  }

  /**
   * Get maximum connections
   */
  getMaxConnections(): number {
    return this.maxConnections;
  }

  /**
   * Get the last error message
   */
  override getLastError(): string {
    return super.getLastError() || '';
  }

  /**
   * Clear the last error message
   */
  override async clearLastError(): Promise<void> {
    return this.withLock(async () => {
      super.clearLastError();
    });
  }
} 