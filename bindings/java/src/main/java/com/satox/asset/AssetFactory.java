package com.satox.asset;

import java.util.concurrent.locks.ReentrantLock;

public class AssetFactory {
    private static AssetFactory instance;
    private final ReentrantLock lock;
    private boolean initialized;

    private AssetFactory() {
        this.lock = new ReentrantLock();
        this.initialized = false;
    }

    public static synchronized AssetFactory getInstance() {
        if (instance == null) {
            instance = new AssetFactory();
        }
        return instance;
    }

    public void initialize() {
        lock.lock();
        try {
            if (initialized) {
                throw new IllegalStateException("AssetFactory already initialized");
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
                throw new IllegalStateException("AssetFactory not initialized");
            }
            initialized = false;
        } finally {
            lock.unlock();
        }
    }

    public Asset createAsset(String name, String symbol, double totalSupply, String owner) {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("AssetFactory not initialized");
            }
            if (name == null || name.trim().isEmpty()) {
                throw new IllegalArgumentException("Asset name cannot be empty");
            }
            if (symbol == null || symbol.trim().isEmpty()) {
                throw new IllegalArgumentException("Asset symbol cannot be empty");
            }
            if (totalSupply <= 0) {
                throw new IllegalArgumentException("Total supply must be greater than 0");
            }
            if (owner == null || owner.trim().isEmpty()) {
                throw new IllegalArgumentException("Owner cannot be empty");
            }

            return new Asset(name, symbol, totalSupply, owner);
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