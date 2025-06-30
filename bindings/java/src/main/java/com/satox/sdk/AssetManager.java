package com.satox.sdk;

/**
 * Asset Manager for the Satox SDK
 */
public class AssetManager {
    private final long handle;
    
    AssetManager(long handle) {
        this.handle = handle;
    }
    
    public String createAsset(String config, String ownerAddress) {
        return SatoxSDK.createAsset(handle, config, ownerAddress);
    }
    
    public String getAssetInfo(String assetId) {
        return SatoxSDK.getAssetInfo(handle, assetId);
    }
    
    public String transferAsset(String assetId, String fromAddress, String toAddress, long amount) {
        return SatoxSDK.transferAsset(handle, assetId, fromAddress, toAddress, amount);
    }
    
    public long getAssetBalance(String assetId, String address) {
        return SatoxSDK.getAssetBalance(handle, assetId, address);
    }
    
    public String reissueAsset(String assetId, String ownerAddress, long amount) {
        return SatoxSDK.reissueAsset(handle, assetId, ownerAddress, amount);
    }
    
    public String burnAsset(String assetId, String ownerAddress, long amount) {
        return SatoxSDK.burnAsset(handle, assetId, ownerAddress, amount);
    }
    
    public String listAssets() {
        return SatoxSDK.listAssets(handle);
    }
    
    public String getAssetTransactions(String assetId) {
        return SatoxSDK.getAssetTransactions(handle, assetId);
    }
    
    long getHandle() {
        return handle;
    }
} 