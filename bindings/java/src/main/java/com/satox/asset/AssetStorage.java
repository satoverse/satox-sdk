package com.satox.asset;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.locks.ReentrantLock;
import java.util.Map;
import java.util.List;
import java.util.ArrayList;

public class AssetStorage {
    private static AssetStorage instance;
    private final ReentrantLock lock;
    private final Map<String, Asset> storage;
    private boolean initialized;

    private AssetStorage() {
        this.lock = new ReentrantLock();
        this.storage = new ConcurrentHashMap<>();
        this.initialized = false;
    }

    public static synchronized AssetStorage getInstance() {
        if (instance == null) {
            instance = new AssetStorage();
        }
        return instance;
    }

    public void initialize() {
        lock.lock();
        try {
            if (initialized) {
                throw new IllegalStateException("AssetStorage already initialized");
            }
            initialized = true;
        } finally {
            lock.unlock();
        }
    }

    public void shutdown() {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("AssetStorage not initialized");
            }
            storage.clear();
            initialized = false;
        } finally {
            lock.unlock();
        }
    }

    public void store(Asset asset) {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("AssetStorage not initialized");
            }
            if (asset == null) {
                throw new IllegalArgumentException("Asset cannot be null");
            }
            storage.put(asset.getId(), asset);
        } finally {
            lock.unlock();
        }
    }

    public Asset retrieve(String assetId) {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("AssetStorage not initialized");
            }
            return storage.get(assetId);
        } finally {
            lock.unlock();
        }
    }

    public void update(Asset asset) {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("AssetStorage not initialized");
            }
            if (asset == null) {
                throw new IllegalArgumentException("Asset cannot be null");
            }
            storage.put(asset.getId(), asset);
        } finally {
            lock.unlock();
        }
    }

    public void delete(String assetId) {
        lock.lock();
        try {
            storage.remove(assetId);
        } finally {
            lock.unlock();
        }
    }

    public List<Asset> listAll() {
        lock.lock();
        try {
            return new ArrayList<>(storage.values());
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
} 