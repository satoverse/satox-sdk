package com.satox.examples;

import com.satox.sdk.*;
import com.satox.sdk.blockchain.*;
import com.satox.sdk.asset.*;
import com.satox.sdk.nft.*;
import com.satox.sdk.security.*;
import com.satox.sdk.quantum.*;
import com.satox.sdk.wallet.*;
import com.satox.sdk.transaction.*;
import com.satox.sdk.network.*;
import com.satox.sdk.database.*;
import com.satox.sdk.api.*;
import com.satox.sdk.core.*;
import com.satox.sdk.ipfs.*;
import com.satox.sdk.error.SatoxException;

import java.util.List;
import java.util.Map;
import java.util.HashMap;
import java.util.concurrent.CompletableFuture;

public class JavaExample {
    public static void main(String[] args) {
        System.out.println("🚀 Satox SDK Java Example Application");
        System.out.println("=====================================\n");

        try {
            // Initialize core manager
            System.out.println("1. Initializing Core Manager...");
            CoreManager coreManager = CoreManager.getInstance();
            coreManager.initialize();
            System.out.println("   ✅ Core Manager initialized successfully\n");

            // Initialize blockchain manager
            System.out.println("2. Initializing Blockchain Manager...");
            BlockchainManager blockchainManager = BlockchainManager.getInstance();
            blockchainManager.initialize();
            System.out.println("   ✅ Blockchain Manager initialized successfully\n");

            // Initialize security manager
            System.out.println("3. Initializing Security Manager...");
            SecurityManager securityManager = SecurityManager.getInstance();
            securityManager.initialize();
            System.out.println("   ✅ Security Manager initialized successfully\n");

            // Initialize quantum manager
            System.out.println("4. Initializing Quantum Manager...");
            QuantumManager quantumManager = QuantumManager.getInstance();
            quantumManager.initialize();
            System.out.println("   ✅ Quantum Manager initialized successfully\n");

            // Initialize wallet manager
            System.out.println("5. Initializing Wallet Manager...");
            WalletManager walletManager = WalletManager.getInstance();
            walletManager.initialize();
            System.out.println("   ✅ Wallet Manager initialized successfully\n");

            // Initialize asset manager
            System.out.println("6. Initializing Asset Manager...");
            AssetManager assetManager = AssetManager.getInstance();
            assetManager.initialize();
            System.out.println("   ✅ Asset Manager initialized successfully\n");

            // Initialize NFT manager
            System.out.println("7. Initializing NFT Manager...");
            NFTManager nftManager = NFTManager.getInstance();
            nftManager.initialize();
            System.out.println("   ✅ NFT Manager initialized successfully\n");

            // Initialize transaction manager
            System.out.println("8. Initializing Transaction Manager...");
            TransactionManager transactionManager = TransactionManager.getInstance();
            transactionManager.initialize();
            System.out.println("   ✅ Transaction Manager initialized successfully\n");

            // Initialize network manager
            System.out.println("9. Initializing Network Manager...");
            NetworkManager networkManager = NetworkManager.getInstance();
            networkManager.initialize();
            System.out.println("   ✅ Network Manager initialized successfully\n");

            // Initialize database manager
            System.out.println("10. Initializing Database Manager...");
            DatabaseManager databaseManager = DatabaseManager.getInstance();
            databaseManager.initialize();
            System.out.println("   ✅ Database Manager initialized successfully\n");

            // Initialize API manager
            System.out.println("11. Initializing API Manager...");
            APIManager apiManager = APIManager.getInstance();
            apiManager.initialize();
            System.out.println("   ✅ API Manager initialized successfully\n");

            // Initialize IPFS manager
            System.out.println("12. Initializing IPFS Manager...");
            IPFSManager ipfsManager = IPFSManager.getInstance();
            ipfsManager.initialize();
            System.out.println("   ✅ IPFS Manager initialized successfully\n");

            // Demonstrate wallet operations
            System.out.println("13. Wallet Operations Demo...");
            String walletAddress = walletManager.createWallet();
            System.out.println("   ✅ Created wallet: " + walletAddress);

            long balance = walletManager.getBalance(walletAddress);
            System.out.println("   ✅ Wallet balance: " + balance + " SATOX");
            System.out.println();

            // Demonstrate quantum security
            System.out.println("14. Quantum Security Demo...");
            QuantumKeyPair quantumKeyPair = quantumManager.generateKeyPair();
            System.out.println("   ✅ Generated quantum-resistant key pair");

            byte[] message = "Hello Quantum World!".getBytes();
            byte[] quantumSignature = quantumManager.sign(message, quantumKeyPair.getPrivateKey());
            System.out.println("   ✅ Created quantum-resistant signature");

            boolean isValid = quantumManager.verify(message, quantumSignature, quantumKeyPair.getPublicKey());
            System.out.println("   ✅ Quantum signature verification: " + isValid);
            System.out.println();

            // Demonstrate asset operations
            System.out.println("15. Asset Operations Demo...");
            Map<String, Object> metadata = new HashMap<>();
            metadata.put("website", "https://example.com");
            metadata.put("category", "utility");

            AssetConfig assetConfig = new AssetConfig();
            assetConfig.setName("Test Token");
            assetConfig.setSymbol("TEST");
            assetConfig.setTotalSupply(1000000L);
            assetConfig.setDecimals(8);
            assetConfig.setDescription("A test token for demonstration");
            assetConfig.setMetadata(metadata);

            String assetId = assetManager.createAsset(assetConfig);
            System.out.println("   ✅ Created asset with ID: " + assetId);

            AssetInfo assetInfo = assetManager.getAssetInfo(assetId);
            System.out.println("   ✅ Asset info: " + assetInfo.getName() + " (" + assetInfo.getSymbol() + ")");
            System.out.println();

            // Demonstrate NFT operations
            System.out.println("16. NFT Operations Demo...");
            Map<String, Object> nftMetadata = new HashMap<>();
            nftMetadata.put("image", "https://example.com/image.png");
            
            Map<String, Object> attributes = new HashMap<>();
            attributes.put("rarity", "common");
            attributes.put("power", 100);
            nftMetadata.put("attributes", attributes);

            NFTConfig nftConfig = new NFTConfig();
            nftConfig.setName("Test NFT");
            nftConfig.setSymbol("TNFT");
            nftConfig.setDescription("A test NFT for demonstration");
            nftConfig.setMetadata(nftMetadata);

            String nftId = nftManager.createNFT(nftConfig);
            System.out.println("   ✅ Created NFT with ID: " + nftId);

            NFTInfo nftInfo = nftManager.getNFTInfo(nftId);
            System.out.println("   ✅ NFT info: " + nftInfo.getName() + " (" + nftInfo.getSymbol() + ")");
            System.out.println();

            // Demonstrate transaction operations
            System.out.println("17. Transaction Operations Demo...");
            TransactionConfig txConfig = new TransactionConfig();
            txConfig.setFrom(walletAddress);
            txConfig.setTo("recipient_address");
            txConfig.setAmount(1000L);
            txConfig.setAssetId(assetId);
            txConfig.setFee(100L);

            String txId = transactionManager.createTransaction(txConfig);
            System.out.println("   ✅ Created transaction with ID: " + txId);

            String txStatus = transactionManager.getTransactionStatus(txId);
            System.out.println("   ✅ Transaction status: " + txStatus);
            System.out.println();

            // Demonstrate blockchain operations
            System.out.println("18. Blockchain Operations Demo...");
            BlockchainInfo blockchainInfo = blockchainManager.getBlockchainInfo();
            System.out.println("   ✅ Blockchain: " + blockchainInfo.getName() + " (Height: " + blockchainInfo.getCurrentHeight() + ")");

            Block latestBlock = blockchainManager.getLatestBlock();
            System.out.println("   ✅ Latest block: " + latestBlock.getHash() + " with " + latestBlock.getTransactions().size() + " transactions");
            System.out.println();

            // Demonstrate network operations
            System.out.println("19. Network Operations Demo...");
            NetworkInfo networkInfo = networkManager.getNetworkInfo();
            System.out.println("   ✅ Network: " + networkInfo.getConnections() + " connections");

            List<Peer> peers = networkManager.getPeers();
            System.out.println("   ✅ Connected peers: " + peers.size());
            System.out.println();

            // Demonstrate IPFS operations
            System.out.println("20. IPFS Operations Demo...");
            byte[] data = "Hello IPFS World!".getBytes();
            String ipfsHash = ipfsManager.uploadData(data);
            System.out.println("   ✅ Uploaded data to IPFS: " + ipfsHash);

            byte[] retrievedData = ipfsManager.downloadData(ipfsHash);
            System.out.println("   ✅ Retrieved data from IPFS: " + retrievedData.length + " bytes");
            System.out.println();

            // Demonstrate database operations
            System.out.println("21. Database Operations Demo...");
            DatabaseInfo dbInfo = databaseManager.getDatabaseInfo();
            System.out.println("   ✅ Database: " + dbInfo.getTableCount() + " tables");

            DatabaseStats dbStats = databaseManager.getDatabaseStats();
            System.out.println("   ✅ Database size: " + dbStats.getSizeMB() + " MB");
            System.out.println();

            // Demonstrate API operations
            System.out.println("22. API Operations Demo...");
            APIInfo apiInfo = apiManager.getAPIInfo();
            System.out.println("   ✅ API: " + apiInfo.getEndpointCount() + " endpoints available");

            APIStats apiStats = apiManager.getAPIStats();
            System.out.println("   ✅ API requests: " + apiStats.getTotalRequests() + " total");
            System.out.println();

            // Demonstrate security operations
            System.out.println("23. Security Operations Demo...");
            SecurityInfo securityInfo = securityManager.getSecurityInfo();
            System.out.println("   ✅ Security: " + securityInfo.getAlgorithmCount() + " algorithms supported");

            byte[] encryptionKey = securityManager.generateEncryptionKey();
            System.out.println("   ✅ Generated encryption key: " + encryptionKey.length + " bytes");
            System.out.println();

            // Demonstrate batch operations
            System.out.println("24. Batch Operations Demo...");
            TransactionConfig tx1 = new TransactionConfig();
            tx1.setFrom(walletAddress);
            tx1.setTo("recipient1");
            tx1.setAmount(100L);
            tx1.setAssetId(assetId);
            tx1.setFee(10L);

            TransactionConfig tx2 = new TransactionConfig();
            tx2.setFrom(walletAddress);
            tx2.setTo("recipient2");
            tx2.setAmount(200L);
            tx2.setAssetId(assetId);
            tx2.setFee(10L);

            BatchTransactionConfig batchConfig = new BatchTransactionConfig();
            batchConfig.setTransactions(List.of(tx1, tx2));

            String batchId = transactionManager.createBatchTransaction(batchConfig);
            System.out.println("   ✅ Created batch transaction with ID: " + batchId);
            System.out.println();

            // Demonstrate error handling
            System.out.println("25. Error Handling Demo...");
            try {
                transactionManager.getTransactionStatus("invalid_tx_id");
                System.out.println("   ❌ Unexpected success");
            } catch (SatoxException e) {
                System.out.println("   ✅ Properly handled error: " + e.getMessage());
            }
            System.out.println();

            // Demonstrate async operations
            System.out.println("26. Async Operations Demo...");
            CompletableFuture<String> asyncWallet = CompletableFuture.supplyAsync(() -> {
                try {
                    return walletManager.createWallet();
                } catch (SatoxException e) {
                    throw new RuntimeException(e);
                }
            });

            CompletableFuture<Long> asyncBalance = asyncWallet.thenApplyAsync(address -> {
                try {
                    return walletManager.getBalance(address);
                } catch (SatoxException e) {
                    throw new RuntimeException(e);
                }
            });

            String asyncWalletAddress = asyncWallet.get();
            long asyncWalletBalance = asyncBalance.get();
            System.out.println("   ✅ Async wallet created: " + asyncWalletAddress + " with balance: " + asyncWalletBalance);
            System.out.println();

            // Demonstrate cleanup
            System.out.println("27. Cleanup Operations...");
            apiManager.shutdown();
            databaseManager.shutdown();
            ipfsManager.shutdown();
            networkManager.shutdown();
            transactionManager.shutdown();
            nftManager.shutdown();
            assetManager.shutdown();
            walletManager.shutdown();
            quantumManager.shutdown();
            securityManager.shutdown();
            blockchainManager.shutdown();
            coreManager.shutdown();
            System.out.println("   ✅ All managers shut down successfully\n");

            System.out.println("🎉 Java Example Application Completed Successfully!");
            System.out.println("All SDK features demonstrated and working correctly.");

        } catch (Exception e) {
            System.err.println("❌ Error in Java example: " + e.getMessage());
            e.printStackTrace();
        }
    }

    // Test methods for demonstration
    public static void testCoreInitialization() throws SatoxException {
        CoreManager coreManager = CoreManager.getInstance();
        coreManager.initialize();
        coreManager.shutdown();
    }

    public static void testQuantumOperations() throws SatoxException {
        QuantumManager quantumManager = QuantumManager.getInstance();
        quantumManager.initialize();

        QuantumKeyPair keyPair = quantumManager.generateKeyPair();
        byte[] message = "Test message".getBytes();
        byte[] signature = quantumManager.sign(message, keyPair.getPrivateKey());
        boolean isValid = quantumManager.verify(message, signature, keyPair.getPublicKey());

        if (!isValid) {
            throw new SatoxException("Quantum signature verification failed");
        }

        quantumManager.shutdown();
    }

    public static void testAssetOperations() throws SatoxException {
        AssetManager assetManager = AssetManager.getInstance();
        assetManager.initialize();

        AssetConfig config = new AssetConfig();
        config.setName("Test Asset");
        config.setSymbol("TEST");
        config.setTotalSupply(1000000L);
        config.setDecimals(8);
        config.setDescription("Test asset");
        config.setMetadata(new HashMap<>());

        String assetId = assetManager.createAsset(config);
        AssetInfo assetInfo = assetManager.getAssetInfo(assetId);

        if (!"Test Asset".equals(assetInfo.getName())) {
            throw new SatoxException("Unexpected asset name: " + assetInfo.getName());
        }

        assetManager.shutdown();
    }

    public static void testConcurrentOperations() throws SatoxException {
        // Test concurrent wallet operations
        CompletableFuture<String> wallet1 = CompletableFuture.supplyAsync(() -> {
            try {
                WalletManager wm = WalletManager.getInstance();
                wm.initialize();
                String address = wm.createWallet();
                wm.shutdown();
                return address;
            } catch (SatoxException e) {
                throw new RuntimeException(e);
            }
        });

        CompletableFuture<String> wallet2 = CompletableFuture.supplyAsync(() -> {
            try {
                WalletManager wm = WalletManager.getInstance();
                wm.initialize();
                String address = wm.createWallet();
                wm.shutdown();
                return address;
            } catch (SatoxException e) {
                throw new RuntimeException(e);
            }
        });

        try {
            String address1 = wallet1.get();
            String address2 = wallet2.get();
            System.out.println("   ✅ Concurrent wallets created: " + address1 + ", " + address2);
        } catch (Exception e) {
            throw new SatoxException("Concurrent operations failed: " + e.getMessage());
        }
    }
} 