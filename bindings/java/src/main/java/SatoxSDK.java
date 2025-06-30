/**
 * @file SatoxSDK.java
 * @brief Main Java binding for the Satox SDK
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

package com.satox.sdk;

import com.satox.sdk.core.*;
import com.satox.sdk.wallet.*;
import com.satox.sdk.security.*;
import com.satox.sdk.asset.*;
import com.satox.sdk.nft.*;
import com.satox.sdk.blockchain.*;
import com.satox.sdk.ipfs.*;
import com.satox.sdk.network.*;
import com.satox.sdk.database.*;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.locks.ReentrantReadWriteLock;
import java.util.concurrent.locks.ReadWriteLock;

/**
 * Main SDK class for Satox blockchain operations
 */
public class SatoxSDK implements AutoCloseable {
    
    private CoreManager coreManager;
    private WalletManager walletManager;
    private SecurityManager securityManager;
    private AssetManager assetManager;
    private NFTManager nftManager;
    private BlockchainManager blockchainManager;
    private IPFSManager ipfsManager;
    private NetworkManager networkManager;
    private DatabaseManager databaseManager;
    
    private SDKConfig config;
    private final AtomicBoolean isRunning;
    private final ReadWriteLock lock;
    
    static {
        System.loadLibrary("satox-sdk");
    }
    
    /**
     * Default constructor with default configuration
     */
    public SatoxSDK() {
        this(new SDKConfig());
    }
    
    /**
     * Constructor with custom configuration
     * 
     * @param config The SDK configuration
     */
    public SatoxSDK(SDKConfig config) {
        this.config = config != null ? config : new SDKConfig();
        this.isRunning = new AtomicBoolean(false);
        this.lock = new ReentrantReadWriteLock();
    }
    
    /**
     * Initialize the SDK with the current configuration
     * 
     * @return CompletableFuture that completes when initialization is done
     */
    public CompletableFuture<Void> initialize() {
        return CompletableFuture.runAsync(() -> {
            lock.writeLock().lock();
            try {
                if (isRunning.get()) {
                    throw new IllegalStateException("SDK is already running");
                }
                
                // Initialize core manager
                coreManager = new CoreManager();
                coreManager.initialize(config);
                
                // Initialize wallet manager
                walletManager = new WalletManager();
                walletManager.initialize(config);
                
                // Initialize security manager
                securityManager = new SecurityManager();
                securityManager.initialize(config);
                
                // Initialize asset manager
                assetManager = new AssetManager();
                assetManager.initialize(config);
                
                // Initialize NFT manager
                nftManager = new NFTManager();
                nftManager.initialize(config);
                
                // Initialize blockchain manager
                blockchainManager = new BlockchainManager();
                blockchainManager.initialize(config);
                
                // Initialize IPFS manager
                ipfsManager = new IPFSManager();
                ipfsManager.initialize(config);
                
                // Initialize network manager
                networkManager = new NetworkManager();
                networkManager.initialize(config);
                
                // Initialize database manager
                databaseManager = new DatabaseManager();
                databaseManager.initialize(config);
                
                isRunning.set(true);
                
            } finally {
                lock.writeLock().unlock();
            }
        });
    }
    
    /**
     * Start all SDK services
     * 
     * @return CompletableFuture that completes when all services are started
     */
    public CompletableFuture<Void> start() {
        return CompletableFuture.runAsync(() -> {
            lock.readLock().lock();
            try {
                if (!isRunning.get()) {
                    throw new IllegalStateException("SDK is not initialized");
                }
                
                // Start all managers
                coreManager.start();
                walletManager.start();
                securityManager.start();
                assetManager.start();
                nftManager.start();
                blockchainManager.start();
                ipfsManager.start();
                networkManager.start();
                databaseManager.start();
                
            } finally {
                lock.readLock().unlock();
            }
        });
    }
    
    /**
     * Stop all SDK services
     * 
     * @return CompletableFuture that completes when all services are stopped
     */
    public CompletableFuture<Void> stop() {
        return CompletableFuture.runAsync(() -> {
            lock.writeLock().lock();
            try {
                if (!isRunning.get()) {
                    return;
                }
                
                // Stop all managers
                if (coreManager != null) coreManager.stop();
                if (walletManager != null) walletManager.stop();
                if (securityManager != null) securityManager.stop();
                if (assetManager != null) assetManager.stop();
                if (nftManager != null) nftManager.stop();
                if (blockchainManager != null) blockchainManager.stop();
                if (ipfsManager != null) ipfsManager.stop();
                if (networkManager != null) networkManager.stop();
                if (databaseManager != null) databaseManager.stop();
                
                isRunning.set(false);
                
            } finally {
                lock.writeLock().unlock();
            }
        });
    }
    
    /**
     * Connect to the blockchain network
     * 
     * @return CompletableFuture that completes when connected
     */
    public CompletableFuture<Void> connect() {
        return CompletableFuture.runAsync(() -> {
            lock.readLock().lock();
            try {
                if (!isRunning.get()) {
                    throw new IllegalStateException("SDK is not running");
                }
                networkManager.connect();
            } finally {
                lock.readLock().unlock();
            }
        });
    }
    
    /**
     * Disconnect from the blockchain network
     * 
     * @return CompletableFuture that completes when disconnected
     */
    public CompletableFuture<Void> disconnect() {
        return CompletableFuture.runAsync(() -> {
            lock.readLock().lock();
            try {
                if (!isRunning.get()) {
                    return;
                }
                networkManager.disconnect();
            } finally {
                lock.readLock().unlock();
            }
        });
    }
    
    /**
     * Check if the SDK is running
     * 
     * @return true if running, false otherwise
     */
    public boolean isRunning() {
        lock.readLock().lock();
        try {
            return isRunning.get();
        } finally {
            lock.readLock().unlock();
        }
    }
    
    /**
     * Get the core manager
     * 
     * @return The core manager instance
     */
    public CoreManager getCoreManager() {
        return coreManager;
    }
    
    /**
     * Get the wallet manager
     * 
     * @return The wallet manager instance
     */
    public WalletManager getWalletManager() {
        return walletManager;
    }
    
    /**
     * Get the security manager
     * 
     * @return The security manager instance
     */
    public SecurityManager getSecurityManager() {
        return securityManager;
    }
    
    /**
     * Get the asset manager
     * 
     * @return The asset manager instance
     */
    public AssetManager getAssetManager() {
        return assetManager;
    }
    
    /**
     * Get the NFT manager
     * 
     * @return The NFT manager instance
     */
    public NFTManager getNFTManager() {
        return nftManager;
    }
    
    /**
     * Get the blockchain manager
     * 
     * @return The blockchain manager instance
     */
    public BlockchainManager getBlockchainManager() {
        return blockchainManager;
    }
    
    /**
     * Get the IPFS manager
     * 
     * @return The IPFS manager instance
     */
    public IPFSManager getIPFSManager() {
        return ipfsManager;
    }
    
    /**
     * Get the network manager
     * 
     * @return The network manager instance
     */
    public NetworkManager getNetworkManager() {
        return networkManager;
    }
    
    /**
     * Get the database manager
     * 
     * @return The database manager instance
     */
    public DatabaseManager getDatabaseManager() {
        return databaseManager;
    }
    
    /**
     * Get the current configuration
     * 
     * @return The SDK configuration
     */
    public SDKConfig getConfig() {
        return config;
    }
    
    /**
     * Get the SDK version
     * 
     * @return The version string
     */
    public static String getVersion() {
        return "1.0.0";
    }
    
    /**
     * Get the SDK build date
     * 
     * @return The build date string
     */
    public static String getBuildDate() {
        return "2025-06-30";
    }
    
    /**
     * Close the SDK (alias for stop)
     */
    @Override
    public void close() {
        stop().join();
    }
    
    /**
     * Shutdown the SDK gracefully
     * 
     * @return CompletableFuture that completes when shutdown is done
     */
    public CompletableFuture<Void> shutdown() {
        return stop();
    }
} 