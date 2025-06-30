package com.satox.bindings;

import java.util.Map;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

public class APIManager {
    private final ReadWriteLock lock;
    private boolean initialized;
    private boolean serverRunning;
    private String host;
    private int port;
    private boolean sslEnabled;
    private String sslCertPath;
    private String sslKeyPath;

    public APIManager() {
        this.lock = new ReentrantReadWriteLock();
        this.initialized = false;
        this.serverRunning = false;
    }

    public void initialize() {
        lock.writeLock().lock();
        try {
            if (initialized) {
                throw new IllegalStateException("APIManager already initialized");
            }
            initialized = true;
        } finally {
            lock.writeLock().unlock();
        }
    }

    public void startServer(String host, int port, boolean enableSSL, String sslCertPath, String sslKeyPath) {
        ensureInitialized();
        lock.writeLock().lock();
        try {
            if (serverRunning) {
                throw new IllegalStateException("Server is already running");
            }
            this.host = host;
            this.port = port;
            this.sslEnabled = enableSSL;
            this.sslCertPath = sslCertPath;
            this.sslKeyPath = sslKeyPath;
            this.serverRunning = true;
        } finally {
            lock.writeLock().unlock();
        }
    }

    public void stopServer() {
        ensureInitialized();
        lock.writeLock().lock();
        try {
            if (!serverRunning) {
                throw new IllegalStateException("Server is not running");
            }
            this.serverRunning = false;
        } finally {
            lock.writeLock().unlock();
        }
    }

    public boolean isServerRunning() {
        lock.readLock().lock();
        try {
            return serverRunning;
        } finally {
            lock.readLock().unlock();
        }
    }

    public void registerEndpoint(String path, String method, Object handler) {
        ensureInitialized();
        ensureServerRunning();
        // Implementation for registering API endpoints
    }

    public void unregisterEndpoint(String path, String method) {
        ensureInitialized();
        ensureServerRunning();
        // Implementation for unregistering API endpoints
    }

    public void setMiddleware(Object middleware) {
        ensureInitialized();
        ensureServerRunning();
        // Implementation for setting API middleware
    }

    public Map<String, Object> getServerStatus() {
        ensureInitialized();
        lock.readLock().lock();
        try {
            return Map.of(
                "running", serverRunning,
                "host", host,
                "port", port,
                "ssl_enabled", sslEnabled
            );
        } finally {
            lock.readLock().unlock();
        }
    }

    public void shutdown() {
        lock.writeLock().lock();
        try {
            if (serverRunning) {
                stopServer();
            }
            initialized = false;
        } finally {
            lock.writeLock().unlock();
        }
    }

    private void ensureInitialized() {
        if (!initialized) {
            throw new IllegalStateException("APIManager not initialized");
        }
    }

    private void ensureServerRunning() {
        if (!serverRunning) {
            throw new IllegalStateException("Server is not running");
        }
    }
} 