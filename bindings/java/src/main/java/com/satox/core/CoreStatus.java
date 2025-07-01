package com.satox.core;

import java.util.Map;

public class CoreStatus {
    private boolean initialized;
    private String version;
    private double uptime;
    private Map<String, Object> memoryUsage;
    private int threadCount;

    public CoreStatus() {}

    public boolean isInitialized() { return initialized; }
    public void setInitialized(boolean initialized) { this.initialized = initialized; }

    public String getVersion() { return version; }
    public void setVersion(String version) { this.version = version; }

    public double getUptime() { return uptime; }
    public void setUptime(double uptime) { this.uptime = uptime; }

    public Map<String, Object> getMemoryUsage() { return memoryUsage; }
    public void setMemoryUsage(Map<String, Object> memoryUsage) { this.memoryUsage = memoryUsage; }

    public int getThreadCount() { return threadCount; }
    public void setThreadCount(int threadCount) { this.threadCount = threadCount; }
} 