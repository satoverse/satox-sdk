package com.satox.blockchain;

import com.satox.transaction.Transaction;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.locks.ReentrantLock;
import java.util.Map;
import java.util.List;
import java.util.ArrayList;

public class BlockchainManager {
    private static BlockchainManager instance;
    private final ReentrantLock lock;
    private final Map<String, Node> nodes;
    private final NetworkManager networkManager;
    private boolean initialized;

    public BlockchainManager() {
        this.lock = new ReentrantLock();
        this.nodes = new ConcurrentHashMap<>();
        this.networkManager = new NetworkManager();
        this.initialized = false;
    }

    public static synchronized BlockchainManager getInstance() {
        if (instance == null) {
            instance = new BlockchainManager();
        }
        return instance;
    }

    public void initialize() {
        lock.lock();
        try {
            if (initialized) {
                throw new IllegalStateException("BlockchainManager already initialized");
            }
            networkManager.initialize();
            initialized = true;
        } finally {
            lock.unlock();
        }
    }

    public void shutdown() {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("BlockchainManager not initialized");
            }
            networkManager.shutdown();
            nodes.clear();
            initialized = false;
        } finally {
            lock.unlock();
        }
    }

    public void addNode(String nodeId, String address) {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("BlockchainManager not initialized");
            }
            Node node = new Node(nodeId, address);
            nodes.put(nodeId, node);
            networkManager.connect(node);
        } finally {
            lock.unlock();
        }
    }

    public void removeNode(String nodeId) {
        lock.lock();
        try {
            Node node = nodes.remove(nodeId);
            if (node != null) {
                networkManager.disconnect(node);
            }
        } finally {
            lock.unlock();
        }
    }

    public Node getNode(String nodeId) {
        lock.lock();
        try {
            Node node = nodes.get(nodeId);
            if (node == null) {
                throw new IllegalStateException("Node " + nodeId + " not found");
            }
            return node;
        } finally {
            lock.unlock();
        }
    }

    public List<Node> listNodes() {
        lock.lock();
        try {
            return new ArrayList<>(nodes.values());
        } finally {
            lock.unlock();
        }
    }

    public Block getLatestBlock() {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("BlockchainManager not initialized");
            }
            return networkManager.getLatestBlock();
        } finally {
            lock.unlock();
        }
    }

    public void broadcastTransaction(Transaction transaction) {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("BlockchainManager not initialized");
            }
            networkManager.broadcastTransaction(transaction);
        } finally {
            lock.unlock();
        }
    }

    public boolean isInitialized() {
        lock.lock();
        try {
            return initialized;
        } finally {
            lock.unlock();
        }
    }
} 