package com.satox.sdk;

/**
 * IPFS Manager for the Satox SDK
 */
public class IPFSManager {
    private final long handle;
    
    IPFSManager(long handle) {
        this.handle = handle;
    }
    
    public String uploadFile(String filePath) {
        return SatoxSDK.uploadFile(handle, filePath);
    }
    
    public String uploadData(byte[] data, String filename) {
        return SatoxSDK.uploadData(handle, data, filename);
    }
    
    public int downloadFile(String hash, String outputPath) {
        return SatoxSDK.downloadFile(handle, hash, outputPath);
    }
    
    public String getFileInfo(String hash) {
        return SatoxSDK.getFileInfo(handle, hash);
    }
    
    public int pinFile(String hash) {
        return SatoxSDK.pinFile(handle, hash);
    }
    
    public int unpinFile(String hash) {
        return SatoxSDK.unpinFile(handle, hash);
    }
    
    public String getGatewayUrl(String hash) {
        return SatoxSDK.getGatewayUrl(handle, hash);
    }
    
    public String listPinnedFiles() {
        return SatoxSDK.listPinnedFiles(handle);
    }
    
    public String getStorageStats() {
        return SatoxSDK.getStorageStats(handle);
    }
    
    long getHandle() {
        return handle;
    }
} 