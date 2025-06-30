import { satox_native } from './native/satox_native';
import { ThreadSafeManager } from './thread-safe';

export interface BlockchainConfig {
  host?: string;
  port?: number;
  enableSSL?: boolean;
}

export interface Block {
  hash: string;
  previousHash: string;
  height: number;
  timestamp: number;
  merkleRoot: string;
  version: number;
  bits: number;
  nonce: number;
}

export interface Transaction {
  hash: string;
  version: number;
  lockTime: number;
}

export interface BlockchainInfo {
  name: string;
  version: string;
  type: number;
  currentHeight: number;
  bestBlockHash: string;
  difficulty: number;
  networkHashRate: number;
  connections: number;
  isInitialBlockDownload: boolean;
}

export interface BlockchainStats {
  totalBlocks: number;
  totalTransactions: number;
  connectedNodes: number;
  averageBlockTime: number;
  averageTransactionTime: number;
}

export enum BlockchainState {
  DISCONNECTED = 0,
  CONNECTING = 1,
  CONNECTED = 2,
  SYNCING = 3,
  SYNCED = 4,
  ERROR = 5
}

export class BlockchainManager extends ThreadSafeManager {
  private manager: any;
  private stats: BlockchainStats = {
    totalBlocks: 0,
    totalTransactions: 0,
    connectedNodes: 0,
    averageBlockTime: 0,
    averageTransactionTime: 0
  };

  constructor() {
    super();
    this.manager = new satox_native.BlockchainManager();
  }

  /**
   * Initialize the blockchain manager
   */
  async initialize(config: BlockchainConfig): Promise<boolean> {
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
        console.error('Failed to initialize Blockchain Manager:', error);
        return false;
      }
    });
  }

  /**
   * Shutdown the blockchain manager
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
        console.error('Failed to shutdown Blockchain Manager:', error);
        throw error;
      }
    });
  }

  /**
   * Connect to a specific node
   */
  async connect(nodeAddress: string): Promise<boolean> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.connect(nodeAddress);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return false;
      }
    });
  }

  /**
   * Disconnect from the current node
   */
  async disconnect(): Promise<boolean> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.disconnect();
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return false;
      }
    });
  }

  /**
   * Check if connected to a node
   */
  async isConnected(): Promise<boolean> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.isConnected();
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return false;
      }
    });
  }

  /**
   * Get the current blockchain state
   */
  async getBlockchainState(): Promise<BlockchainState> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.getState();
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return BlockchainState.ERROR;
      }
    });
  }

  /**
   * Get a block by its hash
   */
  async getBlock(blockHash: string): Promise<Block | null> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.getBlock(blockHash);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return null;
      }
    });
  }

  /**
   * Get a block by its height
   */
  async getBlockByHeight(height: number): Promise<Block | null> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.getBlockByHeight(height);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return null;
      }
    });
  }

  /**
   * Get the latest block
   */
  async getLatestBlock(): Promise<Block | null> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.getLatestBlock();
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return null;
      }
    });
  }

  /**
   * Get a range of blocks
   */
  async getBlocks(startHeight: number, endHeight: number): Promise<Block[]> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.getBlocks(startHeight, endHeight);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return [];
      }
    });
  }

  /**
   * Get a transaction by its hash
   */
  async getTransaction(txHash: string): Promise<Transaction | null> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.getTransaction(txHash);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return null;
      }
    });
  }

  /**
   * Broadcast a transaction to the network
   */
  async broadcastTransaction(transaction: Transaction): Promise<boolean> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.broadcastTransaction(transaction);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return false;
      }
    });
  }

  /**
   * Validate a transaction
   */
  async validateTransaction(transaction: Transaction): Promise<boolean> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.validateTransaction(transaction);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return false;
      }
    });
  }

  /**
   * Get all transactions in a block
   */
  async getTransactionsByBlock(blockHash: string): Promise<Transaction[]> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.getTransactionsByBlock(blockHash);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return [];
      }
    });
  }

  /**
   * Get blockchain information
   */
  async getInfo(): Promise<BlockchainInfo> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.getInfo();
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        throw error;
      }
    });
  }

  /**
   * Get current blockchain height
   */
  async getCurrentHeight(): Promise<number> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.getCurrentHeight();
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return 0;
      }
    });
  }

  /**
   * Get the best block hash
   */
  async getBestBlockHash(): Promise<string> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.getBestBlockHash();
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return '';
      }
    });
  }

  /**
   * Get current difficulty
   */
  async getDifficulty(): Promise<number> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.getDifficulty();
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return 0;
      }
    });
  }

  /**
   * Get network hash rate
   */
  async getNetworkHashRate(): Promise<number> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.getNetworkHashRate();
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return 0;
      }
    });
  }

  /**
   * Get blockchain statistics
   */
  async getStats(): Promise<BlockchainStats> {
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
   * Reset statistics
   */
  async resetStats(): Promise<void> {
    return this.withLock(async () => {
      this.ensureInitialized();
      this.stats = {
        totalBlocks: 0,
        totalTransactions: 0,
        connectedNodes: 0,
        averageBlockTime: 0,
        averageTransactionTime: 0
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