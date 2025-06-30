package com.satox.core;

public interface Component {
    void initialize();
    void shutdown();
    String getName();
    boolean isInitialized();
} 