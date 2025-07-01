package com.satox.core;

public class CoreConfig {
    private String name = "satox_sdk";
    private boolean enableLogging = true;
    private String logLevel = "info";
    private String logPath;
    private int maxThreads = 4;
    private int timeout = 30;

    public CoreConfig() {}

    public String getName() { return name; }
    public void setName(String name) { this.name = name; }

    public boolean isEnableLogging() { return enableLogging; }
    public void setEnableLogging(boolean enableLogging) { this.enableLogging = enableLogging; }

    public String getLogLevel() { return logLevel; }
    public void setLogLevel(String logLevel) { this.logLevel = logLevel; }

    public String getLogPath() { return logPath; }
    public void setLogPath(String logPath) { this.logPath = logPath; }

    public int getMaxThreads() { return maxThreads; }
    public void setMaxThreads(int maxThreads) { this.maxThreads = maxThreads; }

    public int getTimeout() { return timeout; }
    public void setTimeout(int timeout) { this.timeout = timeout; }
} 