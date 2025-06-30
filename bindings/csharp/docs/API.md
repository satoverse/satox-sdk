# Satox SDK C# API Documentation

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
Add the Satox SDK to your project using NuGet:

```xml
<PackageReference Include="Satox.SDK" Version="1.0.0" />
```

### Basic Usage
```csharp
using Satox.SDK;
using Satox.SDK.Blockchain;
using Satox.SDK.Asset;
using Satox.SDK.NFT;

class Program
{
    static void Main(string[] args)
    {
        // Initialize SDK
        var sdk = SatoxSDK.Instance;
        sdk.Initialize();

        // Get managers
        var blockchainManager = sdk.BlockchainManager;
        var assetManager = sdk.AssetManager;
        var nftManager = sdk.NFTManager;

        // Use managers...
    }
}
```

## Core Components

### SatoxSDK
The main entry point for the SDK.

```csharp
public class SatoxSDK
{
    // Get singleton instance
    public static SatoxSDK Instance { get; }

    // Initialize SDK with configuration
    public bool Initialize(SDKConfig config);

    // Shutdown SDK
    public void Shutdown();

    // Get component managers
    public BlockchainManager BlockchainManager { get; }
    public AssetManager AssetManager { get; }
    public NFTManager NFTManager { get; }
    public SecurityManager SecurityManager { get; }
    public IPFSManager IPFSManager { get; }
}
```

### SDKConfig
Configuration for the SDK.

```csharp
public class SDKConfig
{
    public string Network { get; set; }        // "mainnet" or "testnet"
    public string DataDir { get; set; }        // Data directory path
    public bool EnableMining { get; set; }     // Enable mining
    public int MiningThreads { get; set; }     // Number of mining threads
    public bool EnableSync { get; set; }       // Enable blockchain sync
    public int SyncInterval { get; set; }      // Sync interval in milliseconds
    public string RPCEndpoint { get; set; }    // RPC endpoint URL
    public string RPCUsername { get; set; }    // RPC username
    public string RPCPassword { get; set; }    // RPC password
    public int Timeout { get; set; }           // Timeout in milliseconds
}
```

## Blockchain Integration

### BlockchainManager
Manages blockchain operations.

```csharp
public class BlockchainManager
{
    // Get blockchain info
    public BlockchainInfo GetBlockchainInfo();

    // Get block by hash
    public Block GetBlock(string hash);

    // Get block by height
    public Block GetBlock(int height);

    // Get transaction info
    public Transaction GetTransaction(string txid);

    // Send raw transaction
    public string SendRawTransaction(string rawTx);

    // Create raw transaction
    public string CreateRawTransaction(List<Input> inputs, List<Output> outputs);

    // Sign raw transaction
    public string SignRawTransaction(string rawTx, List<string> privateKeys);
}
```

## Asset Management

### AssetManager
Manages asset operations.

```csharp
public class AssetManager
{
    // Create asset
    public string CreateAsset(AssetMetadata metadata);

    // Get asset info
    public Asset GetAsset(string assetId);

    // Update asset
    public bool UpdateAsset(string assetId, AssetMetadata metadata);

    // Transfer asset
    public bool TransferAsset(string assetId, string from, string to, long amount);

    // Get asset balance
    public long GetBalance(string address, string assetId);

    // Get asset history
    public List<AssetEvent> GetAssetHistory(string assetId);
}
```

## NFT Management

### NFTManager
Manages NFT operations.

```csharp
public class NFTManager
{
    // Create NFT
    public string CreateNFT(NFTMetadata metadata);

    // Get NFT info
    public NFT GetNFT(string nftId);

    // Transfer NFT
    public bool TransferNFT(string nftId, string from, string to);

    // Get NFT owner
    public string GetNFTOwner(string nftId);

    // Get NFT collection
    public List<NFT> GetNFTCollection(string collectionId);
}
```

## Security

### SecurityManager
Manages security features.

```csharp
public class SecurityManager
{
    // Initialize security manager
    public void Initialize();

    // Shutdown security manager
    public void Shutdown();

    // Set security policy
    public void SetPolicy(SecurityPolicy policy);

    // Get security status
    public SecurityStatus GetStatus();

    // Enable security feature
    public void EnableFeature(SecurityFeature feature);

    // Disable security feature
    public void DisableFeature(SecurityFeature feature);
}
```

## Examples

### Creating and Transferring an Asset
```csharp
// Initialize SDK
var sdk = SatoxSDK.Instance;
sdk.Initialize();

// Get asset manager
var assetManager = sdk.AssetManager;

// Create asset metadata
var metadata = new AssetMetadata
{
    Name = "My Asset",
    Symbol = "MYA",
    TotalSupply = 1000000,
    Decimals = 8,
    Reissuable = true
};

// Create asset
string assetId = assetManager.CreateAsset(metadata);

// Transfer asset
string from = "my_address";
string to = "recipient_address";
long amount = 1000;
bool success = assetManager.TransferAsset(assetId, from, to, amount);
```

### Creating and Managing NFTs
```csharp
// Get NFT manager
var nftManager = sdk.NFTManager;

// Create NFT metadata
var metadata = new NFTMetadata
{
    Name = "My NFT",
    Description = "A unique NFT",
    Image = "ipfs://image-hash",
    Attributes = new Dictionary<string, string>()
};

// Create NFT
string nftId = nftManager.CreateNFT(metadata);

// Transfer NFT
string from = "my_address";
string to = "recipient_address";
bool success = nftManager.TransferNFT(nftId, from, to);
```

### Blockchain Operations
```csharp
// Get blockchain manager
var blockchainManager = sdk.BlockchainManager;

// Get blockchain info
var info = blockchainManager.GetBlockchainInfo();
Console.WriteLine($"Current height: {info.Height}");

// Get block by hash
var block = blockchainManager.GetBlock("block_hash");
Console.WriteLine($"Block timestamp: {block.Timestamp}");

// Create and send transaction
var inputs = new List<Input>();
var outputs = new List<Output>();
// Add inputs and outputs...

string rawTx = blockchainManager.CreateRawTransaction(inputs, outputs);
string signedTx = blockchainManager.SignRawTransaction(rawTx, privateKeys);
string txId = blockchainManager.SendRawTransaction(signedTx);
``` 