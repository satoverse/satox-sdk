# Satox SDK Java Bindings

## Overview
The Satox SDK Java bindings provide a high-level interface for interacting with the Satox blockchain. This library enables Java developers to create, manage, and interact with Satox assets, transactions, and blockchain data.

## Installation

### Maven
```xml
<dependency>
    <groupId>com.satox</groupId>
    <artifactId>satox-sdk</artifactId>
    <version>1.0.0</version>
</dependency>
```

### Gradle
```groovy
implementation 'com.satox:satox-sdk:1.0.0'
```

## Quick Start

### Initialize the SDK
```java
import com.satox.SatoxSDK;
import com.satox.config.SDKConfig;

public class Example {
    public static void main(String[] args) {
        // Create configuration
        SDKConfig config = new SDKConfig.Builder()
            .setNetwork("mainnet")
            .setRpcPort(7777)
            .setP2pPort(60777)
            .build();

        // Initialize SDK
        SatoxSDK sdk = SatoxSDK.getInstance();
        sdk.initialize(config);
    }
}
```

### Create and Manage Assets
```java
import com.satox.asset.AssetManager;
import com.satox.asset.Asset;

public class AssetExample {
    public static void main(String[] args) {
        AssetManager assetManager = SatoxSDK.getInstance().getAssetManager();

        // Create a new asset
        Asset asset = new Asset.Builder()
            .setName("MY_ASSET")
            .setOwner("ADDRESS")
            .setSupply(1000000)
            .addMetadata("description", "My custom asset")
            .build();

        // Create asset on blockchain
        String assetId = assetManager.createAsset(asset);
        System.out.println("Created asset with ID: " + assetId);

        // Transfer asset
        assetManager.transferAsset(assetId, "FROM_ADDRESS", "TO_ADDRESS", 1000);

        // Get asset info
        Asset info = assetManager.getAsset(assetId);
        System.out.println("Asset info: " + info);
    }
}
```

### Handle Transactions
```java
import com.satox.transaction.TransactionManager;
import com.satox.transaction.Transaction;

public class TransactionExample {
    public static void main(String[] args) {
        TransactionManager txManager = SatoxSDK.getInstance().getTransactionManager();

        // Create transaction
        Transaction tx = new Transaction.Builder()
            .setFrom("FROM_ADDRESS")
            .setTo("TO_ADDRESS")
            .setAmount(1000)
            .setAssetId("ASSET_ID")
            .build();

        // Sign and broadcast transaction
        String txId = txManager.createAndBroadcastTransaction(tx);
        System.out.println("Transaction ID: " + txId);

        // Get transaction status
        TransactionStatus status = txManager.getTransactionStatus(txId);
        System.out.println("Transaction status: " + status);
    }
}
```

### IPFS Integration
```java
import com.satox.ipfs.IPFSManager;
import com.satox.ipfs.IPFSFile;

public class IPFSExample {
    public static void main(String[] args) {
        IPFSManager ipfsManager = SatoxSDK.getInstance().getIPFSManager();

        // Add file to IPFS
        IPFSFile file = new IPFSFile.Builder()
            .setPath("/path/to/file.txt")
            .setName("file.txt")
            .build();

        String hash = ipfsManager.addFile(file);
        System.out.println("File added to IPFS with hash: " + hash);

        // Get file from IPFS
        IPFSFile retrieved = ipfsManager.getFile(hash);
        System.out.println("Retrieved file: " + retrieved);
    }
}
```

### Error Handling
```java
import com.satox.exception.SatoxException;

public class ErrorHandlingExample {
    public static void main(String[] args) {
        try {
            // SDK operations
            SatoxSDK sdk = SatoxSDK.getInstance();
            sdk.initialize(config);
        } catch (SatoxException e) {
            System.err.println("Error: " + e.getMessage());
            System.err.println("Error code: " + e.getErrorCode());
        }
    }
}
```

## API Reference

### SatoxSDK
The main entry point for the SDK.

#### Methods
- `getInstance()`: Get the SDK instance
- `initialize(SDKConfig)`: Initialize the SDK with configuration
- `shutdown()`: Shutdown the SDK
- `getAssetManager()`: Get the asset manager
- `getTransactionManager()`: Get the transaction manager
- `getIPFSManager()`: Get the IPFS manager

### AssetManager
Manages asset creation, transfer, and queries.

#### Methods
- `createAsset(Asset)`: Create a new asset
- `transferAsset(String, String, String, long)`: Transfer asset
- `getAsset(String)`: Get asset information
- `searchAssets(Map<String, String>)`: Search assets by criteria
- `getAssetHistory(String)`: Get asset transaction history

### TransactionManager
Handles transaction creation, signing, and broadcasting.

#### Methods
- `createTransaction(Transaction)`: Create a new transaction
- `signTransaction(Transaction)`: Sign a transaction
- `broadcastTransaction(Transaction)`: Broadcast a transaction
- `getTransactionStatus(String)`: Get transaction status
- `getTransactionHistory(String)`: Get transaction history

### IPFSManager
Manages IPFS file operations.

#### Methods
- `addFile(IPFSFile)`: Add file to IPFS
- `getFile(String)`: Get file from IPFS
- `pinFile(String)`: Pin file in IPFS
- `unpinFile(String)`: Unpin file from IPFS
- `searchFiles(String)`: Search files in IPFS

## Best Practices

### Error Handling
- Always use try-catch blocks for SDK operations
- Check error codes and messages for proper error handling
- Implement retry logic for network operations
- Handle timeouts appropriately

### Resource Management
- Call `shutdown()` when done with the SDK
- Release resources properly
- Use connection pooling for network operations
- Implement proper cleanup in finally blocks

### Performance
- Use batch operations when possible
- Implement caching for frequently accessed data
- Use async operations for long-running tasks
- Monitor memory usage

### Security
- Never store private keys in plain text
- Use secure storage for sensitive data
- Implement proper access control
- Follow security best practices

## Examples

### Complete Asset Management Example
```java
import com.satox.SatoxSDK;
import com.satox.asset.AssetManager;
import com.satox.asset.Asset;
import com.satox.exception.SatoxException;

public class CompleteAssetExample {
    public static void main(String[] args) {
        try {
            // Initialize SDK
            SatoxSDK sdk = SatoxSDK.getInstance();
            sdk.initialize(new SDKConfig.Builder()
                .setNetwork("mainnet")
                .build());

            // Get asset manager
            AssetManager assetManager = sdk.getAssetManager();

            // Create asset
            Asset asset = new Asset.Builder()
                .setName("MY_ASSET")
                .setOwner("ADDRESS")
                .setSupply(1000000)
                .addMetadata("description", "My custom asset")
                .addMetadata("type", "TOKEN")
                .addMetadata("decimals", "8")
                .build();

            String assetId = assetManager.createAsset(asset);
            System.out.println("Created asset: " + assetId);

            // Transfer asset
            assetManager.transferAsset(assetId, "FROM_ADDRESS", "TO_ADDRESS", 1000);

            // Get asset info
            Asset info = assetManager.getAsset(assetId);
            System.out.println("Asset info: " + info);

            // Search assets
            Map<String, String> criteria = new HashMap<>();
            criteria.put("type", "TOKEN");
            List<Asset> assets = assetManager.searchAssets(criteria);
            System.out.println("Found " + assets.size() + " assets");

            // Get asset history
            List<AssetTransaction> history = assetManager.getAssetHistory(assetId);
            System.out.println("Asset history: " + history);

        } catch (SatoxException e) {
            System.err.println("Error: " + e.getMessage());
        } finally {
            // Shutdown SDK
            SatoxSDK.getInstance().shutdown();
        }
    }
}
```

### Complete Transaction Example
```java
import com.satox.SatoxSDK;
import com.satox.transaction.TransactionManager;
import com.satox.transaction.Transaction;
import com.satox.exception.SatoxException;

public class CompleteTransactionExample {
    public static void main(String[] args) {
        try {
            // Initialize SDK
            SatoxSDK sdk = SatoxSDK.getInstance();
            sdk.initialize(new SDKConfig.Builder()
                .setNetwork("mainnet")
                .build());

            // Get transaction manager
            TransactionManager txManager = sdk.getTransactionManager();

            // Create transaction
            Transaction tx = new Transaction.Builder()
                .setFrom("FROM_ADDRESS")
                .setTo("TO_ADDRESS")
                .setAmount(1000)
                .setAssetId("ASSET_ID")
                .setFee(0.001)
                .build();

            // Sign and broadcast
            String txId = txManager.createAndBroadcastTransaction(tx);
            System.out.println("Transaction ID: " + txId);

            // Wait for confirmation
            TransactionStatus status;
            do {
                Thread.sleep(1000);
                status = txManager.getTransactionStatus(txId);
                System.out.println("Status: " + status);
            } while (status == TransactionStatus.PENDING);

            // Get transaction history
            List<Transaction> history = txManager.getTransactionHistory("FROM_ADDRESS");
            System.out.println("Transaction history: " + history);

        } catch (SatoxException | InterruptedException e) {
            System.err.println("Error: " + e.getMessage());
        } finally {
            // Shutdown SDK
            SatoxSDK.getInstance().shutdown();
        }
    }
}
```

## Troubleshooting

### Common Issues
1. Connection Issues
   - Check network connectivity
   - Verify RPC and P2P ports
   - Check firewall settings

2. Transaction Issues
   - Verify sufficient funds
   - Check transaction fees
   - Verify address validity

3. Asset Issues
   - Check asset permissions
   - Verify asset existence
   - Check transfer limits

### Debugging
- Enable debug logging
- Check error messages
- Monitor network traffic
- Use test network for development

## Support
For support, please contact:
- Email: support@satox.com
- GitHub: https://github.com/satoverse/satox-sdk
- Documentation: https://docs.satox.com

## ⚠️ Limitations

### **Blockchain Compatibility**

- **No Smart Contract Support:** Satox is based on Ravencoin and does **not** support smart contracts or EVM compatibility
- **Asset-Based:** Focuses on asset creation, management, and transfer rather than programmable contracts
- **UTXO Model:** Uses Unspent Transaction Output (UTXO) model like Bitcoin/Ravencoin, not account-based like Ethereum

### **Platform Support**

- **Desktop Focus:** Primarily designed for desktop/server environments
- **Mobile Limitations:** Mobile SDK provides limited functionality compared to core SDK
- **Browser Support:** WebAssembly bindings have performance limitations for complex operations

### **Network Requirements**

- **Internet Connection:** Requires active internet connection for blockchain operations
- **Node Synchronization:** Full node operations require significant storage and bandwidth
- **API Dependencies:** Some features depend on external API services

### **Development Considerations**

- **Learning Curve:** Ravencoin/UTXO model differs from Ethereum/smart contract development
- **Testing Environment:** Testnet required for development and testing
- **Asset Management:** Asset creation and management have specific requirements and limitations


## License
This SDK is licensed under the MIT License. See LICENSE file for details. 
