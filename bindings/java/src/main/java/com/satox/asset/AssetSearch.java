package com.satox.asset;

import java.util.concurrent.locks.ReentrantLock;
import java.util.List;
import java.util.ArrayList;
import java.util.stream.Collectors;

public class AssetSearch {
    private static AssetSearch instance;
    private final ReentrantLock lock;
    private AssetStorage storage;
    private boolean initialized;

    private AssetSearch() {
        this.lock = new ReentrantLock();
        this.initialized = false;
    }

    public static synchronized AssetSearch getInstance() {
        if (instance == null) {
            instance = new AssetSearch();
        }
        return instance;
    }

    public void initialize() {
        lock.lock();
        try {
            if (initialized) {
                throw new IllegalStateException("AssetSearch already initialized");
            }
            this.storage = AssetStorage.getInstance();
            initialized = true;
        } finally {
            lock.unlock();
        }
    }

    public void shutdown() {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("AssetSearch not initialized");
            }
            initialized = false;
        } finally {
            lock.unlock();
        }
    }

    public List<Asset> search(String query) {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("AssetSearch not initialized");
            }
            if (query == null || query.trim().isEmpty()) {
                return new ArrayList<>();
            }

            String lowerQuery = query.toLowerCase();
            return storage.listAll().stream()
                .filter(asset -> matchesQuery(asset, lowerQuery))
                .collect(Collectors.toList());
        } finally {
            lock.unlock();
        }
    }

    private boolean matchesQuery(Asset asset, String query) {
        return asset.getName().toLowerCase().contains(query) ||
               asset.getSymbol().toLowerCase().contains(query) ||
               asset.getOwner().toLowerCase().contains(query) ||
               matchesMetadata(asset, query);
    }

    private boolean matchesMetadata(Asset asset, String query) {
        return asset.getMetadata().entrySet().stream()
            .anyMatch(entry -> {
                String value = String.valueOf(entry.getValue()).toLowerCase();
                return value.contains(query);
            });
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