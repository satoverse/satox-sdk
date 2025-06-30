package com.satox.sdk;

/**
 * Network Manager for the Satox SDK
 */
public class NetworkManager {
    private final long handle;
    
    NetworkManager(long handle) {
        this.handle = handle;
    }
    
    public int start() {
        return SatoxSDK.startNetwork(handle);
    }
    
    public int stop() {
        return SatoxSDK.stopNetwork(handle);
    }
    
    public String getPeers() {
        return SatoxSDK.getPeers(handle);
    }
    
    public String getStats() {
        return SatoxSDK.getNetworkStats(handle);
    }
    
    public int addPeer(String address, int port) {
        return SatoxSDK.addPeer(handle, address, port);
    }
    
    public int removePeer(String address, int port) {
        return SatoxSDK.removePeer(handle, address, port);
    }
    
    public int sendMessage(String address, int port, String message) {
        return SatoxSDK.sendMessage(handle, address, port, message);
    }
    
    public int broadcastMessage(String message) {
        return SatoxSDK.broadcastMessage(handle, message);
    }
    
    public int getConnectionCount() {
        return SatoxSDK.getConnectionCount(handle);
    }
    
    long getHandle() {
        return handle;
    }
} 