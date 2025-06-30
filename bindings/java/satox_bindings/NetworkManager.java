package com.satox.bindings;

import java.util.Map;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

public class NetworkManager {
    private final ReadWriteLock lock;
    private boolean initialized;
    private String currentNetwork;
    private int port;
    private boolean isConnected;

    public NetworkManager() {
        this.lock = new ReentrantReadWriteLock();
        this.initialized = false;
        this.isConnected = false;
    }

    public void initialize() {
        lock.writeLock().lock();
        try {
            if (initialized) {
                throw new IllegalStateException("NetworkManager already initialized");
            }
            initialized = true;
        } finally {
            lock.writeLock().unlock();
        }
    }

    public void connect(String networkId, int port) {
        ensureInitialized();
        lock.writeLock().lock();
        try {
            if (isConnected) {
                throw new IllegalStateException("Already connected to network");
            }
            this.currentNetwork = networkId;
            this.port = port;
            this.isConnected = true;
        } finally {
            lock.writeLock().unlock();
        }
    }

    public void disconnect() {
        ensureInitialized();
        lock.writeLock().lock();
        try {
            if (!isConnected) {
                throw new IllegalStateException("Not connected to any network");
            }
            this.isConnected = false;
            this.currentNetwork = null;
        } finally {
            lock.writeLock().unlock();
        }
    }

    public boolean isConnected() {
        lock.readLock().lock();
        try {
            return isConnected;
        } finally {
            lock.readLock().unlock();
        }
    }

    public String getCurrentNetwork() {
        lock.readLock().lock();
        try {
            if (!isConnected) {
                throw new IllegalStateException("Not connected to any network");
            }
            return currentNetwork;
        } finally {
            lock.readLock().unlock();
        }
    }

    public int getPort() {
        lock.readLock().lock();
        try {
            if (!isConnected) {
                throw new IllegalStateException("Not connected to any network");
            }
            return port;
        } finally {
            lock.readLock().unlock();
        }
    }

    public void sendMessage(String peerId, byte[] message) {
        ensureInitialized();
        ensureConnected();
        // Implementation for sending messages to peers
    }

    public byte[] receiveMessage(String peerId) {
        ensureInitialized();
        ensureConnected();
        // Implementation for receiving messages from peers
        return new byte[0];
    }

    public Map<String, Object> getPeerInfo(String peerId) {
        ensureInitialized();
        ensureConnected();
        // Implementation for getting peer information
        return null;
    }

    public void shutdown() {
        lock.writeLock().lock();
        try {
            if (isConnected) {
                disconnect();
            }
            initialized = false;
        } finally {
            lock.writeLock().unlock();
        }
    }

    private void ensureInitialized() {
        if (!initialized) {
            throw new IllegalStateException("NetworkManager not initialized");
        }
    }

    private void ensureConnected() {
        if (!isConnected) {
            throw new IllegalStateException("Not connected to any network");
        }
    }
} 