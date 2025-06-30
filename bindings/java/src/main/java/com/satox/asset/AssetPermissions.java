package com.satox.asset;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.locks.ReentrantLock;
import java.util.Map;
import java.util.Set;
import java.util.HashSet;

public class AssetPermissions {
    private static AssetPermissions instance;
    private final ReentrantLock lock;
    private final Map<String, Map<String, Set<String>>> permissions;
    private boolean initialized;

    private AssetPermissions() {
        this.lock = new ReentrantLock();
        this.permissions = new ConcurrentHashMap<>();
        this.initialized = false;
    }

    public static synchronized AssetPermissions getInstance() {
        if (instance == null) {
            instance = new AssetPermissions();
        }
        return instance;
    }

    public void initialize() {
        lock.lock();
        try {
            if (initialized) {
                throw new IllegalStateException("AssetPermissions already initialized");
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
                throw new IllegalStateException("AssetPermissions not initialized");
            }
            permissions.clear();
            initialized = false;
        } finally {
            lock.unlock();
        }
    }

    public boolean check(String assetId, String userId, String permission) {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("AssetPermissions not initialized");
            }
            Map<String, Set<String>> assetPermissions = permissions.get(assetId);
            if (assetPermissions == null) {
                return false;
            }
            Set<String> userPermissions = assetPermissions.get(userId);
            return userPermissions != null && userPermissions.contains(permission);
        } finally {
            lock.unlock();
        }
    }

    public void grant(String assetId, String userId, String permission) {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("AssetPermissions not initialized");
            }
            permissions.computeIfAbsent(assetId, k -> new ConcurrentHashMap<>())
                      .computeIfAbsent(userId, k -> new HashSet<>())
                      .add(permission);
        } finally {
            lock.unlock();
        }
    }

    public void revoke(String assetId, String userId, String permission) {
        lock.lock();
        try {
            Map<String, Set<String>> assetPermissions = permissions.get(assetId);
            if (assetPermissions != null) {
                Set<String> userPermissions = assetPermissions.get(userId);
                if (userPermissions != null) {
                    userPermissions.remove(permission);
                    if (userPermissions.isEmpty()) {
                        assetPermissions.remove(userId);
                    }
                    if (assetPermissions.isEmpty()) {
                        permissions.remove(assetId);
                    }
                }
            }
        } finally {
            lock.unlock();
        }
    }

    public Set<String> getUserPermissions(String assetId, String userId) {
        lock.lock();
        try {
            Map<String, Set<String>> assetPermissions = permissions.get(assetId);
            if (assetPermissions != null) {
                Set<String> userPermissions = assetPermissions.get(userId);
                if (userPermissions != null) {
                    return new HashSet<>(userPermissions);
                }
            }
            return new HashSet<>();
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