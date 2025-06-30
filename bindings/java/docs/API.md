# Satox SDK Java API Documentation

## Table of Contents
1. [Getting Started](#getting-started)
2. [Core Components](#core-components)
3. [Blockchain Integration](#blockchain-integration)
4. [Asset Management](#asset-management)
5. [NFT Management](#nft-management)
6. [Security](#security)
7. [Examples](#examples)

## Getting Started

### Installation
Add the Satox SDK to your project using Maven:

```xml
<dependency>
    <groupId>com.satox</groupId>
    <artifactId>satox-sdk</artifactId>
    <version>1.0.0</version>
</dependency>
```

### Basic Usage
```java
import com.satox.sdk.SatoxSDK;
import com.satox.sdk.blockchain.BlockchainManager;
import com.satox.sdk.asset.AssetManager;
import com.satox.sdk.nft.NFTManager;

public class SatoxExample {
    public static void main(String[] args) {
        // Initialize SDK
        SatoxSDK sdk = SatoxSDK.getInstance();
        sdk.initialize();

        // Get managers
        BlockchainManager blockchainManager = sdk.getBlockchainManager();
        AssetManager assetManager = sdk.getAssetManager();
        NFTManager nftManager = sdk.getNFTManager();

        // Use managers...
    }
}
```

## Core Components

### SatoxSDK
The main entry point for the SDK.

```java
public class SatoxSDK {
    // Get singleton instance
    public static SatoxSDK getInstance();

    // Initialize SDK with configuration
    public boolean initialize(SDKConfig config);

    // Shutdown SDK
    public void shutdown();

    // Get component managers
    public BlockchainManager getBlockchainManager();
    public AssetManager getAssetManager();
    public NFTManager getNFTManager();
    public SecurityManager getSecurityManager();
    public IPFSManager getIPFSManager();
}
```

### SDKConfig
Configuration for the SDK.

```java
public class SDKConfig {
    private String network;        // "mainnet" or "testnet"
    private String dataDir;        // Data directory path
    private boolean enableMining;  // Enable mining
    private int miningThreads;     // Number of mining threads
    private boolean enableSync;    // Enable blockchain sync
    private int syncInterval;      // Sync interval in milliseconds
    private String rpcEndpoint;    // RPC endpoint URL
    private String rpcUsername;    // RPC username
    private String rpcPassword;    // RPC password
    private int timeout;           // Timeout in milliseconds
}
```

## Blockchain Integration

### BlockchainManager
Manages blockchain operations.

```java
public class BlockchainManager {
    // Get blockchain info
    public BlockchainInfo getBlockchainInfo();

    // Get block by hash
    public Block getBlock(String hash);

    // Get block by height
    public Block getBlock(int height);

    // Get transaction info
    public Transaction getTransaction(String txid);

    // Send raw transaction
    public String sendRawTransaction(String rawTx);

    // Create raw transaction
    public String createRawTransaction(List<Input> inputs, List<Output> outputs);

    // Sign raw transaction
    public String signRawTransaction(String rawTx, List<String> privateKeys);
}
```

## Asset Management

### AssetManager
Manages asset operations.

```java
public class AssetManager {
    // Create asset
    public String createAsset(AssetMetadata metadata);

    // Get asset info
    public Asset getAsset(String assetId);

    // Update asset
    public boolean updateAsset(String assetId, AssetMetadata metadata);

    // Transfer asset
    public boolean transferAsset(String assetId, String from, String to, long amount);

    // Get asset balance
    public long getBalance(String address, String assetId);

    // Get asset history
    public List<AssetEvent> getAssetHistory(String assetId);
}
```

## NFT Management

### NFTManager
Manages NFT operations.

```java
public class NFTManager {
    // Create NFT
    public String createNFT(NFTMetadata metadata);

    // Get NFT info
    public NFT getNFT(String nftId);

    // Transfer NFT
    public boolean transferNFT(String nftId, String from, String to);

    // Get NFT owner
    public String getNFTOwner(String nftId);

    // Get NFT collection
    public List<NFT> getNFTCollection(String collectionId);
}
```

## Security

### SecurityManager
Manages security features.

```java
public class SecurityManager {
    // Initialize security manager
    public void initialize();

    // Shutdown security manager
    public void shutdown();

    // Set security policy
    public void setPolicy(SecurityPolicy policy);

    // Get security status
    public SecurityStatus getStatus();

    // Enable security feature
    public void enableFeature(SecurityFeature feature);

    // Disable security feature
    public void disableFeature(SecurityFeature feature);
}
```

## Examples

### Creating and Transferring an Asset
```java
// Initialize SDK
SatoxSDK sdk = SatoxSDK.getInstance();
sdk.initialize();

// Get asset manager
AssetManager assetManager = sdk.getAssetManager();

// Create asset metadata
AssetMetadata metadata = new AssetMetadata();
metadata.setName("My Asset");
metadata.setSymbol("MYA");
metadata.setTotalSupply(1000000);
metadata.setDecimals(8);
metadata.setReissuable(true);

// Create asset
String assetId = assetManager.createAsset(metadata);

// Transfer asset
String from = "my_address";
String to = "recipient_address";
long amount = 1000;
boolean success = assetManager.transferAsset(assetId, from, to, amount);
```

### Creating and Managing NFTs
```java
// Get NFT manager
NFTManager nftManager = sdk.getNFTManager();

// Create NFT metadata
NFTMetadata metadata = new NFTMetadata();
metadata.setName("My NFT");
metadata.setDescription("A unique NFT");
metadata.setImage("ipfs://image-hash");
metadata.setAttributes(new HashMap<>());

// Create NFT
String nftId = nftManager.createNFT(metadata);

// Transfer NFT
String from = "my_address";
String to = "recipient_address";
boolean success = nftManager.transferNFT(nftId, from, to);
```

### Blockchain Operations
```java
// Get blockchain manager
BlockchainManager blockchainManager = sdk.getBlockchainManager();

// Get blockchain info
BlockchainInfo info = blockchainManager.getBlockchainInfo();
System.out.println("Current height: " + info.getHeight());

// Get block by hash
Block block = blockchainManager.getBlock("block_hash");
System.out.println("Block timestamp: " + block.getTimestamp());

// Create and send transaction
List<Input> inputs = new ArrayList<>();
List<Output> outputs = new ArrayList<>();
// Add inputs and outputs...

String rawTx = blockchainManager.createRawTransaction(inputs, outputs);
String signedTx = blockchainManager.signRawTransaction(rawTx, privateKeys);
String txId = blockchainManager.sendRawTransaction(signedTx);
``` 