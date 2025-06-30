package com.satox.sdk;

/**
 * Security Manager for the Satox SDK
 */
public class SecurityManager {
    private final long handle;
    
    SecurityManager(long handle) {
        this.handle = handle;
    }
    
    public String generateKeypair() {
        return SatoxSDK.generateKeypair(handle);
    }
    
    public String signData(String data, String privateKey) {
        return SatoxSDK.signData(handle, data, privateKey);
    }
    
    public boolean verifySignature(String data, String signature, String publicKey) {
        return SatoxSDK.verifySignature(handle, data, signature, publicKey);
    }
    
    public String sha256(String data) {
        return SatoxSDK.sha256(handle, data);
    }
    
    public String sha512(String data) {
        return SatoxSDK.sha512(handle, data);
    }
    
    public String encryptData(String data, String publicKey) {
        return SatoxSDK.encryptData(handle, data, publicKey);
    }
    
    public String decryptData(String encryptedData, String privateKey) {
        return SatoxSDK.decryptData(handle, encryptedData, privateKey);
    }
    
    public String generateRandomBytes(int length) {
        return SatoxSDK.generateRandomBytes(handle, length);
    }
    
    public String hashPassword(String password) {
        return SatoxSDK.hashPassword(handle, password);
    }
    
    public boolean verifyPassword(String password, String hash) {
        return SatoxSDK.verifyPassword(handle, password, hash);
    }
    
    long getHandle() {
        return handle;
    }
} 