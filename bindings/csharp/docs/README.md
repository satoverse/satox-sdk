# Satox SDK C# Bindings

## Overview
The Satox SDK C# bindings provide a comprehensive interface for C# developers to interact with the Satox blockchain. These bindings enable seamless integration of Satox blockchain features into C# applications, including asset management, transaction handling, and IPFS integration.

## Installation

### NuGet Package
```xml
<PackageReference Include="Satox.SDK" Version="1.0.0" />
```

### Manual Installation
1. Download the latest release from the releases page
2. Add the DLL reference to your project
3. Add required dependencies:
   - Newtonsoft.Json
   - System.Security.Cryptography

## Quick Start

### Initialize the SDK
```csharp
using Satox.SDK;

var config = new SDKConfig
{
    Network = "testnet",
    RpcPort = 7777,
    P2pPort = 60777
};

var sdk = SatoxSDK.Instance;
sdk.Initialize(config);
```

### Asset Management
```csharp
// Create an asset
var asset = new Asset
{
    Name = "TEST_ASSET",
    Owner = "TEST_ADDRESS",
    Supply = 1000000,
    Metadata = new Dictionary<string, string>
    {
        { "description", "Test asset" },
        { "type", "TOKEN" },
        { "decimals", "8" }
    }
};

string assetId = await sdk.AssetManager.CreateAssetAsync(asset);

// Transfer an asset
await sdk.AssetManager.TransferAssetAsync(assetId, "FROM_ADDRESS", "TO_ADDRESS", 1000);

// Search assets
var criteria = new Dictionary<string, string> { { "type", "TOKEN" } };
var assets = await sdk.AssetManager.SearchAssetsAsync(criteria);
```

### Transaction Handling
```csharp
// Create and broadcast a transaction
var transaction = new Transaction
{
    From = "FROM_ADDRESS",
    To = "TO_ADDRESS",
    Amount = 1000,
    AssetId = "ASSET_ID",
    Fee = 0.001m
};

string txId = await sdk.TransactionManager.CreateAndBroadcastTransactionAsync(transaction);

// Get transaction status
var status = await sdk.TransactionManager.GetTransactionStatusAsync(txId);

// Get transaction history
var history = await sdk.TransactionManager.GetTransactionHistoryAsync("FROM_ADDRESS");
```

### IPFS Integration
```csharp
// Add file to IPFS
var file = new IPFSFile
{
    Path = "/path/to/test.txt",
    Name = "test.txt"
};

string hash = await sdk.IPFSManager.AddFileAsync(file);

// Get file from IPFS
var retrievedFile = await sdk.IPFSManager.GetFileAsync(hash);
```

### Error Handling
```csharp
try
{
    await sdk.AssetManager.CreateAssetAsync(asset);
}
catch (SatoxException ex)
{
    Console.WriteLine($"Error: {ex.Message}");
    Console.WriteLine($"Error Code: {ex.ErrorCode}");
}
```

## API Reference

### SatoxSDK
Main entry point for the SDK.

#### Properties
- `Instance`: Singleton instance of the SDK
- `AssetManager`: Asset management operations
- `TransactionManager`: Transaction operations
- `IPFSManager`: IPFS operations

#### Methods
- `Initialize(SDKConfig config)`: Initialize the SDK
- `Shutdown()`: Shutdown the SDK

### AssetManager
Manages asset-related operations.

#### Methods
- `CreateAssetAsync(Asset asset)`: Create a new asset
- `GetAssetAsync(string assetId)`: Get asset details
- `TransferAssetAsync(string assetId, string from, string to, decimal amount)`: Transfer an asset
- `SearchAssetsAsync(Dictionary<string, string> criteria)`: Search assets

### TransactionManager
Handles transaction operations.

#### Methods
- `CreateAndBroadcastTransactionAsync(Transaction transaction)`: Create and broadcast a transaction
- `GetTransactionStatusAsync(string txId)`: Get transaction status
- `GetTransactionHistoryAsync(string address)`: Get transaction history

### IPFSManager
Manages IPFS operations.

#### Methods
- `AddFileAsync(IPFSFile file)`: Add a file to IPFS
- `GetFileAsync(string hash)`: Get a file from IPFS
- `PinFileAsync(string hash)`: Pin a file in IPFS
- `UnpinFileAsync(string hash)`: Unpin a file from IPFS

## Best Practices

### Error Handling
- Always use try-catch blocks when calling SDK methods
- Check error codes for specific error handling
- Implement proper logging for errors
- Handle network-related errors appropriately

### Resource Management
- Use `using` statements for proper resource disposal
- Call `Shutdown()` when the SDK is no longer needed
- Implement proper cleanup in your application

### Performance
- Use async/await for all SDK operations
- Implement proper caching strategies
- Batch operations when possible
- Monitor memory usage

### Security
- Never store private keys in plain text
- Use secure storage for sensitive data
- Implement proper access control
- Follow security best practices

## Examples

### Asset Management Example
```csharp
public class AssetManagementExample
{
    private readonly SatoxSDK _sdk;

    public AssetManagementExample()
    {
        _sdk = SatoxSDK.Instance;
    }

    public async Task CreateAndTransferAsset()
    {
        try
        {
            // Create asset
            var asset = new Asset
            {
                Name = "MY_ASSET",
                Owner = "MY_ADDRESS",
                Supply = 1000000,
                Metadata = new Dictionary<string, string>
                {
                    { "description", "My test asset" },
                    { "type", "TOKEN" }
                }
            };

            string assetId = await _sdk.AssetManager.CreateAssetAsync(asset);
            Console.WriteLine($"Asset created: {assetId}");

            // Transfer asset
            await _sdk.AssetManager.TransferAssetAsync(
                assetId,
                "MY_ADDRESS",
                "RECIPIENT_ADDRESS",
                1000
            );
            Console.WriteLine("Asset transferred successfully");
        }
        catch (SatoxException ex)
        {
            Console.WriteLine($"Error: {ex.Message}");
        }
    }
}
```

### Transaction Management Example
```csharp
public class TransactionManagementExample
{
    private readonly SatoxSDK _sdk;

    public TransactionManagementExample()
    {
        _sdk = SatoxSDK.Instance;
    }

    public async Task CreateAndMonitorTransaction()
    {
        try
        {
            // Create transaction
            var transaction = new Transaction
            {
                From = "MY_ADDRESS",
                To = "RECIPIENT_ADDRESS",
                Amount = 1000,
                AssetId = "ASSET_ID",
                Fee = 0.001m
            };

            string txId = await _sdk.TransactionManager.CreateAndBroadcastTransactionAsync(transaction);
            Console.WriteLine($"Transaction created: {txId}");

            // Monitor transaction
            while (true)
            {
                var status = await _sdk.TransactionManager.GetTransactionStatusAsync(txId);
                Console.WriteLine($"Transaction status: {status}");

                if (status == TransactionStatus.Confirmed)
                {
                    break;
                }

                await Task.Delay(1000);
            }
        }
        catch (SatoxException ex)
        {
            Console.WriteLine($"Error: {ex.Message}");
        }
    }
}
```

## Troubleshooting

### Common Issues

1. **Connection Issues**
   - Verify network connectivity
   - Check RPC and P2P ports
   - Ensure firewall settings allow connections

2. **Transaction Failures**
   - Verify sufficient balance
   - Check transaction parameters
   - Ensure proper fee calculation

3. **Asset Creation Issues**
   - Verify asset parameters
   - Check permissions
   - Ensure unique asset names

### Debugging Tips
- Enable detailed logging
- Use try-catch blocks
- Check error messages
- Monitor network traffic
- Verify configuration

## Support
For support, please:
1. Check the documentation
2. Search existing issues
3. Create a new issue if needed
4. Contact support team

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
This project is licensed under the MIT License - see the LICENSE file for details. 
