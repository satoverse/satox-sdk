package com.satox.sdk;

/**
 * Blockchain Manager for the Satox SDK
 */
public class BlockchainManager {
    private final long handle;
    
    BlockchainManager(long handle) {
        this.handle = handle;
    }
    
    public long getBlockHeight() {
        return SatoxSDK.getBlockHeight(handle);
    }
    
    public String getBlockInfo(String hash) {
        return SatoxSDK.getBlockInfo(handle, hash);
    }
    
    public String getTransactionInfo(String txid) {
        return SatoxSDK.getTransactionInfo(handle, txid);
    }
    
    public String getBestBlockHash() {
        return SatoxSDK.getBestBlockHash(handle);
    }
    
    public String getBlockHash(long height) {
        return SatoxSDK.getBlockHash(handle, height);
    }
    
    public double getDifficulty() {
        return SatoxSDK.getDifficulty(handle);
    }
    
    public double getNetworkHashrate() {
        return SatoxSDK.getNetworkHashrate(handle);
    }
    
    public boolean validateTransaction(String txid) {
        return SatoxSDK.validateTransaction(handle, txid);
    }
    
    long getHandle() {
        return handle;
    }
} 