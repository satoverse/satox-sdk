const { EventEmitter } = require("events");
const crypto = require("crypto");

class AssetManager extends EventEmitter {
  constructor() {
    super();
    this._initialized = false;
    this._assets = new Map();
    this._metadata = new Map();
  }

  /**
   * Initialize the Asset Manager
   * @returns {Promise<void>}
   */
  async initialize() {
    if (this._initialized) {
      throw new Error("AssetManager already initialized");
    }

    try {
      this._initialized = true;
      this.emit("initialized");
    } catch (error) {
      throw new Error(`Failed to initialize AssetManager: ${error.message}`);
    }
  }

  /**
   * Check if the manager is initialized
   * @returns {boolean}
   */
  isInitialized() {
    return this._initialized;
  }

  /**
   * Create a new asset
   * @param {Object} assetData - Asset creation data
   * @param {string} assetData.name - Asset name
   * @param {string} assetData.symbol - Asset symbol
   * @param {number} assetData.amount - Initial amount
   * @param {Object} assetData.metadata - Asset metadata
   * @returns {Promise<Object>}
   */
  async createAsset(assetData) {
    if (!this._initialized) {
      throw new Error("AssetManager not initialized");
    }

    try {
      const { name, symbol, amount, metadata } = assetData;

      // Validate input
      if (!name || !symbol || !amount) {
        throw new Error("Missing required asset data");
      }

      if (amount <= 0) {
        throw new Error("Amount must be greater than 0");
      }

      // Generate unique asset ID
      const assetId = this._generateAssetId(name, symbol);

      // Create asset object
      const asset = {
        id: assetId,
        name,
        symbol,
        amount,
        metadata: metadata || {},
        createdAt: new Date().toISOString(),
        status: "active",
      };

      // Store asset
      this._assets.set(assetId, asset);
      this._metadata.set(assetId, metadata || {});

      this.emit("assetCreated", asset);
      return asset;
    } catch (error) {
      throw new Error(`Failed to create asset: ${error.message}`);
    }
  }

  /**
   * Get asset by ID
   * @param {string} assetId - Asset ID
   * @returns {Promise<Object>}
   */
  async getAsset(assetId) {
    if (!this._initialized) {
      throw new Error("AssetManager not initialized");
    }

    try {
      const asset = this._assets.get(assetId);
      if (!asset) {
        throw new Error("Asset not found");
      }
      return { ...asset };
    } catch (error) {
      throw new Error(`Failed to get asset: ${error.message}`);
    }
  }

  /**
   * Update asset metadata
   * @param {string} assetId - Asset ID
   * @param {Object} metadata - New metadata
   * @returns {Promise<Object>}
   */
  async updateMetadata(assetId, metadata) {
    if (!this._initialized) {
      throw new Error("AssetManager not initialized");
    }

    try {
      const asset = this._assets.get(assetId);
      if (!asset) {
        throw new Error("Asset not found");
      }

      const updatedAsset = {
        ...asset,
        metadata: {
          ...asset.metadata,
          ...metadata,
        },
      };

      this._assets.set(assetId, updatedAsset);
      this._metadata.set(assetId, updatedAsset.metadata);

      this.emit("metadataUpdated", {
        assetId,
        metadata: updatedAsset.metadata,
      });
      return updatedAsset;
    } catch (error) {
      throw new Error(`Failed to update metadata: ${error.message}`);
    }
  }

  /**
   * Reissue asset
   * @param {string} assetId - Asset ID
   * @param {number} amount - Amount to reissue
   * @returns {Promise<Object>}
   */
  async reissueAsset(assetId, amount) {
    if (!this._initialized) {
      throw new Error("AssetManager not initialized");
    }

    try {
      const asset = this._assets.get(assetId);
      if (!asset) {
        throw new Error("Asset not found");
      }

      if (amount <= 0) {
        throw new Error("Reissue amount must be greater than 0");
      }

      const updatedAsset = {
        ...asset,
        amount: asset.amount + amount,
        lastReissue: new Date().toISOString(),
      };

      this._assets.set(assetId, updatedAsset);
      this.emit("assetReissued", {
        assetId,
        amount,
        newTotal: updatedAsset.amount,
      });
      return updatedAsset;
    } catch (error) {
      throw new Error(`Failed to reissue asset: ${error.message}`);
    }
  }

  /**
   * Transfer asset
   * @param {string} assetId - Asset ID
   * @param {string} fromAddress - Source address
   * @param {string} toAddress - Destination address
   * @param {number} amount - Amount to transfer
   * @returns {Promise<Object>}
   */
  async transferAsset(assetId, fromAddress, toAddress, amount) {
    if (!this._initialized) {
      throw new Error("AssetManager not initialized");
    }

    try {
      const asset = this._assets.get(assetId);
      if (!asset) {
        throw new Error("Asset not found");
      }

      if (amount <= 0) {
        throw new Error("Transfer amount must be greater than 0");
      }

      if (amount > asset.amount) {
        throw new Error("Insufficient asset balance");
      }

      const transfer = {
        assetId,
        fromAddress,
        toAddress,
        amount,
        timestamp: new Date().toISOString(),
      };

      this.emit("assetTransferred", transfer);
      return transfer;
    } catch (error) {
      throw new Error(`Failed to transfer asset: ${error.message}`);
    }
  }

  /**
   * Search assets
   * @param {Object} criteria - Search criteria
   * @returns {Promise<Array>}
   */
  async searchAssets(criteria) {
    if (!this._initialized) {
      throw new Error("AssetManager not initialized");
    }

    try {
      const assets = Array.from(this._assets.values());
      return assets.filter((asset) => {
        return Object.entries(criteria).every(([key, value]) => {
          return asset[key] === value;
        });
      });
    } catch (error) {
      throw new Error(`Failed to search assets: ${error.message}`);
    }
  }

  /**
   * Generate unique asset ID
   * @private
   * @param {string} name - Asset name
   * @param {string} symbol - Asset symbol
   * @returns {string}
   */
  _generateAssetId(name, symbol) {
    const data = `${name}:${symbol}:${Date.now()}`;
    return crypto.createHash("sha256").update(data).digest("hex");
  }

  /**
   * Clean up resources
   * @returns {Promise<void>}
   */
  async cleanup() {
    if (!this._initialized) {
      return;
    }

    try {
      this._assets.clear();
      this._metadata.clear();
      this._initialized = false;
      this.emit("cleanup");
    } catch (error) {
      throw new Error(`Cleanup failed: ${error.message}`);
    }
  }
}

module.exports = AssetManager;
