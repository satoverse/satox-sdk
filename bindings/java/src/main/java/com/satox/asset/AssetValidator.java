package com.satox.asset;

import java.util.concurrent.locks.ReentrantLock;
import java.util.Map;

public class AssetValidator {
    private static AssetValidator instance;
    private final ReentrantLock lock;
    private boolean initialized;

    private AssetValidator() {
        this.lock = new ReentrantLock();
        this.initialized = false;
    }

    public static synchronized AssetValidator getInstance() {
        if (instance == null) {
            instance = new AssetValidator();
        }
        return instance;
    }

    public void initialize() {
        lock.lock();
        try {
            if (initialized) {
                throw new IllegalStateException("AssetValidator already initialized");
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
                throw new IllegalStateException("AssetValidator not initialized");
            }
            initialized = false;
        } finally {
            lock.unlock();
        }
    }

    public boolean validate(Asset asset) {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("AssetValidator not initialized");
            }
            if (asset == null) {
                return false;
            }

            // Validate basic properties
            if (asset.getName() == null || asset.getName().trim().isEmpty()) {
                return false;
            }
            if (asset.getSymbol() == null || asset.getSymbol().trim().isEmpty()) {
                return false;
            }
            if (asset.getTotalSupply() <= 0) {
                return false;
            }
            if (asset.getOwner() == null || asset.getOwner().trim().isEmpty()) {
                return false;
            }

            // Validate metadata
            if (!validateMetadata(asset.getMetadata())) {
                return false;
            }

            // Validate status
            if (asset.getStatus() == null) {
                return false;
            }

            return true;
        } finally {
            lock.unlock();
        }
    }

    private boolean validateMetadata(Map<String, Object> metadata) {
        if (metadata == null) {
            return false;
        }
        // Add specific metadata validation rules here
        return true;
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