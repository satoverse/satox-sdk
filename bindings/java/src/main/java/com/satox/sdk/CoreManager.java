package com.satox.sdk;

/**
 * Core Manager for the Satox SDK
 * 
 * Manages the core functionality of the SDK including initialization,
 * configuration, and status monitoring.
 */
public class CoreManager {
    private final long handle;
    
    CoreManager(long handle) {
        this.handle = handle;
    }
    
    /**
     * Initialize the core manager
     * 
     * @throws SatoxException if initialization fails
     */
    public void initialize() throws SatoxException {
        int result = SatoxSDK.initializeCoreManager(handle);
        if (result != 0) {
            throw new SatoxException("Failed to initialize core manager");
        }
    }
    
    /**
     * Shutdown the core manager
     */
    public void shutdown() {
        SatoxSDK.shutdownCoreManager(handle);
    }
    
    /**
     * Get the status of the core manager
     * 
     * @return Status JSON string
     */
    public String getStatus() {
        return SatoxSDK.getCoreManagerStatus(handle);
    }
    
    /**
     * Get the configuration of the core manager
     * 
     * @return Configuration JSON string
     */
    public String getConfig() {
        return SatoxSDK.getCoreManagerConfig(handle);
    }
    
    /**
     * Set the configuration of the core manager
     * 
     * @param config Configuration JSON string
     * @throws SatoxException if setting config fails
     */
    public void setConfig(String config) throws SatoxException {
        int result = SatoxSDK.setCoreManagerConfig(handle, config);
        if (result != 0) {
            throw new SatoxException("Failed to set core manager config");
        }
    }
    
    /**
     * Get the native handle
     * 
     * @return Native handle
     */
    long getHandle() {
        return handle;
    }
} 