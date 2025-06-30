import { satox_native } from './native/satox_native';
import { ThreadSafeManager } from './thread-safe';

export interface NFT {
  id: string;
  name: string;
  description: string;
  creator: string;
  owner: string;
  assetId: string;
  tokenId: string;
  metadata: string;
  createdAt: number;
  updatedAt: number;
}

export interface NFTUpdate {
  name?: string;
  description?: string;
  metadata?: Record<string, any>;
}

export interface NFTTransfer {
  nftId: string;
  fromAddress: string;
  toAddress: string;
  fee: number;
}

export interface OwnershipRecord {
  nftId: string;
  fromAddress: string;
  toAddress: string;
  txHash: string;
  timestamp: number;
  type: number;
}

export interface NFTStats {
  totalNFTs: number;
  totalTransfers: number;
  totalCreators: number;
  totalOwners: number;
  averageNFTValue: number;
  mostTradedNFT: string;
}

export class NFTManager extends ThreadSafeManager {
  private manager: any;
  private stats: NFTStats = {
    totalNFTs: 0,
    totalTransfers: 0,
    totalCreators: 0,
    totalOwners: 0,
    averageNFTValue: 0,
    mostTradedNFT: ''
  };

  constructor() {
    super();
    this.manager = new satox_native.NFTManager();
  }

  /**
   * Initialize the NFT manager
   */
  async initialize(): Promise<boolean> {
    return this.withLock(async () => {
      try {
        this.ensureNotDisposed();
        const result = this.manager.initialize();
        this.setInitialized(result);
        this.setError(null);
        return result;
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        console.error('Failed to initialize NFT Manager:', error);
        return false;
      }
    });
  }

  /**
   * Shutdown the NFT manager
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
        console.error('Failed to shutdown NFT Manager:', error);
        throw error;
      }
    });
  }

  /**
   * Create a new NFT
   */
  async createNFT(nft: Omit<NFT, 'id' | 'createdAt' | 'updatedAt'>): Promise<string> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.createNFT(nft);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return '';
      }
    });
  }

  /**
   * Get an NFT by its ID
   */
  async getNFT(nftId: string): Promise<NFT | null> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.getNFT(nftId);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return null;
      }
    });
  }

  /**
   * Update an NFT
   */
  async updateNFT(nftId: string, update: NFTUpdate): Promise<boolean> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.updateNFT(nftId, update);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return false;
      }
    });
  }

  /**
   * Delete an NFT
   */
  async deleteNFT(nftId: string): Promise<boolean> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.deleteNFT(nftId);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return false;
      }
    });
  }

  /**
   * List NFTs with pagination
   */
  async listNFTs(limit: number = 100, offset: number = 0): Promise<NFT[]> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.listNFTs(limit, offset);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return [];
      }
    });
  }

  /**
   * Search NFTs by query
   */
  async searchNFTs(query: string, limit: number = 100): Promise<NFT[]> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.searchNFTs(query, limit);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return [];
      }
    });
  }

  /**
   * Transfer an NFT between addresses
   */
  async transferNFT(transfer: NFTTransfer): Promise<string> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.transferNFT(transfer);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return '';
      }
    });
  }

  /**
   * Get NFT ownership history
   */
  async getOwnershipHistory(nftId: string, limit: number = 100): Promise<OwnershipRecord[]> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.getOwnershipHistory(nftId, limit);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return [];
      }
    });
  }

  /**
   * Validate NFT metadata
   */
  async validateNFTMetadata(metadata: Record<string, any>): Promise<boolean> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.validateNFTMetadata(metadata);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return false;
      }
    });
  }

  /**
   * Get NFT statistics
   */
  async getNFTStats(): Promise<NFTStats> {
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