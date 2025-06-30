package com.satox;

import com.satox.asset.AssetManager;
import com.satox.ipfs.IPFSManager;

public class SatoxSDK {
    private static SatoxSDK instance = new SatoxSDK();
    private AssetManager assetManager = AssetManager.getInstance();
    private IPFSManager ipfsManager = new IPFSManager();
    private Object transactionManager = new Object(); // Stub
    private SatoxSDK() {}
    public static SatoxSDK getInstance() { return instance; }
    public void initialize() {}
    public void shutdown() {}
    public AssetManager getAssetManager() { return assetManager; }
    public IPFSManager getIPFSManager() { return ipfsManager; }
    public Object getTransactionManager() { return transactionManager; }
}
