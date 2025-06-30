import { satox_native } from './native/satox_native';
import { ThreadSafeManager } from './thread-safe';

export interface IPFSConfig {
  gateway?: string;
  apiEndpoint?: string;
  timeout?: number;
  maxFileSize?: number;
}

export interface IPFSFile {
  cid: string;
  name: string;
  data: string;
  size: number;
  type: string;
  addedAt: number;
}

export interface IPFSDirectory {
  cid: string;
  name: string;
  files: IPFSFile[];
  addedAt: number;
}

export interface IPFSStats {
  totalFiles: number;
  totalDirectories: number;
  totalPinnedFiles: number;
  totalSize: number;
  averageFileSize: number;
  mostAccessedFile: string;
}

export class IPFSManager extends ThreadSafeManager {
  private manager: any;
  private stats: IPFSStats = {
    totalFiles: 0,
    totalDirectories: 0,
    totalPinnedFiles: 0,
    totalSize: 0,
    averageFileSize: 0,
    mostAccessedFile: ''
  };

  constructor() {
    super();
    this.manager = new satox_native.IPFSManager();
  }

  /**
   * Initialize the IPFS manager
   */
  async initialize(config: IPFSConfig): Promise<boolean> {
    return this.withLock(async () => {
      try {
        this.ensureNotDisposed();
        const result = this.manager.initialize(config);
        this.setInitialized(result);
        this.setError(null);
        return result;
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        console.error('Failed to initialize IPFS Manager:', error);
        return false;
      }
    });
  }

  /**
   * Shutdown the IPFS manager
   */
  async shutdown(): Promise<void> {
    return this.withLock(async () => {
      try {
        this.ensureNotDisposed();
        this.manager.shutdown();
        this.setInitialized(false);
        this.setError(null);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        console.error('Failed to shutdown IPFS Manager:', error);
        throw error;
      }
    });
  }

  /**
   * Add a file to IPFS
   */
  async addFile(fileData: string, fileName: string = ''): Promise<string> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.addFile(fileData, fileName);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return '';
      }
    });
  }

  /**
   * Get a file from IPFS by CID
   */
  async getFile(cid: string): Promise<IPFSFile | null> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.getFile(cid);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return null;
      }
    });
  }

  /**
   * Remove a file from IPFS
   */
  async removeFile(cid: string): Promise<boolean> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.removeFile(cid);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return false;
      }
    });
  }

  /**
   * List files with pagination
   */
  async listFiles(limit: number = 100, offset: number = 0): Promise<IPFSFile[]> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.listFiles(limit, offset);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return [];
      }
    });
  }

  /**
   * Search files by query
   */
  async searchFiles(query: string, limit: number = 100): Promise<IPFSFile[]> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.searchFiles(query, limit);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return [];
      }
    });
  }

  /**
   * Pin a file to keep it available
   */
  async pinFile(cid: string): Promise<boolean> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.pinFile(cid);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return false;
      }
    });
  }

  /**
   * Unpin a file
   */
  async unpinFile(cid: string): Promise<boolean> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.unpinFile(cid);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return false;
      }
    });
  }

  /**
   * Get all pinned files
   */
  async getPinnedFiles(): Promise<IPFSFile[]> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.getPinnedFiles();
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return [];
      }
    });
  }

  /**
   * Add a directory to IPFS
   */
  async addDirectory(directory: Omit<IPFSDirectory, 'cid' | 'addedAt'>): Promise<string> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.addDirectory(directory);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return '';
      }
    });
  }

  /**
   * Get a directory from IPFS by CID
   */
  async getDirectory(cid: string): Promise<IPFSDirectory | null> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.getDirectory(cid);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return null;
      }
    });
  }

  /**
   * Validate a CID
   */
  async validateCID(cid: string): Promise<boolean> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.validateCID(cid);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return false;
      }
    });
  }

  /**
   * Get IPFS statistics
   */
  async getIPFSStats(): Promise<IPFSStats> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return { ...this.stats };
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return { ...this.stats };
      }
    });
  }

  /**
   * Check if the manager is initialized
   */
  isInitialized(): boolean {
    return this.isInitialized();
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