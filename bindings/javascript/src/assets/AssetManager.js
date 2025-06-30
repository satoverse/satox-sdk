const { EventEmitter } = require('events');

class AssetManager extends EventEmitter {
    constructor() {
        super();
        this.assets = new Map();
        this.balances = new Map();
        this.initialized = false;
    }

    initialize() {
        this.initialized = true;
        this.emit('initialized');
    }

    isInitialized() {
        return this.initialized;
    }

    createAsset(name, totalSupply) {
        if (!this.initialized) {
            throw new Error('AssetManager not initialized');
        }

        const asset = {
            id: this.generateAssetId(),
            name,
            totalSupply,
            createdAt: Date.now(),
            metadata: {}
        };

        this.assets.set(asset.id, asset);
        this.emit('assetCreated', asset);
        return asset;
    }

    validateAsset(asset) {
        if (!this.initialized) {
            throw new Error('AssetManager not initialized');
        }

        if (!asset || !asset.id || !asset.name || !asset.totalSupply) {
            throw new Error('Invalid asset');
        }

        return true;
    }

    transferAsset(assetId, from, to, amount) {
        if (!this.initialized) {
            throw new Error('AssetManager not initialized');
        }

        const asset = this.assets.get(assetId);
        if (!asset) {
            throw new Error('Asset not found');
        }

        const fromBalance = this.getBalance(assetId, from);
        if (fromBalance < amount) {
            throw new Error('Insufficient balance');
        }

        this.updateBalance(assetId, from, fromBalance - amount);
        const toBalance = this.getBalance(assetId, to);
        this.updateBalance(assetId, to, toBalance + amount);

        const transfer = {
            assetId,
            from,
            to,
            amount,
            timestamp: Date.now()
        };

        this.emit('assetTransferred', transfer);
        return transfer;
    }

    getBalance(assetId, address) {
        const key = `${assetId}:${address}`;
        return this.balances.get(key) || 0;
    }

    updateBalance(assetId, address, amount) {
        const key = `${assetId}:${address}`;
        this.balances.set(key, amount);
        this.emit('balanceUpdated', { assetId, address, amount });
    }

    searchAssets(query) {
        if (!this.initialized) {
            throw new Error('AssetManager not initialized');
        }

        return Array.from(this.assets.values()).filter(asset => 
            asset.name.toLowerCase().includes(query.toLowerCase()) ||
            asset.id.toLowerCase().includes(query.toLowerCase())
        );
    }

    getAssetHistory(assetId) {
        if (!this.initialized) {
            throw new Error('AssetManager not initialized');
        }

        const asset = this.assets.get(assetId);
        if (!asset) {
            throw new Error('Asset not found');
        }

        return {
            asset,
            transfers: Array.from(this.balances.entries())
                .filter(([key]) => key.startsWith(assetId))
                .map(([key, amount]) => ({
                    address: key.split(':')[1],
                    amount,
                    timestamp: Date.now()
                }))
        };
    }

    generateAssetId() {
        return `asset_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`;
    }
}

module.exports = AssetManager; 