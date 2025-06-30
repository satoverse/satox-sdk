package com.satox.core;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.locks.ReentrantLock;
import java.util.Map;
import java.util.logging.Logger;
import java.util.logging.Level;

public class ErrorHandler {
    private static ErrorHandler instance;
    private final ReentrantLock lock;
    private final Map<String, ErrorCallback> errorCallbacks;
    private final Logger logger;
    private boolean initialized;

    private ErrorHandler() {
        this.lock = new ReentrantLock();
        this.errorCallbacks = new ConcurrentHashMap<>();
        this.logger = Logger.getLogger(ErrorHandler.class.getName());
        this.initialized = false;
    }

    public static synchronized ErrorHandler getInstance() {
        if (instance == null) {
            instance = new ErrorHandler();
        }
        return instance;
    }

    public void initialize() {
        lock.lock();
        try {
            if (initialized) {
                throw new IllegalStateException("ErrorHandler already initialized");
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
                throw new IllegalStateException("ErrorHandler not initialized");
            }
            errorCallbacks.clear();
            initialized = false;
        } finally {
            lock.unlock();
        }
    }

    public void registerErrorCallback(String errorType, ErrorCallback callback) {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("ErrorHandler not initialized");
            }
            errorCallbacks.put(errorType, callback);
        } finally {
            lock.unlock();
        }
    }

    public void handleError(String errorType, String message, Throwable throwable) {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("ErrorHandler not initialized");
            }
            logger.log(Level.SEVERE, message, throwable);
            ErrorCallback callback = errorCallbacks.get(errorType);
            if (callback != null) {
                callback.onError(errorType, message, throwable);
            }
        } finally {
            lock.unlock();
        }
    }

    public void unregisterErrorCallback(String errorType) {
        lock.lock();
        try {
            errorCallbacks.remove(errorType);
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

    @FunctionalInterface
    public interface ErrorCallback {
        void onError(String errorType, String message, Throwable throwable);
    }
} 