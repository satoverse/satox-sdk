package com.satox.sdk;

/**
 * Main entry point for the Satox SDK Java bindings
 * 
 * This class provides Java bindings to the C++ Satox SDK, allowing Java applications
 * to interact with the Satoxcoin blockchain, manage assets, NFTs, and IPFS content.
 */
public class SatoxSDK {
    static {
        try {
            System.loadLibrary("satox-sdk-java");
        } catch (UnsatisfiedLinkError e) {
            throw new RuntimeException("Failed to load Satox SDK native library", e);
        }
    }

    // Native method declarations
    private static native int initializeSDK();
    private static native void shutdownSDK();
    private static native String getSDKVersion();
    
    // Core Manager native methods
    public static native long nativeCreateCoreManager(String config);
    public static native void destroyCoreManager(long handle);
    public static native int initializeCoreManager(long handle);
    public static native void shutdownCoreManager(long handle);
    public static native String getCoreManagerStatus(long handle);
    public static native String getCoreManagerConfig(long handle);
    public static native int setCoreManagerConfig(long handle, String config);
    
    // Wallet Manager native methods
    public static native long nativeCreateWalletManager(String config);
    public static native void destroyWalletManager(long handle);
    public static native String createWallet(long handle, String password);
    public static native int importWallet(long handle, String walletData, String password);
    public static native String exportWallet(long handle, String password);
    public static native String getBalance(long handle, String address);
    public static native String sendTransaction(long handle, String fromAddress, String toAddress, 
                                                String assetId, long amount, String password);
    public static native String getTransactionHistory(long handle, String address);
    public static native String getAddress(long handle);
    public static native boolean validateAddress(long handle, String address);
    
    // Asset Manager native methods
    public static native long nativeCreateAssetManager(String config);
    public static native void destroyAssetManager(long handle);
    public static native String createAsset(long handle, String config, String ownerAddress);
    public static native String getAssetInfo(long handle, String assetId);
    public static native String transferAsset(long handle, String assetId, String fromAddress, 
                                              String toAddress, long amount);
    public static native long getAssetBalance(long handle, String assetId, String address);
    public static native String reissueAsset(long handle, String assetId, String ownerAddress, long amount);
    public static native String burnAsset(long handle, String assetId, String ownerAddress, long amount);
    public static native String listAssets(long handle);
    public static native String getAssetTransactions(long handle, String assetId);
    
    // Blockchain Manager native methods
    public static native long nativeCreateBlockchainManager(String config);
    public static native void destroyBlockchainManager(long handle);
    public static native long getBlockHeight(long handle);
    public static native String getBlockInfo(long handle, String hash);
    public static native String getTransactionInfo(long handle, String txid);
    public static native String getBestBlockHash(long handle);
    public static native String getBlockHash(long handle, long height);
    public static native double getDifficulty(long handle);
    public static native double getNetworkHashrate(long handle);
    public static native boolean validateTransaction(long handle, String txid);
    
    // Database Manager native methods
    public static native long nativeCreateDatabaseManager(String config);
    public static native void destroyDatabaseManager(long handle);
    public static native int createDatabase(long handle, String name);
    public static native int deleteDatabase(long handle, String name);
    public static native String listDatabases(long handle);
    public static native int createCollection(long handle, String dbName, String collectionName);
    public static native int deleteCollection(long handle, String dbName, String collectionName);
    public static native String insertDocument(long handle, String dbName, String collectionName, String document);
    public static native int updateDocument(long handle, String dbName, String collectionName, 
                                           String query, String update);
    public static native int deleteDocument(long handle, String dbName, String collectionName, String query);
    public static native String findDocuments(long handle, String dbName, String collectionName, String query);
    public static native String getDocument(long handle, String dbName, String collectionName, String id);
    
    // Security Manager native methods
    public static native long nativeCreateSecurityManager();
    public static native void destroySecurityManager(long handle);
    public static native String generateKeypair(long handle);
    public static native String signData(long handle, String data, String privateKey);
    public static native boolean verifySignature(long handle, String data, String signature, String publicKey);
    public static native String sha256(long handle, String data);
    public static native String sha512(long handle, String data);
    public static native String encryptData(long handle, String data, String publicKey);
    public static native String decryptData(long handle, String encryptedData, String privateKey);
    public static native String generateRandomBytes(long handle, int length);
    public static native String hashPassword(long handle, String password);
    public static native boolean verifyPassword(long handle, String password, String hash);
    
    // Network Manager native methods
    public static native long nativeCreateNetworkManager(String config);
    public static native void destroyNetworkManager(long handle);
    public static native int startNetwork(long handle);
    public static native int stopNetwork(long handle);
    public static native String getPeers(long handle);
    public static native String getNetworkStats(long handle);
    public static native int addPeer(long handle, String address, int port);
    public static native int removePeer(long handle, String address, int port);
    public static native int sendMessage(long handle, String address, int port, String message);
    public static native int broadcastMessage(long handle, String message);
    public static native int getConnectionCount(long handle);
    
    // IPFS Manager native methods
    public static native long nativeCreateIPFSManager(String config);
    public static native void destroyIPFSManager(long handle);
    public static native String uploadFile(long handle, String filePath);
    public static native String uploadData(long handle, byte[] data, String filename);
    public static native int downloadFile(long handle, String hash, String outputPath);
    public static native String getFileInfo(long handle, String hash);
    public static native int pinFile(long handle, String hash);
    public static native int unpinFile(long handle, String hash);
    public static native String getGatewayUrl(long handle, String hash);
    public static native String listPinnedFiles(long handle);
    public static native String getStorageStats(long handle);

    /**
     * Initialize the Satox SDK
     * 
     * @throws SatoxException if initialization fails
     */
    public static void initialize() throws SatoxException {
        int result = initializeSDK();
        if (result != 0) {
            throw new SatoxException("Failed to initialize Satox SDK");
        }
    }

    /**
     * Shutdown the Satox SDK
     */
    public static void shutdown() {
        shutdownSDK();
    }

    /**
     * Get the SDK version
     * 
     * @return SDK version string
     */
    public static String getVersion() {
        return getSDKVersion();
    }

    /**
     * Create a new Core Manager instance
     * 
     * @param config Configuration JSON string
     * @return Core Manager instance
     */
    public static CoreManager createCoreManager(String config) {
        long handle = nativeCreateCoreManager(config);
        return new CoreManager(handle);
    }

    /**
     * Create a new Wallet Manager instance
     * 
     * @param config Configuration JSON string
     * @return Wallet Manager instance
     */
    public static WalletManager createWalletManager(String config) {
        long handle = nativeCreateWalletManager(config);
        return new WalletManager(handle);
    }

    /**
     * Create a new Asset Manager instance
     * 
     * @param config Configuration JSON string
     * @return Asset Manager instance
     */
    public static AssetManager createAssetManager(String config) {
        long handle = nativeCreateAssetManager(config);
        return new AssetManager(handle);
    }

    /**
     * Create a new Blockchain Manager instance
     * 
     * @param config Configuration JSON string
     * @return Blockchain Manager instance
     */
    public static BlockchainManager createBlockchainManager(String config) {
        long handle = nativeCreateBlockchainManager(config);
        return new BlockchainManager(handle);
    }

    /**
     * Create a new Database Manager instance
     * 
     * @param config Configuration JSON string
     * @return Database Manager instance
     */
    public static DatabaseManager createDatabaseManager(String config) {
        long handle = nativeCreateDatabaseManager(config);
        return new DatabaseManager(handle);
    }

    /**
     * Create a new Security Manager instance
     * 
     * @return Security Manager instance
     */
    public static SecurityManager createSecurityManager() {
        long handle = nativeCreateSecurityManager();
        return new SecurityManager(handle);
    }

    /**
     * Create a new Network Manager instance
     * 
     * @param config Configuration JSON string
     * @return Network Manager instance
     */
    public static NetworkManager createNetworkManager(String config) {
        long handle = nativeCreateNetworkManager(config);
        return new NetworkManager(handle);
    }

    /**
     * Create a new IPFS Manager instance
     * 
     * @param config Configuration JSON string
     * @return IPFS Manager instance
     */
    public static IPFSManager createIPFSManager(String config) {
        long handle = nativeCreateIPFSManager(config);
        return new IPFSManager(handle);
    }
} 