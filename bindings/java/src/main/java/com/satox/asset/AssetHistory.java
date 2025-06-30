package com.satox.asset;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.locks.ReentrantLock;
import java.util.Map;
import java.util.List;
import java.util.ArrayList;
import java.time.Instant;

public class AssetHistory {
    private static AssetHistory instance;
    private final ReentrantLock lock;
    private final Map<String, List<Entry>> history;
    private boolean initialized;

    private AssetHistory() {
        this.lock = new ReentrantLock();
        this.history = new ConcurrentHashMap<>();
        this.initialized = false;
    }

    public static synchronized AssetHistory getInstance() {
        if (instance == null) {
            instance = new AssetHistory();
        }
        return instance;
    }

    public void initialize() {
        lock.lock();
        try {
            if (initialized) {
                throw new IllegalStateException("AssetHistory already initialized");
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
                throw new IllegalStateException("AssetHistory not initialized");
            }
            history.clear();
            initialized = false;
        } finally {
            lock.unlock();
        }
    }

    public void recordCreation(Asset asset) {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("AssetHistory not initialized");
            }
            addEntry(asset.getId(), new Entry(EntryType.CREATION, asset));
        } finally {
            lock.unlock();
        }
    }

    public void recordUpdate(Asset asset) {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("AssetHistory not initialized");
            }
            addEntry(asset.getId(), new Entry(EntryType.UPDATE, asset));
        } finally {
            lock.unlock();
        }
    }

    public void recordDeletion(Asset asset) {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("AssetHistory not initialized");
            }
            addEntry(asset.getId(), new Entry(EntryType.DELETION, asset));
        } finally {
            lock.unlock();
        }
    }

    private void addEntry(String assetId, Entry entry) {
        history.computeIfAbsent(assetId, k -> new ArrayList<>()).add(entry);
    }

    public List<Entry> getHistory(String assetId) {
        lock.lock();
        try {
            List<Entry> entries = history.get(assetId);
            return entries != null ? new ArrayList<>(entries) : new ArrayList<>();
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

    public List<Entry> getCreationHistory(String assetId) {
        return getHistory(assetId).stream()
            .filter(e -> e.getType() == EntryType.CREATION)
            .collect(java.util.stream.Collectors.toList());
    }

    public List<Entry> getUpdateHistory(String assetId) {
        return getHistory(assetId).stream()
            .filter(e -> e.getType() == EntryType.UPDATE)
            .collect(java.util.stream.Collectors.toList());
    }

    public List<Entry> getDeletionHistory(String assetId) {
        return getHistory(assetId).stream()
            .filter(e -> e.getType() == EntryType.DELETION)
            .collect(java.util.stream.Collectors.toList());
    }

    public static class Entry {
        private final EntryType type;
        private final Asset asset;
        private final Instant timestamp;

        public Entry(EntryType type, Asset asset) {
            this.type = type;
            this.asset = asset;
            this.timestamp = Instant.now();
        }

        public EntryType getType() {
            return type;
        }

        public Asset getAsset() {
            return asset;
        }

        public Instant getTimestamp() {
            return timestamp;
        }
    }

    public enum EntryType {
        CREATION,
        UPDATE,
        DELETION
    }
} 
