package com.satox.asset;

import java.util.concurrent.locks.ReentrantLock;
import java.util.List;
import java.util.Set;
import com.satox.quantum.SatoxException;

public class AssetManager {
    private static AssetManager instance;
    private final ReentrantLock lock;
    private AssetFactory factory;
    private AssetStorage storage;
    private AssetValidator validator;
    private AssetHistory history;
    private AssetSearch search;
    private AssetPermissions permissions;
    private boolean initialized;

    private AssetManager() {
        this.lock = new ReentrantLock();
        this.initialized = false;
    }

    public static synchronized AssetManager getInstance() {
        if (instance == null) {
            instance = new AssetManager();
        }
        return instance;
    }

    public void initialize() throws SatoxException {
        lock.lock();
        try {
            if (initialized) {
                throw new SatoxException("AssetManager already initialized");
            }
            this.factory = AssetFactory.getInstance();
            this.storage = AssetStorage.getInstance();
            this.validator = AssetValidator.getInstance();
            this.history = AssetHistory.getInstance();
            this.search = AssetSearch.getInstance();
            this.permissions = AssetPermissions.getInstance();
            initialized = true;
        } finally {
            lock.unlock();
        }
    }

    public void shutdown() throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            initialized = false;
        } finally {
            lock.unlock();
        }
    }

    public Asset createAsset(String name, String symbol, double totalSupply, String owner) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            Asset asset = factory.createAsset(name, symbol, totalSupply, owner);
            if (validator.validate(asset)) {
                storage.store(asset);
                history.recordCreation(asset);
                permissions.grant(asset.getId(), owner, "OWNER");
                return asset;
            }
            throw new SatoxException("Invalid asset data");
        } finally {
            lock.unlock();
        }
    }

    public Asset getAsset(String assetId) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            return storage.retrieve(assetId);
        } finally {
            lock.unlock();
        }
    }

    public void updateAsset(Asset asset) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            if (validator.validate(asset)) {
                storage.update(asset);
                history.recordUpdate(asset);
            } else {
                throw new SatoxException("Invalid asset data");
            }
        } finally {
            lock.unlock();
        }
    }

    public void deleteAsset(String assetId) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            Asset asset = storage.retrieve(assetId);
            if (asset != null) {
                storage.delete(assetId);
                history.recordDeletion(asset);
            }
        } finally {
            lock.unlock();
        }
    }

    public List<Asset> listAll() throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            return storage.listAll();
        } finally {
            lock.unlock();
        }
    }

    public List<Asset> searchAssets(String query) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            return search.search(query);
        } finally {
            lock.unlock();
        }
    }

    public List<Asset> searchAssetsByName(String name) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            return search.search(name);
        } finally {
            lock.unlock();
        }
    }

    public List<Asset> searchAssetsBySymbol(String symbol) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            return search.search(symbol);
        } finally {
            lock.unlock();
        }
    }

    public List<Asset> searchAssetsByOwner(String owner) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            return search.search(owner);
        } finally {
            lock.unlock();
        }
    }

    public boolean validateAsset(Asset asset) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            return validator.validate(asset);
        } finally {
            lock.unlock();
        }
    }

    public Asset createAssetFromFactory(String name, String symbol, double totalSupply, String owner) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            return factory.createAsset(name, symbol, totalSupply, owner);
        } finally {
            lock.unlock();
        }
    }

    public void storeAsset(Asset asset) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            storage.store(asset);
        } finally {
            lock.unlock();
        }
    }

    public void updateAssetInStorage(Asset asset) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            storage.update(asset);
        } finally {
            lock.unlock();
        }
    }

    public void deleteAssetFromStorage(String assetId) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            storage.delete(assetId);
        } finally {
            lock.unlock();
        }
    }

    public void recordAssetCreation(Asset asset) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            history.recordCreation(asset);
        } finally {
            lock.unlock();
        }
    }

    public void recordAssetUpdate(Asset asset) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            history.recordUpdate(asset);
        } finally {
            lock.unlock();
        }
    }

    public void recordAssetDeletion(Asset asset) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            history.recordDeletion(asset);
        } finally {
            lock.unlock();
        }
    }

    public boolean checkPermission(String assetId, String userId, String permission) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            return permissions.check(assetId, userId, permission);
        } finally {
            lock.unlock();
        }
    }

    public void grantPermission(String assetId, String userId, String permission) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            permissions.grant(assetId, userId, permission);
        } finally {
            lock.unlock();
        }
    }

    public void revokePermission(String assetId, String userId, String permission) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            permissions.revoke(assetId, userId, permission);
        } finally {
            lock.unlock();
        }
    }

    public Set<String> getUserPermissions(String assetId, String userId) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            return permissions.getUserPermissions(assetId, userId);
        } finally {
            lock.unlock();
        }
    }

    public void grantOwnerPermission(String assetId, String userId) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            permissions.grant(assetId, userId, "OWNER");
        } finally {
            lock.unlock();
        }
    }

    public void revokeOwnerPermission(String assetId, String userId) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            permissions.revoke(assetId, userId, "OWNER");
        } finally {
            lock.unlock();
        }
    }

    public void grantReadPermission(String assetId, String userId) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            permissions.grant(assetId, userId, "READ");
        } finally {
            lock.unlock();
        }
    }

    public void revokeReadPermission(String assetId, String userId) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            permissions.revoke(assetId, userId, "READ");
        } finally {
            lock.unlock();
        }
    }

    public void grantWritePermission(String assetId, String userId) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            permissions.grant(assetId, userId, "WRITE");
        } finally {
            lock.unlock();
        }
    }

    public void revokeWritePermission(String assetId, String userId) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            permissions.revoke(assetId, userId, "WRITE");
        } finally {
            lock.unlock();
        }
    }

    public List<AssetHistory.Entry> getAssetHistory(String assetId) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            return history.getHistory(assetId);
        } finally {
            lock.unlock();
        }
    }

    public List<AssetHistory.Entry> getAssetCreationHistory(String assetId) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            return history.getCreationHistory(assetId);
        } finally {
            lock.unlock();
        }
    }

    public List<AssetHistory.Entry> getAssetUpdateHistory(String assetId) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            return history.getUpdateHistory(assetId);
        } finally {
            lock.unlock();
        }
    }

    public List<AssetHistory.Entry> getAssetDeletionHistory(String assetId) throws SatoxException {
        lock.lock();
        try {
            if (!initialized) {
                throw new SatoxException("AssetManager not initialized");
            }
            return history.getDeletionHistory(assetId);
        } finally {
            lock.unlock();
        }
    }

    public boolean isInitialized() {
        lock.lock();
        try {
            return initialized;
        } finally {
            lock.unlock();
        }
    }

    public Asset createAsset(String name, String symbol, double totalSupply) throws SatoxException {
        return createAsset(name, symbol, totalSupply, "test-owner");
    }
} 
