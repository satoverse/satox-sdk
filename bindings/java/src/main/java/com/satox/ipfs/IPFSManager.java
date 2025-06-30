package com.satox.ipfs;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.UUID;

public class IPFSManager {
    private final Map<String, byte[]> storage = new ConcurrentHashMap<>();
    public IPFSManager() {}
    public void initialize() {}
    public void shutdown() {}

    public String addFile(String filename, byte[] content) {
        String hash = UUID.randomUUID().toString();
        storage.put(hash, content);
        return hash;
    }
    public byte[] getFile(String hash) {
        return storage.getOrDefault(hash, new byte[0]);
    }
}
