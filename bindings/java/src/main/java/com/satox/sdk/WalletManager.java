package com.satox.sdk;

/**
 * Wallet Manager for the Satox SDK
 */
public class WalletManager {
    private final long handle;
    
    WalletManager(long handle) {
        this.handle = handle;
    }
    
    public String createWallet(String password) {
        return SatoxSDK.createWallet(handle, password);
    }
    
    public int importWallet(String walletData, String password) {
        return SatoxSDK.importWallet(handle, walletData, password);
    }
    
    public String exportWallet(String password) {
        return SatoxSDK.exportWallet(handle, password);
    }
    
    public String getBalance(String address) {
        return SatoxSDK.getBalance(handle, address);
    }
    
    public String sendTransaction(String fromAddress, String toAddress, String assetId, long amount, String password) {
        return SatoxSDK.sendTransaction(handle, fromAddress, toAddress, assetId, amount, password);
    }
    
    public String getTransactionHistory(String address) {
        return SatoxSDK.getTransactionHistory(handle, address);
    }
    
    public String getAddress() {
        return SatoxSDK.getAddress(handle);
    }
    
    public boolean validateAddress(String address) {
        return SatoxSDK.validateAddress(handle, address);
    }
    
    long getHandle() {
        return handle;
    }
} 