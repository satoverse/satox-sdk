import { satox_native } from './native/satox_native';
import { ThreadSafeManager } from './thread-safe';

export interface Asset {
  id: string;
  name: string;
  symbol: string;
  description: string;
  quantity: number;
  decimals: number;
  reissuable: boolean;
  owner: string;
  createdAt: number;
  updatedAt: number;
}

export interface AssetUpdate {
  name?: string;
  description?: string;
  quantity?: number;
}

export interface AssetTransfer {
  assetId: string;
  fromAddress: string;
  toAddress: string;
  quantity: number;
  fee: number;
}

export interface AssetHistory {
  assetId: string;
  fromAddress: string;
  toAddress: string;
  quantity: number;
  txHash: string;
  timestamp: number;
  type: number;
}

export interface AssetStats {
  totalAssets: number;
  totalTransfers: number;
  totalVolume: number;
  averageAssetValue: number;
  mostTradedAsset: string;
}

export class AssetManager extends ThreadSafeManager {
  private manager: any;
  private stats: AssetStats = {
    totalAssets: 0,
    totalTransfers: 0,
    totalVolume: 0,
    averageAssetValue: 0,
    mostTradedAsset: ''
  };

  constructor() {
    super();
    this.manager = new satox_native.AssetManager();
  }

  /**
   * Initialize the asset manager
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
        console.error('Failed to initialize Asset Manager:', error);
        return false;
      }
    });
  }

  /**
   * Shutdown the asset manager
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
        console.error('Failed to shutdown Asset Manager:', error);
        throw error;
      }
    });
  }

  /**
   * Create a new asset
   */
  async createAsset(asset: Omit<Asset, 'id' | 'createdAt' | 'updatedAt'>): Promise<string> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.createAsset(asset);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return '';
      }
    });
  }

  /**
   * Get an asset by its ID
   */
  async getAsset(assetId: string): Promise<Asset | null> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.getAsset(assetId);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return null;
      }
    });
  }

  /**
   * Get asset metadata
   */
  async getAssetMetadata(assetId: string): Promise<Asset | null> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.getAssetMetadata(assetId);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return null;
      }
    });
  }

  /**
   * Update an asset
   */
  async updateAsset(assetId: string, update: AssetUpdate): Promise<boolean> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.updateAsset(assetId, update);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return false;
      }
    });
  }

  /**
   * Delete an asset
   */
  async deleteAsset(assetId: string): Promise<boolean> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.deleteAsset(assetId);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return false;
      }
    });
  }

  /**
   * List assets with pagination
   */
  async listAssets(limit: number = 100, offset: number = 0): Promise<Asset[]> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.listAssets(limit, offset);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return [];
      }
    });
  }

  /**
   * Search assets by query
   */
  async searchAssets(query: string, limit: number = 100): Promise<Asset[]> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.searchAssets(query, limit);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return [];
      }
    });
  }

  /**
   * Get asset balance for an address
   */
  async getAssetBalance(assetId: string, address: string): Promise<number> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.getAssetBalance(assetId, address);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return 0;
      }
    });
  }

  /**
   * Transfer assets between addresses
   */
  async transferAsset(transfer: AssetTransfer): Promise<string> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.transferAsset(transfer);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return '';
      }
    });
  }

  /**
   * Get asset transfer history
   */
  async getAssetHistory(assetId: string, limit: number = 100): Promise<AssetHistory[]> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.getAssetHistory(assetId, limit);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return [];
      }
    });
  }

  /**
   * Validate asset name
   */
  async validateAssetName(assetName: string): Promise<boolean> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.validateAssetName(assetName);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return false;
      }
    });
  }

  /**
   * Get asset statistics
   */
  async getAssetStats(): Promise<AssetStats> {
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